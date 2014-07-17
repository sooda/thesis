#include "optionspanel.h"
#include "gphotogrid.h"

OptionsPanel::OptionsPanel(MainFrame* parent) : wxPanel(parent), main(parent) {
	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(btn = new wxButton(this, wxID_ANY, "rescan cameras"),
			0, wxEXPAND);
	btn->Bind(wxEVT_BUTTON, &OptionsPanel::onButton, this, wxID_ANY);
	btn->Disable();

	sizer->Add(enablecheck = new wxCheckBox(this, wxID_ANY, "enable render"));
	enablecheck->Bind(wxEVT_CHECKBOX, &OptionsPanel::onCheckbox, this);
	enablecheck->SetValue(true);

	sizer->Add(synccheck = new wxCheckBox(this, wxID_ANY, "\"sync\" grabbers"));
	synccheck->Disable();
	synccheck->Bind(wxEVT_CHECKBOX, &OptionsPanel::onSyncbox, this);

	sizer->Add(followcheck = new wxCheckBox(this, wxID_ANY, "autoscroll timeline"));
	followcheck->Bind(wxEVT_CHECKBOX, &OptionsPanel::onFollowbox, this);

	sizer->Add(timelinecheck = new wxCheckBox(this, wxID_ANY, "show timeline"));
	timelinecheck->SetValue(true);
	timelinecheck->Bind(wxEVT_CHECKBOX, &OptionsPanel::onTimelinebox, this);

	selectors.resize(3);
	static const char* labels[] = { "aperture", "shutter speed", "iso speed" };
	for (size_t i = 0; i < selectors.size(); i++) {
		sizer->Add(new wxStaticText(this, wxID_ANY, labels[i]));
		sizer->Add(selectors[i] = new wxChoice(this, i),
				0, wxEXPAND | wxALL, 3); // no proportion, small border (padding)
		selectors[i]->Bind(wxEVT_CHOICE, &OptionsPanel::onSelect, this);
	}

	//sizer->SetSizeHints(this);
	SetSizer(sizer);
}

void OptionsPanel::setSelections(int id, const std::vector<std::string>& options, int current) {
	std::vector<wxString> tempcontents(options.begin(), options.end());
	selectors.at(id)->Set(tempcontents.size(), &tempcontents[0]);
	selectors.at(id)->SetSelection(current);
}

void OptionsPanel::onSelect(wxCommandEvent& evt) {
	wxChoice& obj = *static_cast<wxChoice*>(evt.GetEventObject());
	if (obj.GetSelection() != wxNOT_FOUND)
		main->slider(obj.GetId(), obj.GetSelection());
}

void OptionsPanel::onButton(wxCommandEvent&) {
	main->reloadGphoto();
}

void OptionsPanel::onCheckbox(wxCommandEvent& evt) {
	btn->Enable(!evt.IsChecked());
	synccheck->Enable(!evt.IsChecked());
	main->enableRender(evt.IsChecked());
}

void OptionsPanel::onSyncbox(wxCommandEvent& evt) {
	main->syncGrabbers(evt.IsChecked());
}

void OptionsPanel::onFollowbox(wxCommandEvent& evt) {
	main->timelineFollow(evt.IsChecked());
}

void OptionsPanel::onTimelinebox(wxCommandEvent& evt) {
	main->timelineEnable(evt.IsChecked());
	if (evt.IsChecked()) {
		followcheck->Enable();
	} else {
		followcheck->SetValue(false);
		followcheck->Disable();
	}
}
