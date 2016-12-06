#!/usr/bin/python3

t0 = "\t"
t1 = "\n"

def intro(f):
    f.write("\n")
    f.write("#include <matrix.hpp>\n")

def func_start(f, tag):
    f.write("\n")
    f.write("\n")
    f.write("void Matrix::local_mul" + tag + "(Matrix_view ret, Const_matrix_view ma, Const_matrix_view mb,\n")
    f.write("\t" + len(tag) * " " + "size_t x0, size_t y0, size_t n0)\n")
    f.write("{\n")
    f.write(t0 + "size_t da = ma.delta, db = mb.delta, dr = ret.delta;\n")
    f.write(t0 + "__m256 r0, r1, r2, r3, r4, r5, r6, r7;\n")
    f.write(t0 + "__m256 a0, a1, a2, a3, b0, bs, bak0, bak1;\n")
    f.write(t1)
    

def func_end(f):
    f.write("}\n")


def ptr_decl(f):
    f.write(t0 + "float * pr;\n")
    f.write(t0 + "const float * pa, * pb;\n")


def ptr_line(f, x, y, n):
    sx = str(x)
    sy = str(y)
    sn = str(n)
    f.write(t0 + "pa = ma.ptr(y0 + " + sy + ", n0 + " + sn + ");\n")
    f.write(t0 + "pb = mb.ptr(x0 + " + sx + ", n0 + " + sn + ");\n")
    f.write(t0 + "pr = ret.ptr(x0 + " + sx + ", y0 + " + sy + ");\n")


def gen8x8(f, lbit, sbit):
    if lbit:
        f.write(t0 + "r0 = _mm256_set1_ps(0.0);\n")
        f.write(t0 + "r1 = _mm256_set1_ps(0.0);\n")
        f.write(t0 + "r2 = _mm256_set1_ps(0.0);\n")
        f.write(t0 + "r3 = _mm256_set1_ps(0.0);\n")
        f.write(t0 + "r4 = _mm256_set1_ps(0.0);\n")
        f.write(t0 + "r5 = _mm256_set1_ps(0.0);\n")
        f.write(t0 + "r6 = _mm256_set1_ps(0.0);\n")
        f.write(t0 + "r7 = _mm256_set1_ps(0.0);\n")
    
    
    for i in range(8):
        f.write(t1)
        f.write(t0 + "a0 = _mm256_load_ps(pa + " + str(i) + " * da);\n")
        f.write(t0 + "b0 = _mm256_load_ps(pb + " + str(i) + " * db);\n")
        
        f.write(t0 + "bs = _mm256_permute2f128_ps(b0, b0, 1);\n")
        f.write(t0 + "a1 = _mm256_permute_ps(a0, 0x39);\n")
        f.write(t0 + "a2 = _mm256_permute_ps(a0, 0x4e);\n")
        f.write(t0 + "a3 = _mm256_permute_ps(a0, 0x93);\n")
        
        f.write(t0 + "r0 = _mm256_fmadd_ps(a0, b0, r0);\n")
        f.write(t0 + "r1 = _mm256_fmadd_ps(a1, b0, r1);\n")
        f.write(t0 + "r2 = _mm256_fmadd_ps(a2, b0, r2);\n")
        f.write(t0 + "r3 = _mm256_fmadd_ps(a3, b0, r3);\n")
        f.write(t0 + "r4 = _mm256_fmadd_ps(a0, bs, r4);\n")
        f.write(t0 + "r5 = _mm256_fmadd_ps(a1, bs, r5);\n")
        f.write(t0 + "r6 = _mm256_fmadd_ps(a2, bs, r6);\n")
        f.write(t0 + "r7 = _mm256_fmadd_ps(a3, bs, r7);\n")
    
    if sbit:
        f.write(t1)
        f.write(t0 + "a0 = _mm256_load_ps(pr);\n")
        f.write(t0 + "a1 = _mm256_load_ps(pr + dr);\n")
        f.write(t0 + "a2 = _mm256_load_ps(pr + 2 * dr);\n")
        f.write(t0 + "a3 = _mm256_load_ps(pr + 3 * dr);\n")
        f.write(t0 + "b0 = _mm256_load_ps(pr + 4 * dr);\n")
        f.write(t0 + "bs = _mm256_load_ps(pr + 5 * dr);\n")
        f.write(t0 + "bak0 = _mm256_load_ps(pr + 6 * dr);\n")
        f.write(t0 + "bak1 = _mm256_load_ps(pr + 7 * dr);\n")
        
        f.write(t0 + "r0 = _mm256_add_ps(r0, a0);\n")
        f.write(t0 + "r1 = _mm256_add_ps(r1, a1);\n")
        f.write(t0 + "r2 = _mm256_add_ps(r2, a2);\n")
        f.write(t0 + "r3 = _mm256_add_ps(r3, a3);\n")
        f.write(t0 + "r4 = _mm256_add_ps(r4, b0);\n")
        f.write(t0 + "r5 = _mm256_add_ps(r5, bs);\n")
        f.write(t0 + "r6 = _mm256_add_ps(r6, bak0);\n")
        f.write(t0 + "r7 = _mm256_add_ps(r7, bak1);\n")
        
        f.write(t0 + "_mm256_store_ps(pr, r0);\n")
        f.write(t0 + "_mm256_store_ps(pr + dr, r1);\n")
        f.write(t0 + "_mm256_store_ps(pr + 2 * dr, r2);\n")
        f.write(t0 + "_mm256_store_ps(pr + 3 * dr, r3);\n")
        f.write(t0 + "_mm256_store_ps(pr + 4 * dr, r4);\n")
        f.write(t0 + "_mm256_store_ps(pr + 5 * dr, r5);\n")
        f.write(t0 + "_mm256_store_ps(pr + 6 * dr, r6);\n")
        f.write(t0 + "_mm256_store_ps(pr + 7 * dr, r7);\n")
        

def gen16x16(f):
    ptr_line(f, 0, 0, 0)
    gen8x8(f, True, False)
    ptr_line(f, 0, 0, 8)
    gen8x8(f, False, True)
    
    ptr_line(f, 0, 8, 0)
    gen8x8(f, True, False)
    ptr_line(f, 0, 8, 8)
    gen8x8(f, False, True)
    
    ptr_line(f, 8, 0, 0)
    gen8x8(f, True, False)
    ptr_line(f, 8, 0, 8)
    gen8x8(f, False, True)
    
    ptr_line(f, 8, 8, 0)
    gen8x8(f, True, False)
    ptr_line(f, 8, 8, 8)
    gen8x8(f, False, True)


def main():
    f = open("generated.cpp", "w")
    intro(f)
    
    func_start(f, "8x8")
    ptr_decl(f)
    ptr_line(f, 0, 0, 0)
    gen8x8(f, True, True)
    func_end(f)
    
    func_start(f, "16x16")
    ptr_decl(f)
    gen16x16(f)
    func_end(f)


if __name__ == "__main__":
    main()
