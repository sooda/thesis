// Paraphotos - parallel photo downloader
// Just for testing how this works instead of a shell script with
// a number of gphoto2 --capture-tethered processes. Seems to work fine.
//
// Launch a downloader thread for each camera, and notify the user with
// an "all done" message when everyone has downloaded its next image.
// Ctrl+c (SIGINT) stops all this.

#include "gpwrap.h"
#include "gputil.h"
#include "semaphore.h"
#include "signalhandler.h"

#include <vector>
#include <iostream>

#include <thread>
#include <mutex>
#include <condition_variable>

#include <sys/stat.h>
#include <unistd.h>

// Note: the
// "Downloading 'IMG_NNNN.JPG' from folder '/store_XXXXXXXX/DCIM/100CANON'..."
// message comes only if the camera has been set to write images to memory card
// instead of its internal buffers. Internal buffers have folder "/"

enum Verbosity {
	VERBOSE_SILENT,    // only print "all ready" messages and other main level info
	VERBOSE_NORMAL,    // print "camera <name> downloaded" messages and gphoto2's own msgs
	VERBOSE_TALKATIVE, // print libgphoto events for all but unknown events
	VERBOSE_HIGH       // print also unknown libgphoto events
};

struct Notifylocks {
	Semaphore readycount;
	Semaphore usernotified;
	int sequenceid;
};

struct Config {
	Verbosity verboselevel;
	std::string filedir;
	bool delete_on_download;
	bool name_seqid; // name by sequence id instead of camera-original name
};

// rename file, preserving its suffix
std::string rename_to_number(std::string filename, int replace_num) {
	size_t suffix_start = filename.find(".");
	// no suffix? replace whole string
	if (suffix_start == std::string::npos)
		return std::to_string(replace_num);
	filename.replace(0, suffix_start, std::to_string(replace_num));
	return filename;
}

// Downloader task for one camera: listen to events, print, and download files
void download_task(gp::Camera& cam, Notifylocks& locks, Config cfg, bool& running) {
	std::string name(cam.config()["artist"].get<std::string>());

	std::string localdir = cfg.filedir + "/" + name;
	mkdir(localdir.c_str(), 0777);
	int evts = 0;

	if (cfg.verboselevel >= VERBOSE_NORMAL)
		std::cout << name << " running..." << std::endl;
	locks.readycount.notify_one();
	locks.usernotified.wait();

	while (running) {
		gp::CameraEvent ev = cam.wait_event(10);

		using Ce = gp::CameraEvent;

		if (ev.type() != Ce::EVENT_TIMEOUT && cfg.verboselevel >= VERBOSE_TALKATIVE) {
			if (ev.type() != Ce::EVENT_UNKNOWN || cfg.verboselevel == VERBOSE_HIGH)
				std::cout << "Cam " << name << "#" << evts << ": "
					<< ev.type() << ": " << ev.typestr();
		}

		switch (ev.type()) {
		case Ce::EVENT_UNKNOWN:
			if (cfg.verboselevel == VERBOSE_HIGH)
				std::cout << ": " << ev.get<Ce::EVENT_UNKNOWN>() << std::endl;
			break;
		case Ce::EVENT_TIMEOUT:
			// no event, no problem
			break;
		case Ce::EVENT_FILE_ADDED: {
			auto pathinfo = ev.get<Ce::EVENT_FILE_ADDED>();
			if (cfg.verboselevel >= VERBOSE_TALKATIVE) {
				std::cout << ": " << pathinfo.first << " "
						<< pathinfo.second << std::endl;
			}
			// camera folder and file name separately
			std::string filename = cfg.name_seqid
				? rename_to_number(pathinfo.second, locks.sequenceid)
				: pathinfo.second;
			std::string localdest(localdir + "/" + filename);
			cam.save_file(pathinfo.first, pathinfo.second, localdest, cfg.delete_on_download);
			if (cfg.verboselevel >= VERBOSE_NORMAL)
				std::cout << name << " downloaded " << pathinfo.second
					<< " -> " << localdest << "..." << std::endl;

			locks.readycount.notify_one();
			// TODO: download queue and try_wait, not to clog gphoto event
			// queue? probably all cameras have pretty same speed, so it's
			// just unnecessarily complicated
			locks.usernotified.wait();
			}
			break;
		case Ce::EVENT_FOLDER_ADDED: {
			auto pathinfo = ev.get<Ce::EVENT_FOLDER_ADDED>();
			if (cfg.verboselevel >= VERBOSE_TALKATIVE) {
				std::cout << ": " << pathinfo.first << " "
						<< pathinfo.second << std::endl;
			}
			}
			break;
		case Ce::EVENT_CAPTURE_COMPLETE:
			// wat
			if (cfg.verboselevel >= VERBOSE_TALKATIVE) {
				std::cout << "(?)" << std::endl;
			}
			break;
		}

		evts++;
	}
	if (cfg.verboselevel >= VERBOSE_NORMAL)
		std::cout << "Cam " << name << " stopping after " << evts << " events..." << std::endl;
	// wake up the allready notifier for the last time
	locks.readycount.notify_one();
}

// Auxiliary handler for telling the user when the sequence is finished
// Especially useful in waiting for the last download to finish so that
// the program can be exited
void allready_notifier(Notifylocks& locks, int n, bool& running) {
	while (running) {
		locks.readycount.wait(n);
		std::cout << "All ready (#" << locks.sequenceid << ")" << std::endl;
		locks.sequenceid++;
		locks.usernotified.notify_all(n);
	}
}

// Main thread wakes up with this
// stop initially false because global
struct {
	std::condition_variable cv;
	bool stop;
} stopsignal;

void setquit(int) {
	stopsignal.stop = true;
	stopsignal.cv.notify_one();
}

// Actual work; start download threads and wait for exit
void do_download(std::vector<gp::Camera>& cams, Config config) {
	std::cout << "Found " << cams.size() << " camera"
		<< (cams.size() > 1 ? "s" : "") << ", starting to save under "
		<< config.filedir << "/" << std::endl;

	bool running = true;
	Notifylocks locks{};

	std::thread notifier(allready_notifier, std::ref(locks), cams.size(), std::ref(running));

	std::vector<std::thread> threads;
	for (auto& cam: cams)
		threads.emplace_back(download_task, std::ref(cam), std::ref(locks),
				config, std::ref(running));

	trap_ctrlc(setquit);

	std::mutex mutex;
	std::unique_lock<std::mutex> guard(mutex);
	stopsignal.cv.wait(guard, []{ return stopsignal.stop; });

	running = false;
	for (auto& t: threads)
		t.join();
	notifier.join();
}
bool preparedir(std::string dir) {
	if (mkdir(dir.c_str(), 0777) != 0 && errno != EEXIST) {
		std::cout << "Cannot create picture folder" << std::endl;
		perror("mkdir");
		return false;
	}
	std::string test = dir + "/testdir";
	if (mkdir(test.c_str(), 0777) != 0) {
		std::cout << "Cannot create camera folders" << std::endl;
		perror("mkdir");
		return false;
	}
	rmdir(test.c_str());
	return true;
}
int app(Config config) {
	if (!preparedir(config.filedir))
		return 4;
	gp::Context gpcontext(config.verboselevel >= VERBOSE_TALKATIVE);
	std::vector<gp::Camera> cams(gpcontext.all_cameras());
	if (!cams.empty())
		do_download(cams, config);
	else
		std::cout << "No cameras." << std::endl;
	return 0;
}

void usage(const char* program) {
	std::cout
		<< "Usage: " << program << " [-h] [-v LEVEL] FOLDER" << std::endl
		<< "    -h: print this help and quit" << std::endl
		<< std::endl
		<< "    -v LEVEL: verbosity level" << std::endl
		<< "              0=silent (default), only bulk messages" << std::endl
		<< "              1=normal, also per-camera messages" << std::endl
		<< "              2=talkative, also libgphoto events" << std::endl
		<< "              3=high, also unknown events" << std::endl
		<< std::endl
		<< "    -d: delete file on camera after download (default no)" << std::endl
		<< std::endl
		<< "    -n: rename images by sequence number instead of camera originals" << std::endl
		<< std::endl
		<< "    -s: shot ordering mode. by default, pics go to" << std::endl
		<< "        FOLDER/<camname>/<filename>" << std::endl
		<< "        but with this, FOLDER/<sequence-id>/<camname> is used" << std::endl
		<< "        (implies -n)" << std::endl
		<< std::endl
		<< "    FOLDER: download pics under this folder (must exist)" << std::endl
		<< std::endl
		<< "Export GPWRAP_LOG_DEBUG=1 to print debug messages in verb level >=2."
		<< std::endl
	;
}

int main(int argc, char *argv[]) {
	Config config;
	config.verboselevel = VERBOSE_SILENT;
	config.delete_on_download = false;
	std::string h("-h"), v("-v"), d("-d"), n("-n");
	for (int i = 1; i < argc; i++) {
		if (argv[i] == h) {
			usage(argv[0]);
			return 0;
		} else if (argv[i] == v) {
			if (i == argc-1) {
				std::cout << "error: -v requires an argument" << std::endl;
				return 1;
			}
			int level;
			try {
				level = std::stoi(argv[i + 1]);
			} catch (...) {
				level = -1;
			}
			if (level < 0 || level > (int)VERBOSE_HIGH) {
				std::cout << "error: bad number for -v" << std::endl;
				return 1;
			}
			config.verboselevel = (Verbosity)level;
			i++;
		} else if (argv[i] == d) {
			config.delete_on_download = true;
		} else if (argv[i] == n) {
			config.name_seqid = true;
		} else {
			config.filedir = argv[i];
		}
	}
	if (config.filedir == "") {
		std::cout << "error: no folder given" << std::endl;
		return 3;
	}
	return app(config);
}
