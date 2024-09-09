#include "common.h"
#include <riscv_vector.h>

#ifdef ENABLE_PARSEC_HOOKS
#include <hooks.h>
#endif

// index arithmetic
void index_golden(double *a, double *b, double *c, int n) {
  for (int i = 0; i < n; ++i) {
    a[i] = b[i] + (double)i * c[i];
  }
}

void index_(double *a, double *b, double *c, int n) {
  size_t vlmax = __riscv_vsetvlmax_e32m1();
  vuint32m1_t vec_i = __riscv_vid_v_u32m1(vlmax);
  for (size_t vl; n > 0; n -= vl, a += vl, b += vl, c += vl) {
    vl = __riscv_vsetvl_e64m2(n);

    vfloat64m2_t vec_i_double = __riscv_vfwcvt_f_xu_v_f64m2(vec_i, vl);

    vfloat64m2_t vec_b = __riscv_vle64_v_f64m2(b, vl);
    vfloat64m2_t vec_c = __riscv_vle64_v_f64m2(c, vl);

    vfloat64m2_t vec_a =
        __riscv_vfadd_vv_f64m2(vec_b, __riscv_vfmul_vv_f64m2(vec_c, vec_i_double, vl), vl);
    __riscv_vse64_v_f64m2(a, vec_a, vl);

    vec_i = __riscv_vadd_vx_u32m1(vec_i, vl, vl);
  }
}

int main(int argc, char** argv) {
  const int N = 31;
  const uint32_t seed = 0xdeadbeef;
  srand(seed);

  // data gen
  double B[N], C[N];
  gen_rand_1d(B, N);
  gen_rand_1d(C, N);

  // compute
  double golden[N], actual[N];
  index_golden(golden, B, C, N);
#ifdef ENABLE_PARSEC_HOOKS
  __parsec_roi_begin();
#endif
  index_(actual, B, C, N);

  // compare
  printf("%s: %s\n", argv[0], compare_1d(golden, actual, N) ? "pass" : "fail");
#ifdef ENABLE_PARSEC_HOOKS
  __parsec_roi_end();
#endif
}
