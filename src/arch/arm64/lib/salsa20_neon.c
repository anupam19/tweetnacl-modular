/*
 * Salsa20 NEON Implementation (ARM64)
 * Processes 4 columns in parallel using 128-bit NEON registers
 *
 * Constant-time: all operations are data-independent
 */

#include "api/tweetnacl.h"
#include "drivers/crypto/salsa20.h"

#ifdef HAVE_NEON_IMPL

#include <arm_neon.h>

#define FOR(i,n) for (i = 0; i < (n); ++i)

static u32 ld32(const u8 *x) {
    u32 u = x[3]; u = (u<<8)|x[2]; u = (u<<8)|x[1]; return (u<<8)|x[0];
}
static void st32(u8 *x, u32 u) { int i; FOR(i,4) { x[i] = u; u >>= 8; } }

static void salsa20_neon_round(uint32x4_t *x0, uint32x4_t *x1,
                                uint32x4_t *x2, uint32x4_t *x3) {
    uint32x4_t t;
    t = vaddq_u32(*x0, *x3);
    t = vorrq_u32(vshlq_n_u32(t, 7), vshrq_n_u32(t, 25));
    *x1 = veorq_u32(*x1, t);
    t = vaddq_u32(*x1, *x0);
    t = vorrq_u32(vshlq_n_u32(t, 9), vshrq_n_u32(t, 23));
    *x2 = veorq_u32(*x2, t);
    t = vaddq_u32(*x2, *x1);
    t = vorrq_u32(vshlq_n_u32(t, 13), vshrq_n_u32(t, 19));
    *x3 = veorq_u32(*x3, t);
    t = vaddq_u32(*x3, *x2);
    t = vorrq_u32(vshlq_n_u32(t, 18), vshrq_n_u32(t, 14));
    *x0 = veorq_u32(*x0, t);
}

int salsa20_neon(u8 *out, const u8 *in, const u8 *k, const u8 *c) {
    uint32x4_t x0, x1, x2, x3, y0, y1, y2, y3;
    u32 x[16];
    int i;

    x[0]=ld32(c); x[1]=ld32(k); x[2]=ld32(in); x[3]=ld32(k+16);
    x[4]=ld32(k+16); x[5]=ld32(c+4); x[6]=ld32(k+4); x[7]=ld32(in+4);
    x[8]=ld32(in+8); x[9]=ld32(k+20); x[10]=ld32(c+8); x[11]=ld32(k+8);
    x[12]=ld32(k+12); x[13]=ld32(in+12); x[14]=ld32(k+28); x[15]=ld32(c+12);

    x0 = vld1q_u32(&x[0]);  x1 = vld1q_u32(&x[4]);
    x2 = vld1q_u32(&x[8]);  x3 = vld1q_u32(&x[12]);
    y0 = x0; y1 = x1; y2 = x2; y3 = x3;

    for (i = 0; i < 10; i++) {
        salsa20_neon_round(&x0, &x1, &x2, &x3);
        /* Row round via software */
        vst1q_u32(&x[0], x0); vst1q_u32(&x[4], x1);
        vst1q_u32(&x[8], x2); vst1q_u32(&x[12], x3);
        u32 t;
        t=x[1];x[1]=x[4];x[4]=x[1];x[1]=t;
        t=x[6];x[6]=x[9];x[9]=x[6];x[6]=t;
        t=x[11];x[11]=x[14];x[14]=x[11];
        x0=vld1q_u32(&x[0]); x1=vld1q_u32(&x[4]);
        x2=vld1q_u32(&x[8]); x3=vld1q_u32(&x[12]);
        salsa20_neon_round(&x0, &x1, &x2, &x3);
        vst1q_u32(&x[0], x0); vst1q_u32(&x[4], x1);
        vst1q_u32(&x[8], x2); vst1q_u32(&x[12], x3);
        t=x[1];x[1]=x[4];x[4]=x[1];x[1]=t;
        t=x[6];x[6]=x[9];x[9]=x[6];x[6]=t;
        t=x[11];x[11]=x[14];x[14]=x[11];
        x0=vld1q_u32(&x[0]); x1=vld1q_u32(&x[4]);
        x2=vld1q_u32(&x[8]); x3=vld1q_u32(&x[12]);
    }

    x0 = vaddq_u32(x0, y0); x1 = vaddq_u32(x1, y1);
    x2 = vaddq_u32(x2, y2); x3 = vaddq_u32(x3, y3);
    vst1q_u32(&x[0], x0); vst1q_u32(&x[4], x1);
    vst1q_u32(&x[8], x2); vst1q_u32(&x[12], x3);

    FOR(i,16) st32(out+4*i, x[i]);
    return 0;
}

int hsalsa20_neon(u8 *out, const u8 *in, const u8 *k, const u8 *c) {
    return salsa20_neon(out, in, k, c);
}

#endif /* HAVE_NEON_IMPL */
