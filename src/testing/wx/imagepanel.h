#ifndef IMAGEPANEL_H
#define IMAGEPANEL_H

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

class ImagePanel : public wxPanel {
public:
	ImagePanel(wxFrame* parent, const std::string& file);

	void paintEvent(wxPaintEvent& evt);
	void paintNow();

	void render(wxDC& dc);
private:
	void setImage(const std::string& file);

	wxBitmap image;
};

#endif
