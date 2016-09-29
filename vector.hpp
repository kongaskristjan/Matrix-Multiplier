#ifndef __VECTOR_HPP__
#define __VECTOR_HPP__

#include <vector>
#include <algorithm>
#include <iostream>
#include <cassert>
#include "immintrin.h"

using namespace std;

class Vector {
  float * data;
  vector<float> alloc;
  size_t sz, rz;
  
public:
  inline float operator[](size_t z) const
  {
    return data[z];
  }
  
  inline float & operator[](size_t z)
  {
    return data[z];
  }
  
  inline float * ptr(size_t z)
  {
    return data + z;
  }
  
  inline const float * ptr(size_t z) const
  {
    return data + z;
  }
  
  inline size_t size() const
  {
    return sz;
  }
  
private:
  void allocate(); // sz must be initialized
  
public:
  Vector()
  {}
  
  Vector(size_t);
  Vector(const Vector &);
  void operator=(const Vector &);
  
  void operator+=(const Vector &);
  void operator-=(const Vector &);
  Vector operator+(const Vector &) const;
  Vector operator-(const Vector &) const;
  float operator*(const Vector &) const;
  void operator*=(float);
  Vector operator*(float) const;
  
  void adj_mul(const Vector &);
  
  template<class T>
  void apply();
  template<class T>
  void apply_mm256();
  
  void debug_print() const;
  void full_print() const;
};

ostream & operator<<(ostream &, const Vector &);

template<class T>
void Vector::apply()
{
  for (size_t i = 0; i < sz; ++i)
    data[i] = T::apply(data[i]);
}

template<class T>
void Vector::apply_mm256()
{
  for (size_t i = 0; i < rz; i += 8){
    __m256 a = _mm256_load_ps(ptr(i));
    a = T::apply(a);
    _mm256_store_ps(ptr(i), a);
  }
  
  for (size_t i = sz; i < rz; ++i)
    data[i] = 0;
}

#endif
