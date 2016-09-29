
#include <vector.hpp>

// Private

void Vector::allocate()
{
  rz = sz + 7 - (sz + 7) % 8;
  alloc.resize(rz + 8);
  size_t ptr = (size_t) & alloc[0];
  data = (float *) (ptr + 31 - (ptr + 31) % 32);
}

// Public

Vector::Vector(size_t _sz) : sz(_sz)
{
  allocate();
}

Vector::Vector(const Vector & cpy)
{
  * this = cpy;
}

void Vector::operator=(const Vector & cpy)
{
  sz = cpy.sz;
  allocate();
  for (size_t z = 0; z < rz; z += 8){
    __m256 c = _mm256_load_ps(cpy.data + z);
    _mm256_store_ps(data + z, c);
  }
}

void Vector::operator+=(const Vector & rhs)
{
  for (size_t z = 0; z < rz; z += 8){
    __m256 a = _mm256_load_ps(data + z);
    __m256 b = _mm256_load_ps(rhs.data + z);
    __m256 sum = _mm256_add_ps(a, b);
    _mm256_store_ps(data + z, sum);
  }
}

void Vector::operator-=(const Vector & rhs)
{
  for (size_t z = 0; z < rz; z += 8){
    __m256 a = _mm256_load_ps(data + z);
    __m256 b = _mm256_load_ps(rhs.data + z);
    __m256 diff = _mm256_sub_ps(a, b);
    _mm256_store_ps(data + z, diff);
  }
}

Vector Vector::operator+(const Vector & rhs) const
{
  Vector ret(* this);
  ret += rhs;
  return ret;
}

Vector Vector::operator-(const Vector & rhs) const
{
  Vector ret(* this);
  ret -= rhs;
  return ret;
}

float Vector::operator*(const Vector & rhs) const
{
  __m256 sum = _mm256_set1_ps(0.0);
  for (size_t z = 0; z < rz; z += 8){
    __m256 a = _mm256_load_ps(data + z);
    __m256 b = _mm256_load_ps(rhs.data + z);
    __m256 prod = _mm256_mul_ps(a, b);
    sum = _mm256_add_ps(sum, prod);
  }
  
  // summing up
  __m128 hi4 = _mm256_extractf128_ps(sum, 1);
  __m128 lo4 = _mm256_castps256_ps128(sum);
  __m128 sum4 = _mm_add_ps(lo4, hi4);
  __m128 lo2 = sum4;
  __m128 hi2 = _mm_movehl_ps(sum4, sum4);
  __m128 sum2 = _mm_add_ps(lo2, hi2);
  __m128 lo1 = sum2;
  __m128 hi1 = _mm_shuffle_ps(sum2, sum2, 0x1);
  __m128 sum1 = _mm_add_ss(lo1, hi1);
  
  return _mm_cvtss_f32(sum1);
}

void Vector::operator*=(float rhs)
{
  __m256 mul = _mm256_set1_ps(rhs);
  for (size_t z = 0; z < rz; z += 8){
    __m256 a = _mm256_load_ps(this->ptr(z));
    * ((__m256 *) this->ptr(z)) = _mm256_mul_ps(a, mul);
  }
}

Vector Vector::operator*(float rhs) const
{
  Vector ret(* this);
  ret *= rhs;
  return ret;
}

void Vector::adj_mul(const Vector & rhs)
{
  assert(sz == rhs.size());
  
  for (size_t z = 0; z < rz; z += 8){
    __m256 a = _mm256_load_ps(ptr(z));
    __m256 b = _mm256_load_ps(rhs.ptr(z));
    _mm256_store_ps(ptr(z), _mm256_mul_ps(a, b));
  }
}

void Vector::debug_print() const
{
  cerr << "alloc: " << alloc.data() << "\tdata: " << data << "\tsz: " << sz << "\trz: " << rz << endl;
}

void Vector::full_print() const
{
  cerr << "(";
  for (size_t z = 0; z < rz; ++z)
    cerr << data[z] << ", ";
  cerr << ")" << endl;
}

ostream & operator<<(ostream & out, const Vector & v)
{
  out << "(";
  for (size_t i = 0; i < v.size() - 1; ++i)
    out << v[i] << ", ";
  out << v[v.size() - 1] << ")";
  return out;
}
