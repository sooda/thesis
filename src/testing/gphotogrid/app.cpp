#include "app.h"
#include <iostream>

App::App(const std::string& input_cfg) {
	reloadGphoto();
}

App::App() : App("") {
}

void App::reloadGphoto() {
	cams.clear();
	cams = gpcontext.all_cameras();
	std::cout << "CAMS: " << cams.size() << std::endl;
}
