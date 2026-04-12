/*
 * Salsa20 AVX2 Implementation
 * Processes 8 columns in parallel using 256-bit registers
 *
 * Constant-time: all operations are data-independent
 */

#include "api/tweetnacl.h"
#include "drivers/crypto/salsa20.h"

#ifdef HAVE_AVX2_IMPL

#include <immintrin.h>

#define FOR(i,n) for (i = 0; i < (n); ++i)

static u32 ld32(const u8 *x) {
    u32 u = x[3]; u = (u<<8)|x[2]; u = (u<<8)|x[1]; return (u<<8)|x[0];
}
static void st32(u8 *x, u32 u) { int i; FOR(i,4) { x[i] = u; u >>= 8; } }

/* AVX2 round: process 8 columns in parallel */
static void salsa20_avx2_round(__m256i *x0, __m256i *x1, __m256i *x2, __m256i *x3) {
    __m256i t;
    t = _mm256_add_epi32(*x0, *x3);
    t = _mm256_or_si256(_mm256_slli_epi32(t, 7), _mm256_srli_epi32(t, 25));
    *x1 = _mm256_xor_si256(*x1, t);
    t = _mm256_add_epi32(*x1, *x0);
    t = _mm256_or_si256(_mm256_slli_epi32(t, 9), _mm256_srli_epi32(t, 23));
    *x2 = _mm256_xor_si256(*x2, t);
    t = _mm256_add_epi32(*x2, *x1);
    t = _mm256_or_si256(_mm256_slli_epi32(t, 13), _mm256_srli_epi32(t, 19));
    *x3 = _mm256_xor_si256(*x3, t);
    t = _mm256_add_epi32(*x3, *x2);
    t = _mm256_or_si256(_mm256_slli_epi32(t, 18), _mm256_srli_epi32(t, 14));
    *x0 = _mm256_xor_si256(*x0, t);
}

int salsa20_avx2(u8 *out, const u8 *in, const u8 *k, const u8 *c) {
    u32 x[16];
    __m256i x0, x1, x2, x3, y0, y1, y2, y3;
    int i;

    x[0]=ld32(c); x[1]=ld32(k); x[2]=ld32(in); x[3]=ld32(k+16);
    x[4]=ld32(k+16); x[5]=ld32(c+4); x[6]=ld32(k+4); x[7]=ld32(in+4);
    x[8]=ld32(in+8); x[9]=ld32(k+20); x[10]=ld32(c+8); x[11]=ld32(k+8);
    x[12]=ld32(k+12); x[13]=ld32(in+12); x[14]=ld32(k+28); x[15]=ld32(c+12);

    x0 = _mm256_setr_epi32(x[0],x[4],x[8],x[12],x[0],x[4],x[8],x[12]);
    x1 = _mm256_setr_epi32(x[1],x[5],x[9],x[13],x[1],x[5],x[9],x[13]);
    x2 = _mm256_setr_epi32(x[2],x[6],x[10],x[14],x[2],x[6],x[10],x[14]);
    x3 = _mm256_setr_epi32(x[3],x[7],x[11],x[15],x[3],x[7],x[11],x[15]);

    y0=x0; y1=x1; y2=x2; y3=x3;

    for (i=0; i<10; i++) {
        salsa20_avx2_round(&x0,&x1,&x2,&x3);
        /* Row round (software for simplicity) */
        _mm256_storeu_si256((__m256i*)x, x0);
        _mm256_storeu_si256((__m256i*)(x+8), x1);
        u32 t;
        t=x[1];x[1]=x[4];x[4]=x[1];x[1]=t;
        t=x[6];x[6]=x[9];x[9]=x[6];x[6]=t;
        t=x[11];x[11]=x[14];x[14]=x[11];
        x0=_mm256_setr_epi32(x[0],x[4],x[8],x[12],0,0,0,0);
        x1=_mm256_setr_epi32(x[1],x[5],x[9],x[13],0,0,0,0);
        x2=_mm256_setr_epi32(x[2],x[6],x[10],x[14],0,0,0,0);
        x3=_mm256_setr_epi32(x[3],x[7],x[11],x[15],0,0,0,0);
        salsa20_avx2_round(&x0,&x1,&x2,&x3);
        _mm256_storeu_si256((__m256i*)x, x0);
        _mm256_storeu_si256((__m256i*)(x+8), x1);
        t=x[1];x[1]=x[4];x[4]=x[1];x[1]=t;
        t=x[6];x[6]=x[9];x[9]=x[6];x[6]=t;
        t=x[11];x[11]=x[14];x[14]=x[11];
        x0=_mm256_setr_epi32(x[0],x[4],x[8],x[12],0,0,0,0);
        x1=_mm256_setr_epi32(x[1],x[5],x[9],x[13],0,0,0,0);
        x2=_mm256_setr_epi32(x[2],x[6],x[10],x[14],0,0,0,0);
        x3=_mm256_setr_epi32(x[3],x[7],x[11],x[15],0,0,0,0);
    }

    x0=_mm256_add_epi32(x0,y0); x1=_mm256_add_epi32(x1,y1);
    x2=_mm256_add_epi32(x2,y2); x3=_mm256_add_epi32(x3,y3);

    _mm256_storeu_si256((__m256i*)x, x0);
    _mm256_storeu_si256((__m256i*)(x+8), x1);

    FOR(i,16) st32(out+4*i, x[i]);
    return 0;
}

int hsalsa20_avx2(u8 *out, const u8 *in, const u8 *k, const u8 *c) {
    return salsa20_avx2(out, in, k, c);
}

#endif /* HAVE_AVX2_IMPL */
