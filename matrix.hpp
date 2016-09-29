#ifndef __MATRIX_HPP__
#define __MATRIX_HPP__

#include <vector.hpp>
#include <vector>
#include <iostream>
#include <cassert>
#include <algorithm>
#include "immintrin.h"

using namespace std;

class Matrix {
  float * data;
  vector<float> alloc;
  size_t sx, sy, rx, ry;
  bool transpose_flag;
  
public:
  inline float get(size_t x, size_t y) const
  {
    assert(x < rx && y < ry);
    return data[rx * y + x];
  }
  
  inline float & get(size_t x, size_t y)
  {
    assert(x < rx && y < ry);
    return data[rx * y + x];
  }
  
  inline float * ptr(size_t x, size_t y)
  {
    return data + rx * y + x;
  }
  
  inline const float * ptr(size_t x, size_t y) const
  {
    return data + rx * y + x;
  }
  
  inline float operator()(size_t x, size_t y) const
  {
    if (transpose_flag)
      return get(y, x);
    return get(x, y);
  }
  
  inline float & operator()(size_t x, size_t y)
  {
    if (transpose_flag)
      return get(y, x);
    return get(x, y);
  }
  
  bool get_transpose() const
  {
    return transpose_flag;
  }
private:
  void allocate(); // sx and sy must be initialized
  void force_flip();
  
  void local_mul8x8(Matrix & ret, const Matrix & a, const Matrix & b,
		    size_t x, size_t y, size_t n) const;
  void local_mul16x16(Matrix & ret, const Matrix & a, const Matrix & b,
		      size_t x, size_t y, size_t n) const;
  void local_transform4x4(Matrix &, size_t, size_t) const;
  void local_transform8x8(Matrix &, size_t, size_t) const;
  void z_order(Matrix & ret, const Matrix & a, const Matrix & b,
  	       size_t cube, size_t x, size_t y, size_t n) const;
  
public:
  Matrix()
  {}
  
  Matrix(size_t, size_t);
  void operator=(const Matrix &);
  
  Vector just_multiply(const Vector &) const;
  void transpose();
  void force_transpose(bool);
  void operator+=(const Matrix &);
  void operator-=(const Matrix &);
  void operator*=(float rhs);
  Matrix operator+(const Matrix &) const;
  Matrix operator-(const Matrix &) const;
  Matrix operator*(const Matrix &) const;
  Matrix operator*(float rhs) const;
  Vector operator*(const Vector &) const;
  
  void add(const Vector &, bool);
  
  void adj_mul(const Matrix &);
  Vector slice(size_t) const;
  Matrix slice(size_t, size_t) const;
  
  template<typename T>
  void apply();
  template<typename T>
  void apply_mm256();
  
  size_t size_x() const
  {
    return sx;
  }
  
  size_t size_y() const
  {
    return sy;
  }
  
  size_t size() const
  {
    return sx * sy;
  }
  
  void debug_print() const;
  void full_print() const;
};

ostream & operator<<(ostream &, const Matrix &);
Matrix operator^(const Vector &, const Vector &);
Vector operator*(const Vector &, const Matrix &);

template<class T>
void Matrix::apply()
{
  for (size_t y = 0; y < ry; ++y)
    for (size_t x = 0; x < rx; ++x)
      get(x, y) = T::apply(get(x, y));
}

template<class T>
void Matrix::apply_mm256()
{
  size_t nx = sx, ny = sy;
  if (transpose_flag)
    swap(nx, ny);
  
  for (size_t y = 0; y < ny; ++y){
    for (size_t x = 0; x < nx; x += 8){
      __m256 a = _mm256_load_ps(ptr(x, y));
      a = T::apply(a);
      _mm256_store_ps(ptr(x, y), a);
    }
    
    for (size_t x = nx; x < rx; ++x)
      get(x, y) = 0;
  }
}

#endif
