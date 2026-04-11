# TweetNaCl-Modular API Reference

**Last Updated:** 2025-01-15  
**Version:** 2.0.0

---

## Table of Contents

1. [Naming Conventions](#1-naming-conventions)
2. [Core Functions](#2-core-functions)
3. [Secure Memory API](#3-secure-memory-api)
4. [Random Bytes API](#4-random-bytes-api)
5. [PQC API](#5-pqc-api)
6. [Type Definitions](#6-type-definitions)
7. [Constants Reference](#7-constants-reference)

---

## 1. Naming Conventions

### 1.1 Function Naming Pattern

All cryptographic functions follow the pattern:

```
crypto_<primitive>_<operation>
```

| Component | Description | Examples |
|-----------|-------------|----------|
| `crypto_` | Prefix for all cryptographic functions | `crypto_box`, `crypto_sign` |
| `<primitive>` | Algorithm or construction name | `box`, `sign`, `secretbox`, `hash` |
| `<operation>` | Action performed | `keypair`, `encrypt`, `verify`, `open` |

### 1.2 Size Constant Suffixes

```
<PRIMITIVE>_<SIZE_TYPE>_BYTES
```

| Suffix | Meaning | Example |
|--------|---------|---------|
| `_BYTES` | General size (output, message) | `crypto_hash_BYTES` |
| `_KEYBYTES` | Key size | `crypto_secretbox_KEYBYTES` |
| `_NONCEBYTES` | Nonce/IV size | `crypto_box_NONCEBYTES` |
| `_PUBLICKEYBYTES` | Public key size | `crypto_sign_PUBLICKEYBYTES` |
| `_SECRETKEYBYTES` | Secret key size | `crypto_box_SECRETKEYBYTES` |
| `_SIGNATUREBYTES` | Signature size | `crypto_sign_BYTES` |

### 1.3 Algorithm String Constants

```
<PRIMITIVE>_<VERSION>
```

Example:
```c
#define TWEETNACL_VERSION "2.0.0"
#define TWEETNACL_PRIMITIVE_BOX "curve25519-xsalsa20-poly1305"
```

---

## 2. Core Functions

### 2.1 crypto_box - Public-Key Encryption

**Algorithm:** Curve25519 + XSalsa20 + Poly1305

#### crypto_box_keypair

```c
int crypto_box_keypair(
    u8 *pk,           /* out: 32-byte public key */
    u8 *sk            /* out: 32-byte secret key */
) __attribute__((nonnull(1, 2)));
```

| Parameter | Direction | Size | Description |
|-----------|-----------|------|-------------|
| `pk` | Output | 32 bytes | Public key |
| `sk` | Output | 32 bytes | Secret key (protect carefully) |

**Return Value:** `0` on success, `-1` on failure (RNG error)

**Constant-Time:** Partial (key generation uses RNG, but internal operations are CT)

**Thread Safety:** Yes (assumes thread-safe `randombytes()`)

**Stack Usage:** ~64 bytes

**Example:**
```c
u8 pk[32], sk[32];
if (crypto_box_keypair(pk, sk) != 0) {
    fprintf(stderr, "Key generation failed\n");
    return -1;
}
/* Use keys... */
secure_zero(sk, sizeof(sk));  /* WIPE secret key when done */
```

#### crypto_box

```c
int crypto_box(
    u8 *c,                    /* out: ciphertext (mlen bytes) */
    const u8 *m,              /* in: plaintext (mlen bytes) */
    unsigned long long mlen,  /* in: message length */
    const u8 *n,              /* in: 24-byte nonce */
    const u8 *pk,             /* in: recipient's public key */
    const u8 *sk              /* in: sender's secret key */
) __attribute__((nonnull(1, 2, 3, 4, 5)));
```

| Parameter | Direction | Size | Description |
|-----------|-----------|------|-------------|
| `c` | Output | `mlen` bytes | Ciphertext |
| `m` | Input | `mlen` bytes | Plaintext (first 32 bytes can be zero padding) |
| `mlen` | Input | - | Message length (must be ≥32 for padding) |
| `n` | Input | 24 bytes | Unique nonce (never repeat with same key pair) |
| `pk` | Input | 32 bytes | Recipient's public key |
| `sk` | Input | 32 bytes | Sender's secret key |

**Return Value:** `0` on success, `-1` on failure

**Constant-Time:** Yes

**Thread Safety:** Yes

**Stack Usage:** ~2048 bytes

**Preconditions:**
- `mlen >= 32` (for zero padding)
- Nonce must be unique per `(pk, sk)` pair
- First 32 bytes of `m` should be zeros (TweetNaCl convention)

**Postconditions:**
- `c` contains encrypted message with authentication tag
- Shared secret is wiped from stack

**Example:**
```c
u8 message[100] = {0};  /* Zero-initialized */
u8 ciphertext[100];
u8 nonce[24];
randombytes(nonce, 24);

/* Copy actual message after padding */
memcpy(message + 32, "Secret data", 11);

if (crypto_box(ciphertext, message, 100, nonce, pk_bob, sk_alice) != 0) {
    fprintf(stderr, "Encryption failed\n");
}
```

#### crypto_box_open

```c
int crypto_box_open(
    u8 *m,                    /* out: plaintext */
    const u8 *c,              /* in: ciphertext */
    unsigned long long clen,  /* in: ciphertext length */
    const u8 *n,              /* in: 24-byte nonce */
    const u8 *pk,             /* in: sender's public key */
    const u8 *sk              /* in: recipient's secret key */
) __attribute__((nonnull(1, 2, 3, 4, 5)));
```

**Return Value:** `0` on success (authentication passed), `-1` on failure (authentication failed)

**Constant-Time:** Yes (verification is constant-time)

**Stack Usage:** ~2048 bytes

---

### 2.2 crypto_sign - Digital Signatures

**Algorithm:** Ed25519 (Edwards-curve Digital Signature Algorithm)

#### crypto_sign_keypair

```c
int crypto_sign_keypair(
    u8 *pk,           /* out: 32-byte public key */
    u8 *sk            /* out: 64-byte secret key */
) __attribute__((nonnull(1, 2)));
```

| Parameter | Direction | Size | Description |
|-----------|-----------|------|-------------|
| `pk` | Output | 32 bytes | Public key |
| `sk` | Output | 64 bytes | Secret key (includes public key suffix) |

**Return Value:** `0` on success, `-1` on failure

**Stack Usage:** ~128 bytes

#### crypto_sign

```c
int crypto_sign(
    u8 *sm,                              /* out: signed message */
    unsigned long long *smlen,           /* out: signed message length */
    const u8 *m,                         /* in: original message */
    unsigned long long mlen,             /* in: message length */
    const u8 *sk                         /* in: 64-byte secret key */
) __attribute__((nonnull(1, 2, 3, 5)));
```

| Parameter | Direction | Size | Description |
|-----------|-----------|------|-------------|
| `sm` | Output | `mlen + 64` bytes | Message with signature prepended |
| `smlen` | Output | - | Total length (`mlen + 64`) |
| `m` | Input | `mlen` bytes | Original message |
| `mlen` | Input | - | Message length |
| `sk` | Input | 64 bytes | Signing secret key |

**Return Value:** `0` on success, `-1` on failure

**Constant-Time:** Yes (signature generation is constant-time)

**Stack Usage:** ~3072 bytes

**Example:**
```c
u8 message[] = "Document to sign";
u8 signed_msg[sizeof(message) + 64];
unsigned long long signed_len;

if (crypto_sign(signed_msg, &signed_len, message, sizeof(message), sk) != 0) {
    fprintf(stderr, "Signing failed\n");
}
printf("Signed message is %llu bytes\n", signed_len);
```

#### crypto_sign_open

```c
int crypto_sign_open(
    u8 *m,                               /* out: verified message */
    unsigned long long *mlen,            /* out: message length */
    const u8 *sm,                        /* in: signed message */
    unsigned long long smlen,            /* in: signed message length */
    const u8 *pk                         /* in: 32-byte public key */
) __attribute__((nonnull(1, 2, 3, 5)));
```

**Return Value:** `0` if signature is valid, `-1` if invalid

**Constant-Time:** Yes (verification is constant-time)

**Stack Usage:** ~3072 bytes

---

### 2.3 crypto_secretbox - Symmetric Encryption

**Algorithm:** XSalsa20 + Poly1305

#### crypto_secretbox

```c
int crypto_secretbox(
    u8 *c,                    /* out: ciphertext */
    const u8 *m,              /* in: plaintext */
    unsigned long long mlen,  /* in: message length */
    const u8 *n,              /* in: 24-byte nonce */
    const u8 *k               /* in: 32-byte key */
) __attribute__((nonnull(1, 2, 3, 4)));
```

| Parameter | Direction | Size | Description |
|-----------|-----------|------|-------------|
| `c` | Output | `mlen` bytes | Ciphertext |
| `m` | Input | `mlen` bytes | Plaintext |
| `mlen` | Input | - | Message length |
| `n` | Input | 24 bytes | Nonce (unique per key) |
| `k` | Input | 32 bytes | Secret key |

**Return Value:** `0` on success, `-1` on failure

**Constant-Time:** Yes

**Stack Usage:** ~512 bytes

#### crypto_secretbox_open

```c
int crypto_secretbox_open(
    u8 *m,                    /* out: plaintext */
    const u8 *c,              /* in: ciphertext */
    unsigned long long clen,  /* in: ciphertext length */
    const u8 *n,              /* in: 24-byte nonce */
    const u8 *k               /* in: 32-byte key */
) __attribute__((nonnull(1, 2, 3, 4)));
```

**Return Value:** `0` if authentication passes, `-1` if fails

---

### 2.4 crypto_hash - Cryptographic Hashing

**Algorithm:** SHA-512

#### crypto_hash

```c
int crypto_hash(
    u8 *out,                  /* out: 64-byte hash */
    const u8 *in,             /* in: input data */
    unsigned long long inlen  /* in: input length */
) __attribute__((nonnull(1, 2)));
```

| Parameter | Direction | Size | Description |
|-----------|-----------|------|-------------|
| `out` | Output | 64 bytes | SHA-512 hash |
| `in` | Input | `inlen` bytes | Input data |
| `inlen` | Input | - | Input length |

**Return Value:** `0` always (hash cannot fail)

**Constant-Time:** Yes (SHA-512 is constant-time)

**Stack Usage:** ~256 bytes

**Example:**
```c
const char* data = "Message to hash";
u8 hash[64];

crypto_hash(hash, (const u8*)data, strlen(data));

printf("SHA-512: ");
for (int i = 0; i < 64; i++) {
    printf("%02x", hash[i]);
}
printf("\n");
```

---

### 2.5 crypto_scalarmult - Elliptic Curve Scalar Multiplication

**Algorithm:** Curve25519 Montgomery Ladder

#### crypto_scalarmult

```c
int crypto_scalarmult(
    u8 *q,            /* out: 32-byte shared secret */
    const u8 *n,      /* in: 32-byte scalar (private key) */
    const u8 *p       /* in: 32-byte point (public key) */
) __attribute__((nonnull(1, 2, 3)));
```

| Parameter | Direction | Size | Description |
|-----------|-----------|------|-------------|
| `q` | Output | 32 bytes | Shared secret (ECDH result) |
| `n` | Input | 32 bytes | Scalar (your secret key) |
| `p` | Input | 32 bytes | Point (other party's public key) |

**Return Value:** `0` on success, `-1` if result is weak (all-zero)

**Constant-Time:** Yes (Montgomery ladder is constant-time)

**Stack Usage:** ~1024 bytes

**Example (ECDH Key Exchange):**
```c
u8 my_sk[32], my_pk[32];
u8 their_pk[32];
u8 shared_secret[32];

/* Generate our key pair */
crypto_box_keypair(my_pk, my_sk);

/* Assume their_pk received from network... */

/* Compute shared secret */
if (crypto_scalarmult(shared_secret, my_sk, their_pk) != 0) {
    fprintf(stderr, "Weak shared secret!\n");
}

/* Derive encryption key from shared secret */
u8 encryption_key[32];
crypto_hash(encryption_key, shared_secret, 32);

secure_zero(shared_secret, sizeof(shared_secret));
```

#### crypto_scalarmult_base

```c
int crypto_scalarmult_base(
    u8 *q,            /* out: 32-byte public key */
    const u8 *n       /* in: 32-byte scalar (private key) */
) __attribute__((nonnull(1, 2)));
```

**Purpose:** Compute `q = n * G` where G is the Curve25519 base point.

**Usage:** Alternative to `crypto_box_keypair` for deriving public key from secret.

---

### 2.6 crypto_onetimeauth - Message Authentication

**Algorithm:** Poly1305

#### crypto_onetimeauth

```c
int crypto_onetimeauth(
    u8 *out,                  /* out: 16-byte MAC */
    const u8 *in,             /* in: input data */
    unsigned long long inlen, /* in: input length */
    const u8 *k               /* in: 32-byte key */
) __attribute__((nonnull(1, 2, 4)));
```

**Return Value:** `0` on success

**Constant-Time:** Yes

**⚠️ WARNING:** Key `k` must NEVER be reused. For most applications, use `crypto_secretbox` instead.

#### crypto_onetimeauth_verify

```c
int crypto_onetimeauth_verify(
    const u8 *h,                /* in: 16-byte MAC */
    const u8 *in,               /* in: input data */
    unsigned long long inlen,   /* in: input length */
    const u8 *k                 /* in: 32-byte key */
) __attribute__((nonnull(1, 2, 4)));
```

**Return Value:** `0` if MAC is valid, `-1` if invalid

---

## 3. Secure Memory API

### 3.1 secure_memset

```c
void secure_memset(
    volatile void* dest,  /* in/out: memory region to zero */
    int c,                /* in: value to set (usually 0) */
    size_t count          /* in: number of bytes */
) __attribute__((nonnull(1)));
```

**Purpose:** Set memory to a value in a way that cannot be optimized away.

**When to Use:**
- Wiping secret keys before function return
- Clearing buffers that contained sensitive data
- Preparing memory for reuse after cryptographic operations

**Performance Implications:** ~10-20% slower than `memset()` due to `volatile` and compiler barrier.

**Example:**
```c
u8 secret_key[32];
randombytes(secret_key, 32);

/* Use key... */

/* WIPE before scope ends */
secure_memset(secret_key, 0, sizeof(secret_key));
```

### 3.2 secure_zero

```c
void secure_zero(
    volatile void* dest,  /* in/out: memory region to zero */
    size_t count          /* in: number of bytes */
) __attribute__((nonnull(1)));
```

**Purpose:** Convenience wrapper for `secure_memset(dest, 0, count)`.

**Example:**
```c
secure_zero(password_buffer, password_len);
```

### 3.3 safe_memcpy

```c
int safe_memcpy(
    void* dest,           /* out: destination buffer */
    const void* src,      /* in: source buffer */
    size_t dest_size,     /* in: size of destination buffer */
    size_t count          /* in: number of bytes to copy */
) __attribute__((nonnull(1, 2)));
```

**Return Value:** `0` on success, `-1` on failure (NULL pointer or overflow)

**Purpose:** Bounds-checked memory copy to prevent buffer overflows.

**Example:**
```c
u8 buffer[64];
u8 input[128];

if (safe_memcpy(buffer, input, sizeof(buffer), 64) != 0) {
    /* Handle error - would have overflowed */
}
```

### 3.4 secure_memcmp

```c
int secure_memcmp(
    const void* x,        /* in: first buffer */
    const void* y,        /* in: second buffer */
    size_t n              /* in: number of bytes to compare */
) __attribute__((nonnull(1, 2)));
```

**Return Value:** `0` if equal, `-1` if different

**Constant-Time:** Yes (always compares all `n` bytes)

**Purpose:** Constant-time comparison for authentication tags, passwords, etc.

**Example:**
```c
u8 computed_tag[16];
u8 received_tag[16];

/* ... compute and receive tags ... */

if (secure_memcmp(computed_tag, received_tag, 16) != 0) {
    /* Authentication failed - DO NOT reveal which byte differed */
    fprintf(stderr, "Invalid authentication tag\n");
}
```

---

## 4. Random Bytes API

### 4.1 randombytes

```c
void randombytes(
    u8* buf,      /* out: buffer to fill with random bytes */
    size_t len    /* in: number of bytes to generate */
) __attribute__((nonnull(1)));
```

**Interface Contract:** Fills `buf` with `len` cryptographically secure random bytes.

**Implementation Requirements:**
- Must use CSPRNG (Cryptographically Secure Pseudo-Random Number Generator)
- Default implementation reads from `/dev/urandom` (POSIX) or `arc4random_buf()` (BSD/macOS)
- Must block or fail if insufficient entropy (not applicable to `/dev/urandom`)

**Thread Safety:** Depends on implementation. Default is thread-safe.

**Fork Safety:** Default implementation reopens `/dev/urandom` after fork.

**Custom Implementation Injection:**

```c
/* In your code, before including tweetnacl.h */
#define RANDOMBYTES_CUSTOM_IMPLEMENTATION

void randombytes(u8* buf, size_t len) {
    /* Your custom RNG - e.g., hardware RNG, Windows CryptoAPI */
    #ifdef _WIN32
        HCRYPTPROV hProv;
        CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT);
        CryptGenRandom(hProv, len, buf);
        CryptReleaseContext(hProv, 0);
    #else
        /* Fallback to /dev/urandom */
        FILE* f = fopen("/dev/urandom", "rb");
        fread(buf, 1, len, f);
        fclose(f);
    #endif
}

#include <tweetnacl/tweetnacl.h>
```

---

## 5. PQC API (Post-Quantum Cryptography)

### 5.1 Type Definitions

```c
typedef enum {
    PQC_SUCCESS = 0,
    PQC_ERR_INVALID_PARAM = -1,
    PQC_ERR_BUFFER_TOO_SMALL = -2,
    PQC_ERR_VERIFICATION_FAILED = -3,
    PQC_ERR_UNSUPPORTED_ALGORITHM = -4,
    PQC_ERR_KEY_GENERATION_FAILED = -5,
    PQC_ERR_ENCAPSULATION_FAILED = -6,
    PQC_ERR_DECAPSULATION_FAILED = -7
} pqc_result_t;

typedef enum {
    PQC_KYBER512 = 1,
    PQC_KYBER768 = 2,
    PQC_KYBER1024 = 3,
    PQC_DILITHIUM2 = 10,
    PQC_DILITHIUM3 = 11,
    PQC_DILITHIUM5 = 12,
    PQC_FALCON512 = 20,
    PQC_FALCON1024 = 21,
    PQC_SPHINCS_SHA2_128F = 30,
    PQC_SPHINCS_SHAKE_128F = 31
} pqc_algorithm_t;
```

### 5.2 pqc_keygen

```c
pqc_result_t pqc_keygen(
    pqc_algorithm_t alg,    /* in: algorithm selection */
    u8* pk,                 /* out: public key */
    size_t* pk_len,         /* in/out: public key length */
    u8* sk,                 /* out: secret key */
    size_t* sk_len          /* in/out: secret key length */
);
```

**Example:**
```c
u8 pk[2000], sk[2500];
size_t pk_len = sizeof(pk), sk_len = sizeof(sk);

pqc_result_t result = pqc_keygen(PQC_KYBER768, pk, &pk_len, sk, &sk_len);
if (result != PQC_SUCCESS) {
    fprintf(stderr, "Keygen failed: %s\n", pqc_result_to_string(result));
}
```

### 5.3 pqc_encapsulate / pqc_decapsulate

```c
pqc_result_t pqc_encapsulate(
    pqc_algorithm_t alg,    /* in: algorithm selection */
    const u8* pk,           /* in: public key */
    size_t pk_len,          /* in: public key length */
    u8* ct,                 /* out: ciphertext */
    size_t* ct_len,         /* out: ciphertext length */
    u8* ss,                 /* out: shared secret */
    size_t ss_len           /* in: shared secret buffer size */
);

pqc_result_t pqc_decapsulate(
    pqc_algorithm_t alg,    /* in: algorithm selection */
    const u8* ct,           /* in: ciphertext */
    size_t ct_len,          /* in: ciphertext length */
    const u8* sk,           /* in: secret key */
    size_t sk_len,          /* in: secret key length */
    u8* ss,                 /* out: shared secret */
    size_t ss_len           /* in: shared secret buffer size */
);
```

### 5.4 Hybrid Mode Functions

```c
pqc_result_t pqc_hybrid_keygen(
    pqc_algorithm_t alg,        /* in: PQC algorithm */
    u8* hybrid_pk,              /* out: combined public key */
    size_t* hybrid_pk_len,      /* out: combined PK length */
    u8* hybrid_sk,              /* out: combined secret key */
    size_t* hybrid_sk_len       /* out: combined SK length */
);

pqc_result_t pqc_hybrid_encapsulate(
    pqc_algorithm_t alg,
    const u8* hybrid_pk,
    size_t hybrid_pk_len,
    u8* hybrid_ct,
    size_t* hybrid_ct_len,
    u8* hybrid_ss,              /* Combined shared secret */
    size_t hybrid_ss_len
);
```

### 5.5 Error Code Reference

| Code | Name | Description |
|------|------|-------------|
| 0 | `PQC_SUCCESS` | Operation completed successfully |
| -1 | `PQC_ERR_INVALID_PARAM` | NULL pointer or invalid parameter |
| -2 | `PQC_ERR_BUFFER_TOO_SMALL` | Output buffer insufficient |
| -3 | `PQC_ERR_VERIFICATION_FAILED` | Signature/MAC verification failed |
| -4 | `PQC_ERR_UNSUPPORTED_ALGORITHM` | Algorithm not compiled in |
| -5 | `PQC_ERR_KEY_GENERATION_FAILED` | RNG failure during keygen |
| -6 | `PQC_ERR_ENCAPSULATION_FAILED` | KEM encapsulation error |
| -7 | `PQC_ERR_DECAPSULATION_FAILED` | KEM decapsulation error |

---

## 6. Type Definitions

### 6.1 Fixed-Width Integers

```c
typedef uint8_t  u8;    /* Unsigned 8-bit integer */
typedef uint32_t u32;   /* Unsigned 32-bit integer */
typedef uint64_t u64;   /* Unsigned 64-bit integer */
typedef int64_t  i64;   /* Signed 64-bit integer */
```

**Source:** `<stdint.h>` via `tweetnacl_types.h`

### 6.2 Galois Field Element (Curve25519)

```c
typedef u64 gf[10];  /* 10-limb representation of GF(2^255-19) element */
```

**Internal Representation:** Radix-2^25.5 representation for efficient reduction modulo 2^255-19.

**Note:** This type is exposed in internal headers but should not be used directly by application code.

### 6.3 PQC Algorithm Enum

See Section 5.1 for complete definition.

---

## 7. Constants Reference

### 7.1 Size Constants Table

| Constant | Value | Description |
|----------|-------|-------------|
| `crypto_box_PUBLICKEYBYTES` | 32 | Curve25519 public key size |
| `crypto_box_SECRETKEYBYTES` | 32 | Curve25519 secret key size |
| `crypto_box_NONCEBYTES` | 24 | XSalsa20 nonce size |
| `crypto_box_ZEROBYTES` | 32 | Plaintext zero-padding requirement |
| `crypto_box_BOXZEROBYTES` | 16 | Ciphertext overhead |
| `crypto_sign_PUBLICKEYBYTES` | 32 | Ed25519 public key size |
| `crypto_sign_SECRETKEYBYTES` | 64 | Ed25519 secret key size |
| `crypto_sign_BYTES` | 64 | Ed25519 signature size |
| `crypto_secretbox_KEYBYTES` | 32 | XSalsa20-Poly1305 key size |
| `crypto_secretbox_NONCEBYTES` | 24 | XSalsa20 nonce size |
| `crypto_secretbox_ZEROBYTES` | 32 | Plaintext zero-padding |
| `crypto_secretbox_BOXZEROBYTES` | 16 | Ciphertext overhead |
| `crypto_hash_BYTES` | 64 | SHA-512 output size |
| `crypto_onetimeauth_BYTES` | 16 | Poly1305 MAC size |
| `crypto_onetimeauth_KEYBYTES` | 32 | Poly1305 key size |
| `crypto_verify_16_BYTES` | 16 | Verification input size |
| `crypto_verify_32_BYTES` | 32 | Verification input size |

### 7.2 Maximum Message Sizes

| Function | Practical Limit | Rationale |
|----------|-----------------|-----------|
| `crypto_box` | ~1 MB | Stack usage considerations |
| `crypto_secretbox` | ~1 MB | Stack usage considerations |
| `crypto_sign` | Unlimited (heap-safe) | Signs in place |
| `crypto_hash` | Unlimited | Streaming capable |

> ⚠️ **WARNING**: For messages larger than 1 MB, consider chunking or using a streaming API pattern.

### 7.3 Stack Size Limits

| Scenario | Recommended Minimum Stack |
|----------|--------------------------|
| Basic hashing | 1 KB |
| Secretbox operations | 2 KB |
| Box (public-key encryption) | 4 KB |
| Sign (Ed25519) | 8 KB |
| PQC hybrid operations | 16 KB |

**Embedded Systems:** Ensure at least 16 KB stack for full functionality.

---

## References

1. Bernstein, D.J., et al. "Ed25519: high-speed high-security signatures." CHES 2011.
2. Bernstein, D.J. "Curve25519: new Diffie-Hellman speed records." PKC 2006.
3. NIST FIPS 180-4. "Secure Hash Standard (SHS)." 2015.
4. RFC 8439. "ChaCha20 and Poly1305 for IETF Protocols." 2018.

---

*This document is part of the TweetNaCl-Modular documentation suite. See also: [ARCHITECTURE.md](ARCHITECTURE.md), [SECURITY.md](SECURITY.md), [BUILD.md](BUILD.md).*
