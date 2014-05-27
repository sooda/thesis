#include "gpwrap.h"

namespace gp {

class RadioWidget {
public:
	RadioWidget(int value, std::vector<std::string> choices);
	RadioWidget(const RadioWidget&) = default;
	int index() const;
	void set(int value);
	const std::string& text() const;
	const std::vector<std::string>& choices() const;
	int size() const;

private:
	int val;
	std::vector<std::string> data;
};

template <class Value>
struct RadioTraits {
	static const Widget::WidgetType type = Widget::WIDGET_RADIO;
	static Value read(Widget& w) {
		return Widget::Traits<RadioWidget>::read(w);
	}
	static void write(Widget& w, const Value& value) {
		Widget::Traits<RadioWidget>::write(w, value);
	}
};

class Aperture : public RadioWidget {
public:
	static const char* gpname; // "aperture"
	Aperture(const RadioWidget& r) : RadioWidget(r) {}
};
template <>
struct Widget::Traits<Aperture> : RadioTraits<Aperture> {};

class ShutterSpeed : public RadioWidget {
public:
	static const char* gpname; // "shutterspeed"
	ShutterSpeed(const RadioWidget& r) : RadioWidget(r) {}
};
template <>
struct Widget::Traits<ShutterSpeed> : RadioTraits<ShutterSpeed> {};

class Iso : public RadioWidget {
public:
	static const char* gpname; // "iso"
	Iso(const RadioWidget& r) : RadioWidget(r) {}
};
template <>
struct Widget::Traits<Iso> : RadioTraits<Iso> {};

}
