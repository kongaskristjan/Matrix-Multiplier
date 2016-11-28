#ifndef __TESTS_HPP__
#define __TESTS_HPP__

#include <cassert>
#include <cmath>
#include <cstdlib>
#include <matrix.hpp>
#include <vector.hpp>

inline void approx_equal(float x, float y, float margin);

inline int rand_less(int);
void test_up_to(int);
void run_tests();


inline int rand_less(int mx)
{
	return rand() % mx;
}


inline void approx_equal(float x, float y, float margin)
{
	assert(abs(x - y) < margin);
}

#endif

