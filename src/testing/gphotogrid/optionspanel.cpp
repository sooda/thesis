#include "optionspanel.h"
#include "gphotogrid.h"

OptionsPanel::OptionsPanel(MainFrame* parent) : wxPanel(parent), main(parent) {
	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(btn = new wxButton(this, wxID_ANY, "foo"),
			0, wxEXPAND);
	btn->Bind(wxEVT_BUTTON, &OptionsPanel::onButton, this, wxID_ANY);

	sliders.resize(3);

	int id = 0;
	for (auto& slider: sliders) {
		sizer->Add(slider = new wxSlider(this, id++, 0, 0, 1,
						wxDefaultPosition, wxDefaultSize, wxSL_LABELS),
				0, wxEXPAND);
		slider->Bind(wxEVT_SLIDER, &OptionsPanel::onSlider, this);
	}

	//sizer->SetSizeHints(this);
	SetSizer(sizer);
}

void OptionsPanel::setSliderRange(int id, int min, int max) {
	sliders.at(id)->SetRange(min, max);
}

void OptionsPanel::onSlider(wxCommandEvent& evt) {
	wxSlider& slid = *static_cast<wxSlider*>(evt.GetEventObject());
	main->slider(slid.GetId(), slid.GetValue());
}

void OptionsPanel::onButton(wxCommandEvent&) {
	main->reloadGphoto();
}
