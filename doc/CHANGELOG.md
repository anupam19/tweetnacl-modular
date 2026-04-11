# TweetNaCl-Modular Changelog

**Last Updated:** 2025-01-15

---

## Version 2.0.0 (Current - Refactored)

**Release Date:** January 2025

### Major Changes

#### Modular Architecture with Internal Headers

- **Complete restructure** from monolithic `tweetnacl.c` to modular design
- **New internal headers** in `includes/internal/` for each primitive:
  - `tweetnacl_salsa20.h` - Salsa20 stream cipher interface
  - `tweetnacl_poly1305.h` - Poly1305 MAC interface
  - `tweetnacl_sha512.h` - SHA-512 hash interface
  - `tweetnacl_curve25519.h` - Curve25519 scalar multiplication
  - `tweetnacl_ed25519.h` - Ed25519 signatures
- **Include discipline**: Every `.c` file includes its `.h` first for compile-time verification
- **Eliminated forward declarations** through topological ordering

#### Post-Quantum Cryptography Support

- **NIST-standardized algorithms** (FIPS 203, 204, 205):
  - ML-KEM (Kyber) for key encapsulation
  - ML-DSA (Dilithium) for digital signatures
  - SLH-DSA (SPHINCS+) for hash-based signatures
- **Hybrid mode** combining classical (X25519) + PQC (Kyber768)
- **Algorithm agility** via runtime selection
- **New API functions**:
  - `pqc_keygen()` - Generate PQC keypairs
  - `pqc_encapsulate()` / `pqc_decapsulate()` - KEM operations
  - `pqc_sign()` / `pqc_verify()` - Signature operations
  - `pqc_hybrid_*()` - Hybrid classical/PQC operations

#### Secure Memory Hardening

- **New secure memory API**:
  - `secure_memset()` - Volatile memory set with compiler barriers
  - `secure_zero()` - Convenience wrapper for zeroing
  - `safe_memcpy()` - Bounds-checked memory copy
  - `secure_memcmp()` - Constant-time comparison
- **LTO-safe wiping** using extern declarations to prevent inlining
- **Debug build enhancements**:
  - Poison/unpoison patterns for ASAN integration
  - Stack clearing on function return (optional)

#### Multi-Architecture Build System

- **Architecture detection** via `arch_config.h`
- **Supported architectures**:
  - x86_64 (native)
  - x86_32 (multilib)
  - ARM64 (aarch64)
  - ARM32 (armhf)
  - RISC-V 64-bit
  - RISC-V 32-bit (experimental)
- **Optional assembly optimizations** in `arch/` directory
- **Cross-compilation support** with QEMU testing

### API Changes

#### New Functions

```c
/* Secure memory */
void secure_zero(volatile void* dest, size_t count);
int safe_memcpy(void* dest, const void* src, size_t dest_size, size_t count);

/* PQC KEM */
pqc_result_t pqc_keygen(pqc_algorithm_t alg, u8* pk, size_t* pk_len, u8* sk, size_t* sk_len);
pqc_result_t pqc_encapsulate(pqc_algorithm_t alg, const u8* pk, size_t pk_len,
                              u8* ct, size_t* ct_len, u8* ss, size_t ss_len);
pqc_result_t pqc_decapsulate(pqc_algorithm_t alg, const u8* ct, size_t ct_len,
                              const u8* sk, size_t sk_len, u8* ss, size_t ss_len);

/* Hybrid mode */
pqc_result_t pqc_hybrid_keygen(pqc_algorithm_t alg, u8* hybrid_pk, size_t* pk_len,
                                u8* hybrid_sk, size_t* sk_len);
```

#### Deprecated Functions

None (backward compatible with original TweetNaCl API)

### Bug Fixes

- Fixed potential compiler optimization of memory wipes on GCC 12+
- Corrected endianness handling on big-endian architectures
- Fixed stack usage calculation for Ed25519 signing

### Performance Improvements

- **10-20% faster** with LTO enabled (cross-module inlining)
- **Reduced code size** through dead code elimination
- **Optional AVX2/NEON optimizations** for Poly1305 and Salsa20

### Documentation

- **Comprehensive documentation suite**:
  - `ARCHITECTURE.md` - Deep dive into modular architecture
  - `SECURITY.md` - Threat model and security guarantees
  - `API_REFERENCE.md` - Complete API documentation
  - `BUILD.md` - Build and compilation guide
  - `PORTING_GUIDE.md` - Migration and platform adaptation
  - `CONSTANT_TIME.md` - Constant-time implementation details
  - `PQC.md` - Post-quantum cryptography guide
  - `FAQ.md` - Frequently asked questions

### Testing

- **Unit test coverage**: >90% for core primitives
- **Fuzzing**: Continuous fuzzing with AFL++ and libFuzzer
- **Constant-time verification**: dudect integration in CI
- **Valgrind testing**: memcheck, cachegrind, helgrind

### Build System Changes

- **New Makefile features**:
  - `ARCH=` variable for architecture selection
  - `DEBUG=1` for debug builds
  - `SANITIZE=address|memory|undefined|thread` for sanitizers
  - `LTO=1` for link-time optimization
- **CMake support** via `add_subdirectory()` or FetchContent
- **pkg-config file** generation for easy integration

---

## Version 1.x (Original TweetNaCl-usable Base)

### Version 1.1.0

**Release Date:** December 2023

#### Changes

- Initial fork from tweetnacl-usable repository
- Added secure memory wiping (`secure_zero`)
- Improved randombytes() implementation for Windows
- Minor bug fixes for big-endian systems

### Version 1.0.0

**Release Date:** Based on original TweetNaCl 20140917

#### Original TweetNaCl Features

- **crypto_box**: Public-key authenticated encryption (Curve25519 + XSalsa20 + Poly1305)
- **crypto_sign**: Digital signatures (Ed25519)
- **crypto_secretbox**: Symmetric authenticated encryption (XSalsa20 + Poly1305)
- **crypto_hash**: SHA-512 hashing
- **crypto_scalarmult**: Curve25519 scalar multiplication
- **crypto_onetimeauth**: Poly1305 one-time authenticator
- **crypto_verify**: Constant-time comparison

#### Original Design Principles

- **100 tweets**: Designed to fit in Twitter's character limit
- **Public domain**: No copyright, no restrictions
- **Auditable**: Minimal code for maximum transparency
- **Portable C**: Works on any platform with a C compiler

---

## Upgrade Path from v1.x to v2.0.0

### For Existing Users

1. **Update include paths**:

```c
/* Old */
#include "tweetnacl.h"

/* New */
#include <tweetnacl/tweetnacl.h>
```

2. **Replace manual memory wipes**:

```c
/* Old */
for (int i = 0; i < 32; i++) sk[i] = 0;

/* New */
secure_zero(sk, sizeof(sk));
```

3. **Enable new features (optional)**:

```c
/* Add PQC support */
#include <tweetnacl/pqc.h>

/* Use hybrid key exchange */
pqc_hybrid_keygen(PQC_KYBER768, pk, &pk_len, sk, &sk_len);
```

### Breaking Changes

None - v2.0.0 maintains source compatibility with v1.x for all original API functions.

### Recommended Actions

- Enable LTO for better performance: `make LTO=1`
- Consider migrating to hybrid PQC for long-term security
- Review [PORTING_GUIDE.md](PORTING_GUIDE.md) for detailed migration steps

---

## Future Roadmap

### Version 2.1.0 (Planned Q2 2025)

- Assembly optimizations for x86_64 (AVX2) and ARM64 (NEON)
- Formal verification of `crypto_verify_*` functions
- Additional PQC algorithms (Falcon, additional SPHINCS+ variants)

### Version 2.2.0 (Planned Q4 2025)

- Streaming API for large message encryption
- Hardware RNG integration (RDRAND, ARM TRNG)
- Kernel module support improvements

### Version 3.0.0 (Planned 2026)

- Potential breaking changes for API refinement
- Complete formal verification of core primitives
- Removal of deprecated algorithms (if any)

---

## Security Advisories

| ID | Date | Severity | Description | Fixed In |
|----|------|----------|-------------|----------|
| TNSA-2025-001 | 2025-01-10 | Low | Timing variation in randombytes() on some platforms | 2.0.0 |

**Reporting Vulnerabilities:**

See [SECURITY.md](SECURITY.md#7-vulnerability-disclosure-policy) for our vulnerability disclosure policy.

---

## Contributors

Version 2.0.0 was made possible by contributions from:

- Core development team
- External security reviewers
- Community contributors (bug reports, patches, documentation)

For a complete list, see the Git repository history.

---

*This changelog is part of the TweetNaCl-Modular documentation suite.*
