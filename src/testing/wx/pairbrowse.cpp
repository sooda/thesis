#include "pairbrowse.h"
#include <iostream>
#include <boost/filesystem.hpp>
#include <algorithm>

OptionsPanel::OptionsPanel(MyFrame* parent) : wxPanel(parent), boss(parent) {
	btn = new wxButton(this, wxID_ANY, "Moi", wxPoint(20, 20));
	slider = new wxSlider(this, wxID_ANY, 1, 1, 10); // TODO: style
	
	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

	sizer->Add(btn, 1, wxEXPAND);
	sizer->Add(slider, 1, wxEXPAND);
	SetSizer(sizer);
	Bind(wxEVT_SLIDER, &OptionsPanel::OnSlider, this, wxID_ANY);
}

bool MyApp::OnInit()
{
	wxInitAllImageHandlers();
	loadImages("img/left", images[0]);
	loadImages("img/right", images[1]);
	wxFrame *mainframe = new MyFrame("Hello World", images);
	mainframe->Show(true);
	return true;
}

MyFrame::MyFrame(const wxString& title, std::vector<std::string> (&images)[2]) : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(1024, 768)) , images(images) {
	doMenus();
	doPanels();
}
void MyApp::loadImages(const std::string& pathname, std::vector<std::string>& vec) {
	namespace fs = boost::filesystem;
	fs::path path(pathname);
	if (is_directory(path)) {
		std::vector<fs::directory_entry> entries;
		std::copy(fs::directory_iterator(path), fs::directory_iterator(),
				std::back_inserter(entries));
		std::transform(entries.begin(), entries.end(), std::back_inserter(vec),
				[](fs::directory_entry& e) { return e.path().string(); });
		std::sort(vec.begin(), vec.end());
	}
}

void MyFrame::doPanels() {
	wxGridSizer* sizer = new wxGridSizer(2, wxSize(3, 3));

	leftpic = new ImagePanel(this, images[0][0]);
	sizer->Add(leftpic, 1, wxEXPAND);
	rightpic = new ImagePanel(this, images[1][0]);
	sizer->Add(rightpic, 1, wxEXPAND);

	OptionsPanel * op = new OptionsPanel(this);
	sizer->Add(op, 1, wxEXPAND);

	disppic = new ImagePanel(this);
	sizer->Add(disppic);

	SetSizer(sizer);
}

void MyFrame::doMenus() {
	Bind(wxEVT_COMMAND_MENU_SELECTED, &MyFrame::OnExit, this, wxID_EXIT);

	wxMenu *menuFile = new wxMenu;
	menuFile->AppendSeparator();
	menuFile->Append(wxID_EXIT);
	wxMenuBar *menuBar = new wxMenuBar;
	menuBar->Append(menuFile, "&File");
	SetMenuBar(menuBar);

	CreateStatusBar();
	SetStatusText("Hello world");
}

void MyFrame::selectImage(int n) {
	std::cout << "image " << n << std::endl;
	leftpic->setImage(images[0][n]);
	rightpic->setImage(images[1][n]);
}


void OptionsPanel::OnSlider(wxCommandEvent& event) {
	boss->selectImage(slider->GetValue());
}

void MyFrame::OnExit(wxCommandEvent& event) {
	Close(true);
}

wxIMPLEMENT_APP(MyApp);
