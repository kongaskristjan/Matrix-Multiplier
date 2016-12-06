# Matrix-Multiplier
My library implementation for handling large matrix operations. The main goal is to provide an efficient multiplication algorithm. Using a cache-oblivious and multithreaded work distribution algorithm combined with AVX2, a performance very comparable to openBLAS is achieved.

In fact, as shown by figure (speedup for my implementation), for an i5-5200U, my library's performance is approximately 3x worse for small matrices (~100x100), but is consistently ~1.4x better for huge matrices (~10000x10000). See speedup_over_openblas.png for details.
