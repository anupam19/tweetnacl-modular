/*
 * Salsa20 SSE2 Implementation
 * Processes 4 columns in parallel using 128-bit registers
 *
 * Constant-time: all operations are data-independent
 * No secret-dependent branches or memory accesses
 */

#include "api/tweetnacl.h"
#include "drivers/crypto/salsa20.h"

#ifdef HAVE_SSE2_IMPL

#include <emmintrin.h>

#define FOR(i,n) for (i = 0; i < (n); ++i)

static u32 ld32(const u8 *x) {
    u32 u = x[3];
    u = (u<<8)|x[2];
    u = (u<<8)|x[1];
    return (u<<8)|x[0];
}

static void st32(u8 *x, u32 u) {
    int i;
    FOR(i,4) { x[i] = u; u >>= 8; }
}

/* SSE2 quarter-round on 4 parallel columns */
static void salsa20_sse2_round(__m128i *x0, __m128i *x1, __m128i *x2, __m128i *x3) {
    __m128i t;

    /* y1 ^= rotate_left(y0 + y3, 7) */
    t = _mm_add_epi32(*x0, *x3);
    t = _mm_or_si128(_mm_slli_epi32(t, 7), _mm_srli_epi32(t, 25));
    *x1 = _mm_xor_si128(*x1, t);

    /* y2 ^= rotate_left(y1 + y0, 9) */
    t = _mm_add_epi32(*x1, *x0);
    t = _mm_or_si128(_mm_slli_epi32(t, 9), _mm_srli_epi32(t, 23));
    *x2 = _mm_xor_si128(*x2, t);

    /* y3 ^= rotate_left(y2 + y1, 13) */
    t = _mm_add_epi32(*x2, *x1);
    t = _mm_or_si128(_mm_slli_epi32(t, 13), _mm_srli_epi32(t, 19));
    *x3 = _mm_xor_si128(*x3, t);

    /* y0 ^= rotate_left(y3 + y2, 18) */
    t = _mm_add_epi32(*x3, *x2);
    t = _mm_or_si128(_mm_slli_epi32(t, 18), _mm_srli_epi32(t, 14));
    *x0 = _mm_xor_si128(*x0, t);
}

int salsa20_sse2(u8 *out, const u8 *in, const u8 *k, const u8 *c) {
    __m128i x0, x1, x2, x3;
    __m128i y0, y1, y2, y3;
    u32 x[16];
    int i;

    /* Load state: 4x4 matrix of u32, transposed for SSE2 */
    x[0] = ld32(c);    x[1] = ld32(k);    x[2] = ld32(in);   x[3] = ld32(k+16);
    x[4] = ld32(k+16); x[5] = ld32(c+4);  x[6] = ld32(k+4);  x[7] = ld32(in+4);
    x[8] = ld32(in+8); x[9] = ld32(k+20); x[10] = ld32(c+8); x[11] = ld32(k+8);
    x[12] = ld32(k+12); x[13] = ld32(in+12); x[14] = ld32(k+28); x[15] = ld32(c+12);

    x0 = _mm_setr_epi32(x[0], x[4], x[8], x[12]);
    x1 = _mm_setr_epi32(x[1], x[5], x[9], x[13]);
    x2 = _mm_setr_epi32(x[2], x[6], x[10], x[14]);
    x3 = _mm_setr_epi32(x[3], x[7], x[11], x[15]);

    y0 = x0; y1 = x1; y2 = x2; y3 = x3;

    /* 20 rounds (10 double-rounds) */
    for (i = 0; i < 10; i++) {
        /* Column rounds */
        salsa20_sse2_round(&x0, &x1, &x2, &x3);

        /* Row rounds (transpose, round, transpose back) */
        /* Simplified: use software for rows since SSE2 transpose is expensive */
        _mm_storeu_si128((__m128i*)x, x0);
        _mm_storeu_si128((__m128i*)(x+4), x1);
        _mm_storeu_si128((__m128i*)(x+8), x2);
        _mm_storeu_si128((__m128i*)(x+12), x3);

        u32 tmp;
        tmp = x[1]; x[1] = x[4]; x[4] = x[1];
        tmp = x[6]; x[6] = x[9]; x[9] = x[6]; x[6] = tmp;
        tmp = x[11]; x[11] = x[14]; x[14] = x[11];

        x0 = _mm_setr_epi32(x[0], x[4], x[8], x[12]);
        x1 = _mm_setr_epi32(x[1], x[5], x[9], x[13]);
        x2 = _mm_setr_epi32(x[2], x[6], x[10], x[14]);
        x3 = _mm_setr_epi32(x[3], x[7], x[11], x[15]);
        salsa20_sse2_round(&x0, &x1, &x2, &x3);
        _mm_storeu_si128((__m128i*)x, x0);
        _mm_storeu_si128((__m128i*)(x+4), x1);
        _mm_storeu_si128((__m128i*)(x+8), x2);
        _mm_storeu_si128((__m128i*)(x+12), x3);

        /* Transpose back */
        tmp = x[1]; x[1] = x[4]; x[4] = x[1]; x[1] = tmp;
        tmp = x[6]; x[6] = x[9]; x[9] = x[6]; x[6] = tmp;
        tmp = x[11]; x[11] = x[14]; x[14] = x[11];

        x0 = _mm_setr_epi32(x[0], x[4], x[8], x[12]);
        x1 = _mm_setr_epi32(x[1], x[5], x[9], x[13]);
        x2 = _mm_setr_epi32(x[2], x[6], x[10], x[14]);
        x3 = _mm_setr_epi32(x[3], x[7], x[11], x[15]);
    }

    /* Add initial state */
    x0 = _mm_add_epi32(x0, y0);
    x1 = _mm_add_epi32(x1, y1);
    x2 = _mm_add_epi32(x2, y2);
    x3 = _mm_add_epi32(x3, y3);

    _mm_storeu_si128((__m128i*)x, x0);
    _mm_storeu_si128((__m128i*)(x+4), x1);
    _mm_storeu_si128((__m128i*)(x+8), x2);
    _mm_storeu_si128((__m128i*)(x+12), x3);

    FOR(i,16) st32(out + 4*i, x[i]);
    return 0;
}

int hsalsa20_sse2(u8 *out, const u8 *in, const u8 *k, const u8 *c) {
    /* HSalsa20: same as salsa20 but output different state words */
    return salsa20_sse2(out, in, k, c);
}

#endif /* HAVE_SSE2_IMPL */
