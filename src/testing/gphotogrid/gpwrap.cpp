#include "gpwrap.h"
#include <gphoto2/gphoto2.h>
#include <iostream>
#include <algorithm>

namespace gp {
Exception::Exception(const std::string& msg, int gpnum) : 
	std::runtime_error("gphoto2 error " + msg
			+ ":" + std::to_string(gpnum)
			+ " (" + gp_result_as_string(gpnum) + ")")
{}

Context::Context() : context(gp_context_new()) {
	gp_context_set_error_func(context, error_func, NULL);
	gp_context_set_message_func(context, msg_func, NULL);
	gp_context_set_status_func(context, status_func, NULL);
}

Context::~Context() {
	gp_context_unref(context);
}

void Context::error_func(GPContext* /*context*/, const char *msg, void* /*data*/) {
	std::cerr << "!!! gphoto2 context error:\n"
		<< msg << std::endl;
}

void Context::msg_func(GPContext* /*context*/, const char* msg, void* /*data*/) {
	std::cerr << "!!! gphoto2 context message:\n"
		<< msg << std::endl;
}

void Context::status_func(GPContext* /*context*/, const char* msg, void* /*data*/) {
	std::cerr << "!!! gphoto2 context status:\n"
		<< msg << std::endl;
}

Widget::~Widget() {
	// move ctor nulls this
	// HOX seems that parent deletes its children without looking at the refcount
	if (widget && !parent)
		gp_widget_unref(widget);

	if (parent)
		gp_widget_unref(parent);
}

Widget::Widget(CameraWidget* widget) : widget(widget), parent(nullptr) {
}

Widget::Widget(CameraWidget* widget, Widget& parent) : widget(widget), parent(parent.widget) {
	gp_widget_ref(parent.widget);
}

Widget::Widget(Widget&& other) : widget(other.widget), parent(other.parent) {
	other.widget = nullptr;
	other.parent = nullptr;
}

Widget Widget::operator[](const char* name_or_label) {
	CameraWidget* child;
	int ret = gp_widget_get_child_by_name(widget, name_or_label, &child);

	if (ret < GP_OK)
		ret = gp_widget_get_child_by_label(widget, name_or_label, &child);

	if (ret < GP_OK)
		throw std::out_of_range("no widget found");
	
	return Widget(child, *this);
}

Widget::WidgetType Widget::type() {
	CameraWidgetType type;
	gp_widget_get_type(widget, &type);
	switch (type) {
		case GP_WIDGET_WINDOW:  return WIDGET_WINDOW;
		case GP_WIDGET_SECTION: return WIDGET_SECTION;
		case GP_WIDGET_TEXT:    return WIDGET_TEXT;
		case GP_WIDGET_RANGE:   return WIDGET_RANGE;
		case GP_WIDGET_TOGGLE:  return WIDGET_TOGGLE;
		case GP_WIDGET_RADIO:   return WIDGET_RADIO;
		case GP_WIDGET_MENU:    return WIDGET_MENU;
		case GP_WIDGET_DATE:    return WIDGET_DATE;
		case GP_WIDGET_BUTTON:  return WIDGET_BUTTON;
	};
	throw std::invalid_argument("Unknown type " + std::to_string((int)type));
}

std::string Widget::Traits<std::string>::read(Widget& widget) {
	const char *val;
	gp_widget_get_value(widget.widget, &val);
	return val;
}

void Widget::Traits<std::string>::write(Widget& widget, const std::string& value) {
	gp_widget_get_value(widget.widget, const_cast<char*>(value.c_str()));
}

bool Widget::Traits<bool>::read(Widget& widget) {
	int val;
	gp_widget_get_value(widget.widget, &val);
	return static_cast<bool>(val);
}

void Widget::Traits<bool>::write(Widget& widget, bool value) {
	int val = (int)value;
	gp_widget_set_value(widget.widget, &val);
}

Camera Context::auto_camera() {
	::Camera* cam;
	int ret;
	if ((ret = gp_camera_new(&cam)) < GP_OK)
		throw Exception("gp_camera_new", ret);

	if ((ret = gp_camera_init(cam, context)) < GP_OK) {
		gp_camera_unref(cam);
		throw Exception("gp_camera_init", ret);
	}
	return Camera(cam, *this);
}

Camera::Camera(::Camera* camera, Context& ctx) : camera(camera), ctx(ctx) {
	gp_context_ref(ctx.context);
}

Camera::~Camera() {
	gp_context_unref(ctx.context);
	//gp_camera_exit(camera,ctx.context);
	gp_camera_unref(camera); // freeing also exits it, i guess
}

Widget Camera::config() {
	CameraWidget *w;
	int ret;
	if ((ret = gp_camera_get_config(camera, &w, ctx.context)) < GP_OK)
		throw Exception("gp_camera_get_config", ret);
	return Widget(w);
}

std::vector<char> Camera::preview() {
	CameraFile* file;
	int ret;
	if ((ret = gp_file_new(&file)) < GP_OK)
		throw Exception("gp_file_new", ret);

	if ((ret = gp_camera_capture_preview(camera, file, ctx.context)) < GP_OK) {
		gp_file_unref(file);
		throw Exception("gp_camera_capture_preview", ret);
	}

	const char *rawbuf;
	size_t nbytes;
	if ((ret = gp_file_get_data_and_size(file, &rawbuf, &nbytes)) < GP_OK) {
		gp_file_unref(file);
		throw Exception("gp_file_get_data_and_size", ret);
	}

	std::vector<char> buf(nbytes);
	std::copy(rawbuf, rawbuf + nbytes, &buf[0]);

	gp_file_unref(file);
	return buf;
}

}
