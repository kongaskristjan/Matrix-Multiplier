
#include <matrix.hpp>

// Private

void Matrix::allocate()
{
  int nx = sx, ny = sy;
  if (transpose_flag){
    nx = sy;
    ny = sx;
  }
  
  rx = nx + 7 - (nx + 7) % 8;
  ry = ny + 7 - (ny + 7) % 8;
  alloc.resize(rx * ry + 8);
  size_t ptr = (size_t) & alloc[0];
  data = (float *) (ptr + 31 - (ptr + 31) % 32);
}

// brute force
void Matrix::force_flip()
{
  transpose_flag = !transpose_flag;
  vector<float> alloc2;
  alloc2.swap(alloc);
  float * data2 = data;
  allocate();
  
  // There seems to be no measurable difference in speed with
  // large matrices between cache-optimized algorithms and this here
  for (size_t y = 0; y < ry; y += 4)
    for (size_t x = 0; x < rx; x += 4){
      __m128 a = _mm_load_ps(data2 + x * ry + y);
      __m128 b = _mm_load_ps(data2 + (x + 1) * ry + y);
      __m128 c = _mm_load_ps(data2 + (x + 2) * ry + y);
      __m128 d = _mm_load_ps(data2 + (x + 3) * ry + y);
      
      _MM_TRANSPOSE4_PS(a, b, c, d);
      
      _mm_store_ps(data + y * rx + x, a);
      _mm_store_ps(data + (y + 1) * rx + x, b);
      _mm_store_ps(data + (y + 2) * rx + x, c);
      _mm_store_ps(data + (y + 3) * rx + x, d);
    }
}

// Public

Matrix::Matrix(size_t _sx, size_t _sy):
  sx(_sx), sy(_sy), transpose_flag(false)
{
  allocate();
}

void Matrix::operator=(const Matrix & cpy)
{
  sx = cpy.sx;
  sy = cpy.sy;
  transpose_flag = cpy.transpose_flag;
  allocate();
  for (size_t y = 0; y < ry; ++y)
    for (size_t x = 0; x < rx; x += 8){
      __m256 a = _mm256_load_ps(cpy.ptr(x, y));
      _mm256_store_ps(ptr(x, y), a);
    }
}

void Matrix::transpose()
{
  transpose_flag = !transpose_flag;
  swap(sx, sy);
}

void Matrix::force_transpose(bool _transpose_flag)
{
  if (_transpose_flag != transpose_flag)
    force_flip();
}

void Matrix::operator+=(const Matrix & rhs)
{
  assert(sx == rhs.sx);
  assert(sy == rhs.sy);
  
  Matrix * nprhs = 0;
  const Matrix * prhs = & rhs;
  
  if (transpose_flag != rhs.transpose_flag){
    nprhs = new Matrix(rhs);
    nprhs->force_transpose(transpose_flag);
    prhs = (Matrix *) nprhs;
  }
  
  for (size_t y = 0; y < ry; ++y)
    for (size_t x = 0; x < rx; x += 8){
      __m256 a = _mm256_load_ps(this->ptr(x, y));
      __m256 b = _mm256_load_ps(prhs->ptr(x, y));
      * ((__m256 *) this->ptr(x, y)) = _mm256_add_ps(a, b);
    }
  
  delete nprhs;
}

void Matrix::operator-=(const Matrix & rhs)
{
  assert(sx == rhs.sx);
  assert(sy == rhs.sy);
  
  Matrix * nprhs = 0;
  const Matrix * prhs = & rhs;
  
  if (transpose_flag != rhs.transpose_flag){
    nprhs = new Matrix(rhs);
    nprhs->force_transpose(transpose_flag);
    prhs = (Matrix *) nprhs;
  }
  
  for (size_t y = 0; y < ry; ++y)
    for (size_t x = 0; x < rx; x += 8){
      __m256 a = _mm256_load_ps(this->ptr(x, y));
      __m256 b = _mm256_load_ps(prhs->ptr(x, y));
      * ((__m256 *) this->ptr(x, y)) = _mm256_sub_ps(a, b);
    }
  
  delete nprhs;
}

void Matrix::operator*=(float rhs)
{
  __m256 mul = _mm256_set1_ps(rhs);
  for (size_t y = 0; y < ry; ++y)
    for (size_t x = 0; x < rx; x += 8){
      __m256 a = _mm256_load_ps(this->ptr(x, y));
      * ((__m256 *) this->ptr(x, y)) = _mm256_mul_ps(a, mul);
    }
}

Matrix Matrix::operator+(const Matrix & rhs) const
{
  Matrix ret(* this);
  ret += rhs;
  return ret;
}

void Matrix::add(const Vector & rhs, bool vec_tr)
{
  if (! (transpose_flag ^ vec_tr))
    for (size_t x = 0; x < rx; x += 8){
      __m256 a = _mm256_load_ps(rhs.ptr(x));
      for (size_t y = 0; y < ry; ++y){
	__m256 b = _mm256_load_ps(ptr(x, y));
	__m256 sum = _mm256_add_ps(a, b);
	_mm256_store_ps(ptr(x, y), sum);
      }
    }
  
  else
    for (size_t y = 0; y < ry; ++y){
      __m256 a = _mm256_broadcast_ss(rhs.ptr(y));
      for (size_t x = 0; x < rx; x += 8){
	__m256 b = _mm256_load_ps(ptr(x, y));
	__m256 sum = _mm256_add_ps(a, b);
	_mm256_store_ps(ptr(x, y), sum);
      }
    }
}

Matrix Matrix::operator-(const Matrix & rhs) const
{
  Matrix ret(* this);
  ret -= rhs;
  return ret;
}

Matrix Matrix::operator*(float rhs) const
{
  Matrix ret(* this);
  ret *= rhs;
  return ret;
}

void Matrix::z_order(Matrix & ret, const Matrix & a, const Matrix & b,
		     size_t cube, size_t x, size_t y, size_t n) const
{
  if (x >= ret.rx || y >= ret.ry || n >= a.ry)
    return;
  
  if (cube == 16 && x + 16 <= ret.rx && y + 16 <= ret.ry && n + 16 <= a.ry){
    local_mul16x16(ret, a, b, x, y, n);
    return;
  }
  
  if (cube == 8){
    local_mul8x8(ret, a, b, x, y, n);
    return;
  }
  
  size_t cube_2 = cube / 2;
  
  z_order(ret, a, b, cube_2, x, y, n);
  z_order(ret, a, b, cube_2, x, y, n + cube_2);
  z_order(ret, a, b, cube_2, x + cube_2, y, n);
  z_order(ret, a, b, cube_2, x + cube_2, y, n + cube_2);
  z_order(ret, a, b, cube_2, x, y + cube_2, n);
  z_order(ret, a, b, cube_2, x, y + cube_2, n + cube_2);
  z_order(ret, a, b, cube_2, x + cube_2, y + cube_2, n);
  z_order(ret, a, b, cube_2, x + cube_2, y + cube_2, n + cube_2);
}

void Matrix::local_transform4x4(Matrix & ret, size_t x0, size_t y0) const
{
  float sw0, sw1;
  sw0 = ret.get(x0 + 1, y0);
  ret.get(x0 + 1, y0) = ret.get(x0 + 1, y0 + 3);
  ret.get(x0 + 1, y0 + 3) = ret.get(x0 + 1, y0 + 2);
  ret.get(x0 + 1, y0 + 2) = ret.get(x0 + 1, y0 + 1);
  ret.get(x0 + 1, y0 + 1) = sw0;
  
  sw0 = ret.get(x0 + 2, y0);
  sw1 = ret.get(x0 + 2, y0 + 1);
  ret.get(x0 + 2, y0) = ret.get(x0 + 2, y0 + 2);
  ret.get(x0 + 2, y0 + 1) = ret.get(x0 + 2, y0 + 3);
  ret.get(x0 + 2, y0 + 2) = sw0;
  ret.get(x0 + 2, y0 + 3) = sw1;
  
  sw0 = ret.get(x0 + 3, y0);
  ret.get(x0 + 3, y0) = ret.get(x0 + 3, y0 + 1);
  ret.get(x0 + 3, y0 + 1) = ret.get(x0 + 3, y0 + 2);
  ret.get(x0 + 3, y0 + 2) = ret.get(x0 + 3, y0 + 3);
  ret.get(x0 + 3, y0 + 3) = sw0;
}

void Matrix::local_transform8x8(Matrix & ret, size_t x0, size_t y0) const
{
  local_transform4x4(ret, x0, y0);
  local_transform4x4(ret, x0 + 4, y0);
  local_transform4x4(ret, x0, y0 + 4);
  local_transform4x4(ret, x0 + 4, y0 + 4);
  
  for (size_t y = y0; y < y0 + 4; ++y)
    for (size_t x = x0; x < x0 + 4; ++x){
      float sw = ret.get(x + 4, y);
      ret.get(x + 4, y) = ret.get(x, y + 4);
      ret.get(x, y + 4) = ret.get(x + 4, y + 4);
      ret.get(x + 4, y + 4) = sw;
    }
}

Matrix Matrix::operator*(const Matrix & rhs) const
{
  assert(sx == rhs.sy);
  Matrix * ncpa = 0, * ncpb = 0;
  const Matrix * pa = this, * pb = & rhs;
  
  if (! transpose_flag){
    ncpa = new Matrix(* this);
    ncpa->force_transpose(1);
    pa = (Matrix *) ncpa;
  }
  
  if (rhs.transpose_flag){
    ncpb = new Matrix(rhs);
    ncpb->force_transpose(0);
    pb = (Matrix *) ncpb;
  }
  
  Matrix ret(rhs.sx, sy);
  size_t cube = 1 << (16 * sizeof(size_t) -
		      __builtin_clzl(max(max(ret.rx, ret.ry), pa->ry) - 1));
  z_order(ret, * pa, * pb, cube, 0, 0, 0);
  
  for (size_t y = 0; y < ret.ry; y += 8)
    for (size_t x = 0; x < ret.rx; x += 8)
      local_transform8x8(ret, x, y);
  
  delete ncpa;
  delete ncpb;
  return ret;
}

Vector Matrix::operator*(const Vector & rhs) const
{
  assert(sx == rhs.size());
  
  Matrix * ncp = 0;
  const Matrix * p = this;
  if (transpose_flag){
    ncp = new Matrix(* this);
    ncp->force_transpose(0);
    p = (Matrix *) ncp;
  }
  
  Vector ret(p->just_multiply(rhs));
  delete ncp;
  return ret;
}

Vector operator*(const Vector & lhs, const Matrix & rhs)
{
  assert(rhs.size_y() == lhs.size());
  
  Matrix * ncp = 0;
  const Matrix * p = & rhs;
  if (! rhs.get_transpose()){
    ncp = new Matrix(rhs);
    ncp->force_transpose(1);
    p = (Matrix *) ncp;
  }
  
  Vector ret(p->just_multiply(lhs));
  delete ncp;
  return ret;
}

// no-check multiply
Vector Matrix::just_multiply(const Vector & rhs) const
{
  size_t ans_sz = sy;
  if (transpose_flag)
    ans_sz = sx;
  Vector ret(ans_sz);
  for (size_t y = 0; y < ry; ++y){
    __m256 sum = _mm256_set1_ps(0.0);
    for (size_t x = 0; x < rx; x += 8){
      __m256 a = _mm256_load_ps(rhs.ptr(x));
      __m256 b = _mm256_load_ps(ptr(x, y));
      sum = _mm256_fmadd_ps(a, b, sum);
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
    
    ret[y] = _mm_cvtss_f32(sum1);
  }
  return ret;
}

void Matrix::adj_mul(const Matrix & rhs)
{
  assert(sx == rhs.size_x());
  assert(sy == rhs.size_y());
  
  Matrix * ncp = 0;
  const Matrix * p = & rhs;
  if (transpose_flag != rhs.get_transpose()){
    ncp = new Matrix(rhs);
    ncp->force_transpose(transpose_flag);
    p = (Matrix *) ncp;
  }
  
  for (size_t y = 0; y < ry; ++y)
    for (size_t x = 0; x < rx; x += 8){
      __m256 a = _mm256_load_ps(ptr(x, y));
      __m256 b = _mm256_load_ps(p->ptr(x, y));
      _mm256_store_ps(ptr(x, y), _mm256_mul_ps(a, b));
    }
  
  delete ncp;
}

Vector Matrix::slice(size_t y) const
{
  Vector ret(sx);
  for (size_t x = 0; x < sx; x += 8){
    __m256 a = _mm256_load_ps(ptr(x, y));
    _mm256_store_ps(ret.ptr(x), a);
  }
  return ret;
}

Matrix Matrix::slice(size_t y0, size_t width) const
{
  width = min(width, sy - y0);
  if (y0 == 0 && width == sy)
    return * this;
  
  Matrix ret(sx, width);
  for (size_t y = 0; y < width; ++y)
    for (size_t x = 0; x < sx; x += 8){
      __m256 a = _mm256_load_ps(ptr(x, y0 + y));
      _mm256_store_ps(ret.ptr(x, y), a);
    }
  return ret;
}

void Matrix::debug_print() const
{
  cerr << "sx: " << sx << "\tsy: " << sy << "\trx: " << rx << "\try: " << ry
       << "\talloc size: " << alloc.size()
       << "\ttranspose: " << transpose_flag << endl;
}

void Matrix::full_print() const
{
  for (size_t y = 0; y < ry; ++y){
    for (size_t x = 0; x < rx; ++x)
      cerr << get(x, y) << " ";
    cerr << endl;
  }
  cerr << endl;
}

ostream & operator<<(ostream & out, const Matrix & mat)
{
  size_t sx = mat.size_x();
  size_t sy = mat.size_y();
  for (size_t y = 0; y < sy - 1; ++y){
    out << "(";
    for (size_t x = 0; x < sx - 1; ++x)
      out << mat(x, y) << ", ";
    out << mat(sx - 1, y) << ")" << endl;
  }
  
  out << "(";
  for (size_t x = 0; x < sx - 1; ++x)
    out << mat(x, sy - 1) << ", ";
  out << mat(sx - 1, sy - 1) << ")" << endl;
  return out;
}

Matrix operator^(const Vector & lhs, const Vector & rhs)
{
  Matrix ret(rhs.size(), lhs.size());
  for (size_t y = 0; y < lhs.size(); ++y){
    __m256 cmul = _mm256_set1_ps(lhs[y]);
    for (size_t x = 0; x < rhs.size(); x += 8){
      __m256 m = _mm256_load_ps(rhs.ptr(x));
      _mm256_store_ps(ret.ptr(x, y), _mm256_mul_ps(cmul, m));
    }
  }
  
  return ret;
}

