#include "gpwrap.h"
#include "gputil.h"

#include <vector>
#include <iostream>

#include <signal.h>
#include <thread>
#include <mutex>
#include <condition_variable>

#include <sys/stat.h>

void dnload(gp::Camera& cam, const std::string& name,
		const std::string& folder, const std::string& file) {
}

void download_task(gp::Camera& cam, bool& running) {
	std::string name(cam.config()["artist"].get<std::string>());
	int evts = 0;

	std::string localdir = "files/" + name;
	mkdir(localdir.c_str(), 0777);

	bool verbose = false;

	std::cout << name << " running..." << std::endl;
	while (running) {
		gp::CameraEvent ev = cam.wait_event(200);

		if (ev.type() != gp::CameraEvent::EVENT_TIMEOUT) {
			if (ev.type() != gp::CameraEvent::EVENT_UNKNOWN || verbose)
				std::cout << name << "#" << evts << ": " << ev.type() << ": " << ev.typestr();
		}

		using ce = gp::CameraEvent;
		switch (ev.type()) {
		case ce::EVENT_UNKNOWN:
			if (verbose)
				std::cout << ": " << ev.get<ce::EVENT_UNKNOWN>() << std::endl;
			break;
		case ce::EVENT_TIMEOUT:
			break;
		case ce::EVENT_FILE_ADDED: {
			auto pathinfo = ev.get<ce::EVENT_FILE_ADDED>();
			//std::cout << ": " << pathinfo.first << " "
			//		<< pathinfo.second << std::endl;
			cam.save_file(pathinfo.first, pathinfo.second, localdir + "/" + pathinfo.second);
			std::cout << name << " downloaded" << std::endl;
			}
			break;
		case ce::EVENT_FOLDER_ADDED: {
			auto pathinfo = ev.get<ce::EVENT_FOLDER_ADDED>();
			std::cout << ": " << pathinfo.first << " "
					<< pathinfo.second << std::endl;
			}
			break;
		case ce::EVENT_CAPTURE_COMPLETE:
			break;
		}

		evts++;
	}
	std::cout << name << " stopping after " << evts << " events" << std::endl;
}

void trap_ctrlc(void (*handfunc)(int)) {
	struct sigaction sigIntHandler;

	sigIntHandler.sa_handler = handfunc;
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;

	sigaction(SIGINT, &sigIntHandler, NULL);
}

struct {
	std::mutex mutex;
	std::condition_variable cv;
	bool stop;
} stopsignal;

void setquit(int) {
	stopsignal.stop = true;
	stopsignal.cv.notify_one();
}

void do_download(std::vector<gp::Camera>& cams) {
	std::cout << "Found " << cams.size() << " camera"
		<< (cams.size() > 1 ? "s" : "") << "." << std::endl;

	std::vector<std::thread> threads;
	bool running = true;
	for (auto& cam: cams) {
		threads.emplace_back(download_task, std::ref(cam), std::ref(running));
	}

	trap_ctrlc(setquit);
	std::unique_lock<std::mutex> guard(stopsignal.mutex);
	stopsignal.cv.wait(guard, []{ return stopsignal.stop; });
	running = false;
	for (auto& t: threads)
		t.join();
}

int main(int argc, char *argv[]) {
	gp::Context gpcontext;
	std::vector<gp::Camera> cams(gpcontext.all_cameras());
	if (!cams.empty())
		do_download(cams);
	else
		std::cout << "No cameras." << std::endl;
}
