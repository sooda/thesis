#include "timeline.h"
#include <string>
#include <algorithm>
#include <iomanip>
#include <sstream>

Timeline::Timeline(wxFrame* parent, int lines, bool follow) : wxPanel(parent),
		lines(lines), follow(follow),
		backgroundColor("black"),
		headColor("dark grey"),
		majorTickColor("red"),
		minorTickColor("black"),
		cursorColor("green"),
		cursorBackColor("dark green"),
		textColor("grey"),
		markColor("white"),
		zoomratio(1.5), scrollpanratio(0.1), followratio(0.9),
		majorTicks(9), minorTicks(4),
		start(0.0), length(10.0),
		mousetick(0.0), dragtick(-1.0) {
	Bind(wxEVT_PAINT, &Timeline::onPaint, this);

	Bind(wxEVT_SIZE, &Timeline::onSize, this);

	Bind(wxEVT_MOTION, &Timeline::onMotion, this);
	Bind(wxEVT_MOUSEWHEEL, &Timeline::onWheel, this);
	Bind(wxEVT_LEFT_DOWN, &Timeline::onClick, this);
	Bind(wxEVT_LEFT_UP, &Timeline::onClick, this);
	Bind(wxEVT_MIDDLE_DOWN, &Timeline::onClick, this);
	Bind(wxEVT_MIDDLE_UP, &Timeline::onClick, this);
	Bind(wxEVT_RIGHT_DOWN, &Timeline::onClick, this);
	Bind(wxEVT_RIGHT_UP, &Timeline::onClick, this);
}

void Timeline::insert(int line, Timeline::Mark mark) {
	// assume that the user doesn't insert ordered points
	//marks.push_back(std::make_pair(mark, line));
	auto m = std::make_pair(mark, line);
	auto pos = std::upper_bound(marks.begin(), marks.end(), m);
	marks.insert(pos, m);
	if (follow) {
		// (mark - start) / length = followratio
		start = mark - followratio * length;
	}
	Refresh();
}

void Timeline::clear() {
	marks.clear();
	Refresh();
}

void Timeline::followInsertions(bool f) {
	follow = f;
}

bool Timeline::getFollow() const {
	return follow;
}

size_t Timeline::size() const {
	return marks.size();
}

void Timeline::onMotion(wxMouseEvent& evt) {
	if (evt.Dragging()) {
		// extend painted area
		if (evt.LeftIsDown() || evt.RightIsDown()) {
			if (dragtick == -1)
				dragtick = mousetick;
			mousetick = px2tick(evt.GetX(), GetSize().GetWidth());
			Refresh();
		} else if (evt.MiddleIsDown()) {
			// pan
			int w = GetSize().GetWidth();
			start = startdrag - px2tick(evt.GetX(), w) + px2tick(mousex, w);
			Refresh();
		}
	}
}

void Timeline::onWheel(wxMouseEvent& evt) {
	if (evt.GetWheelAxis() == wxMOUSE_WHEEL_VERTICAL) {
		// wheel downwards is negative
		int rot = evt.GetWheelRotation() / evt.GetWheelDelta();
		if (evt.ControlDown()) {
			// zoom
			Mark mousepos = px2tick(evt.GetX(), GetSize().GetWidth());
			double proportion = rot < 0 ? 1.0 / (zoomratio * -rot) : zoomratio * rot;
			length = proportion * length;
			// pos stays at the mouse cursor while zooming
			// (pos-start)/length = (pos-newstart)/(prop*length)
			start = mousepos - proportion * (mousepos - start);
		} else {
			// pan
			start -= rot * length * scrollpanratio;
		}
		Refresh();
	}
}

void Timeline::onClick(wxMouseEvent& evt) {
	if (evt.LeftDown()) {
		// mark and/or start drag paint
		dragtick = -1;
		mousex = evt.GetX();
		mousetick = px2tick(mousex, GetSize().GetWidth());
		startdrag = start;
		Refresh();
	} else if (evt.MiddleDown()) {
		// start pan
		mousex = evt.GetX();
		startdrag = start;
	} else if (evt.RightDown()) {
		// extend paint
		mousetick = px2tick(evt.GetX(), GetSize().GetWidth());
		Refresh();
	}
	evt.Skip();
}

void Timeline::onPaint(wxPaintEvent&) {
	wxPaintDC dc(this);

	wxSize size(GetSize());
	int w(size.GetWidth()), h(size.GetHeight());
	wxSize em(dc.GetTextExtent("m"));
	int emw(em.GetWidth()), emh(em.GetHeight());

	// (heading is actually on the bottom)
	int headingh = 2 * emh;
	// draw area size
	int areah = h - headingh;
	// indice column width
	int indw = 2*emw;

	// much easier to work with a full-width drawing area (mouse scalings etc.)
	// mouse gets drawn on top of these
	paintHeadBck(dc, 0, areah, w, headingh);
	paintBackg(dc, 0, 0, w, areah);

	// mouse first so that dragged area doesn't overwrite text and lines
	paintMouse(dc, 0, 0, w, h);

	// x axis heading and screen lines
	paintHeading(dc, 0, areah, w, headingh);
	paintPoints(dc, 0, 0, w, areah);
	// line indices; just discard a section of the lines area
	paintIndices(dc, 0, 0, indw, areah);
	// mouse pos etc
	paintInfo(dc, indw, 0, w-indw, areah);
}

void Timeline::onSize(wxSizeEvent&) {
	Refresh();
}

// y starts from 0
// divide "     "
// into   " | | "
// equal pieces with ticks in between
int Timeline::dividepos(int y, int h, int places) const {
	return (y + 1) * h / (places + 1);
}
int Timeline::linepos(int y, int h) const {
	return dividepos(y, h, lines);
}

// tick in absolute units (meaningful only when it's displayed, though)
int Timeline::tick2px(Mark tick, int w) const {
	return (tick - start) * w / length;
}

Timeline::Mark Timeline::px2tick(int x, int w) const {
	return start + x * length / w;
}
// print to string in some digits
static std::string float2str(double f, int precision) {
	std::stringstream ss;
	ss << std::setprecision(precision) << f;
	return ss.str();
}

// background for point area
void Timeline::paintBackg(wxPaintDC& dc, int ox, int oy, int w, int h) {
	dc.SetPen(backgroundColor);
	dc.SetBrush(backgroundColor);
	dc.DrawRectangle(ox, oy, w, h);
}

// points and stuff in drawing area
void Timeline::paintPoints(wxPaintDC& dc, int ox, int oy, int w, int h) {
	// horizontal bars
	// `lines` number of thin bars, divides space to 1+`lines` spaces
	// 1/n, 2/n, ..., (1-n)/n
	dc.SetPen(majorTickColor);
	for (int i = 0; i < lines; i++) {
		int y = linepos(i, h);
		dc.DrawLine(ox, oy+y, ox+w, oy+y);
	}

	if (marks.empty())
		return;

	// points are sorted; get the bounds for pts on the screen
	auto lower = std::lower_bound(marks.begin(), marks.end(),
			std::make_pair(start, 0));
	auto upper = std::upper_bound(marks.begin(), marks.end(),
			std::make_pair(start+length, lines-1));

	dc.SetPen(markColor);
	dc.SetBrush(markColor);
	int radius = linepos(0, h) / 8; // this looks nice enough, dunno if should fix a pixel size
	for (auto it = lower; it < upper; it++) {
		Mark m = it->first;
		if (m >= start && m < start + length) {
			int x = tick2px(m, w);
			int y = linepos(it->second, h);
			dc.DrawCircle(ox + x, oy + y, radius);
			// line from current to bottom looks confusing because the lines overlap the next lines.
			// so, just draw a line from top to bottom
			// dc.DrawLine(ox + x, oy + y, ox + x, oy + h);
			dc.DrawLine(ox + x, oy, ox + x, oy + h);
		}
	}
}

// mouse line or large dragpainted area
void Timeline::paintMouse(wxPaintDC& dc, int ox, int oy, int w, int h) {
	dc.SetPen(cursorColor);
	dc.SetBrush(cursorBackColor);

	int mousex = tick2px(mousetick, w);
	int dragx = tick2px(dragtick, w);

	dc.DrawLine(ox + mousex, oy, ox + mousex, oy + h);
	if (dragtick != -1) {
		dc.DrawLine(ox + dragx, oy, ox + dragx, oy + h);
		dc.DrawRectangle(ox + dragx, oy, mousex - dragx, h);
	}
}

// back fill color of heading
void Timeline::paintHeadBck(wxPaintDC& dc, int ox, int oy, int w, int h) {
	dc.SetPen(headColor);
	dc.SetBrush(headColor);
	dc.DrawRectangle(ox, oy, w, h);
}

// x axis ticks and label hints
void Timeline::paintHeading(wxPaintDC& dc, int ox, int oy, int w, int /*h*/) {
	dc.SetTextForeground(backgroundColor);
	dc.SetTextBackground(headColor);

	dc.SetPen(minorTickColor);
	// majorticks + 1 open spaces to tickify
	// all but last one need also one minor tick that align with majors
	paintTicks(dc, ox, oy, w, (majorTicks + 1) * (minorTicks + 1) - 1, false);

	dc.SetPen(majorTickColor);
	paintTicks(dc, ox, oy, w, majorTicks, true);
}

// ticks at uniform spacing
void Timeline::paintTicks(wxPaintDC& dc, int ox, int oy, int w, int nticks, bool text) {
	wxSize em(dc.GetTextExtent("m"));
	int emh(em.GetHeight());

	double resolution = length / (nticks + 1);
	int in_tick = start / resolution;
	for (Mark tick = in_tick * resolution; tick <= start + length; tick += resolution) {
		int x = tick2px(tick, w);
		std::string num(float2str(tick, 5));
		dc.DrawLine(ox + x, oy, ox + x, oy + emh);
		if (text)
			dc.DrawText(num, ox + x - dc.GetTextExtent(num).GetWidth() / 2, oy + emh);
	}
}

// index column for the horizontal lines
void Timeline::paintIndices(wxPaintDC& dc, int ox, int oy, int w, int h) {
	wxSize em(dc.GetTextExtent("m"));
	int emh(em.GetHeight());

	dc.SetPen(headColor);
	dc.SetBrush(headColor);
	dc.DrawRectangle(ox, oy, w, h);

	dc.SetTextBackground(headColor);
	dc.SetTextForeground(backgroundColor);

	for (int i = 0; i < lines; i++) {
		int y = linepos(i, h);
		dc.DrawText(std::to_string(i), ox + emh/2, oy + y - emh/2);
	}
}

// mouse cursor information
void Timeline::paintInfo(wxPaintDC& dc, int ox, int oy, int /*w*/, int /*h*/) {
	dc.SetTextForeground(textColor);
	dc.SetTextBackground(backgroundColor);
	std::string msg = "mouse tick " + float2str(mousetick, 5);
	if (dragtick != -1) {
		msg += " drag start " + float2str(dragtick, 5);
		msg += " difference " + float2str(mousetick - dragtick, 5);
	}
	dc.DrawText(msg, ox, oy);
}
