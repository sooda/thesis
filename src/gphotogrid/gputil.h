#include "gpwrap.h"

// utilities for gpwrap's Widget data types, more complex than plain strings or
// bools. It's safer to carry some semantic meaning along the objects. These
// are safe to copy around.

namespace gp {

// Radio selection has multiple possible values and one currently selected.
// Top-level class for other more specialized ones with a specific name.
class RadioWidget {
public:
	RadioWidget(int value, std::vector<std::string> choices);
	RadioWidget(const RadioWidget&) = default;

	// state
	int index() const;
	const std::string& text() const;
	void set(int value);

	// choices
	const std::vector<std::string>& choices() const;
	int size() const;

private:
	int val;
	std::vector<std::string> data;
};

// All derived classes use the same style to read and write the state
template <class Value>
struct RadioTraits {
	static const Widget::WidgetType type = Widget::WIDGET_RADIO;
	static Value read(const Widget& w) {
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
