#ifndef __TIMER_HPP__
#define __TIMER_HPP__

#include <chrono>

// All times are returned in seconds

class Timer {
public:
	Timer();
	double snap();
	double count() const;

private:
	std::chrono::steady_clock::time_point prev;
};

#endif

