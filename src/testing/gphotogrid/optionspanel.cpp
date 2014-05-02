#include "optionspanel.h"
#include "gphotogrid.h"

OptionsPanel::OptionsPanel(MainFrame* parent) : wxPanel(parent), main(parent) {
	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(btn = new wxButton(this, wxID_ANY, "rescan cameras"),
			0, wxEXPAND);
	btn->Bind(wxEVT_BUTTON, &OptionsPanel::onButton, this, wxID_ANY);

	sliders.resize(3);
	static const char* labels[] = { "aperture", "shutter speed", "iso speed" };
	for (int i = 0; i < sliders.size(); i++) {
		sizer->Add(new wxStaticText(this, wxID_ANY, labels[i]));
		sizer->Add(sliders[i] = new wxSlider(this, i, 0, 0, 0, // val min max
						wxDefaultPosition, wxDefaultSize, wxSL_LABELS),
				0, wxEXPAND | wxALL, 3); // no proportion, small border (padding)
		sliders[i]->Bind(wxEVT_SLIDER, &OptionsPanel::onSlider, this);
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
