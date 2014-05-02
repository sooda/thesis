#include "app.h"
#include "gphotogrid.h"
#include "gputil.h"
#include <iostream>
#include <algorithm>
#include <boost/filesystem.hpp>
#include <wx/timer.h>
#include <wx/mstream.h>

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

void MainFrame::create_panels() {
	auto sizer = new wxBoxSizer(wxHORIZONTAL);
	// opts lots thinner than image panel
	sizer->Add(options = new OptionsPanel(this), 1, wxEXPAND);
	sizer->Add(create_imagegrid(), 4, wxEXPAND);
	SetSizer(sizer);

	readCameraParams();
}

void MainFrame::readCameraParams() {
	if (app.cams.size() == 0)
		return;

	auto aperture = app.cams[0].config()["aperture"].get<gp::Aperture>();
	options->setSliderRange(0, 0, aperture.size());
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

void MainFrame::slider(int n) {
	std::cout << "slider " << n << std::endl;
	int i = 0;
	for (auto& cam: app.cams) {
		auto cfg = cam.config()["aperture"];
		auto ap = cfg.get<gp::Aperture>();

		if (n >= 0 && n < ap.size()) {
			ap.set(n);
			std::cout << "cam " << i << " new aperture " << ap.text() << std::endl;

			cfg.set(ap);
		}

		i++;
	}
}

void MainFrame::reloadGphoto() {
	// i hope that this is in the same thread as the render loop
	// (no race conditions)
	app.reloadGphoto();
}

void MainFrame::refresh() {
	updatePhotos();
	for (auto im: images) {
		im->Refresh();
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
