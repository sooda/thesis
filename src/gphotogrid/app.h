#ifndef APP_H
#define APP_H

#include "gpwrap.h"
#include "previewfeed.h"
#include <string>
#include <vector>

class App {
public:
	App(const std::string& input_cfg);
	App();
	void reloadGphoto();
	size_t numcams() const;

	gp::Context gpcontext;
	std::vector<gp::Camera> cams;
	PreviewFeed previewfeed;

private:
	void sortCams();
};

#endif
