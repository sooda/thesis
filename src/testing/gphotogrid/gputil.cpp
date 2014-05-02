#include "gputil.h"
#include <gphoto2/gphoto2.h>
#include <cstring>

namespace gp {

Aperture::Aperture(int value, std::vector<std::string> choices) : val(value), data(choices) {
}
int Aperture::index() const {
	return val;
}
void Aperture::set(int value) {
	if (value < 0 || value >= (int)data.size())
		throw std::out_of_range("bad aperture index");
	val = value;
}
const std::string& Aperture::text() const {
	return data[val];
}
const std::vector<std::string>& Aperture::choices() const {
	return data;
}
int Aperture::size() const {
	return (int)data.size();
}

Aperture Widget::Traits<Aperture>::read(Widget& widget) {
	const char* value;
	gp_widget_get_value(widget.widget, &value);

	std::vector<std::string> choices;
	int len = gp_widget_count_choices(widget.widget);
	choices.reserve(len);

	int value_id = 0;
	for (int i = 0; i < len; i++) {
		const char* choice;
		gp_widget_get_choice(widget.widget, i, &choice);

		if (strcmp(choice, value) == 0)
			value_id = i;

		choices.push_back(choice);
	}

	return Aperture(value_id, std::move(choices));
}

void Widget::Traits<Aperture>::write(Widget& widget, const Aperture& value) {
	int len = gp_widget_count_choices(widget.widget);
	if (len != value.size())
		throw std::invalid_argument("bad choice");
	gp_widget_set_value(widget.widget, const_cast<char*>(value.text().c_str()));
}

}
