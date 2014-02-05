#ifndef PAIRBROWSE_H
#define PAIRBROWSE_H

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

class OptionsPanel : public wxPanel
{
public:
	OptionsPanel(wxFrame* parent);
private:
	wxButton* btn;
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

class MyApp: public wxApp
{
public:
	virtual bool OnInit();
};


#endif
