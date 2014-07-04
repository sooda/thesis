#ifndef GPWRAP_H
#define GPWRAP_H

/* constant leak from:
==30236==    definitely lost: 288 bytes in 3 blocks
==30236==    indirectly lost: 38,964 bytes in 99 blocks
==9534==    at 0x4C2A820: calloc (in /usr/lib/valgrind/vgpreload_memcheck-amd64-linux.so)
==9534==    by 0x811DEA9: ???
==9534==    by 0x6AF42F5: ???
==9534==    by 0x5C79714: ??? (in /usr/lib/libgphoto2_port.so.10.1.1)
==9534==    by 0x60A1DBA: ??? (in /usr/lib/libltdl.so.7.3.0)
==9534==    by 0x60A1857: ??? (in /usr/lib/libltdl.so.7.3.0)
==9534==    by 0x60A2338: lt_dlforeachfile (in /usr/lib/libltdl.so.7.3.0)
==9534==    by 0x5C79B38: gp_port_info_list_load (in /usr/lib/libgphoto2_port.so.10.1.1)
==9534==    by 0x4E3D6A1: gp_camera_init (in /usr/lib/libgphoto2.so.6.0.0)
*/

// TODO noncopyables etc

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

// for unknown errors when ret != GP_OK
// others might throw e.g. std::out_of_range
class Exception : public std::runtime_error {
public:
	Exception(const std::string& msg, int gpnum);
};

class Camera;

class Context {
public:
	Context(bool use_messages=true);
	Camera auto_camera();
	std::vector<Camera> all_cameras();
	~Context();

	friend class Camera;
private:
	void init_messages();
	static void error_func(_GPContext *context, const char *msg, void *data);
	static void msg_func(_GPContext *context, const char *msg, void *data);
	static void status_func(_GPContext *context, const char *msg, void *data);
	static void log_func(int level, const char *domain, const char *str, void *data);

	_GPContext* context;

};

class Widget {
public:
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

	template <class V>
	struct Traits {
		static const WidgetType type;
		static V read(Widget& w);
		static void write(Widget& w, const V& value);
	};

	virtual ~Widget();

	friend class Camera;

	Widget operator[](const char* name_or_label);
	WidgetType type();
	//Widget* as_typed();

	/* invalid_argument when type does not match */
	template <class V>
	V get() {
		if (type() != Traits<V>::type)
			throw std::invalid_argument("widget type mismatch");
		return Traits<V>::read(*this);
	}

	/* invalid_argument when type does not match */
	template <class V>
	void set(const V& val) {
		if (type() != Traits<V>::type)
			throw std::invalid_argument("widget type mismatch");
		Traits<V>::write(*this, val);
		set_changed();
	}
	Widget(Widget&& other);

protected:
	_CameraWidget* widget;
private:
	void set_changed();
	Widget(_CameraWidget* widget, Camera& cam);
	Widget(_CameraWidget* widget, Widget& parent);
	Widget* parent;
	Camera* camera;
};

template <>
struct Widget::Traits<std::string> {
	static const WidgetType type = WIDGET_TEXT;
	static std::string read(Widget& w);
	static void write(Widget& w, const std::string& value);
};

template <>
struct Widget::Traits<bool> {
	static const WidgetType type = WIDGET_TOGGLE;
	static bool read(Widget& w);
	static void write(Widget& w, bool value);
};

class CameraEvent {
public:
	enum EventType {
		EVENT_UNKNOWN,
		EVENT_TIMEOUT,
		EVENT_FILE_ADDED,
		EVENT_FOLDER_ADDED,
		EVENT_CAPTURE_COMPLETE
	};
	CameraEvent(EventType type, void* data);
	CameraEvent(CameraEvent&&) = default;
	~CameraEvent();
	EventType type() const;
	const char *typestr() const;

	template <EventType t> class Traits {};

	template <EventType t>
	typename Traits<t>::type get() {
		if (etype != t)
			throw std::invalid_argument("cameraevent type mismatch");
		return Traits<t>::get(data);
	}
private:
	CameraEvent(const CameraEvent&) = delete;
	CameraEvent& operator=(const CameraEvent&) = delete;
	EventType etype;
	void* data;
};

template <>
struct CameraEvent::Traits<CameraEvent::EVENT_UNKNOWN> {
	typedef std::string type;
	static type get(void* v);
};
template <>
struct CameraEvent::Traits<CameraEvent::EVENT_FILE_ADDED> {
	typedef std::pair<std::string, std::string> type;
	static type get(void* v);
};
template <>
struct CameraEvent::Traits<CameraEvent::EVENT_FOLDER_ADDED> {
	typedef std::pair<std::string, std::string> type;
	static type get(void* v);
};

// XXX TODO FIXME store config here, set_config() with no params
// another RootWidget
class Camera {
public:
	Camera(Camera&& other);
	~Camera();

	Widget config();
	std::vector<char> preview();
	void save_preview(const std::string& fname);
	CameraEvent wait_event(int timeout);
	void save_file(const std::string& folder, const std::string& name,
			const std::string& localfile, bool delete_from_cam=false);

	// ctx constructs me
	friend class Context;
	// widgets are able to set configs
	friend class Widget;

private:
	void set_config(const Widget& cfg);
	Camera(_Camera* camera, Context& ctx);
	Camera(const char *model, const char *port, Context& ctx);

	_Camera* camera;
	Context& ctx;

	// gphoto2 isn't thread safe; e.g. config while preview crashes
	std::mutex mutex;
	// get_config can go initially wrong when gphoto2 is loading libs
	static std::mutex configmutex;
};

}

#endif
