#include "timeline.h"
#include <iostream>
#include <random>
#include <algorithm>

class MyApp : public wxApp {
public:
	virtual bool OnInit();
};

class MyFrame : public wxFrame {
public:
	MyFrame();
	Timeline* timeline;
	wxSlider* slider;
};

MyFrame::MyFrame() :
wxFrame(NULL, wxID_ANY, "timeline test", wxDefaultPosition, wxSize(800, 600)) {
	int nlines = 9;

	auto sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(slider = new wxSlider(this, wxID_ANY, 0, 0, 100, // val min max
					wxDefaultPosition, wxDefaultSize, wxSL_LABELS),
			1, wxEXPAND);
	sizer->Add(timeline = new Timeline(this, nlines), 1, wxEXPAND);
	SetSizer(sizer);
	
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> line(0, nlines-1);
	std::uniform_real_distribution<> mark(0, 100);
	std::vector<double> x(500);
	std::generate(x.begin(), x.end(), [&]() { return mark(gen); });
	std::sort(x.begin(), x.end());
	for (auto xi: x)
		timeline->insert(line(gen), xi);
}

bool MyApp::OnInit()
{
	wxFrame *frame = new MyFrame();
	frame->Show(true);
	return true;
}

wxIMPLEMENT_APP(MyApp);
