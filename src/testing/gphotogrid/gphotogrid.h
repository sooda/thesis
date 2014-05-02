#ifndef GPHOTOGRID_H
#define GPHOTOGRID_H

#include "app.h"
#include "imagepanel.h"
#include "optionspanel.h"
#include <wx/wx.h>

// image reload loop with a timer instead of idle events
// seems to hog the events somehow
// #define LOOP_TIMER

class GfxUi : public wxApp {
public:
	virtual bool OnInit();
	App app;
};

#ifdef LOOP_TIMER
class MainFrame;
class RenderTimer : public wxTimer {
	MainFrame& root;
public:
	RenderTimer(MainFrame& root);
	void Notify();
	void start();
};
#endif

class MainFrame: public wxFrame {
public:
	MainFrame(const wxString& title, App& app);
	void slider(int n);
	void updatePhotos();
	void refresh();
	void onIdle(wxIdleEvent& ev);
	void reloadGphoto();

private:
	void onExit(wxCommandEvent& event);
	void onMenu(wxCommandEvent& event);

	void create_menus();
	void create_panels();
	wxSizer* create_imagegrid();
	void loadImages();

	App& app;
	std::vector<ImagePanel*> images;
	OptionsPanel* options;
#ifdef LOOP_TIMER
	RenderTimer* timer;
#endif
};

#endif
