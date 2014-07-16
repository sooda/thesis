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
	std::vector<std::string> camNames;
	std::vector<std::string> camOrientations; // "0", "90", "-90", "180"
	PreviewFeed previewfeed;

private:
	void sortCams();
	void loadNames();
	void loadOrientations();
};

#endif
