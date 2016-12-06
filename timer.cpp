
#include <timer.hpp>

Timer::Timer()
{
	snap();
}


double Timer::snap()
{
	auto now = std::chrono::steady_clock::now();
	auto total = std::chrono::duration<double>(now - prev).count();
	prev = now;
	return total;
}


double Timer::count() const
{
	auto now = std::chrono::steady_clock::now();
	return std::chrono::duration<double>(now - prev).count();
}

