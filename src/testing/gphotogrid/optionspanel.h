#ifndef OPTIONSPANEL_H
#define OPTIONSPANEL_H

#include <wx/wx.h>
class MainFrame;

class OptionsPanel : public wxPanel {
public:
	OptionsPanel(MainFrame* parent);
	void setSliderRange(int id, int min, int max, int current);
	void onSlider(wxCommandEvent& event);
	void onButton(wxCommandEvent& event);
	void onCheckbox(wxCommandEvent& event);
	void onSyncbox(wxCommandEvent& event);

private:
	MainFrame* main;

	wxButton* btn;
	std::vector<wxSlider*> sliders;
	wxCheckBox* enablecheck;
	wxCheckBox* synccheck;
};

#endif
