#include "imagepanel.h"
#include <opencv2/highgui/highgui.hpp>

ImagePanel::ImagePanel(wxFrame* parent, const std::string& file) : wxPanel(parent) {
	setImage(file);
	Bind(wxEVT_PAINT, &ImagePanel::paintEvent, this);
}

ImagePanel::ImagePanel(wxFrame* parent) : wxPanel(parent) {
	Bind(wxEVT_PAINT, &ImagePanel::paintEvent, this);
}

void ImagePanel::setImage(const std::string& file) {
	cv::Mat img = cv::imread(file, CV_LOAD_IMAGE_COLOR);
	unsigned char* pixels = static_cast<unsigned char*>(img.data);
	wxImage im(img.cols, img.rows, pixels, true); // does not take ownership
	image = wxBitmap(im);
	paintNow();
}

void ImagePanel::paintEvent(wxPaintEvent & evt) {
	wxPaintDC dc(this);
	render(dc);
}

void ImagePanel::paintNow() {
	wxClientDC dc(this);
	render(dc);
}

void ImagePanel::render(wxDC&  dc) {
	if (image.IsOk())
		dc.DrawBitmap(image, 0, 0, false);
}

