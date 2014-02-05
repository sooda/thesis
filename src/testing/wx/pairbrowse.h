#ifndef PAIRBROWSE_H
#define PAIRBROWSE_H

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "imagepanel.h"

class MyFrame;

class OptionsPanel : public wxPanel
{
public:
	OptionsPanel(MyFrame* parent);
private:
	void OnSlider(wxCommandEvent& event);
	wxButton* btn;
	wxSlider* slider;
	MyFrame* boss;
};

class MyApp: public wxApp
{
public:
	virtual bool OnInit();
	void loadImages(const std::string& pathname, std::vector<std::string>& vec);

	std::vector<std::string> images[2]; // left, right
};

class MyFrame: public wxFrame
{
public:
	MyFrame(const wxString& title, std::vector<std::string> (&images)[2]);
	void selectImage(int n);
private:
	void OnExit(wxCommandEvent& event);

	void doMenus();
	void doPanels();
	void loadImages();

	ImagePanel* leftpic;
	ImagePanel* rightpic;
	ImagePanel* disppic;

	std::vector<std::string> (&images)[2]; // left, right
};

#endif
