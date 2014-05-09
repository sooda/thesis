#ifndef IMAGEPANEL_H
#define IMAGEPANEL_H

#include <wx/wx.h>

class ImagePanel : public wxPanel {
public:
	ImagePanel(wxFrame* parent);
	void paintEvent(wxPaintEvent&);
	void paintNow();
	void render(wxDC& dc);
	void setImage(wxImage& im);
	void clearImage();

private:
	wxBitmap bitmap;
};

#endif
