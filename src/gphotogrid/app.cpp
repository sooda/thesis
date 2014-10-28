#include "app.h"
#include <iostream>
#include <fstream>
#include <map>
#include <algorithm>

App::App(const std::string& /*input_cfg*/) : gpcontext(), cams(), previewfeed(cams) {
	reloadGphoto();
}

App::App() : App("") {
}

void App::sortCams() {
	std::ifstream ifs("camera_order.txt");
	if (!ifs.is_open()) {
		std::cout << "not sorting cameras, no camera_order.txt given" << std::endl;
		return;
	}

	std::vector<std::string> order;
	std::string camname;
	for (int i = 0; ifs >> camname; i++)
		order.push_back(camname);

	auto nth = [](const std::vector<std::string>& v, const std::string& x) {
		return std::find(v.begin(), v.end(), x) - v.begin();
	};
	std::sort(cams.begin(), cams.end(),
		[&](const gp::Camera& a, const gp::Camera& b) {
			return nth(order, a.config()["artist"].get<std::string>())
				< nth(order, b.config()["artist"].get<std::string>());
		}
	);
}

void App::reloadGphoto() {
	previewfeed.disable();
	cams.clear();

	cams = gpcontext.all_cameras();
	std::cout << "CAMS: " << cams.size() << std::endl;
	sortCams();
	loadNames();
	loadOrientations();
	previewfeed = PreviewFeed(cams);
}

void App::loadNames() {
	camNames.clear();
	for (auto& c: cams)
		camNames.push_back(c.config()["artist"].get<std::string>());
}

void App::loadOrientations() {
	std::ifstream ifs("camera_orientation.txt");
	if (!ifs.is_open()) {
		std::cout << "All cameras oriented normally, no camera_order.txt given" << std::endl;
		return;
	}
	camOrientations.clear();
	std::string orientation;
	for (int i = 0; ifs >> orientation; i++)
		camOrientations.push_back(orientation);

}

size_t App::numcams() const {
	return cams.size();
}
