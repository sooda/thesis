#include "gpwrap.h"

namespace gp {

class Aperture {
public:
	Aperture(int value, std::vector<std::string> choices);
	Aperture(const Aperture&);
	int index() const;
	void set(int value);
	const std::string& text() const;
	const std::vector<std::string>& choices() const;
	int size() const;
private:
	int val;
	std::vector<std::string> data;
};

template <>
struct Widget::Traits<Aperture> {
	static const WidgetType type = WIDGET_RADIO;
	static Aperture read(Widget& w);
	static void write(Widget& w, const Aperture& value);
};

}
