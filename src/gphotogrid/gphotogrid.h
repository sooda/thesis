#ifndef GPHOTOGRID_H
#define GPHOTOGRID_H

#include "app.h"
#include "imagepanel.h"
#include "optionspanel.h"
#include "timeline.h"
#include <wx/wx.h>
#include <chrono>

// image reload loop with a timer instead of idle events
// seems to hog the events somehow
// #define LOOP_TIMER

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
	void slider(int id, int value);
	void updatePhotos();
	void refresh();
	void onIdle(wxIdleEvent& ev);
	void reloadGphoto();
	void enableRender(bool enable);
	void syncGrabbers(bool sync);
	void timelineFollow(bool follow);
	void timelineEnable(bool enable);
private:
	typedef std::chrono::high_resolution_clock Clock;

	void onExit(wxCommandEvent& event);
	void onMenu(wxCommandEvent& event);

	void create_menus();
	void create_panels();
	void readCameraParams();
	wxSizer* create_imagegrid();

	template <class Obj>
	void setRadioConfig(int cam, int value);
	float time_since(Clock::time_point now, Clock::time_point before) const;

	App& app;
	std::vector<ImagePanel*> images;
	OptionsPanel* options;

	Timeline* timeline;
	std::vector<int> numpics;

#ifdef LOOP_TIMER
	RenderTimer* timer;
#endif
	Clock::time_point lasttime;
	int frames;
	void framecalc();

	// background threads for preview image grabbing
	Clock::time_point renderinittime;
};

#endif
