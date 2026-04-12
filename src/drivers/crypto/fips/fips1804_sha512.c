/*
 * FIPS 180-4 SHA-512/256 and SHA-512/224 Implementation
 * Same as SHA-512 but with different initial hash values (FIPS 180-4 Section 6.7)
 */

#include "drivers/crypto/fips/fips1804_sha512.h"
#include "api/tweetnacl.h"

#define FOR(i,n) for (i = 0; i < (n); ++i)

static u64 R(u64 x, int c) { return (x >> c) | (x << (64 - c)); }
static u64 Ch(u64 x, u64 y, u64 z) { return (x & y) ^ (~x & z); }
static u64 Maj(u64 x, u64 y, u64 z) { return (x & y) ^ (x & z) ^ (y & z); }
static u64 Sigma0(u64 x) { return R(x, 28) ^ R(x, 34) ^ R(x, 39); }
static u64 Sigma1(u64 x) { return R(x, 14) ^ R(x, 18) ^ R(x, 41); }
static u64 sigma0(u64 x) { return R(x, 1) ^ R(x, 8) ^ (x >> 7); }
static u64 sigma1(u64 x) { return R(x, 19) ^ R(x, 61) ^ (x >> 6); }

static const u64 K[80] = {
    0x428a2f98d728ae22ULL, 0x7137449123ef65cdULL, 0xb5c0fbcfec4d3b2fULL, 0xe9b5dba58189dbbcULL,
    0x3956c25bf348b538ULL, 0x59f111f1b605d019ULL, 0x923f82a4af194f9bULL, 0xab1c5ed5da6d8118ULL,
    0xd807aa98a3030242ULL, 0x12835b0145706fbeULL, 0x243185be4ee4b28cULL, 0x550c7dc3d5ffb4e2ULL,
    0x72be5d74f27b896fULL, 0x80deb1fe3b1696b1ULL, 0x9bdc06a725c71235ULL, 0xc19bf174cf692694ULL,
    0xe49b69c19ef14ad2ULL, 0xefbe4786384f25e3ULL, 0x0fc19dc68b8cd5b5ULL, 0x240ca1cc77ac9c65ULL,
    0x2de92c6f592b0275ULL, 0x4a7484aa6ea6e483ULL, 0x5cb0a9dcbd41fbd4ULL, 0x76f988da831153b5ULL,
    0x983e5152ee66dfabULL, 0xa831c66d2db43210ULL, 0xb00327c898fb213fULL, 0xbf597fc7beef0ee4ULL,
    0xc6e00bf33da88fc2ULL, 0xd5a79147930aa725ULL, 0x06ca6351e003826fULL, 0x142929670a0e6e70ULL,
    0x27b70a8546d22ffcULL, 0x2e1b21385c26c926ULL, 0x4d2c6dfc5ac42aedULL, 0x53380d139d95b3dfULL,
    0x650a73548baf63deULL, 0x766a0abb3c77b2a8ULL, 0x81c2c92e47edaee6ULL, 0x92722c851482353bULL,
    0xa2bfe8a14cf10364ULL, 0xa81a664bbc423001ULL, 0xc24b8b70d0f89791ULL, 0xc76c51a30654be30ULL,
    0xd192e819d6ef5218ULL, 0xd69906245565a910ULL, 0xf40e35855771202aULL, 0x106aa07032bbd1b8ULL,
    0x19a4c116b8d2d0c8ULL, 0x1e376c085141ab53ULL, 0x2748774cdf8eeb99ULL, 0x34b0bcb5e19b48a8ULL,
    0x391c0cb3c5c95a63ULL, 0x4ed8aa4ae3418acbULL, 0x5b9cca4f7763e373ULL, 0x682e6ff3d6b2b8a3ULL,
    0x748f82ee5defb2fcULL, 0x78a5636f43172f60ULL, 0x84c87814a1f0ab72ULL, 0x8cc702081a6439ecULL,
    0x90befffa23631e28ULL, 0xa4506cebde82bde9ULL, 0xbef9a3f7b2c67915ULL, 0xc67178f2e372532bULL,
    0xca273eceea26619cULL, 0xd186b8c721c0c207ULL, 0xeada7dd6cde0eb1eULL, 0xf57d4f7fee6ed178ULL,
    0x06f067aa72176fbaULL, 0x0a637dc5a2c898a6ULL, 0x113f9804bef90daeULL, 0x1b710b35131c471bULL,
    0x28db77f523047d84ULL, 0x32caab7b40c72493ULL, 0x3c9ebe0a15c9bebcULL, 0x431d67c49c100d4cULL,
    0x4cc5d4becb3e42b6ULL, 0x597f299cfc657e2aULL, 0x5fcb6fab3ad6faecULL, 0x6c44198c4a475817ULL
};

/* SHA-512/256 initial hash values (FIPS 180-4 Section 6.7.2) */
/* Stored as 8 big-endian u64 values */
static const uint8_t iv_sha512_256[64] = {
    0x22, 0x31, 0x85, 0x46, 0x96, 0x36, 0x79, 0x91,
    0xc1, 0x05, 0x9e, 0xd6, 0xd8, 0xe6, 0x6f, 0x55,
    0x13, 0xb0, 0x1c, 0x7e, 0x12, 0xa8, 0x84, 0xe8,
    0x91, 0xa2, 0x9c, 0x6b, 0x27, 0x22, 0x0a, 0x83,
    0x16, 0x82, 0x34, 0x16, 0x8a, 0x70, 0x59, 0x97,
    0x88, 0x35, 0x75, 0x5e, 0x41, 0x88, 0x09, 0xa1,
    0xc8, 0x66, 0x6d, 0x49, 0xef, 0x48, 0x44, 0xa3,
    0xe2, 0x50, 0xe2, 0x9b, 0x6b, 0x6f, 0x98, 0x2d
};

/* SHA-512/224 initial hash values (FIPS 180-4 Section 6.7.1) */
static const uint8_t iv_sha512_224[64] = {
    0x8c, 0x3d, 0x37, 0xc8, 0x19, 0x54, 0x4d, 0xa2,
    0x73, 0xfa, 0xa8, 0xa2, 0xc5, 0x62, 0x10, 0x4f,
    0xf4, 0x3c, 0x2c, 0x95, 0x91, 0x70, 0x39, 0x19,
    0xb6, 0x20, 0x75, 0x6b, 0x8e, 0xd7, 0x52, 0x34,
    0x8e, 0xc5, 0xb9, 0x84, 0xb2, 0x59, 0x0c, 0x04,
    0x1c, 0xd8, 0xb7, 0x68, 0xba, 0x26, 0xc8, 0x59,
    0xe7, 0x5d, 0x95, 0x8f, 0x5f, 0x58, 0xbc, 0x3d,
    0x6b, 0xba, 0x4a, 0x92, 0x1d, 0xc8, 0xc1, 0x34
};

static u64 dl64(const u8 *x) {
    u64 u = 0; int i;
    FOR(i, 8) u = (u << 8) | x[i];
    return u;
}

static void ts64(u8 *x, u64 u) {
    int i;
    for (i = 7; i >= 0; --i) { x[i] = u; u >>= 8; }
}

static void sha512_blocks(u8 *x, const u8 *m, u64 n, const uint8_t *iv) {
    u64 z[8], a[8], w[16], t;
    int i, j;

    FOR(i, 8) z[i] = a[i] = dl64(iv + 8 * i);

    while (n >= 128) {
        FOR(i, 16) w[i] = dl64(m + 8 * i);
        FOR(i, 80) {
            FOR(j, 8) a[j] = z[j];
            t = a[7] + Sigma1(a[4]) + Ch(a[4], a[5], a[6]) + K[i] + w[i % 16];
            a[7] = t + Sigma0(a[0]) + Maj(a[0], a[1], a[2]);
            a[3] += t;
            FOR(j, 8) z[(j + 1) % 8] = a[j];
            if (i % 16 == 15)
                FOR(j, 16)
                    w[j] += w[(j + 9) % 16] + sigma0(w[(j + 1) % 16]) + sigma1(w[(j + 14) % 16]);
        }
        FOR(i, 8) { a[i] += z[i]; z[i] = a[i]; }
        m += 128;
        n -= 128;
    }
    FOR(i, 8) ts64(x + 8 * i, z[i]);
}

static int hash_with_iv(uint8_t *out, const uint8_t *m, uint64_t n,
                        const uint8_t *iv, size_t outlen) {
    uint8_t h[64], x[256];
    uint64_t b = n;
    uint64_t i;

    sha512_blocks(h, m, n, iv);
    m += n;
    n &= 127;
    m -= n;

    FOR(i, 256) x[i] = 0;
    FOR(i, n) x[i] = m[i];
    x[n] = 128;
    n = 256 - 128 * (n < 112);
    x[n - 9] = b >> 61;
    ts64(x + n - 8, b << 3);
    sha512_blocks(h, x, n, iv);

    FOR(i, outlen) out[i] = h[i];
    return 0;
}

int crypto_hash_sha512_256(uint8_t *out, const uint8_t *in, uint64_t inlen) {
    return hash_with_iv(out, in, inlen, iv_sha512_256, 32);
}

int crypto_hash_sha512_224(uint8_t *out, const uint8_t *in, uint64_t inlen) {
    return hash_with_iv(out, in, inlen, iv_sha512_224, 28);
}
