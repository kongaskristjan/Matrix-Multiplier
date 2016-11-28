
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
	for (int i = 0; i < 30000; ++i)
		test_up_to(32);
	for (int i = 0; i < 3000; ++i)
		test_up_to(128);
}

