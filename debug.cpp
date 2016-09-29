
#include <debug.hpp>

ostream & operator<<(ostream & out, __m256 data)
{
  vector<float> vdata(8);
  _mm256_storeu_ps(& vdata[0], data);
  out << "(";
  for (int i = 0; i < 8; ++i)
    out << vdata[i] << ", ";
  out << ")";
  return out;
}
