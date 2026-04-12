# FIPS Algorithm Compliance

## Overview

This library provides **optional FIPS algorithm-level compliance** for:

| Standard | Algorithm | Implementation |
|----------|-----------|----------------|
| **FIPS 186-5** | Ed25519 (Digital Signature Standard) | `crypto_sign_ed25519ctx()`, `crypto_sign_ed25519ph()` |
| **FIPS 180-4** | SHA-512, SHA-512/256, SHA-512/224 | `crypto_hash()`, `crypto_hash_sha512_256()` |
| **FIPS 203** | ML-KEM (formerly Kyber) | `pqc_encapsulate()` with ML-KEM aliases |
| **FIPS 204** | ML-DSA (formerly Dilithium) | `pqc_sign()` with ML-DSA aliases |
| **FIPS 205** | SLH-DSA (formerly SPHINCS+) | `pqc_sign()` with SLH-DSA aliases |

## ⚠️ NOT FIPS 140-3 Certified

This is a **software library**, NOT a FIPS 140-3 certified cryptographic module.

FIPS 140-3 certification requires:
- Hardware security module (HSM) or validated software with physical controls
- CMVP (Cryptographic Module Validation Program) testing
- Extensive documentation and third-party audit

This library implements the **algorithms correctly** but does not provide FIPS 140-3 module certification.

### What This DOES Provide

1. ✅ Algorithm correctness per FIPS standards
2. ✅ Known Answer Test (KAT) verification via CAVP vectors
3. ✅ Self-test framework (power-on, pairwise consistency, CRNGT)
4. ✅ Clear documentation of limitations

### What This Does NOT Provide

1. ❌ FIPS 140-3 module certification
2. ❌ CMVP validation
3. ❌ Government-use certification

### Recommendation

**Do NOT claim FIPS compliance in marketing.** Instead say:
- "Implements FIPS 186-5 Ed25519 algorithms"
- "FIPS 180-4 SHA-512 compliant implementation"
- "NIST PQC FIPS 203/204/205 interface"

## Building with FIPS Mode

```bash
cmake -B build -DENABLE_FIPS_MODE=ON -DENABLE_PQC=ON
cmake --build build --parallel
ctest --output-on-failure
```

## FIPS 186-5 Ed25519

### Ed25519ctx (Context String Support)

FIPS 186-5 Section 7 requires domain separation via context strings:

```c
#include "fips1865_ctx.h"

uint8_t pk[32], sk[64], sig[128], msg[] = "Sign this";
uint64_t smlen;

const char *ctx = "myapp-v1";

/* Sign with context */
crypto_sign_ed25519ctx(sig, &smlen, msg, sizeof(msg)-1, sk, ctx, 8);

/* Verify with same context */
uint8_t m[128];
uint64_t mlen;
crypto_sign_ed25519ctx_open(m, &mlen, sig, smlen, pk, ctx, 8);
```

### Ed25519ph (Prehash Mode)

For large messages, FIPS 186-5 Section 7.6 defines prehashed Ed25519:

```c
crypto_sign_ed25519ph(sig, &smlen, msg, sizeof(msg)-1, sk);
crypto_sign_ed25519ph_open(m, &mlen, sig, smlen, pk);
```

## FIPS 180-4 SHA-512

### SHA-512/256

Truncated SHA-512 with different IV, producing 256-bit output:

```c
uint8_t hash[32];
crypto_hash_sha512_256(hash, msg, msg_len);
```

### SHA-512/224

Truncated SHA-512 with different IV, producing 224-bit output:

```c
uint8_t hash[28];
crypto_hash_sha512_224(hash, msg, msg_len);
```

## FIPS 203/204/205 (PQC)

The PQC module uses FIPS standard names as aliases:

| Old Name | FIPS Name | Standard |
|----------|-----------|----------|
| `PQC_KYBER512` | `ML_KEM_512` | FIPS 203 |
| `PQC_KYBER768` | `ML_KEM_768` | FIPS 203 |
| `PQC_DILITHIUM2` | `ML_DSA_44` | FIPS 204 |
| `PQC_SPHINCS_128S` | `SLH_DSA_SHA2_128S` | FIPS 205 |

```c
#include "pqc_fips_aliases.h"

/* Use FIPS names */
pqc_keygen(ML_KEM_768, pk, sizeof(pk), sk, sizeof(sk), NULL, 0);
pqc_sign(ML_DSA_44, sk, sk_len, msg, msg_len, sig, &sig_len, NULL, 0);
```

## Self-Tests (FIPS 140-3 Section 9)

```c
#include "fips_selftest.h"

/* Power-on self-test (run at startup) */
if (fips_power_on_selftest() != 0) {
    /* Module not in approved state — abort */
    abort();
}

/* Check FIPS mode */
if (nacl_fips_mode_enabled()) {
    printf("Running in FIPS mode\n");
}
```

## CAVP Test Vectors

Run NIST CAVP test vectors:

```bash
cmake -B build -DENABLE_FIPS_MODE=ON
cmake --build build
cd build && ctest -R FipsCavp --output-on-failure
```

## Non-FIPS Algorithms

The following TweetNaCl algorithms are **NOT** FIPS-approved:

| Algorithm | Status | Alternative |
|-----------|--------|-------------|
| Salsa20 | Not approved | AES-GCM (not implemented) |
| XSalsa20 | Not approved | AES-GCM (not implemented) |
| Poly1305 | Not approved | HMAC-SHA-512-256 |
| X25519 | Pending | NIST P-256 (not implemented) |

When `FIPS_STRICT` mode is enabled (future), non-approved algorithms are disabled at compile time.
