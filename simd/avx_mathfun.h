/* SIMD (AVX2+AVX) implementation of sin, cos, exp and log
    
   Inspired by original sse_mathfun code, which was in turn inspired by Intel's 
   Approximate Math library, and based on the corresponding 
   algorithms of the cephes math library

   If there's some mispresentation of zlib license usage - tell me.
*/

/* Copyright (C) 2021 Yegor Suslin
   
  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

  (this is the zlib license)
*/
#ifndef AVXMATHFUN_H
#define AVXMATHFUN_H

#include <immintrin.h>

/* yes I know, the top of this file is quite ugly */

#ifdef _MSC_VER /* visual c++ */
# define ALIGN32_BEG __declspec(align(32))
# define ALIGN32_END 
#else /* gcc or icc */
# define ALIGN32_BEG
# define ALIGN32_END __attribute__((aligned(32)))
#endif

/* __m256 is ugly to write */
typedef __m256 v8sf;  // vector of 8 float (avx)
typedef __m256i v8si; // vector of 8 int (avx)


/* declare some SSE constants -- why can't I figure a better way to do that? */
#define _PS_CONST(Name, Val)                                            \
  static const ALIGN32_BEG float _ps_##Name[8] ALIGN32_END = { Val, Val, Val, Val, Val, Val, Val, Val  }
#define _PI32_CONST(Name, Val)                                            \
  static const ALIGN32_BEG int _pi32_##Name[8] ALIGN32_END = { Val, Val, Val, Val, Val, Val, Val, Val  }
#define _PS_CONST_TYPE(Name, Type, Val)                                 \
  static const ALIGN32_BEG Type _ps_##Name[8] ALIGN32_END = { Val, Val, Val, Val, Val, Val, Val, Val  }

_PS_CONST(1  , 1.0f);
_PS_CONST(0p5, 0.5f);
/* the smallest non denormalized float number */
_PS_CONST_TYPE(min_norm_pos, int, 0x00800000);
_PS_CONST_TYPE(mant_mask, int, 0x7f800000);
_PS_CONST_TYPE(inv_mant_mask, int, ~0x7f800000);

_PS_CONST_TYPE(sign_mask, int, (int)0x80000000);
_PS_CONST_TYPE(inv_sign_mask, int, ~0x80000000);

_PI32_CONST(1, 1);
_PI32_CONST(inv1, ~1);
_PI32_CONST(2, 2);
_PI32_CONST(4, 4);
_PI32_CONST(0x7f, 0x7f);

_PS_CONST(cephes_SQRTHF, 0.707106781186547524);
_PS_CONST(cephes_log_p0, 7.0376836292E-2);
_PS_CONST(cephes_log_p1, - 1.1514610310E-1);
_PS_CONST(cephes_log_p2, 1.1676998740E-1);
_PS_CONST(cephes_log_p3, - 1.2420140846E-1);
_PS_CONST(cephes_log_p4, + 1.4249322787E-1);
_PS_CONST(cephes_log_p5, - 1.6668057665E-1);
_PS_CONST(cephes_log_p6, + 2.0000714765E-1);
_PS_CONST(cephes_log_p7, - 2.4999993993E-1);
_PS_CONST(cephes_log_p8, + 3.3333331174E-1);
_PS_CONST(cephes_log_q1, -2.12194440e-4);
_PS_CONST(cephes_log_q2, 0.693359375);


typedef union ymm_mm_union {
  __m256 ymm;
  __m128 xmm[2];
} ymm_mm_union;

#define COPY_YMM_TO_XMM(ymm_, xmm0_, xmm1_) {          \
    ymm_mm_union u; u.ymm = ymm_;                   \
    xmm0_ = u.xmm[0];                                 \
    xmm1_ = u.xmm[1];                                 \
}

#define COPY_MM_TO_XMM(xmm0_, xmm1_, ymm_) {                         \
    ymm_mm_union u; u.xmm[0]=xmm0_; u.xmm[1]=xmm1_; ymm_ = u.ymm;      \
  }


/* natural logarithm computed for 4 simultaneous float 
   return NaN for x <= 0
*/
static v8sf ln256_ps(v8sf x) {
  v8si emm0;

  v8sf one = *(v8sf*)_ps_1;

  v8sf invalid_mask = _mm256_cmp_ps(x, _mm256_setzero_ps(), _CMP_LE_OQ);

  x = _mm256_max_ps(x, *(v8sf*)_ps_min_norm_pos);  /* cut off denormalized stuff */

  /* part 1: x = frexpf(x, &e); */
  emm0 = _mm256_srli_epi32(_mm256_castps_si256(x), 23);

  /* keep only the fractional part */
  x = _mm256_and_ps(x, *(v8sf*)_ps_inv_mant_mask);
  x = _mm256_or_ps(x, *(v8sf*)_ps_0p5);

  emm0 = _mm256_sub_epi32(emm0, *(v8si*)_pi32_0x7f);
  v8sf e = _mm256_cvtepi32_ps(emm0);

  e = _mm256_add_ps(e, one);

  /* part2: 
     if( x < SQRTHF ) {
       e -= 1;
       x = x + x - 1.0;
     } else { x = x - 1.0; }
  */
  v8sf mask = _mm256_cmp_ps(x, *(v8sf*)_ps_cephes_SQRTHF, _CMP_LT_OQ);
  v8sf tmp = _mm256_and_ps(x, mask);
  x = _mm256_sub_ps(x, one);
  e = _mm256_sub_ps(e, _mm256_and_ps(one, mask));
  x = _mm256_add_ps(x, tmp);


  v8sf z = _mm256_mul_ps(x,x);

  v8sf y = *(v8sf*)_ps_cephes_log_p0;
  y = _mm256_mul_ps(y, x);
  y = _mm256_add_ps(y, *(v8sf*)_ps_cephes_log_p1);
  y = _mm256_mul_ps(y, x);
  y = _mm256_add_ps(y, *(v8sf*)_ps_cephes_log_p2);
  y = _mm256_mul_ps(y, x);
  y = _mm256_add_ps(y, *(v8sf*)_ps_cephes_log_p3);
  y = _mm256_mul_ps(y, x);
  y = _mm256_add_ps(y, *(v8sf*)_ps_cephes_log_p4);
  y = _mm256_mul_ps(y, x);
  y = _mm256_add_ps(y, *(v8sf*)_ps_cephes_log_p5);
  y = _mm256_mul_ps(y, x);
  y = _mm256_add_ps(y, *(v8sf*)_ps_cephes_log_p6);
  y = _mm256_mul_ps(y, x);
  y = _mm256_add_ps(y, *(v8sf*)_ps_cephes_log_p7);
  y = _mm256_mul_ps(y, x);
  y = _mm256_add_ps(y, *(v8sf*)_ps_cephes_log_p8);
  y = _mm256_mul_ps(y, x);

  y = _mm256_mul_ps(y, z);
  

  tmp = _mm256_mul_ps(e, *(v8sf*)_ps_cephes_log_q1);
  y = _mm256_add_ps(y, tmp);


  tmp = _mm256_mul_ps(z, *(v8sf*)_ps_0p5);
  y = _mm256_sub_ps(y, tmp);

  tmp = _mm256_mul_ps(e, *(v8sf*)_ps_cephes_log_q2);
  x = _mm256_add_ps(x, y);
  x = _mm256_add_ps(x, tmp);
  x = _mm256_or_ps(x, invalid_mask); // negative arg will be NAN
  return x;
}

_PS_CONST(exp_hi,	88.3762626647949f);
_PS_CONST(exp_lo,	-88.3762626647949f);

_PS_CONST(cephes_LOG2EF, 1.44269504088896341);
_PS_CONST(cephes_exp_C1, 0.693359375);
_PS_CONST(cephes_exp_C2, -2.12194440e-4);

_PS_CONST(cephes_exp_p0, 1.9875691500E-4);
_PS_CONST(cephes_exp_p1, 1.3981999507E-3);
_PS_CONST(cephes_exp_p2, 8.3334519073E-3);
_PS_CONST(cephes_exp_p3, 4.1665795894E-2);
_PS_CONST(cephes_exp_p4, 1.6666665459E-1);
_PS_CONST(cephes_exp_p5, 5.0000001201E-1);

static v8sf exp256_ps(v8sf x) {
  v8sf tmp = _mm256_setzero_ps(), fx;
  v8si emm0;

  v8sf one = *(v8sf*)_ps_1;

  x = _mm256_min_ps(x, *(v8sf*)_ps_exp_hi);
  x = _mm256_max_ps(x, *(v8sf*)_ps_exp_lo);

  /* express exp(x) as exp(g + n*log(2)) */
  fx = _mm256_mul_ps(x, *(v8sf*)_ps_cephes_LOG2EF);
  fx = _mm256_add_ps(fx, *(v8sf*)_ps_0p5);

  /* how to perform a floorf with SSE: just below */
  emm0 = _mm256_cvttps_epi32(fx);
  tmp  = _mm256_cvtepi32_ps(emm0);
  /* if greater, substract 1 */
  v8sf mask = _mm256_cmp_ps(tmp, fx, _CMP_GT_OQ);    
  mask = _mm256_and_ps(mask, one);
  fx = _mm256_sub_ps(tmp, mask);

  tmp = _mm256_mul_ps(fx, *(v8sf*)_ps_cephes_exp_C1);
  v8sf z = _mm256_mul_ps(fx, *(v8sf*)_ps_cephes_exp_C2);
  x = _mm256_sub_ps(x, tmp);
  x = _mm256_sub_ps(x, z);

  z = _mm256_mul_ps(x,x);
  
  v8sf y = *(v8sf*)_ps_cephes_exp_p0;
  y = _mm256_mul_ps(y, x);
  y = _mm256_add_ps(y, *(v8sf*)_ps_cephes_exp_p1);
  y = _mm256_mul_ps(y, x);
  y = _mm256_add_ps(y, *(v8sf*)_ps_cephes_exp_p2);
  y = _mm256_mul_ps(y, x);
  y = _mm256_add_ps(y, *(v8sf*)_ps_cephes_exp_p3);
  y = _mm256_mul_ps(y, x);
  y = _mm256_add_ps(y, *(v8sf*)_ps_cephes_exp_p4);
  y = _mm256_mul_ps(y, x);
  y = _mm256_add_ps(y, *(v8sf*)_ps_cephes_exp_p5);
  y = _mm256_mul_ps(y, z);
  y = _mm256_add_ps(y, x);
  y = _mm256_add_ps(y, one);

  /* build 2^n */
  emm0 = _mm256_cvttps_epi32(fx);
  emm0 = _mm256_add_epi32(emm0, *(v8si*)_pi32_0x7f);
  emm0 = _mm256_slli_epi32(emm0, 23);
  v8sf pow2n = _mm256_castsi256_ps(emm0);

  y = _mm256_mul_ps(y, pow2n);
  return y;
}

_PS_CONST(ln10_precalc, 2.302585092994);

static v8sf log256_ps(v8sf x)
{
    return _mm256_div_ps (ln256_ps(x) , *( (v8sf*)_ps_ln10_precalc) );
}

_PS_CONST(ln2_precalc, 0.69314718056);
static v8sf log256_ps2(v8sf x)
{
    return _mm256_div_ps(ln256_ps(x), *((v8sf*)_ps_ln2_precalc));
}

static v8sf pow256_ps(v8sf x, v8sf y)
{
    return exp256_ps(_mm256_mul_ps (y, ln256_ps(x)));
}

_PS_CONST(minus_cephes_DP1, -0.78515625);
_PS_CONST(minus_cephes_DP2, -2.4187564849853515625e-4);
_PS_CONST(minus_cephes_DP3, -3.77489497744594108e-8);
_PS_CONST(sincof_p0, -1.9515295891E-4);
_PS_CONST(sincof_p1,  8.3321608736E-3);
_PS_CONST(sincof_p2, -1.6666654611E-1);
_PS_CONST(coscof_p0,  2.443315711809948E-005);
_PS_CONST(coscof_p1, -1.388731625493765E-003);
_PS_CONST(coscof_p2,  4.166664568298827E-002);
_PS_CONST(cephes_FOPI, 1.27323954473516); // 4 / M_PI


/* evaluation of 4 sines at onces, using only SSE1+MMX intrinsics so
   it runs also on old athlons XPs and the pentium III of your grand
   mother.

   The code is the exact rewriting of the cephes sinf function.
   Precision is excellent as long as x < 8192 (I did not bother to
   take into account the special handling they have for greater values
   -- it does not return garbage for arguments over 8192, though, but
   the extra precision is missing).

   Note that it is such that sinf((float)M_PI) = 8.74e-8, which is the
   surprising but correct result.

   Performance is also surprisingly good, 1.33 times faster than the
   macos vsinf SSE2 function, and 1.5 times faster than the
   __vrs4_sinf of amd's ACML (which is only available in 64 bits). Not
   too bad for an SSE1 function (with no special tuning) !
   However the latter libraries probably have a much better handling of NaN,
   Inf, denormalized and other special arguments..

   On my core 1 duo, the execution of this function takes approximately 95 cycles.

   From what I have observed on the experiments with Intel AMath lib, switching to an
   SSE2 version would improve the perf by only 10%.

   Since it is based on SSE intrinsics, it has to be compiled at -O2 to
   deliver full speed.
*/
static v8sf sin256_ps(v8sf x) { // any x
  v8sf xmm1, xmm2 = _mm256_setzero_ps(), xmm3, sign_bit, y;

  v8si emm0, emm2;

  sign_bit = x;
  /* take the absolute value */
  x = _mm256_and_ps(x, *(v8sf*)_ps_inv_sign_mask);
  /* extract the sign bit (upper one) */
  sign_bit = _mm256_and_ps(sign_bit, *(v8sf*)_ps_sign_mask);
  
  /* scale by 4/Pi */
  y = _mm256_mul_ps(x, *(v8sf*)_ps_cephes_FOPI);


  /* store the integer part of y in mm0 */
  emm2 = _mm256_cvttps_epi32(y);
  /* j=(j+1) & (~1) (see the cephes sources) */
  emm2 = _mm256_add_epi32(emm2, *(v8si*)_pi32_1);
  emm2 = _mm256_and_si256(emm2, *(v8si*)_pi32_inv1);
  y = _mm256_cvtepi32_ps(emm2);

  /* get the swap sign flag */
  emm0 = _mm256_and_si256(emm2, *(v8si*)_pi32_4);
  emm0 = _mm256_slli_epi32(emm0, 29);
  /* get the polynom selection mask 
     there is one polynom for 0 <= x <= Pi/4
     and another one for Pi/4<x<=Pi/2

     Both branches will be computed.
  */
  emm2 = _mm256_and_si256(emm2, *(v8si*)_pi32_2);
  emm2 = _mm256_cmpeq_epi32(emm2, _mm256_setzero_si256());
  
  v8sf swap_sign_bit = _mm256_castsi256_ps(emm0);
  v8sf poly_mask = _mm256_castsi256_ps(emm2);
  sign_bit = _mm256_xor_ps(sign_bit, swap_sign_bit);
  
  
  /* The magic pass: "Extended precision modular arithmetic" 
     x = ((x - y * DP1) - y * DP2) - y * DP3; */
  xmm1 = *(v8sf*)_ps_minus_cephes_DP1;
  xmm2 = *(v8sf*)_ps_minus_cephes_DP2;
  xmm3 = *(v8sf*)_ps_minus_cephes_DP3;
  xmm1 = _mm256_mul_ps(y, xmm1);
  xmm2 = _mm256_mul_ps(y, xmm2);
  xmm3 = _mm256_mul_ps(y, xmm3);
  x = _mm256_add_ps(x, xmm1);
  x = _mm256_add_ps(x, xmm2);
  x = _mm256_add_ps(x, xmm3);

  /* Evaluate the first polynom  (0 <= x <= Pi/4) */
  y = *(v8sf*)_ps_coscof_p0;
  v8sf z = _mm256_mul_ps(x,x);

  y = _mm256_mul_ps(y, z);
  y = _mm256_add_ps(y, *(v8sf*)_ps_coscof_p1);
  y = _mm256_mul_ps(y, z);
  y = _mm256_add_ps(y, *(v8sf*)_ps_coscof_p2);
  y = _mm256_mul_ps(y, z);
  y = _mm256_mul_ps(y, z);
  v8sf tmp = _mm256_mul_ps(z, *(v8sf*)_ps_0p5);
  y = _mm256_sub_ps(y, tmp);
  y = _mm256_add_ps(y, *(v8sf*)_ps_1);
  
  /* Evaluate the second polynom  (Pi/4 <= x <= 0) */

  v8sf y2 = *(v8sf*)_ps_sincof_p0;
  y2 = _mm256_mul_ps(y2, z);
  y2 = _mm256_add_ps(y2, *(v8sf*)_ps_sincof_p1);
  y2 = _mm256_mul_ps(y2, z);
  y2 = _mm256_add_ps(y2, *(v8sf*)_ps_sincof_p2);
  y2 = _mm256_mul_ps(y2, z);
  y2 = _mm256_mul_ps(y2, x);
  y2 = _mm256_add_ps(y2, x);

  /* select the correct result from the two polynoms */  
  xmm3 = poly_mask;
  y2 = _mm256_and_ps(xmm3, y2); //, xmm3);
  y = _mm256_andnot_ps(xmm3, y);
  y = _mm256_add_ps(y,y2);
  /* update the sign */
  y = _mm256_xor_ps(y, sign_bit);
  return y;
}

/* almost the same as sin_ps */
static v8sf cos256_ps(v8sf x) { // any x
  v8sf xmm1, xmm2 = _mm256_setzero_ps(), xmm3, y;
  v8si emm0, emm2;

  /* take the absolute value */
  x = _mm256_and_ps(x, *(v8sf*)_ps_inv_sign_mask);
  
  /* scale by 4/Pi */
  y = _mm256_mul_ps(x, *(v8sf*)_ps_cephes_FOPI);
  

  /* store the integer part of y in mm0 */
  emm2 = _mm256_cvttps_epi32(y);
  /* j=(j+1) & (~1) (see the cephes sources) */
  emm2 = _mm256_add_epi32(emm2, *(v8si*)_pi32_1);
  emm2 = _mm256_and_si256(emm2, *(v8si*)_pi32_inv1);
  y = _mm256_cvtepi32_ps(emm2);

  emm2 = _mm256_sub_epi32(emm2, *(v8si*)_pi32_2);
  
  /* get the swap sign flag */
  emm0 = _mm256_andnot_si256(emm2, *(v8si*)_pi32_4);
  emm0 = _mm256_slli_epi32(emm0, 29);
  /* get the polynom selection mask */
  emm2 = _mm256_and_si256(emm2, *(v8si*)_pi32_2);
  emm2 = _mm256_cmpeq_epi32(emm2, _mm256_setzero_si256());
  
  v8sf sign_bit = _mm256_castsi256_ps(emm0);
  v8sf poly_mask = _mm256_castsi256_ps(emm2);

  /* The magic pass: "Extended precision modular arithmetic" 
     x = ((x - y * DP1) - y * DP2) - y * DP3; */
  xmm1 = *(v8sf*)_ps_minus_cephes_DP1;
  xmm2 = *(v8sf*)_ps_minus_cephes_DP2;
  xmm3 = *(v8sf*)_ps_minus_cephes_DP3;
  xmm1 = _mm256_mul_ps(y, xmm1);
  xmm2 = _mm256_mul_ps(y, xmm2);
  xmm3 = _mm256_mul_ps(y, xmm3);
  x = _mm256_add_ps(x, xmm1);
  x = _mm256_add_ps(x, xmm2);
  x = _mm256_add_ps(x, xmm3);
  
  /* Evaluate the first polynom  (0 <= x <= Pi/4) */
  y = *(v8sf*)_ps_coscof_p0;
  v8sf z = _mm256_mul_ps(x,x);

  y = _mm256_mul_ps(y, z);
  y = _mm256_add_ps(y, *(v8sf*)_ps_coscof_p1);
  y = _mm256_mul_ps(y, z);
  y = _mm256_add_ps(y, *(v8sf*)_ps_coscof_p2);
  y = _mm256_mul_ps(y, z);
  y = _mm256_mul_ps(y, z);
  v8sf tmp = _mm256_mul_ps(z, *(v8sf*)_ps_0p5);
  y = _mm256_sub_ps(y, tmp);
  y = _mm256_add_ps(y, *(v8sf*)_ps_1);
  
  /* Evaluate the second polynom  (Pi/4 <= x <= 0) */

  v8sf y2 = *(v8sf*)_ps_sincof_p0;
  y2 = _mm256_mul_ps(y2, z);
  y2 = _mm256_add_ps(y2, *(v8sf*)_ps_sincof_p1);
  y2 = _mm256_mul_ps(y2, z);
  y2 = _mm256_add_ps(y2, *(v8sf*)_ps_sincof_p2);
  y2 = _mm256_mul_ps(y2, z);
  y2 = _mm256_mul_ps(y2, x);
  y2 = _mm256_add_ps(y2, x);

  /* select the correct result from the two polynoms */  
  xmm3 = poly_mask;
  y2 = _mm256_and_ps(xmm3, y2); //, xmm3);
  y = _mm256_andnot_ps(xmm3, y);
  y = _mm256_add_ps(y,y2);
  /* update the sign */
  y = _mm256_xor_ps(y, sign_bit);

  return y;
}

/* since sin_ps and cos_ps are almost identical, sincos_ps could replace both of them..
   it is almost as fast, and gives you a free cosine with your sine */
static void sincos256_ps(v8sf x, v8sf *s, v8sf *c) {
  v8sf xmm1, xmm2, xmm3 = _mm256_setzero_ps(), sign_bit_sin, y;

  v8si emm0, emm2, emm4;

  sign_bit_sin = x;
  /* take the absolute value */
  x = _mm256_and_ps(x, *(v8sf*)_ps_inv_sign_mask);
  /* extract the sign bit (upper one) */
  sign_bit_sin = _mm256_and_ps(sign_bit_sin, *(v8sf*)_ps_sign_mask);
  
  /* scale by 4/Pi */
  y = _mm256_mul_ps(x, *(v8sf*)_ps_cephes_FOPI);
    
  /* store the integer part of y in emm2 */
  emm2 = _mm256_cvttps_epi32(y);

  /* j=(j+1) & (~1) (see the cephes sources) */
  emm2 = _mm256_add_epi32(emm2, *(v8si*)_pi32_1);
  emm2 = _mm256_and_si256(emm2, *(v8si*)_pi32_inv1);
  y = _mm256_cvtepi32_ps(emm2);

  emm4 = emm2;

  /* get the swap sign flag for the sine */
  emm0 = _mm256_and_si256(emm2, *(v8si*)_pi32_4);
  emm0 = _mm256_slli_epi32(emm0, 29);
  v8sf swap_sign_bit_sin = _mm256_castsi256_ps(emm0);

  /* get the polynom selection mask for the sine*/
  emm2 = _mm256_and_si256(emm2, *(v8si*)_pi32_2);
  emm2 = _mm256_cmpeq_epi32(emm2, _mm256_setzero_si256());
  v8sf poly_mask = _mm256_castsi256_ps(emm2);


  /* The magic pass: "Extended precision modular arithmetic" 
     x = ((x - y * DP1) - y * DP2) - y * DP3; */
  xmm1 = *(v8sf*)_ps_minus_cephes_DP1;
  xmm2 = *(v8sf*)_ps_minus_cephes_DP2;
  xmm3 = *(v8sf*)_ps_minus_cephes_DP3;
  xmm1 = _mm256_mul_ps(y, xmm1);
  xmm2 = _mm256_mul_ps(y, xmm2);
  xmm3 = _mm256_mul_ps(y, xmm3);
  x = _mm256_add_ps(x, xmm1);
  x = _mm256_add_ps(x, xmm2);
  x = _mm256_add_ps(x, xmm3);


  emm4 = _mm256_sub_epi32(emm4, *(v8si*)_pi32_2);
  emm4 = _mm256_andnot_si256(emm4, *(v8si*)_pi32_4);
  emm4 = _mm256_slli_epi32(emm4, 29);
  v8sf sign_bit_cos = _mm256_castsi256_ps(emm4);


  sign_bit_sin = _mm256_xor_ps(sign_bit_sin, swap_sign_bit_sin);

  
  /* Evaluate the first polynom  (0 <= x <= Pi/4) */
  v8sf z = _mm256_mul_ps(x,x);
  y = *(v8sf*)_ps_coscof_p0;

  y = _mm256_mul_ps(y, z);
  y = _mm256_add_ps(y, *(v8sf*)_ps_coscof_p1);
  y = _mm256_mul_ps(y, z);
  y = _mm256_add_ps(y, *(v8sf*)_ps_coscof_p2);
  y = _mm256_mul_ps(y, z);
  y = _mm256_mul_ps(y, z);
  v8sf tmp = _mm256_mul_ps(z, *(v8sf*)_ps_0p5);
  y = _mm256_sub_ps(y, tmp);
  y = _mm256_add_ps(y, *(v8sf*)_ps_1);
  
  /* Evaluate the second polynom  (Pi/4 <= x <= 0) */

  v8sf y2 = *(v8sf*)_ps_sincof_p0;
  y2 = _mm256_mul_ps(y2, z);
  y2 = _mm256_add_ps(y2, *(v8sf*)_ps_sincof_p1);
  y2 = _mm256_mul_ps(y2, z);
  y2 = _mm256_add_ps(y2, *(v8sf*)_ps_sincof_p2);
  y2 = _mm256_mul_ps(y2, z);
  y2 = _mm256_mul_ps(y2, x);
  y2 = _mm256_add_ps(y2, x);

  /* select the correct result from the two polynoms */  
  xmm3 = poly_mask;
  v8sf ysin2 = _mm256_and_ps(xmm3, y2);
  v8sf ysin1 = _mm256_andnot_ps(xmm3, y);
  y2 = _mm256_sub_ps(y2,ysin2);
  y = _mm256_sub_ps(y, ysin1);

  xmm1 = _mm256_add_ps(ysin1,ysin2);
  xmm2 = _mm256_add_ps(y,y2);
 
  /* update the sign */
  *s = _mm256_xor_ps(xmm1, sign_bit_sin);
  *c = _mm256_xor_ps(xmm2, sign_bit_cos);
}

#endif