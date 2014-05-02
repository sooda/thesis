#ifndef OPTIONSPANEL_H
#define OPTIONSPANEL_H

#include <wx/wx.h>
class MainFrame;

class OptionsPanel : public wxPanel {
public:
	OptionsPanel(MainFrame* parent);
	void setSliderRange(int id, int min, int max);
	void onSlider(wxCommandEvent& event);
	void onButton(wxCommandEvent& event);

private:
	MainFrame* main;

	wxButton* btn;
	wxSlider* slider;
};

#endif
