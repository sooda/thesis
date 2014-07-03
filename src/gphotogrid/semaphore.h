#include <mutex>
#include <condition_variable>

class Semaphore {
public:
	Semaphore(int count=0) : count(count) {}

	void notify_one(int n=1) {
		std::unique_lock<std::mutex> lock(mtx);
		count += n;
		cv.notify_one();
	}

	void notify_all(int n=1) {
		std::unique_lock<std::mutex> lock(mtx);
		count += n;
		cv.notify_all();
	}

	void wait(int i=1) {
		std::unique_lock<std::mutex> lock(mtx);
		cv.wait(lock, [this, i]() { return count >= i; });
		count -= i;
	}

private:
	std::mutex mtx;
	std::condition_variable cv;
	int count;
};
