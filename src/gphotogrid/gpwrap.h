#ifndef GPWRAP_H
#define GPWRAP_H

// http://libgphoto2.sourcearchive.com/documentation/2.4.10.1-2ubuntu5/gphoto2-widget_8h_ac2407563a7f8c22de8df8d5009f0e4e1.html1
// http://www.gphoto.org/doc/api/gphoto2-camera_8h.html
//
#include <stdexcept>
#include <string>
#include <vector>
#include <mutex>
#include <utility>

// raw C api, duplicated so that the C header can be omitted here
struct _Camera;
struct _GPContext;
struct _CameraWidget;

namespace gp {

// Note on pointers: the objects could store unique_ptrs to the C API with
// deleters specified as the unref/free functions; that would need at least the
// deleter type to be also specified in the template. TODO: investigate.

// Gphoto exception: something went wrong in the library, retval was != GP_OK
// Some places could use return values instead of these for perf, and preparing
// for random exceptions in every place where cameras/configs are constructed
// can be clumsy, but in normal use there shouldn't be any exceptions (unless a
// camera is suddendly disconnected, for example, but don't do that!)

// other operations that don't involve libgphoto might throw e.g.
// std::out_of_range

class Exception : public std::runtime_error {
public:
	Exception(const std::string& msg, int gpnum);
};

class Camera;
// libphoto2 context; there should be only one. Initialises GPContext and sets
// up error, message, status and log functions, if specified (default).  The
// functions are global and don't give any semantic information on a state;
// they're mainly used for debugging or random informational messages for the
// CLI user.
//
// Some camera-specific messages can be important to parse or to show, but no
// such things are yet done here.

class Context {
public:
	Context(bool use_messages=true);
	// Give just some camera - useful if you know that there is only one connected
	Camera auto_camera();
	// Give all found cameras with gp_camera_autodetect with no filterin on
	// names, ports, or anything
	std::vector<Camera> all_cameras();
	~Context();

	// camera uses the context pointer all over the place
	friend class Camera;
private:
	void init_messages();
	static void error_func(_GPContext *context, const char *msg, void *data);
	static void msg_func(_GPContext *context, const char *msg, void *data);
	static void status_func(_GPContext *context, const char *msg, void *data);
	static void log_func(int level, const char *domain, const char *str, void *data);

	_GPContext* context;

};


class Widget;
class CameraEvent;
// Camera is the most interesting thing here and constructs the other friends.
// Camera is thread-safe via a mutex. Please keep Camera alive if you plan to
// save its widgets afterwards.
class Camera {
public:
	// constructed by camera only and cannot be copied
	Camera(Camera&& other);
	~Camera();
	Camera(const Camera&) = delete;
	Camera& operator=(const Camera&) = delete;
	Camera& operator=(Camera&&);

	Widget config();
	const Widget config() const;

	// First preview might timeout e.g. with a DSLR that takes a long time to
	// flip the mirror up. At least some cameras don't need to be in an
	// explicit video mode for previewing to work.
	std::vector<char> preview();
	void save_preview(const std::string& fname);

	CameraEvent wait_event(int timeout_msec);

	// download a previously shot file and save it locally
	void save_file(const std::string& folder, const std::string& name,
			const std::string& localfile, bool delete_from_cam=false);

	friend class Context;
	// widgets are able to set configs
	friend class Widget;

private:
	Camera(Context& ctx);
	Camera(const char *model, const char *port, Context& ctx);

	void set_config(_CameraWidget* rootwindow);

	_Camera* camera;
	Context* ctx; // pointer and not a ref because move assignment

	// gphoto2 isn't thread safe; e.g. config while preview crashes
	mutable std::mutex mutex;
	// config() could go initially wrong when gphoto2 is loading libs
	static std::mutex configmutex;
};

// widgets are owned by the top-level widget (haven't found any docs on that
// except the source code itself) and when the top-level one dies, it frees all
// children recursively and doesn't look at their refcounts.
//
// maintain a pointer to the raw master root widget here and fiddle with its
// reference count when dealing with the children too, so we're safe if the
// root gp::Widget dies. the root is called a 'window' by libgphoto.
//
// (the c++ Widget root might get destroyed before its children; it's fine
// because gp refcounts.)
//
// The c++ camera must exist during the lifetime of Widgets if the widgets are
// changed, of course.

class Widget {
public:
	// redefine types to not pollute namespace with the C lib
	enum WidgetType {
		WIDGET_WINDOW,
		WIDGET_SECTION,
		WIDGET_TEXT,
		WIDGET_RANGE,
		WIDGET_TOGGLE,
		WIDGET_RADIO,
		WIDGET_MENU,
		WIDGET_DATE,
		WIDGET_BUTTON,
	};

	// Widgets can only be moved, and constructed by other Widgets or camera's
	// config()
	Widget(Widget&& other);
	~Widget();
	Widget(const Widget&) = delete;
	Widget& operator=(const Widget&) = delete;

	// private Camera::set_config is interface between cam and this. user
	// cannot touch set_config directly.
	friend class Camera;

	// get a child widget; recursive (by libgphoto); look first by name, then
	// by label. throw out_of_range if not found
	Widget operator[](const std::string& name_or_label);
	const Widget operator[](const std::string& name_or_label) const;
	Widget operator[](const char* name_or_label);
	const Widget operator[](const char* name_or_label) const;
	WidgetType type() const;

	// tools for get()
	template <class V>
	struct Traits {
		static const WidgetType type;
		static V read(const Widget& w);
		static void write(Widget& w, const V& value);
	};

	// read the value under this widget; plain strings are std::string, and
	// some known enumeration types (WIDGET_RADIO) can be handled easier with
	// gputil's helpers. the return value is just data and safe to store after
	// the widget has gone. throw invalid_argument when type does not match -
	// you should only get something whose type you are sure of (by common name
	// for the widget, or by checking its type() explicitly)
	template <class V>
	V get() const {
		if (type() != Traits<V>::type)
			throw std::invalid_argument("widget type mismatch");
		return Traits<V>::read(*this);
	}

	// with a value read with the above get(), write a value back. throw
	// invalid_argument when type does not match
	template <class V>
	void set(const V& val) {
		if (type() != Traits<V>::type)
			throw std::invalid_argument("widget type mismatch");
		Traits<V>::write(*this, val);
		set_changed();
	}

private:
	// propagate changes to the camera
	void set_changed();

	// root widget construction
	Widget(_CameraWidget* widget, Camera& cam);
	// children
	Widget(_CameraWidget* widget, const Widget& parent);

	// just copypasta reduction in cfg getters
	_CameraWidget* get_gp_child(const char *name_or_label) const;

	_CameraWidget* widget;
	_CameraWidget* root;
	Camera* camera;
};

// very common and simple widgets: text (string) and toggle (bool)

template <>
struct Widget::Traits<std::string> {
	static const WidgetType type = WIDGET_TEXT;
	static std::string read(const Widget& w);
	static void write(Widget& w, const std::string& value);
};
template <>
struct Widget::Traits<bool> {
	static const WidgetType type = WIDGET_TOGGLE;
	static bool read(const Widget& w);
	static void write(Widget& w, bool value);
};

// events hold a raw pointer to a malloc'd object, and should be freed by the
// user. as with Widget, get()'s template arg should match the EventType.
//
// TODO: I should fork the gphoto code and write something ptp-specific or
// camera-specific on the events; I'd like to know when a widget changes, and a
// camera tells you that, but it's of unknown type. Handling the ptp
// identifiers would be cool.
class CameraEvent {
public:
	enum EventType {
		EVENT_UNKNOWN,         // std::string
		EVENT_TIMEOUT,         // just informative, no data here
		EVENT_FILE_ADDED,      // std::pair, holds folder and name
		EVENT_FOLDER_ADDED,    // std::pair, holds folder and name
		EVENT_CAPTURE_COMPLETE // just informative, no data
	};

	friend class Camera;

	// can be moved only because of internal pointer; constructed only via
	// Camera.
	CameraEvent(CameraEvent&&);
	~CameraEvent();
	CameraEvent(const CameraEvent&) = delete;
	CameraEvent& operator=(const CameraEvent&) = delete;

	EventType type() const;
	// same as EventType but as strings, for debug prints
	const char* typestr() const;

	// hmm wat, no contents needed here as long as implemented specialisations have them?
	template <EventType E> class Traits {
		// typedef T type;
		// static T get(void*);
	};

	// get something by type(). return value is just data behind the internal
	// pointer, safe to keep alive without this event
	template <EventType E>
	typename Traits<E>::type get() const {
		if (etype != E)
			throw std::invalid_argument("cameraevent type mismatch");
		return Traits<E>::get(data);
	}

private:
	// called by Camera
	CameraEvent(EventType type, void* data);

	EventType etype;
	void* data;
};

// only UNKNOWN, FILE_ADDED and FOLDER_ADDED contain data
template <>
struct CameraEvent::Traits<CameraEvent::EVENT_UNKNOWN> {
	typedef std::string type;
	static type get(const void* v);
};
template <>
struct CameraEvent::Traits<CameraEvent::EVENT_FILE_ADDED> {
	typedef std::pair<std::string, std::string> type;
	static type get(const void* v);
};
template <>
struct CameraEvent::Traits<CameraEvent::EVENT_FOLDER_ADDED> {
	typedef std::pair<std::string, std::string> type;
	static type get(const void* v);
};

}

#endif
