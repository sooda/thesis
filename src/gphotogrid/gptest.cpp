#include "gpwrap.h"
#include "gputil.h"
#include "signalhandler.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <iterator>
#include <cstdlib>

std::string to_str_pad(int i, int pad) {
	std::stringstream ss;
	ss << std::setw(pad) << std::setfill('0') << i;
	return ss.str();
}

void test_single_camera(gp::Context& context, int n) {
	gp::Camera cam(context.auto_camera());
	for (int i = 0; i < n; i++){
		std::cout << cam.config()["serialnumber"].get<std::string>() << std::endl;
		std::cout << cam.config()["cameramodel"].get<std::string>() << std::endl;
		std::cout << cam.config()["main"]["cameramodel"].get<std::string>() << std::endl;

		std::cout << cam.config()["capture"].get<bool>() << std::endl;
		// why it doesn't get set?
		// well, takes pics anyway
		cam.config()["capture"].set(true);
		std::cout << cam.config()["capture"].get<bool>() << std::endl;
		{
			// test root widget dtoring before children
			auto cap = [&]{ return cam.config()["capture"]; }();
			std::cout << cap.get<bool>() << std::endl;
			cap.set(cap.get<bool>());
		}

		gp::Aperture ap = cam.config()["aperture"].get<gp::Aperture>();
		std::cout << ap.index() << ":" << ap.text() << std::endl;
		for (auto& x: ap.choices())
			std::cout << x << " ";
		std::cout << std::endl;

		ap.set((ap.index() + 1) % ap.size());
		cam.config()["aperture"].set(ap);
		auto cfg = cam.config();

		gp::Aperture ap2 = cfg[gp::Aperture::gpname].get<gp::Aperture>();
		std::cout << ap2.index() << ":" << ap2.text() << std::endl;

		auto ss = cfg[gp::ShutterSpeed::gpname].get<gp::ShutterSpeed>();
		std::cout << ss.index() << ":" << ss.text() << std::endl;
		for (auto& x: ss.choices())
			std::cout << x << " ";
		std::cout << std::endl;

		auto iso = cfg[gp::Iso::gpname].get<gp::Iso>();
		std::cout << iso.index() << ":" << iso.text() << std::endl;
		for (auto& x: iso.choices())
			std::cout << x << " ";
		std::cout << std::endl;
	}

	if (n > 1) {
		// first one does nothing, others do after the mirror has flipped
		// TODO order it to lock up and wait for events?
		for (int i = 0; i < 50; i++) {
			std::vector<char> pic;
			try {
				pic = cam.preview();
			} catch (gp::Exception& ex) {
				std::cerr << ex.what() << std::endl;
			}
			if (!pic.empty()) {
				std::cout << "ok " << i << std::endl;
				std::ofstream fs("preview" + to_str_pad(i, 2) + ".jpg");
				std::copy(pic.begin(), pic.end(), std::ostreambuf_iterator<char>(fs));
			}
		}
	}
}
void test_many_cameras(gp::Context& context) {
	auto cams = context.all_cameras();
	int i = 0;
	for (auto& cam: cams) {
		std::cout << "lel " << cam.config()["cameramodel"].get<std::string>() << std::endl;
		try {
			cam.save_preview("preview-cam" + std::to_string(i) + "-a.jpg");
		} catch (gp::Exception& e) {
			std::cout << e.what() << std::endl;
		}
		try {
			cam.save_preview("preview-cam" + std::to_string(i) + "-a.jpg");
		} catch (gp::Exception& e) {
			std::cout << e.what() << std::endl;
		}
		i++;
	}
}

volatile bool running;
void setquit(int) {
	running = false;
}

void test_events(gp::Context& context) {
	gp::Camera cam(context.auto_camera());
	running = true;
	trap_ctrlc(setquit);
	int evts = 0;
	while (running) {
		gp::CameraEvent ev = cam.wait_event(200);

		if (ev.type() != gp::CameraEvent::EVENT_TIMEOUT)
			std::cout << "#" << evts << ": " << ev.type() << ": " << ev.typestr();

		using ce = gp::CameraEvent;
		switch (ev.type()) {
		case ce::EVENT_UNKNOWN:
			std::cout << ": " << ev.get<ce::EVENT_UNKNOWN>() << std::endl;
			break;
		case ce::EVENT_TIMEOUT:
			break;
		case ce::EVENT_FILE_ADDED:
			std::cout << ": " << ev.get<ce::EVENT_FILE_ADDED>().first << " "
					<< ev.get<ce::EVENT_FILE_ADDED>().second << std::endl;
			break;
		case ce::EVENT_FOLDER_ADDED:
			std::cout << ": " << ev.get<ce::EVENT_FOLDER_ADDED>().first << " "
					<< ev.get<ce::EVENT_FOLDER_ADDED>().second << std::endl;
			break;
		case ce::EVENT_CAPTURE_COMPLETE:
			break;
		}

		evts++;
	}
	std::cout << "stopping after " << evts << " events" << std::endl;
}

int main(int argc, char *argv[]) {
	gp::Context context;

	if (argc > 1) {
		std::string arg(argv[1]);

		if (arg == "single" && argc == 3)
			test_single_camera(context, atoi(argv[2]));
		else if (arg == "many")
			test_many_cameras(context);
		else if (arg == "events")
			test_events(context);
		else if (arg == "help")
			std::cout << "single N, many, events" << std::endl;
	}
}
