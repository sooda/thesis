#include "optionspanel.h"
#include "gphotogrid.h"

OptionsPanel::OptionsPanel(MainFrame* parent) : wxPanel(parent), main(parent) {
	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(btn = new wxButton(this, wxID_ANY, "foo"),
			0, wxEXPAND);
	// FIXME load slider values from gphoto
	sizer->Add(slider = new wxSlider(this, wxID_ANY, 0, 0, 9,
					wxDefaultPosition, wxDefaultSize, wxSL_LABELS),
			0, wxEXPAND);

	//sizer->SetSizeHints(this);
	SetSizer(sizer);

	Bind(wxEVT_SLIDER, &OptionsPanel::onSlider, this, wxID_ANY);
	Bind(wxEVT_BUTTON, &OptionsPanel::onButton, this, wxID_ANY);
}

void OptionsPanel::onSlider(wxCommandEvent&) {
	main->slider(slider->GetValue());
}

void OptionsPanel::onButton(wxCommandEvent&) {
	main->reloadGphoto();
}
