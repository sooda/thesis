#ifndef TIMELINE_H
#define TIMELINE_H

#include <wx/wx.h>
#include <chrono>
#include <utility>
#include <vector>

class Timeline : public wxPanel {
public:
#if 0
	typedef std::chrono::high_resolution_clock Clock;
	typedef Clock::time_point Mark;
	typedef Clock::duration Diff;
#endif
	// units in seconds
	typedef double Mark;
	typedef double Diff;

	Timeline(wxFrame* parent, int lines, bool follow = false);
	void insert(int line, Mark mark);
	void clear();
	void followInsertions(bool f);
	bool getFollow() const;
	size_t size() const;
private:
	void onPaint(wxPaintEvent&);
	void onSize(wxSizeEvent&);
	void onMotion(wxMouseEvent&);
	void onWheel(wxMouseEvent&);
	void onClick(wxMouseEvent&);

	void paintBackg(wxPaintDC& dc, int ox, int oy, int w, int h);
	void paintPoints(wxPaintDC& dc, int ox, int oy, int w, int h);
	void paintMouse(wxPaintDC& dc, int ox, int oy, int w, int h);
	void paintHeadBck(wxPaintDC& dc, int ox, int oy, int w, int h);
	void paintHeading(wxPaintDC& dc, int ox, int oy, int w, int h);
	void paintTicks(wxPaintDC& dc, int ox, int oy, int w, int nticks, bool text);
	void paintIndices(wxPaintDC& dc, int ox, int oy, int w, int h);
	void paintInfo(wxPaintDC& dc, int ox, int oy, int w, int h);

	int linepos(int y, int h) const;
	int dividepos(int y, int h, int places) const;
	int tick2px(Mark tick, int w) const;
	Mark px2tick(int x, int w) const;

	const int lines;

	bool follow;

	// render properties.
	// TODO meaningful colors, configurable somewhere?
	wxColor backgroundColor, headColor, majorTickColor, minorTickColor, cursorColor, cursorBackColor, textColor, markColor;
	double zoomratio;
	double scrollpanratio;
	double followratio;

	// small lines on the x axis
	int majorTicks, minorTicks;

	// zoom state.
	// these in seconds
	// (maybe store zoomlength and original length separately?)
	Mark start;
	Diff length;

	// mouse state.
	// these also in seconds so that they stay if the view is scrolled.
	Mark mousetick, dragtick;

	// middle mouse panning
	int mousex;
	Mark startdrag;

	// time, line. this order to be able to binary search properly.
	std::vector<std::pair<Mark, int>> marks;
};

#endif
