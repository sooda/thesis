#ifndef OPTIONSPANEL_H
#define OPTIONSPANEL_H

#include <wx/wx.h>
#include <string>
#include <vector>

class MainFrame;

class OptionsPanel : public wxPanel {
public:
	OptionsPanel(MainFrame* parent);
	void setSelections(int id, const std::vector<std::string>& options, int current);
	void onSelect(wxCommandEvent& event);
	void onButton(wxCommandEvent& event);
	void onCheckbox(wxCommandEvent& event);
	void onSyncbox(wxCommandEvent& event);
	void onFollowbox(wxCommandEvent& event);
	void onTimelinebox(wxCommandEvent& event);

private:
	MainFrame* main;

	wxButton* btn;
	std::vector<wxChoice*> selectors;
	wxCheckBox* enablecheck;
	wxCheckBox* synccheck;
	wxCheckBox* followcheck;
	wxCheckBox* timelinecheck;
};

#endif
