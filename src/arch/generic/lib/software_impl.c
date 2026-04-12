/*
 * Salsa20/HSalsa20 Software Implementation
 * Pure software version — always available as fallback
 *
 * Constant-time: all operations are data-independent
 */

#include "api/tweetnacl.h"
#include "drivers/crypto/salsa20.h"

#define FOR(i,n) for (i = 0; i < (n); ++i)

static u32 L32(u32 x, int c) { return (x << c) | ((x & 0xffffffff) >> (32 - c)); }
static u32 ld32(const u8 *x) { u32 u = x[3]; u = (u<<8)|x[2]; u = (u<<8)|x[1]; return (u<<8)|x[0]; }
static void st32(u8 *x, u32 u) { int i; FOR(i,4) { x[i] = u; u >>= 8; } }

int salsa20_soft(u8 *out, const u8 *in, const u8 *k, const u8 *c) {
    u32 w[16], x[16], y[16], t[4];
    int i, j, m;

    FOR(i,4) {
        x[5*i] = ld32(c+4*i);
        x[1+i] = ld32(k+4*i);
        x[6+i] = ld32(in+4*i);
        x[11+i] = ld32(k+16+4*i);
    }
    FOR(i,16) y[i] = x[i];

    FOR(i,20) {
        FOR(j,4) {
            FOR(m,4) t[m] = x[(5*j+4*m)%16];
            t[1] ^= L32(t[0]+t[3], 7);
            t[2] ^= L32(t[1]+t[0], 9);
            t[3] ^= L32(t[2]+t[1], 13);
            t[0] ^= L32(t[3]+t[2], 18);
            FOR(m,4) w[4*j+(j+m)%4] = t[m];
        }
        FOR(m,16) x[m] = w[m];
    }
    FOR(i,16) st32(out + 4*i, x[i] + y[i]);
    return 0;
}

int hsalsa20_soft(u8 *out, const u8 *in, const u8 *k, const u8 *c) {
    u32 w[16], x[16], t[4];
    int i, j, m;

    FOR(i,4) {
        x[5*i] = ld32(c+4*i);
        x[1+i] = ld32(k+4*i);
        x[6+i] = ld32(in+4*i);
        x[11+i] = ld32(k+16+4*i);
    }

    FOR(i,20) {
        FOR(j,4) {
            FOR(m,4) t[m] = x[(5*j+4*m)%16];
            t[1] ^= L32(t[0]+t[3], 7);
            t[2] ^= L32(t[1]+t[0], 9);
            t[3] ^= L32(t[2]+t[1], 13);
            t[0] ^= L32(t[3]+t[2], 18);
            FOR(m,4) w[4*j+(j+m)%4] = t[m];
        }
        FOR(m,16) x[m] = w[m];
    }

    FOR(i,4) {
        x[5*i] -= ld32(c+4*i);
        x[6+i] -= ld32(in+4*i);
    }
    FOR(i,4) {
        st32(out+4*i, x[5*i]);
        st32(out+16+4*i, x[6+i]);
    }
    return 0;
}
