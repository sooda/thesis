#ifndef PREVIEWFEED_H
#define PREVIEWFEED_H

#include "gpwrap.h"
#include <chrono>
#include <thread>
#include <utility>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <queue>

template <class T>
class ConcurrentQueue {
private:
	typedef std::lock_guard<std::mutex> Guard;

public:
	// FIXME why = delete; not working
	ConcurrentQueue() {}
	// this only in ctoring to vectors here...
	ConcurrentQueue(ConcurrentQueue&& other) : data(std::move(other.data)) {}
	void push(const T& val) {
		Guard lock(mutex);
		data.push(val);
	}

	bool try_pop(T& val) {
		Guard lock(mutex);
		if (data.empty())
			return false;
		val = data.front();
		data.pop();
		return true;
	}

	void clear() {
		Guard lock(mutex);
		data = std::queue<T>();
	}

private:
	std::queue<T> data;
	std::mutex mutex;

	ConcurrentQueue(const ConcurrentQueue&) = delete;
	ConcurrentQueue& operator=(const ConcurrentQueue&) = delete;
};

class Semaphore {
public:
	Semaphore(int count = 0) : count(count) {}
	Semaphore(const Semaphore& other) : count(other.count) {}
	Semaphore& operator=(const Semaphore& other);
	void notify();
	void wait(int wantcount);

private:
	std::mutex mutex;
	std::condition_variable cv;
	int count;
};

class PreviewFeed {
public:
	typedef std::chrono::high_resolution_clock Clock;
	typedef std::vector<char> JpegBuffer;
	typedef std::pair<JpegBuffer, Clock::time_point> TimedJpegBuffer;
	typedef ConcurrentQueue<TimedJpegBuffer> ImageQueue;

	PreviewFeed(std::vector<gp::Camera>& cameras);
	PreviewFeed(PreviewFeed&& other);
	PreviewFeed& operator=(PreviewFeed&& other);
	~PreviewFeed();
	void enable();
	void disable();
	bool enabled();
	void setsync(bool sync);
	ImageQueue& getQueue(size_t i);

private:
	void start_threads();
	void finish_threads();

	void previewThread(int index);
	void syncerThread();

	int size;

	// indexed by camera
	std::vector<gp::Camera>* cameras;
	std::vector<ImageQueue> previewQueues;
	std::vector<std::thread> previewLoaders;
	std::thread syncer;

	std::atomic_bool threads_running;
	std::atomic_bool threads_synced;

	std::atomic_int round; // for syncing the threads together with the watchdog thread
	std::condition_variable nextRound;
	std::mutex nextRoundMutex;
	Semaphore numDone;
};

#endif

