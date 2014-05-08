#include "previewfeed.h"
#include <algorithm>
#include <functional>
#include <iostream>
#include <cassert>
#include <iostream>

void Semaphore::notify() {
	// mutex protects also count, not just cv signaling
	std::unique_lock<std::mutex> lk(mutex);
	count++;
	cv.notify_all();
}

void Semaphore::wait(int wantcount) {
	std::unique_lock<std::mutex> lk(mutex);
	cv.wait(lk, [&]() { return count >= wantcount; });
	count -= wantcount;
}

Semaphore& Semaphore::operator=(const Semaphore& other) {
	std::lock_guard<std::mutex> g(mutex);
	count = other.count;
	cv.notify_all(); // i don't need this, but just for completeness
	return *this;
}

PreviewFeed::PreviewFeed(std::vector<gp::Camera>& cameras) :
		size(cameras.size()), cameras(&cameras),
		previewQueues(size), previewLoaders(size), syncer(),
		threads_running(false), threads_synced(false),
		round(0) {
}

PreviewFeed::PreviewFeed(PreviewFeed&& other) :
		size(other.cameras->size()), cameras(other.cameras),
		previewQueues(size), previewLoaders(size) {
	other.disable();
}

PreviewFeed& PreviewFeed::operator=(PreviewFeed&& other) {
	assert(!other.enabled());
	disable();

	size = other.size;
	cameras = other.cameras;
	previewQueues.resize(size);
	previewLoaders.clear();
	previewLoaders.resize(size);

	other.cameras = nullptr;

	return *this;
}

PreviewFeed::~PreviewFeed() {
	disable();
}

void PreviewFeed::enable() {
	if (size > 0 && !threads_running)
		start_threads();
}

void PreviewFeed::disable() {
	if (size > 0 && threads_running)
		finish_threads();
}

bool PreviewFeed::enabled() {
	return threads_running;
}

void PreviewFeed::setsync(bool sync) {
	assert(!threads_running);
	threads_synced = sync;
}

PreviewFeed::ImageQueue& PreviewFeed::getQueue(size_t i) {
	return previewQueues[i];
}

void PreviewFeed::start_threads() {
	std::cout << "preview threads starting" << std::endl;
	threads_running = true;

	// reset queues because new feeds are starting, don't store any old data
	for (auto& q: previewQueues)
		q.clear();

	numDone = Semaphore();
	round = 0;

	for (int i = 0; i < size; i++)
		previewLoaders[i] = std::thread(&PreviewFeed::previewThread, this, i);

	if (threads_synced)
		syncer = std::thread(&PreviewFeed::syncerThread, this);
}

void PreviewFeed::finish_threads() {
	threads_running = false;

	for (int i = 0; i < size; i++)
		previewLoaders[i].join();

	if (threads_synced)
		syncer.join();

	std::cout << "preview threads stopped" << std::endl;
}

void PreviewFeed::previewThread(int index) {
	auto& cam = (*cameras)[index];
	auto& buffers = previewQueues[index];

	int myround = 0;

	JpegBuffer jpeg;
	while (threads_running) {
		try {
			jpeg = cam.preview();
		} catch (gp::Exception& ex) {
			std::cout << "cam " << index << ": " << ex.what() << std::endl;
			continue;
		}
		buffers.push(std::make_pair(jpeg, Clock::now()));

		if (threads_synced) {
			// lock first notify then; now syncer notifies after we're waiting,
			// we will wake up.
			std::unique_lock<std::mutex> lk(nextRoundMutex);
			numDone.notify();
			nextRound.wait(lk, [&]() { return round == myround + 1; } );
			lk.unlock();
			myround++;
		}
	}
}

void PreviewFeed::syncerThread() {
	while (threads_running) {
		numDone.wait(size);
		// std::this_thread::sleep_for(std::chrono::milliseconds(200)); // debug
		round++;
		nextRound.notify_all();
	}
	// some may have gone sleeping unnoticed of the running flag change
	nextRound.notify_all();
}
