
#include <tests.hpp>

void test_up_to(int mx)
{
	// Matrices (c * a) (b * c)
	int a = rand_less(mx) + 1, b = rand_less(mx) + 1, c = rand_less(mx) + 1;
	int x = rand_less(b), y = rand_less(a), z = rand_less(c);

	Matrix m0(c, a), m1(b, c);
	m0(z, y) = 1;
	m1(x, z) = 1;

	Matrix res(m0 * m1);
	for (int ny = 0; ny < a; ++ny)
		for (int nx = 0; nx < b; ++nx){
			float val = res(nx, ny);
			if (nx == x && ny == y)
				approx_equal(val, 1.0, 1e-4);
			else
				approx_equal(val, 0.0, 1e-4);
		}
}


void run_tests()
{
	srand(time(NULL));

	for (int i = 0; i < 100000; ++i)
		test_up_to(16);
	for (int i = 0; i < 10000; ++i)
		test_up_to(128);
	std::cout << "Correctness tested\n";
}


double performance(int n)
{
	Matrix m0(n, n), m1(n, n);
	for (int i = 0; i < n; ++i)
		for (int j = 0; j < n; ++j){
			m0(i, j) = 1.0;
			m1(i, j) = 1.0;
		}

	Timer timer;
	Matrix ret(m0 * m1);
	return timer.snap();
}

