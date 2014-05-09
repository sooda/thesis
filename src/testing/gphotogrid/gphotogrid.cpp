#include "app.h"
#include "gphotogrid.h"
#include "gputil.h"
#include <iostream>
#include <sstream>
#include <wx/timer.h>
#include <wx/mstream.h>

MainFrame::MainFrame(const wxString& title, App& app) :
		wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(1024, 768)), app(app) {
	create_menus();
	create_panels();

#ifdef LOOP_TIMER
	//timer = new RenderTimer(*this);
	//timer->start();
#else
	frames = 0;
	lasttime = Clock::now();

	enableRender(true);
	Bind(wxEVT_IDLE, &MainFrame::onIdle, this);
#endif
}

#ifdef LOOP_TIMER
RenderTimer::RenderTimer(MainFrame& root) : root(root) {
}
void RenderTimer::Notify() {
	std::cout << "NOTIF" << std::endl;
	root.refresh();
}
void RenderTimer::start() {
	Start(10);
}
#else
void MainFrame::onIdle(wxIdleEvent& ev) {
	//std::cout<<"idle"<<std::endl;
	refresh();
	ev.RequestMore();
}
#endif

void MainFrame::readCameraParams() {
	timeline->Destroy();
	timeline = new Timeline(this, app.numcams());
	GetSizer()->Add(timeline, 1, wxEXPAND);
	Layout();

	if (app.numcams() > 0) {
		// assume that all cameras are the same, use only zeroth

		auto aperture = app.cams[0].config()["aperture"].get<gp::Aperture>();
		options->setSelections(0, aperture.choices(), aperture.index());

		auto shutter = app.cams[0].config()["shutterspeed"].get<gp::ShutterSpeed>();
		options->setSelections(1, shutter.choices(), shutter.index());

		auto iso = app.cams[0].config()["iso"].get<gp::Iso>();
		options->setSelections(2, iso.choices(), iso.index());
	}

	Refresh();
}

void MainFrame::create_panels() {
	auto horsizer = new wxBoxSizer(wxHORIZONTAL);
	// opts lots thinner than image panel
	horsizer->Add(options = new OptionsPanel(this), 1, wxEXPAND);
	horsizer->Add(create_imagegrid(), 4, wxEXPAND);
	auto versizer = new wxBoxSizer(wxVERTICAL);
	versizer->Add(horsizer, 4, wxEXPAND);
	versizer->Add(timeline = new Timeline(this, app.numcams()));
	SetSizer(versizer);

	readCameraParams();
}

wxSizer* MainFrame::create_imagegrid() {
	int rows = 3, cols = 3;
	auto sizer = new wxGridSizer(cols, wxSize(2, 2));
	for (int i = 0; i < rows * cols; i++) {
		auto panel = new ImagePanel(this);
		images.push_back(panel);
		sizer->Add(panel, 1, wxEXPAND);
	}
	return sizer;
}

void MainFrame::create_menus() {
	wxMenu *menuFile = new wxMenu();
	menuFile->AppendSeparator();
	menuFile->Append(1, "foo");
	menuFile->Append(2, "bar");
	menuFile->Append(wxID_EXIT);

	wxMenuBar *menuBar = new wxMenuBar();
	menuBar->Append(menuFile, "&File");

	SetMenuBar(menuBar);

	CreateStatusBar();
	SetStatusText("Hello world");

	Bind(wxEVT_COMMAND_MENU_SELECTED, &MainFrame::onMenu, this, 1);
	Bind(wxEVT_COMMAND_MENU_SELECTED, &MainFrame::onExit, this, wxID_EXIT);
}

void MainFrame::slider(int id, int value) {
	std::cout << "slider " << id << " to " << value << std::endl;
	if (id < 0 || id > 2)
		throw std::out_of_range("slider index bug");

	std::vector<std::thread> setters(app.numcams());
	int i = 0;
	for (int cam = 0; cam < app.numcams(); cam++) {
		setters[cam] = std::thread([=]() {
			switch (id) {
			case 0:
				setRadioConfig<gp::Aperture>(cam, value);
				break;
			case 1:
				setRadioConfig<gp::ShutterSpeed>(cam, value);
				break;
			case 2:
				setRadioConfig<gp::Iso>(cam, value);
				break;
			}
		});
	}
	for (auto& thr: setters)
		thr.join();
}

template <class Obj>
void MainFrame::setRadioConfig(int cam, int value) {
	auto cfg = app.cams[cam].config()[Obj::gpname];
	auto radio = cfg.template get<Obj>();

	if (value >= 0 && value < radio.size()) {
		radio.set(value);
		cfg.set(radio);
		std::cout << "cam " << cam << " new " << Obj::gpname << " " << radio.text() << std::endl;
	}
}

void MainFrame::reloadGphoto() {
	assert(!app.previewfeed.enabled());
	app.reloadGphoto();
	readCameraParams();
}

void MainFrame::refresh() {
	updatePhotos();
	for (auto im: images) {
		im->Refresh();
	}
	framecalc();
}

float MainFrame::time_since(MainFrame::Clock::time_point now, MainFrame::Clock::time_point before) const {
	typedef std::chrono::duration<float> fsec;
	fsec fs = now - before;
	return fs.count();
}

void MainFrame::framecalc() {
	auto clock = Clock::now();
	float secs = time_since(clock, lasttime);
	frames++;

	if (secs > 5.0f) {
		std::cout << frames << " in " << secs << " secs = " << (frames / secs) << " fps" << std::endl;
		frames = 0;
		lasttime = clock;
	}
}
void MainFrame::updatePhotos() {
	// if no sources, no update needed
	if (app.numcams() == 0 || !app.previewfeed.enabled())
		return;

	// all have the same size
	auto newsize = images[0]->GetSize();

	for (int i = 0; i < std::min(app.numcams(), images.size()); i++) {
		PreviewFeed::TimedJpegBuffer capture;
		// test if there is anything
		if (!app.previewfeed.getQueue(i).try_pop(capture))
			continue;
		// log timestamps, and slurp further ones, until got the latest
		do {
			timeline->insert(i, time_since(capture.second, renderinittime));
			numpics[i]++;
		} while (app.previewfeed.getQueue(i).try_pop(capture));

		wxMemoryInputStream stream(&capture.first[0], capture.first.size());
		wxImage im(stream, wxBITMAP_TYPE_JPEG);
		// XXX: aspect ratio correction?
		im.Rescale(newsize.GetWidth(), newsize.GetHeight(), wxIMAGE_QUALITY_NEAREST);

		images[i]->setImage(im);

		if (i == app.numcams() - 1) {
			// copy last image to the rest for testing
			for (int j = i; j < images.size(); j++) {
				images[j]->setImage(im);
			}
		}
	}

	std::stringstream ss;
	ss << app.numcams() << " cameras; frames per cam:";
	for (int i = 0; i < numpics.size(); i++) {
		ss << " " << i << ":" << numpics[i];
	}
	SetStatusText(ss.str());
}

void MainFrame::enableRender(bool enable) {
	if (enable) {
		renderinittime = Clock::now();
		numpics = std::vector<int>(app.numcams());
		timeline->clear();
		app.previewfeed.enable();
	} else {
		app.previewfeed.disable();
	}
}
void MainFrame::syncGrabbers(bool sync) {
	app.previewfeed.setsync(sync);
}

void MainFrame::timelineFollow(bool follow) {
	timeline->followInsertions(follow);
}

void MainFrame::onMenu(wxCommandEvent&) {
	std::cout << "menu" << std::endl;
}
void MainFrame::onExit(wxCommandEvent&) {
	Close(true);
}
