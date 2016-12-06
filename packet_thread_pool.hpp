#ifndef __PACKET_THREAD_POOL_HPP__
#define __PACKET_THREAD_POOL_HPP__

#include <cassert>
#include <vector>
#include <memory>
#include <typeinfo>
#include <iostream>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <atomic>
#include <matrix.hpp>

class Packet_thread_pool {
public:
	Packet_thread_pool(uint64_t = 0); // pass 0 to detect hardware concurrency
	~Packet_thread_pool();

	void push(const Matrix_mul_job &);
	void finish();

private:
	void thread_wait();
	void do_jobs();
	inline void check_invariant() const;

	std::vector<std::thread> threads;
	std::vector<Matrix_mul_job> jobs;
	std::condition_variable worker_cv, finish_cv;
	std::mutex finish_mtx;

	std::atomic<uint64_t> next_job{0};
	int64_t working_threads{0};
	bool wait_flag{true};    // == 1 when no new jobs are available
	bool exit_flag{false};
};


inline void Packet_thread_pool::check_invariant() const
{
	// Assumed syncrhonization should have to be done anyway
	assert(next_job < jobs.size() + threads.size());
	assert(!((! wait_flag) && exit_flag));
	assert(working_threads >= 0);
}

extern Packet_thread_pool _g_thread_pool;

#endif

