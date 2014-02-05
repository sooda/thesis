#include "pairbrowse.h"
#include "imagepanel.h"

OptionsPanel::OptionsPanel(wxFrame* parent) : wxPanel(parent) {
	btn = new wxButton(this, wxID_ANY, "Moi", wxPoint(20, 20));
}

bool MyApp::OnInit()
{
	wxInitAllImageHandlers();
	wxFrame *mainframe = new MyFrame("Hello World", wxPoint(50, 50), wxSize(800, 600));
	mainframe->Show(true);
	return true;
}

MyFrame::MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size) : wxFrame(NULL, wxID_ANY, title, pos, size) {
	doMenus();
	doPanels();
}

void MyFrame::doPanels() {
	wxBoxSizer* sizer1 = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);

	ImagePanel * drawPane;
	drawPane = new ImagePanel(this, "image.jpg");
	sizer->Add(drawPane, 1, wxEXPAND);

	drawPane = new ImagePanel(this, "image2.jpg");
	sizer->Add(drawPane, 1, wxEXPAND);

	OptionsPanel * op = new OptionsPanel(this);
	sizer1->Add(sizer, 1, wxEXPAND);
	sizer1->Add(op, 1, wxEXPAND);

	SetSizer(sizer1);
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

void MyFrame::OnExit(wxCommandEvent& event) {
	Close(true);
}

wxIMPLEMENT_APP(MyApp);
