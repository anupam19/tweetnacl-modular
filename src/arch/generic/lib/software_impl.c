/*
 * Salsa20/HSalsa20 Software Implementation
 * Pure software version — always available as fallback
 *
 * Constant-time: all operations are data-independent
 */

#include "drivers/crypto/salsa20.h"
#include "tweetnacl/tweetnacl.h"

#define FOR(i, n) for (i = 0; i < (n); ++i)

static u32 L32(u32 x, int c) { return (x << c) | ((x & 0xffffffff) >> (32 - c)); }
static u32 ld32(const u8 *x) {
    u32 u = x[3];
    u = (u << 8) | x[2];
    u = (u << 8) | x[1];
    return (u << 8) | x[0];
}
static void st32(u8 *x, u32 u) {
    int i;
    FOR(i, 4) {
        x[i] = u;
        u >>= 8;
    }
}
