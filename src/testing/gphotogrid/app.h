#ifndef APP_H
#define APP_H

#include "gpwrap.h"
#include <string>
#include <vector>

class App {
public:
	App(const std::string& input_cfg);
	App();

	gp::Context gpcontext;
	std::vector<gp::Camera> cams;
	void reloadGphoto();
};

#endif
