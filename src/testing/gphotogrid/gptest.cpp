#include "gpwrap.h"
#include "gputil.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <iterator>

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

		gp::Aperture ap = cam.config()["aperture"].get<gp::Aperture>();
		std::cout << ap.index() << ":" << ap.text() << std::endl;
		for (auto& x: ap.choices())
			std::cout << x << " ";
		std::cout << std::endl;

		ap.set((ap.index() + 1) % ap.size());
		cam.config()["aperture"].set(ap);
		gp::Aperture ap2 = cam.config()["aperture"].get<gp::Aperture>();
		std::cout << ap2.index() << ":" << ap2.text() << std::endl;
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

int main(int argc, char **) {
	gp::Context context;
	test_single_camera(context, argc);
	test_many_cameras(context);
}
