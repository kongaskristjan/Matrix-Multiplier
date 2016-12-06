
#include <packet_thread_pool.hpp>

Packet_thread_pool _g_thread_pool;

Packet_thread_pool::Packet_thread_pool(uint64_t nthreads)
{
	if (nthreads == 0)
		nthreads = std::thread::hardware_concurrency();

	for (uint64_t i = 0; i < nthreads; ++i)
		threads.emplace_back([=] { thread_wait(); });
}


Packet_thread_pool::~Packet_thread_pool()
{
	finish();

	{
		std::lock_guard<std::mutex> lck(finish_mtx);
		exit_flag = true;
	}
	worker_cv.notify_all();

	for (std::thread & thr: threads)
		thr.join();
}


void Packet_thread_pool::push(const Matrix_mul_job & job)
{
	jobs.push_back(job);
}


void Packet_thread_pool::finish()
{
	std::unique_lock<std::mutex> lck(finish_mtx);
	wait_flag = false;

	worker_cv.notify_all();
	while (! wait_flag)
		finish_cv.wait(lck);

	jobs.clear();
	next_job = 0;

	check_invariant();
}


void Packet_thread_pool::thread_wait()
{
	bool me_wait = false;

	std::unique_lock<std::mutex> lck(finish_mtx);
	while (! exit_flag){
		if (me_wait || wait_flag){
			me_wait = false;
			worker_cv.wait(lck);
			continue;
		}

		else {
			++working_threads;
			lck.unlock();
			do_jobs();
			lck.lock();
			--working_threads;

			me_wait = true;
			if (! working_threads){
				wait_flag = true;
				finish_cv.notify_one();
			}
		}
	}

	check_invariant();
}


void Packet_thread_pool::do_jobs()
{
	uint64_t i;
	while ((i = next_job.fetch_add(1, std::memory_order_relaxed)) < jobs.size())
		jobs[i].execute();
}

