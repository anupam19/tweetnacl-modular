# TweetNaCl-Modular Architecture

**Last Updated:** 2025-01-15  
**Version:** 2.0.0

---

## Table of Contents

1. [Design Philosophy](#1-design-philosophy)
2. [Module Hierarchy](#2-module-hierarchy)
3. [File Organization Logic](#3-file-organization-logic)
4. [Constant-Time Architecture](#4-constant-time-architecture)
5. [Memory Safety Architecture](#5-memory-safety-architecture)
6. [Build System Architecture](#6-build-system-architecture)

---

## 1. Design Philosophy

### 1.1 Why Modular? (vs Original Monolithic TweetNaCl)

The original TweetNaCl by Daniel J. Bernstein was designed as a "100 tweets" demonstration—proof that cryptographic primitives could fit in Twitter's character limit. While brilliant for auditability, the monolithic design presents challenges for production use:

| Aspect | Original TweetNaCl | TweetNaCl-Modular |
|--------|-------------------|-------------------|
| File Structure | Single `tweetnacl.c` (~4000 lines) | Multiple focused modules |
| Header Design | Single public header | Layered headers (public/internal) |
| Forward Declarations | Required throughout | Eliminated via topological ordering |
| Architecture Support | Generic C only | Multi-arch with `arch/` layer |
| PQC Integration | Not available | Hybrid classical/PQC support |
| Compile-Time Checking | Limited | Full type verification per module |

**Key Benefits of Modularization:**

1. **Isolated Testing**: Each primitive can be unit-tested independently
2. **Incremental Auditing**: Security reviewers can focus on one module at a time
3. **Selective Compilation**: Embedded systems can exclude unused primitives
4. **Architecture-Specific Optimization**: Critical paths can have assembly implementations without affecting portability

### 1.2 Header-Driven Design Benefits

The modular architecture enforces a strict header-first discipline:

```
Every .c file MUST include its corresponding .h first
```

**Benefits:**

- **Compile-Time Interface Verification**: If implementation doesn't match declaration, compilation fails immediately
- **No Implicit Dependencies**: Headers must be self-contained, exposing all required includes
- **Documentation Co-location**: Function prototypes serve as API documentation
- **IDE Support**: Better autocomplete and navigation when types are fully resolved

**Example Include Discipline:**

```c
/* src/tweetnacl_curve25519.c */
#include "../includes/internal/tweetnacl_curve25519.h"  /* MUST be first */
#include "../includes/internal/tweetnacl_types.h"       /* Included by curve25519.h */
#include "../includes/internal/tweetnacl_verify.h"      /* For secure_memcmp */
/* ... other includes ... */

/* Implementation follows - compiler has verified all signatures */
int crypto_scalarmult(u8 *q, const u8 *n, const u8 *p) {
    /* ... */
}
```

### 1.3 Security Through Transparency

The architecture prioritizes auditable code paths:

1. **No Hidden Control Flow**: All branches are explicit; no macro obfuscation
2. **Visible Constant-Time Patterns**: `crypto_verify_*` uses XOR-accumulate, clearly visible in source
3. **Explicit Memory Management**: No hidden allocations; caller owns all buffers
4. **Traceable Call Graph**: From high-level API to field arithmetic, every call is resolvable

---

## 2. Module Hierarchy

### 2.1 Layer Diagram

```
┌─────────────────────────────────────────────────────────────────┐
│                    APPLICATION LAYER                            │
│              (User code linking against library)                │
└─────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│                   HIGH-LEVEL API LAYER                          │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐           │
│  │ crypto_box   │  │ crypto_sign  │  │crypto_secretbox│         │
│  │ (authenticated │  │ (Ed25519   │  │ (XSalsa20-     │           │
│  │  encryption) │  │  signatures) │  │  Poly1305)    │           │
│  └──────────────┘  └──────────────┘  └──────────────┘           │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐           │
│  │crypto_hash   │  │crypto_scalarmult│ │pqc_hybrid_* │           │
│  │ (SHA-512)    │  │ (X25519 ECDH)│  │ (KEM+ECDH)   │           │
│  └──────────────┘  └──────────────┘  └──────────────┘           │
└─────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│                   PRIMITIVE LAYER                               │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐           │
│  │ salsa20      │  │ poly1305     │  │ sha512       │           │
│  │ (stream cipher)│ │ (MAC)        │  │ (hash core)  │           │
│  └──────────────┘  └──────────────┘  └──────────────┘           │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐           │
│  │curve25519    │  │ ed25519      │  │ hmacsha512   │           │
│  │ (Montgomery) │  │ (Edwards)    │  │ (HMAC)       │           │
│  └──────────────┘  └──────────────┘  └──────────────┘           │
└─────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│                 FIELD ARITHMETIC LAYER                          │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐           │
│  │ gf_add       │  │ gf_mul       │  │ gf_invert    │           │
│  │ (mod 2^255-19)│ │ (Karatsuba)  │  │ (Fermat)     │           │
│  └──────────────┘  └──────────────┘  └──────────────┘           │
│  ┌──────────────┐  ┌──────────────┐                              │
│  │ gf_pack      │  │ gf_unpack    │                              │
│  │ (byte↔field) │  │ (byte↔field) │                              │
│  └──────────────┘  └──────────────┘                              │
└─────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│                   ARCHITECTURE LAYER                            │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐           │
│  │ arch_config.h│  │ arch/x86_64/ │  │ arch/arm64/  │           │
│  │ (CPU detect) │  │ (optimized)  │  │ (optimized)  │           │
│  └──────────────┘  └──────────────┘  └──────────────┘           │
└─────────────────────────────────────────────────────────────────┘
```

### 2.2 Dependency Graph (No Cycles Allowed)

```
                    tweetnacl.h
                        │
        ┌───────────────┼───────────────┐
        │               │               │
   tweetnacl_box    tweetnacl_sign   pqc.h
        │               │               │
        ├───────┬───────┤               │
        │       │       │               │
   curve25519  sha512  ed25519         │
        │       │       │               │
        │   ┌───┴───┐   │               │
        │   │       │   │               │
   salsa20  poly1305  │               │
        │       │     │               │
        └───────┼─────┘               │
                │                     │
           verify.h ◄─────────────────┘
                │
           types.h
                │
           arch_config.h
```

**Dependency Rules:**

1. **Lower layers cannot depend on higher layers**: Field arithmetic knows nothing about signatures
2. **Sibling modules communicate only through common ancestors**: `curve25519` and `ed25519` both use `types.h` but don't include each other
3. **Internal headers may include public headers**: But public headers should not expose internal details

### 2.3 Visibility Layers

| Layer | Location | Purpose | Example Files |
|-------|----------|---------|---------------|
| **Public** | `includes/` | External API, stable interface | `tweetnacl.h`, `secure_mem.h`, `pqc.h` |
| **Internal** | `includes/internal/` | Module interfaces, may change between versions | `tweetnacl_salsa20.h`, `tweetnacl_poly1305.h` |
| **Private** | `src/` | Implementation details, never included directly | `tweetnacl.c`, `secure_mem.c` |

**Access Control by Convention:**

```c
/* Public header - users include this */
#include <tweetnacl/tweetnacl.h>

/* Internal header - only library code includes these */
#include "internal/tweetnacl_salsa20.h"

/* Private implementation - never included, only compiled */
/* gcc -c src/tweetnacl.c */
```

---

## 3. File Organization Logic

### 3.1 Why Each File Exists

| File | Purpose | Rationale |
|------|---------|-----------|
| `tweetnacl.h` | Main public API | Single entry point for users |
| `tweetnacl_types.h` | Base type definitions | Shared foundation, no dependencies |
| `tweetnacl_verify.h` | Constant-time comparison | Used by every authentication primitive |
| `tweetnacl_salsa20.h` | Salsa20 stream cipher | Core of secretbox and box |
| `tweetnacl_poly1305.h` | Poly1305 MAC | Authentication component |
| `tweetnacl_sha512.h` | SHA-512 hash | Used by Ed25519 and HMAC |
| `tweetnacl_curve25519.h` | X25519 key exchange | ECDH primitive for box |
| `tweetnacl_ed25519.h` | Ed25519 signatures | Digital signature primitive |
| `tweetnacl_secretbox.h` | Symmetric encryption | Combines salsa20 + poly1305 |
| `tweetnacl_box.h` | Public-key encryption | Combines curve25519 + secretbox |
| `secure_mem.h` | Secure memory operations | Platform-independent wiping |
| `secure_utils.h` | Utility functions | safe_memcpy, secure_memcmp wrappers |
| `randombytes.h` | CSPRNG interface | Abstracts system RNG |
| `pqc.h` | Post-quantum cryptography | KEM and signature algorithms |
| `arch_config.h` | Architecture detection | CPU feature flags, endianness |

### 3.2 Include Discipline

**Rule: Every `.c` file includes its `.h` first**

```c
/* src/secure_mem.c */
#include "../includes/secure_mem.h"      /* FIRST - verifies implementation matches declaration */
#include "../includes/internal/tweetnacl_types.h"  /* Types used in implementation */
#include <string.h>                       /* Standard library for fallback */
#include <stdint.h>

/* Now implement - compiler has checked all prototypes */
void secure_zero(volatile void* dest, size_t count) {
    /* ... */
}
```

**Why This Matters:**

1. **Catches Signature Mismatches**: If `.h` says `int func(u8*)` but `.c` implements `void func(u32*)`, error occurs immediately
2. **Ensures Self-Contained Headers**: If the `.h` doesn't compile alone, the build fails early
3. **Documents Dependencies**: First include shows primary responsibility

### 3.3 Self-Contained Headers Requirement

Every header must compile standalone:

```c
/* WRONG - missing dependency */
#ifndef TWEETNACL_SALSA20_H
#define TWEETNACL_SALSA20_H
int crypto_core_salsa20(u8 *out, const u8 *in, const u8 *k, const u8 *c);
#endif
/* Error: 'u8' undeclared if included without tweetnacl_types.h */

/* CORRECT - self-contained */
#ifndef TWEETNACL_SALSA20_H
#define TWEETNACL_SALSA20_H
#include "tweetnacl_types.h"  /* Explicit dependency */
int crypto_core_salsa20(u8 *out, const u8 *in, const u8 *k, const u8 *c);
#endif
```

---

## 4. Constant-Time Architecture

### 4.1 Avoiding Secret-Dependent Branches

**Anti-Pattern (Variable-Time):**

```c
/* NEVER DO THIS with secret data */
if (secret_byte == expected) {
    return 0;  /* Early return leaks timing */
}
return -1;
```

**Correct Pattern (Constant-Time):**

```c
/* Accumulate differences without branching */
u8 result = 0;
for (size_t i = 0; i < len; i++) {
    result |= x[i] ^ y[i];  /* XOR accumulates differences */
}
return (result != 0) ? -1 : 0;  /* Single branch on final result */
```

### 4.2 Select/CSwap Patterns

Instead of conditional assignment, use arithmetic selection:

```c
/* Variable-time (BAD) */
if (condition) {
    a = b;
}

/* Constant-time (GOOD) */
/* condition must be 0 or 1 */
a = (condition * b) + ((1 - condition) * a);

/* Or using bitwise operations */
u8 mask = -(u8)condition;  /* All 1s if condition=1, all 0s if condition=0 */
a = (mask & b) | (~mask & a);
```

**Montgomery Ladder CSwap:**

```c
/* From crypto_scalarmult - constant-time conditional swap */
static void cswap(gf p[3], gf q[3], u8 b) {
    u8 mask = -b;  /* 0xFF if b=1, 0x00 if b=0 */
    for (int i = 0; i < 4; i++) {
        u8 x0 = p[0][i], x1 = q[0][i];
        u8 y0 = p[1][i], y1 = q[1][i];
        u8 z0 = p[2][i], z1 = q[2][i];
        
        /* Swap based on mask - no branch */
        p[0][i] = x0 ^ (mask & (x0 ^ x1));
        q[0][i] = x1 ^ (mask & (x0 ^ x1));
        p[1][i] = y0 ^ (mask & (y0 ^ y1));
        q[1][i] = y1 ^ (mask & (y0 ^ y1));
        p[2][i] = z0 ^ (mask & (z0 ^ z1));
        q[2][i] = z1 ^ (mask & (z0 ^ z1));
    }
}
```

### 4.3 Montgomery Ladder Implementation Details

The Curve25519 scalar multiplication uses the Montgomery ladder for constant-time operation:

```
Input: scalar n (32 bytes), point p (32 bytes)
Output: q = n * p (32 bytes)

Algorithm:
  r[0] = 0, r[1] = 1  /* Projective coordinates */
  for i = 254 down to 0:
    bit = (n[i >> 3] >> (i & 7)) & 1
    cswap(r[0], r[1], bit)      /* Conditional swap */
    montgomery_step(r[0], r[1]) /* Differential addition + doubling */
    cswap(r[0], r[1], bit)      /* Swap back */
  return pack(r[0])
```

**Constant-Time Properties:**

- Loop always executes 255 iterations (no early exit)
- `cswap` executes regardless of bit value
- `montgomery_step` performs same operations for all inputs
- Memory access pattern is uniform (no secret-dependent indices)

### 4.4 Verification Methodology

**Tools for Constant-Time Verification:**

1. **Valgrind (Memcheck)**: Detects uninitialized reads, buffer overflows
   ```bash
   valgrind --tool=memcheck --error-exitcode=1 ./test_suite
   ```

2. **Valgrind (CacheGrind)**: Analyzes cache behavior
   ```bash
   valgrind --tool=cachegrind --cachegrind-out-file=cg.out ./test_suite
   cg_annotate cg.out | grep -A5 "SECRET_FUNCTION"
   ```

3. **Dudect**: Statistical timing analysis
   ```bash
   # Run with two input classes (different secrets)
   ./dudect_benchmark --class0=secret_a --class1=secret_b
   # Look for t-statistic < 5 (no significant timing difference)
   ```

4. **Compiler Explorer**: Verify no unexpected branches
   ```bash
   gcc -O2 -S -masm=intel crypto_verify_32.c
   # Inspect assembly for conditional jumps on secret data
   ```

---

## 5. Memory Safety Architecture

### 5.1 Stack-Only Design (No Heap)

**Design Decision:** Zero dynamic allocation in cryptographic paths.

**Rationale:**

1. **Predictable Memory Usage**: Critical for embedded/RTOS environments
2. **No Heap Timing Leaks**: `malloc()` timing varies with heap state
3. **Elimination of Allocation Failures**: No need to handle `NULL` from `malloc()`
4. **Simplified Security Audit**: No complex ownership semantics

**Implications:**

- All buffers are caller-allocated
- Maximum message sizes are bounded by stack limits
- Large data must be processed in chunks (streaming API pattern)

**Stack Usage Guidelines:**

| Function | Approximate Stack Usage |
|----------|------------------------|
| `crypto_box` | ~2 KB |
| `crypto_sign` | ~3 KB |
| `crypto_secretbox` | ~512 bytes |
| `crypto_hash` | ~256 bytes |

### 5.2 Secure Wiping Call Graph

```
secure_zero() (volatile write + barrier)
    │
    ├─► secure_memset() (wrapper with size validation)
    │
    └─► Direct calls in crypto functions
            │
            ├─► crypto_box(): wipes sk, shared_key
            ├─► crypto_sign(): wipes expanded_sk, nonce_prefix
            └─► crypto_secretbox(): wipes subkey
```

**Implementation:**

```c
void secure_zero(volatile void* dest, size_t count) {
    volatile unsigned char* p = (volatile unsigned char*)dest;
    while (count--) {
        *p++ = 0;
    }
    /* Compiler barrier - prevents reordering across zero */
    __asm__ __volatile__("" ::: "memory");
}
```

### 5.3 Poison/Unpoison Patterns (Debug Builds)

For debug builds with AddressSanitizer or Valgrind:

```c
#ifdef DEBUG
#include <sanitizer/asan_interface.h>

#define POISON_MEMORY(ptr, size) ASAN_POISON_MEMORY_REGION(ptr, size)
#define UNPOISON_MEMORY(ptr, size) ASAN_UNPOISON_MEMORY_REGION(ptr, size)
#else
#define POISON_MEMORY(ptr, size) ((void)0)
#define UNPOISON_MEMORY(ptr, size) ((void)0)
#endif

/* Usage in crypto function */
void crypto_function(u8* secret_data, size_t len) {
    POISON_MEMORY(secret_data, len);  /* Mark as inaccessible */
    
    /* ... cryptographic operations ... */
    
    secure_zero(secret_data, len);
    UNPOISON_MEMORY(secret_data, len);  /* Safe to access again */
}
```

### 5.4 Stack Canary Compatibility

The library is compatible with stack protector mechanisms:

```bash
# Build with stack canaries
gcc -fstack-protector-strong -o libtweetnacl.so src/*.c

# Canaries protect against buffer overflows on stack
# Library code does not interfere with canary placement
```

**Considerations:**

- Functions with large local arrays may trigger canary checks
- `-fstack-protector-strong` protects functions with local buffers > 8 bytes
- No custom stack manipulation that would break canary checks

---

## 6. Build System Architecture

### 6.1 Multi-Arch Support Design

**Architecture Detection (`arch_config.h`):**

```c
#ifndef ARCH_CONFIG_H
#define ARCH_CONFIG_H

/* Auto-detect architecture from compiler predefined macros */
#if defined(__x86_64__) || defined(_M_X64)
    #define ARCH_X86_64 1
    #define ARCH_NAME "x86_64"
    #define ARCH_ENDIAN_LITTLE 1
#elif defined(__aarch64__) || defined(_M_ARM64)
    #define ARCH_ARM64 1
    #define ARCH_NAME "arm64"
    #define ARCH_ENDIAN_LITTLE 1
#elif defined(__riscv) && (__riscv_xlen == 64)
    #define ARCH_RISCV64 1
    #define ARCH_NAME "riscv64"
    #define ARCH_ENDIAN_LITTLE 1
/* ... more architectures ... */
#endif

/* Endianness detection */
#if defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
    #define ARCH_ENDIAN_BIG 1
#else
    #define ARCH_ENDIAN_LITTLE 1
#endif

#endif
```

### 6.2 Feature Flags and Conditional Compilation

**Supported Feature Flags:**

| Flag | Purpose | Default |
|------|---------|---------|
| `ARCH_X86_64` | Enable x86_64 optimizations | Auto-detected |
| `ARCH_ARM64` | Enable ARM64 optimizations | Auto-detected |
| `WITH_PQC` | Include post-quantum cryptography | 1 |
| `WITH_POSIX` | Use POSIX threads for RNG | 1 |
| `NDEBUG` | Disable debug assertions | 0 (debug) / 1 (release) |
| `FORTIFY_SOURCE` | Runtime buffer checks | 2 |

**Conditional Compilation Example:**

```c
#if defined(ARCH_X86_64) && defined(__AVX2__)
    /* AVX2-optimized implementation */
    #include <immintrin.h>
    static inline void poly1305_avx2(...) { /* ... */ }
    #define POLY1305_IMPL poly1305_avx2
#else
    /* Portable C implementation */
    static inline void poly1305_portable(...) { /* ... */ }
    #define POLY1305_IMPL poly1305_portable
#endif
```

### 6.3 LTO (Link Time Optimization) Friendliness

**LTO Considerations:**

1. **Inline Functions**: LTO can inline across translation units
2. **Visibility Attributes**: Use `__attribute__((visibility("hidden")))` for internal symbols
3. **Whole-Program Optimization**: Enables dead code elimination for unused primitives

**Build with LTO:**

```bash
# GCC LTO
gcc -flto -O2 -c src/*.c
gcc -flto -o libtweetnacl.a *.o

# Clang ThinLTO
clang -flto=thin -O2 -c src/*.c
clang -flto=thin -o libtweetnacl.a *.o
```

**Expected Benefits:**

- 10-20% performance improvement for tight loops
- Dead code elimination reduces binary size
- Cross-module inlining optimizes API boundaries

---

## References

1. Bernstein, D.J. "Curve25519: new Diffie-Hellman speed records." PKC 2006.
2. Bernstein, D.J., et al. "Ed25519: high-speed high-security signatures." CHES 2011.
3. Bernstein, D.J. "The Poly1305-AES message-authentication code." FSE 2005.
4. CERT C Coding Standard. SEI CERT C Coding Standard, 2016.

---

*This document is part of the TweetNaCl-Modular documentation suite. See also: [SECURITY.md](SECURITY.md), [API_REFERENCE.md](API_REFERENCE.md), [CONSTANT_TIME.md](CONSTANT_TIME.md).*
