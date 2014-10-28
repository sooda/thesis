#include "gpwrap.h"
#include <gphoto2/gphoto2.h>
#include <iostream>
#include <algorithm>
#include <memory>
#include <fstream>
#include <cstdlib>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

namespace gp {

// Use this only for gphoto2 errors, not internal wrapper errors
Exception::Exception(const std::string& msg, int gpnum) : 
	std::runtime_error("gphoto2 error " + msg
			+ ":" + std::to_string(gpnum)
			+ " (" + gp_result_as_string(gpnum) + ")")
{}

// libgphoto has a nasty way of initializing objects by taking a reference to a
// pointer. Wrap it to return a readily constructed object, even in a safe
// unique_ptr with a proper unref deleter, throwing exceptions on errors. we'd
// throw later anyway if the construction would fail.
// (the gp_foo_unref() and gp_foo_free() probably return always GP_OK)
template <class GpObject, class GpCtor, class GpDtor>
std::unique_ptr<GpObject, GpDtor> gp_new_unique(GpCtor ctor, const char* ctorname, GpDtor dtor) {
	GpObject* obj;
	int ret = ctor(&obj);
	if (ret < GP_OK)
		throw Exception(ctorname, ret);
	return std::unique_ptr<GpObject, GpDtor>(obj, dtor);
}

// the gp_ prefix is also common among all funcs, but mandatory for clarity
#define GP_NEW_UNIQUE(obj, ctorname) \
	gp_new_unique<obj>(ctorname ## _new, #ctorname "_new", ctorname ## _unref)

// CONTEXT
// =======

Context::Context(bool use_messages) : context(gp_context_new()) {
	if (use_messages)
		init_messages();
}
void Context::init_messages() {
	gp_context_set_error_func(context, error_func, NULL);
	gp_context_set_message_func(context, msg_func, NULL);
	gp_context_set_status_func(context, status_func, NULL);
	// debug logging is massive
	const char *debug_enable = getenv("GPWRAP_LOG_DEBUG");
	static bool debug_created;
	if (debug_enable && debug_enable[0] == '1' && !debug_created) {
		debug_created = true;
		// maybe should have a top-level class for this as it's not context specific
		// typecast because just enum vs int in header, should be safe
		gp_log_add_func(GP_LOG_DEBUG, reinterpret_cast<GPLogFunc>(log_func), NULL);
	}
}

Context::~Context() {
	gp_context_unref(context);
}

void Context::error_func(GPContext* /*context*/, const char *msg, void* /*data*/) {
	std::cerr << "!!! gphoto2 context error: " << msg << std::endl;
}

void Context::msg_func(GPContext* /*context*/, const char* msg, void* /*data*/) {
	std::cerr << "!!! gphoto2 context message: " << msg << std::endl;
}

void Context::status_func(GPContext* /*context*/, const char* msg, void* /*data*/) {
	std::cerr << "!!! gphoto2 context status:" << msg << std::endl;
}

void Context::log_func(int level, const char* domain, const char* str, void* /*data*/) {
	std::cerr << "[gphoto2 log (level=" << level << ", domain=" << domain
		<< "]: " << str << std::endl;
}

Camera Context::auto_camera() {
	return Camera(*this);
}

std::vector<Camera> Context::all_cameras() {
	auto list = GP_NEW_UNIQUE(CameraList, gp_list);
	int ret;
	if ((ret = gp_camera_autodetect(list.get(), context)) < GP_OK) {
		throw Exception("gp_camera_autodetect", ret);
	}

	std::vector<Camera> cams;
	for (int i = 0, count = ret; i < count; i++) {
		const char *name, *value;
		gp_list_get_name(list.get(), i, &name);
		gp_list_get_value(list.get(), i, &value);
		// XXX: remove this debug message?
		std::cout << name << "|" << value << std::endl;
		// cannot emplace because private ctor
		cams.push_back(Camera(name, value, *this));
	}
	return cams;
}

// CAMERA
// ======

Camera::Camera(Context& ctx) : ctx(&ctx) {
	auto cam = GP_NEW_UNIQUE(::Camera, gp_camera);
	int ret;

	if ((ret = gp_camera_init(cam.get(), ctx.context)) < GP_OK) {
		throw Exception("gp_camera_init", ret);
	}
	camera = cam.release();
	gp_context_ref(ctx.context);
}

static int check_retval(int ret, const char *funcname) {
	if (ret < GP_OK)
		throw Exception(funcname, ret);
	return ret;
}
#define GP_OR_THROW(func, ...) \
	check_retval(func(__VA_ARGS__), #func)

// stolen from libphoto2 examples/autodetect.c
// most of this seems to be in gp_camera_init autodetection too
Camera::Camera(const char *model, const char *port, Context& ctx) : camera(nullptr), ctx(&ctx) {
	// TODO: use this macro elsewhere too?

	// store port info and camera abilities as it seems that they never change.
	// FIXME free these? or init in the context already?
	static GPPortInfoList		*portinfolist = nullptr;
	static CameraAbilitiesList	*abilities = nullptr;

	auto cam = GP_NEW_UNIQUE(::Camera, gp_camera);
	camera = cam.get();

	if (!abilities) {
		/* Load all the camera drivers we have... */
		GP_OR_THROW(gp_abilities_list_new, &abilities);
		GP_OR_THROW(gp_abilities_list_load, abilities, ctx.context);
	}

	/* First lookup the model / driver */
	int model_idx;
    model_idx = GP_OR_THROW(gp_abilities_list_lookup_model, abilities, model);
	CameraAbilities my_abilities;
	GP_OR_THROW(gp_abilities_list_get_abilities, abilities, model_idx, &my_abilities);
	GP_OR_THROW(gp_camera_set_abilities, camera, my_abilities);

	if (!portinfolist) {
		/* Load all the port drivers we have... */
		GP_OR_THROW(gp_port_info_list_new, &portinfolist);
		GP_OR_THROW(gp_port_info_list_load, portinfolist);
	}

	/* Then associate the camera with the specified port */
	int path_idx;
	path_idx = GP_OR_THROW(gp_port_info_list_lookup_path, portinfolist, port);
	GPPortInfo myportinfo;
	GP_OR_THROW(gp_port_info_list_get_info, portinfolist, path_idx, &myportinfo);
	GP_OR_THROW(gp_camera_set_port_info, camera, myportinfo);

	cam.release(); // nothing can fail anymore here
	gp_context_ref(ctx.context);
}


Camera::Camera(Camera&& other) : camera(other.camera), ctx(other.ctx) {
	std::lock_guard<std::mutex> g(other.mutex);
	gp_context_ref(ctx->context);
	other.camera = nullptr;
}

Camera& Camera::operator=(Camera&& other) {
	if (this != &other) {
		std::lock_guard<std::mutex> g(mutex);
		std::lock_guard<std::mutex> go(other.mutex);
		std::swap(camera, other.camera);
		std::swap(ctx, other.ctx);
	}
	return *this;
}

Camera::~Camera() {
	std::lock_guard<std::mutex> g(mutex);
	gp_context_unref(ctx->context);
	
	if (camera)
		gp_camera_unref(camera); // XXX: freeing also exits it, i guess
}

std::mutex Camera::configmutex;

Widget Camera::config() {
	std::lock_guard<std::mutex> cg(configmutex);
	std::lock_guard<std::mutex> g(mutex);

	CameraWidget *w;
	int ret;
	if ((ret = gp_camera_get_config(camera, &w, ctx->context)) < GP_OK)
		throw Exception("gp_camera_get_config", ret);

	return Widget(w, *this);
}

const Widget Camera::config() const {
	std::lock_guard<std::mutex> cg(configmutex);
	std::lock_guard<std::mutex> g(mutex);

	CameraWidget *w;
	int ret;
	if ((ret = gp_camera_get_config(camera, &w, ctx->context)) < GP_OK)
		throw Exception("gp_camera_get_config", ret);

	// FIXME: dirty cast, make const Widget do magic
	return Widget(w, const_cast<gp::Camera&>(*this));
}

void Camera::set_config(CameraWidget* rootwindow) {
	std::lock_guard<std::mutex> g(mutex);

	int ret;
	if ((ret = gp_camera_set_config(camera, rootwindow, ctx->context)) < GP_OK)
		throw Exception("gp_camera_set_config", ret);
}

std::vector<char> Camera::preview() {
	std::lock_guard<std::mutex> g(mutex);

	auto file = GP_NEW_UNIQUE(CameraFile, gp_file);
	int ret;

	if ((ret = gp_camera_capture_preview(camera, file.get(), ctx->context)) < GP_OK)
		throw Exception("gp_camera_capture_preview", ret);

	const char *rawbuf;
	size_t nbytes;
	if ((ret = gp_file_get_data_and_size(file.get(), &rawbuf, &nbytes)) < GP_OK)
		throw Exception("gp_file_get_data_and_size", ret);

	std::vector<char> buf(nbytes);
	std::copy(rawbuf, rawbuf + nbytes, &buf[0]);

	return buf;
}

void Camera::save_preview(const std::string& fname) {
	auto pic = preview();
	std::ofstream fs(fname);
	std::copy(pic.begin(), pic.end(), std::ostreambuf_iterator<char>(fs));
}

CameraEvent Camera::wait_event(int timeout_msec) {
	CameraEventType eventtype;
	void* eventdata = nullptr;
	int ret = gp_camera_wait_for_event(camera, timeout_msec, &eventtype, &eventdata, ctx->context);
	if (ret < GP_OK)
		throw Exception("gp_camera_wait_for_event", ret);

	CameraEvent::EventType t;
	switch (eventtype) {
	case GP_EVENT_UNKNOWN:
		t = CameraEvent::EVENT_UNKNOWN; break;
	case GP_EVENT_TIMEOUT:
		t = CameraEvent::EVENT_TIMEOUT; break;
	case GP_EVENT_FILE_ADDED:
		t = CameraEvent::EVENT_FILE_ADDED; break;
	case GP_EVENT_FOLDER_ADDED:
		t = CameraEvent::EVENT_FOLDER_ADDED; break;
	case GP_EVENT_CAPTURE_COMPLETE:
		t = CameraEvent::EVENT_CAPTURE_COMPLETE; break;
	default:
		throw std::runtime_error("bad camera event type: " + std::to_string(eventtype));
	}

	return CameraEvent(t, eventdata);
}

void Camera::save_file(const std::string& folder, const std::string& name,
		const std::string& localfile, bool delete_from_cam) {
	int ret;

	// TODO smart handle
	int fd = open(localfile.c_str(), O_CREAT | O_WRONLY, 0644);
	if (fd == -1)
		throw std::runtime_error("open() failed: " + std::to_string(errno));

	CameraFile *filep;
	if ((ret = gp_file_new_from_fd(&filep, fd)) < GP_OK) {
		close(fd);
		throw Exception("gp_file_new_from_fd", ret);
	}
	std::unique_ptr<CameraFile, int (*)(CameraFile*)> file(filep, gp_file_unref);

	if ((ret = gp_camera_file_get(camera, folder.c_str(), name.c_str(),
			 GP_FILE_TYPE_NORMAL, filep, ctx->context)) < GP_OK) {
		close(fd);
		throw Exception("gp_camera_file_get", ret);
	}

	if (delete_from_cam) {
		if ((ret = gp_camera_file_delete(camera, folder.c_str(),
						name.c_str(), ctx->context)) < GP_OK) {
			close(fd);
			throw Exception("gp_camera_file_delete", ret);
		}
	}

	close(fd);
}

// WIDGET
// ======

Widget::Widget(CameraWidget* widget, Camera& camera) :
	widget(widget), root(nullptr), camera(&camera) {
	// this is the master window widget, already got one ref
	// don't gp_camera_ref, assume that it exists because we hold gp::Camera
}

Widget::Widget(CameraWidget* widget, const Widget& parent) :
	widget(widget), root(parent.root ? parent.root : parent.widget), camera(parent.camera) {
	// need a ref to be alive if the root gp::Widget dies
	gp_widget_ref(root);
}

Widget::Widget(Widget&& other) :
	widget(other.widget), root(other.root), camera(other.camera) {
	other.widget = nullptr;
	// other's dtor does nothing now
}

Widget::~Widget() {
	// move ctor nulls the ptrs; marks a dead Widget
	// gp's parent deletes its children without looking at the refcount, no
	// need to unref the children, just the root
	if (widget) {
		if (!root) // hello this is root
			gp_widget_unref(widget);
		else
			gp_widget_unref(root);
	}
}

Widget Widget::operator[](const std::string& name_or_label) {
	return Widget(get_gp_child(name_or_label.c_str()), *this);
}

const Widget Widget::operator[](const std::string& name_or_label) const {
	return Widget(get_gp_child(name_or_label.c_str()), *this);
}

Widget Widget::operator[](const char* name_or_label) {
	return Widget(get_gp_child(name_or_label), *this);
}

const Widget Widget::operator[](const char* name_or_label) const {
	return Widget(get_gp_child(name_or_label), *this);
}

CameraWidget* Widget::get_gp_child(const char *name_or_label) const {
	CameraWidget* child;
	int ret = gp_widget_get_child_by_name(widget, name_or_label, &child);

	if (ret < GP_OK)
		ret = gp_widget_get_child_by_label(widget, name_or_label, &child);

	if (ret < GP_OK)
		throw std::out_of_range("no widget found");
	
	return child;
}

Widget::WidgetType Widget::type() const {
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
	throw std::invalid_argument("Unknown type "
			+ std::to_string(static_cast<int>(type)));
}

// tell the camera to save the whole config window
// (todo: more performance by caching stuff?
// saving several in a row might be inefficient)
void Widget::set_changed() {
	camera->set_config(root ? root : widget);
}

// WIDGET DATA TRAITS
// ==================

std::string Widget::Traits<std::string>::read(const Widget& widget) {
	const char *val;
	gp_widget_get_value(widget.widget, &val);
	return val;
}

void Widget::Traits<std::string>::write(Widget& widget, const std::string& value) {
	gp_widget_set_value(widget.widget, value.c_str());
}

bool Widget::Traits<bool>::read(const Widget& widget) {
	int val;
	gp_widget_get_value(widget.widget, &val);
	return static_cast<bool>(val);
}

void Widget::Traits<bool>::write(Widget& widget, bool value) {
	int val = static_cast<int>(value);
	gp_widget_set_value(widget.widget, &val);
}

// CAMERAEVENT
// ===========

CameraEvent::CameraEvent(EventType type, void* data)
	: etype(type), data(data) {
}

CameraEvent::~CameraEvent() {
	free(data);
}

CameraEvent::EventType CameraEvent::type() const {
	return etype;
}

const char *CameraEvent::typestr() const {
	static const char *names[] = {
		"EVENT_UNKNOWN",
		"EVENT_TIMEOUT",
		"EVENT_FILE_ADDED",
		"EVENT_FOLDER_ADDED",
		"EVENT_CAPTURE_COMPLETE"
	};
	return etype >= 0 && etype <= 4 ? names[etype] : "unknown";
}

std::string CameraEvent::Traits<
		CameraEvent::EVENT_UNKNOWN>::get(const void* v) {
	return std::string(static_cast<const char*>(v));
}
std::pair<std::string, std::string> CameraEvent::Traits<
		CameraEvent::EVENT_FILE_ADDED>::get(const void* v) {
	const CameraFilePath* fp = static_cast<const CameraFilePath*>(v);
	return std::pair<std::string, std::string>(fp->folder, fp->name);
}

std::pair<std::string, std::string> CameraEvent::Traits<
		CameraEvent::EVENT_FOLDER_ADDED>::get(const void* v) {
	const CameraFilePath* fp = static_cast<const CameraFilePath*>(v);
	return std::pair<std::string, std::string>(fp->folder, fp->name);
}

}
