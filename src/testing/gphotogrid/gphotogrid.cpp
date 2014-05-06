#include "app.h"
#include "gphotogrid.h"
#include "gputil.h"
#include <iostream>
#include <algorithm>
#include <boost/filesystem.hpp>
#include <wx/timer.h>
#include <wx/mstream.h>
#include <chrono>

bool GfxUi::OnInit()
{
	wxInitAllImageHandlers();
	// note that frames/panes are owned by wx, not leaked
	wxFrame *mainframe = new MainFrame("Hello World", app);
	mainframe->Show(true);
	return true;
}

wxIMPLEMENT_APP(GfxUi);


MainFrame::MainFrame(const wxString& title, App& app) :
		wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(1024, 768)), app(app) {
	create_menus();
	create_panels();
#ifdef LOOP_TIMER
	//timer = new RenderTimer(*this);
	//timer->start();
#else
	lasttime = Clock::now();
	frames = 0;
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
	timeline = new Timeline(this, app.cams.size());
	GetSizer()->Add(timeline, 1, wxEXPAND);
	Layout();

	if (app.cams.size() > 0) {
		// assume that all cameras are the same, use only zeroth

		auto aperture = app.cams[0].config()["aperture"].get<gp::Aperture>();
		options->setSliderRange(0, 0, aperture.size() - 1, aperture.index());

		auto shutter = app.cams[0].config()["shutterspeed"].get<gp::ShutterSpeed>();
		options->setSliderRange(1, 0, shutter.size() - 1, shutter.index());

		auto iso = app.cams[0].config()["iso"].get<gp::Iso>();
		options->setSliderRange(2, 0, iso.size() - 1, iso.index());
	}
	renderinittime = Clock::now();
	Refresh();
}

void MainFrame::create_panels() {
	auto horsizer = new wxBoxSizer(wxHORIZONTAL);
	// opts lots thinner than image panel
	horsizer->Add(options = new OptionsPanel(this), 1, wxEXPAND);
	horsizer->Add(create_imagegrid(), 4, wxEXPAND);
	auto versizer = new wxBoxSizer(wxVERTICAL);
	versizer->Add(horsizer, 4, wxEXPAND);
	versizer->Add(timeline = new Timeline(this, app.cams.size()));
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
	int i = 0;
	for (int cam = 0; cam < app.cams.size(); cam++) {
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
		default:
			throw std::out_of_range("bad slider index");
		}
	}
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
	// i hope that this is in the same thread as the render loop
	// (no race conditions)
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
	float secs = time_since(lasttime, clock);
	frames++;

	if (secs > 5.0f) {
		std::cout << frames << " in " << secs << " secs = " << (frames / secs) << " fps" << std::endl;
		frames = 0;
		lasttime = clock;
	}
}
void MainFrame::updatePhotos() {
	// if no sources, no update needed
	if (app.cams.size() == 0)
		return;

	// all have the same size
	auto newsize = images[0]->GetSize();

	for (int i = 0; i < std::min(app.cams.size(), images.size()); i++) {
		gp::Camera& cam = app.cams[i];
		std::vector<char> jpeg;

		try {
			jpeg = cam.preview();
		} catch (gp::Exception& ex) {
			std::cout << "cam " << i << ": " << ex.what() << std::endl;
			continue;
		}
		timeline->insert(i, time_since(Clock::now(), renderinittime));

		wxMemoryInputStream stream(&jpeg[0], jpeg.size());
		wxImage im(stream, wxBITMAP_TYPE_JPEG);

		// XXX: aspect ratio correction?
		im.Rescale(newsize.GetWidth(), newsize.GetHeight(), wxIMAGE_QUALITY_NEAREST);

		images[i]->setImage(im);

		if (i == app.cams.size() - 1) {
			// copy last image to the rest for testing
			for (int j = i; j < images.size(); j++) {
				images[j]->setImage(im);
			}
		}
	}
}

void MainFrame::onMenu(wxCommandEvent&) {
	std::cout << "menu" << std::endl;
}
void MainFrame::onExit(wxCommandEvent&) {
	Close(true);
}
