#include "gpwrap.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

std::string to_str_pad(int i, int pad) {
	std::stringstream ss;
	ss << std::setw(pad) << std::setfill('0') << i;
	return ss.str();
}

int main(int argc, char **) {
	gp::Context context;
	gp::Camera cam(context.auto_camera());
	for (int i=0;i<argc;i++){
		std::cout << (int)cam.config()["capture"].get<bool>() << std::endl;
		std::cout << cam.config()["lensname"].get<std::string>() << std::endl;
		std::cout << cam.config()["main"]["lensname"].get<std::string>() << std::endl;
		std::cout << cam.config()["capture"].get<bool>() << std::endl;
		// why it doesn't get set?
		// well, takes pics anyway
		cam.config()["capture"].set<bool>(true);
		std::cout << cam.config()["capture"].get<bool>() << std::endl;
	}


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
