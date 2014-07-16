#include "imagepanel.h"

ImagePanel::ImagePanel(wxFrame* parent) : wxPanel(parent) {
	Bind(wxEVT_PAINT, &ImagePanel::paintEvent, this);
}

void ImagePanel::setImage(const wxImage& im, const std::string& l) {
	bitmap = wxBitmap(im);
	label = l;
	SetSize(im.GetSize());
	paintNow();
}

void ImagePanel::clearImage() {
	bitmap = wxBitmap();
	SetSize(0, 0);
	paintNow();
}

void ImagePanel::paintEvent(wxPaintEvent&) {
	wxPaintDC dc(this);
	render(dc);
}

void ImagePanel::paintNow() {
	wxClientDC dc(this); // TODO: Refresh() instead?
	render(dc);
}

void ImagePanel::render(wxDC& dc) {
	if (bitmap.IsOk()) {
		dc.DrawBitmap(bitmap, 0, 0, false);
		dc.SetTextForeground("red");
		dc.SetTextBackground("black");
		wxFont font = dc.GetFont();
		font.SetPixelSize(wxSize(0, 20));
		dc.SetFont(font);
		dc.DrawText(label, 0, 0);
	}
}
