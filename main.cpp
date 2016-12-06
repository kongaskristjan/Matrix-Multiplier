
#include <matrix.hpp>
#include <vector.hpp>
#include <tests.hpp>
#include <iostream>
#include <sstream>
#include <thread>

using namespace std;

struct Multiply {
	static float apply(float x)
	{
		return 2 * x;
	}

	static __m256 apply(__m256 x)
	{
		__m256 mul = _mm256_set1_ps(2.0);
		return _mm256_mul_ps(mul, x);
	}
};

void multiply(int sx, int sy)
{
	srand(time(NULL));
	Matrix mat1(sx, sy);
	for (int y = 0; y < sy; ++y)
		for (int x = 0; x < sx; ++x)
			mat1(x, y) = sx * y + x + 1;
	Matrix mat2(mat1);
	mat1.transpose();
	mat1.force_transpose(1);
	mat2.force_transpose(0);

	Matrix mul(mat1 * mat2);
	double sum = 0;
	for (size_t y = 0; y < mat1.size_y(); ++y)
		for (size_t x = 0; x < mat1.size_x(); ++x)
			sum += mat1(x, y);

	cout << sum << endl;
}

int main2(int argc, char ** argv)
{
	int sy;
	int sx = sy = 4000;
	size_t nthreads = 0;
	if (argc == 2){
		stringstream ss(argv[1]);
		ss >> nthreads;
	}

	if (nthreads > 4 * thread::hardware_concurrency())
		nthreads = thread::hardware_concurrency();

	if (nthreads == 0)
		nthreads = thread::hardware_concurrency();

	cout << "threads: " << nthreads << endl;
	cout << "sx: " << sx << "\tsy: " << sy << endl;
	cout << "Calculations: " << (2.0 * nthreads * sx * sx * sy) * 1e-9 << " GFLOP" << endl;

	vector<thread> threads;
	for (size_t i = 0; i < nthreads; ++i)
		threads.emplace_back(multiply, sx, sy);
	for (size_t i = 0; i < nthreads; ++i)
		threads[i].join();

	return 0;
}

void run_perf_tests(long start, long end, long delta, long coef)
{
	for (long sz = start; sz <= end; sz += delta){
		long ntimes = 1 + (coef / (sz * sz * sz));
		double sum = 0;
		for (long j = 0; j < ntimes; ++j)
			sum += performance(sz);

		std::cout << sz << ", " << sum / ntimes << "\n";
	}
}


int main()
{
	run_tests();

	run_perf_tests(100, 900, 100, 1e11);
	run_perf_tests(1000, 9500, 500, 1e11);
	run_perf_tests(10000, 20000, 2000, 1e11);

	return 0;
}

