#include "gputil.h"
#include <gphoto2/gphoto2.h>
#include <cstring>

namespace gp {

RadioWidget::RadioWidget(int value, std::vector<std::string> choices)
	: val(value), data(std::move(choices)) {
}

int RadioWidget::index() const {
	return val;
}

const std::string& RadioWidget::text() const {
	return data[val];
}

void RadioWidget::set(int value) {
	if (value < 0 || value >= (int)data.size())
		throw std::out_of_range("bad radiowidget index");
	val = value;
}

const std::vector<std::string>& RadioWidget::choices() const {
	return data;
}

int RadioWidget::size() const {
	return (int)data.size();
}

template <>
RadioWidget Widget::Traits<RadioWidget>::read(const Widget& widget) {
	// gp hands out a shallow copy
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

	return RadioWidget(value_id, std::move(choices));
}

template <>
void Widget::Traits<RadioWidget>::write(Widget& widget, const RadioWidget& value) {
	int len = gp_widget_count_choices(widget.widget);
	if (len != value.size())
		throw std::invalid_argument("radiowidget choice size mismatch");
	gp_widget_set_value(widget.widget, value.text().c_str());
}

const char* Aperture::gpname = "aperture";
const char* ShutterSpeed::gpname = "shutterspeed";
const char* Iso::gpname = "iso";

}
