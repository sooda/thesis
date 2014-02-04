#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

class OptionsPanel : public wxPanel
{
	public:
		OptionsPanel(wxFrame* parent);
	private:
		wxButton* btn;
};

OptionsPanel::OptionsPanel(wxFrame* parent) : wxPanel(parent) {
	btn = new wxButton(this, wxID_ANY, "Moi", wxPoint(20, 20));
}

class wxImagePanel : public wxPanel
{
	wxBitmap image;

	public:
	wxImagePanel(wxFrame* parent, wxString file, wxBitmapType format);

	void paintEvent(wxPaintEvent & evt);
	void paintNow();

	void render(wxDC& dc);
};

wxImagePanel::wxImagePanel(wxFrame* parent, wxString file, wxBitmapType format) :
	wxPanel(parent)
{
	cv::Mat img = cv::imread(file, CV_LOAD_IMAGE_COLOR);
	void* pixels = img.data;
	wxImage im(img.cols, img.rows, (unsigned char*)pixels, true);
	image = wxBitmap(im);
	Bind(wxEVT_PAINT, &wxImagePanel::paintEvent, this, -1);
}

void wxImagePanel::paintEvent(wxPaintEvent & evt)
{
	wxPaintDC dc(this);
	render(dc);
}

void wxImagePanel::paintNow()
{
	// depending on your system you may need to look at double-buffered dcs
	wxClientDC dc(this);
	render(dc);
}

void wxImagePanel::render(wxDC&  dc)
{
	dc.DrawBitmap(image, 0, 0, false);
}

class MyApp: public wxApp
{
	public:
		virtual bool OnInit();
};

class MyFrame: public wxFrame
{
	public:
		MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size);
	private:
		void OnExit(wxCommandEvent& event);
		void doMenus();
		void doPanels();
};


wxIMPLEMENT_APP(MyApp);

bool MyApp::OnInit()
{
	wxInitAllImageHandlers();
	MyFrame *frame = new MyFrame("Hello World", wxPoint(50, 50), wxSize(800, 600));
	frame->Show(true);
	return true;
}

	MyFrame::MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size)
: wxFrame(NULL, wxID_ANY, title, pos, size)
{
	doMenus();
	doPanels();
}

void MyFrame::doPanels() {

	wxBoxSizer* sizer1 = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);

	wxImagePanel * drawPane;
	drawPane = new wxImagePanel(this, "image.jpg", wxBITMAP_TYPE_JPEG);
	sizer->Add(drawPane, 1, wxEXPAND);

	drawPane = new wxImagePanel(this, "image2.jpg", wxBITMAP_TYPE_JPEG);
	sizer->Add(drawPane, 1, wxEXPAND);

	sizer1->Add(sizer, wxSizerFlags(2).Expand());
	OptionsPanel * op = new OptionsPanel(this);
	sizer1->Add(op, 1, wxEXPAND);

	//sizer1->Add(sizer, 1, wxEXPAND);

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

void MyFrame::OnExit(wxCommandEvent& event)
{
	Close(true);
}
