#ifndef GFXUI_H
#define GFXUI_H

#include <wx/wx.h>
#include "app.h"

class GfxUi : public wxApp {
public:
	virtual bool OnInit();
	//virtual int OnExit();
	App app;
};

#endif
