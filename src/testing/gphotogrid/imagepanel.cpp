#include "imagepanel.h"

ImagePanel::ImagePanel(wxFrame* parent) : wxPanel(parent) {
	Bind(wxEVT_PAINT, &ImagePanel::paintEvent, this);
}

void ImagePanel::setImage(wxImage& im) {
	bitmap = wxBitmap(im);
	SetSize(im.GetSize());
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
	if (bitmap.IsOk())
		dc.DrawBitmap(bitmap, 0, 0, false);
}
