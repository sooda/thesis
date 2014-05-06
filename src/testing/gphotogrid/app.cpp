#include "app.h"
#include <iostream>

App::App(const std::string& input_cfg) : gpcontext(), cams(), previewfeed(cams) {
	reloadGphoto();
}

App::App() : App("") {
}

void App::reloadGphoto() {
	previewfeed.disable();
	cams.clear();
	cams = gpcontext.all_cameras();
	std::cout << "CAMS: " << cams.size() << std::endl;
	previewfeed = PreviewFeed(cams);
}
