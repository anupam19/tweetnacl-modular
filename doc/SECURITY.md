# TweetNaCl-Modular Security Documentation

**Last Updated:** 2025-01-15  
**Version:** 2.0.0  
**Security Contact:** security@tweetnacl-modular.org

---

## Table of Contents

1. [Threat Model](#1-threat-model)
2. [Constant-Time Guarantees](#2-constant-time-guarantees)
3. [Side-Channel Resistance](#3-side-channel-resistance)
4. [Memory Security](#4-memory-security)
5. [Input Validation](#5-input-validation)
6. [Post-Quantum Security](#6-post-quantum-security)
7. [Vulnerability Disclosure Policy](#7-vulnerability-disclosure-policy)
8. [Security Audit Checklist](#8-security-audit-checklist)

---

## 1. Threat Model

### 1.1 Threats Mitigated

TweetNaCl-Modular is designed to protect against the following threat classes:

#### Timing Attacks

> **Definition:** Attacks that exploit timing variations in cryptographic operations to infer secret data.

**Mitigations:**

- All comparison operations (`crypto_verify_16/32`, `secure_memcmp`) execute in constant time
- Scalar multiplication uses Montgomery ladder with uniform iteration count
- No early-exit branches based on secret data
- Array accesses use uniform patterns (no secret-dependent indices)

**Example - Constant-Time Comparison:**

```c
int secure_memcmp(const void* x, const void* y, size_t n) {
    unsigned char result = 0;
    for (size_t i = 0; i < n; i++) {
        result |= ((volatile unsigned char*)x)[i] ^ ((volatile unsigned char*)y)[i];
    }
    return (result != 0) ? -1 : 0;
}
/* Always reads ALL n bytes, regardless of where differences occur */
```

#### Cache Attacks

> **Definition:** Attacks that monitor cache hit/miss patterns to infer memory access sequences.

**Mitigations:**

- No secret-dependent table lookups (unlike AES S-box implementations)
- Uniform memory access patterns in field arithmetic
- Fixed iteration counts in all loops processing secrets
- No dynamic branching based on secret values

**Verified Functions:**

| Function | Cache-Timing Status |
|----------|---------------------|
| `crypto_scalarmult` | ✅ Resistant |
| `crypto_verify_32` | ✅ Resistant |
| `crypto_sign` | ✅ Resistant |
| `crypto_secretbox` | ✅ Resistant |
| `crypto_hash` | ✅ Resistant |

#### Power Analysis (Basic)

> **Definition:** Attacks that correlate power consumption with computational activity.

**Mitigations:**

- Uniform instruction sequences for secret-dependent operations
- No conditional branches on secret bits
- Constant-time multiply and add operations

> ⚠️ **WARNING**: Basic power analysis resistance is provided, but this library is NOT designed to resist sophisticated Differential Power Analysis (DPA) or Electromagnetic Analysis (EMA). For high-security embedded applications, consider hardware countermeasures or masking schemes.

### 1.2 Threats NOT Mitigated

The following threats are outside the scope of this library:

#### Physical Attacks

- **Side-channel leakage from hardware**: EM emissions, acoustic cryptanalysis
- **Fault injection**: Voltage glitching, clock manipulation, laser fault injection
- **Memory scraping**: Cold boot attacks, DMA attacks via Thunderbolt/PCIe
- **Physical tampering**: Decapsulation, probing, microphotography

#### Speculative Execution Vulnerabilities

- **Spectre (CVE-2017-5753, CVE-2017-5715)**: Branch target injection, bounds check bypass
- **Meltdown (CVE-2017-5754)**: Rogue data cache load
- **Speculative Store Bypass (CVE-2017-5715)**: Variant 4

> ⚠️ **NOTE**: Mitigating speculative execution vulnerabilities requires OS/kernel-level patches and/or CPU microcode updates. Application-level mitigations (e.g., `lfence` instructions) may be added in future versions.

#### Software Vulnerabilities Outside Cryptographic Scope

- **Buffer overflows in calling code**: Library cannot protect against misuse
- **Use-after-free in application**: Secure wiping only covers library-owned stack memory
- **Weak RNG from platform**: Library assumes `randombytes()` provides cryptographic randomness
- **Key management failures**: Storing keys in plaintext, weak key derivation

### 1.3 Platform Assumptions

The security guarantees depend on the following platform assumptions:

| Assumption | Rationale | Verification Method |
|------------|-----------|---------------------|
| **Compiler correctness**: GCC/Clang preserve volatile semantics | `volatile` used for secure wiping | Test with `-O0` through `-O3` |
| **OS provides CSPRNG**: `/dev/urandom` or equivalent is secure | `randombytes()` relies on OS RNG | Check platform documentation |
| **Memory is private**: No unauthorized read access to process memory | Required for key confidentiality | OS isolation, ASLR enabled |
| **Stack is executable-safe**: No return-oriented programming | Stack canaries, NX bit recommended | Enable `-fstack-protector-strong` |
| **Timer resolution limited**: Sub-cycle timing not exposed to attackers | Prevents ultra-fine timing attacks | OS timer throttling |

---

## 2. Constant-Time Guarantees

### 2.1 crypto_verify_16/32 Analysis

**Purpose:** Constant-time comparison for authentication tags.

**Implementation:**

```c
int crypto_verify_16(const unsigned char *x, const unsigned char *y) {
    unsigned int d = 0;
    for (int i = 0; i < 16; i++) {
        d |= x[i] ^ y[i];
    }
    /* Compiler barrier prevents optimization */
    __asm__ __volatile__("" ::: "memory");
    return (1 & ((d - 1) >> 8)) - 1;  /* Returns 0 if equal, -1 if different */
}
```

**Constant-Time Properties:**

1. **Fixed Iteration Count**: Always executes exactly 16 (or 32) iterations
2. **XOR-Accumulate Pattern**: Differences accumulate in `d` without branching
3. **Final Branch Only**: Single comparison at end, independent of input values
4. **Compiler Barrier**: Prevents reordering that might leak intermediate state

**Verification Command:**

```bash
# Compile and inspect assembly
gcc -O2 -S crypto_verify_32.c
grep -E "(j|b|cmp|test)" crypto_verify_32.s
# Should show NO conditional jumps inside the loop
```

### 2.2 Curve25519 Montgomery Ladder Analysis

**Algorithm Overview:**

```
for i = 254 down to 0:
    bit = (scalar[i >> 3] >> (i & 7)) & 1
    cswap(r0, r1, bit)      /* Conditional swap */
    montgomery_step(r0, r1) /* Point addition + doubling */
    cswap(r0, r1, bit)      /* Swap back */
```

**Constant-Time Verification Checklist:**

- [x] Loop runs exactly 255 iterations (fixed)
- [x] Bit extraction uses shifts and masks only (no branches)
- [x] `cswap` executes same instructions regardless of bit value
- [x] `montgomery_step` performs identical operation sequence for all inputs
- [x] Field operations (add, mul, invert) use constant-time algorithms
- [x] Memory access pattern is uniform (array indices don't depend on secrets)

**Reference:** Bernstein, D.J. "Curve25519: new Diffie-Hellman speed records." PKC 2006.

### 2.3 Poly1305 Radix-2^28 Implementation Safety

**Purpose:** One-time authenticator for message integrity.

**Key Safety Properties:**

1. **Clamping Without Branches:**

```c
/* Poly1305 key clamping - constant-time */
r[3] &= 15;        /* Clear top 4 bits */
r[4] &= 15;        /* Clear top 4 bits */
r[0] &= 252;       /* Clear bottom 2 bits (mod 2^130-5) */
/* No conditionals - always executes same operations */
```

2. **Accumulation Without Early Exit:**

```c
/* Process all blocks - no shortcut */
for (size_t i = 0; i < num_blocks; i++) {
    /* Load block, add to accumulator, multiply by r */
    /* Same operations for every block */
}
```

3. **Finalization With Uniform Operations:**

```c
/* Add remaining bytes (if any) - same code path */
/* Pad with 0x01 byte - always executed */
/* Final multiplication by s - always executed */
```

### 2.4 Tools for Verification

#### Valgrind Cachegrind

```bash
# Build with debug symbols
gcc -g -O2 -o test_verify tests/test_verify.c src/*.c

# Run with cachegrind
valgrind --tool=cachegrind --cachegrind-out-file=cg.out ./test_verify

# Analyze cache behavior
cg_annotate cg.out | grep -A10 "crypto_verify"
# Look for consistent cache miss patterns across different inputs
```

#### Dudect (Dual-Mode T-Test)

```bash
# Install dudect
git clone https://github.com/oreparni/dudect.git
cd dudect && make

# Configure for TweetNaCl function
# Edit dudect/constant_time/crypto_verify_32.c with test harness

# Run statistical test
./dudect_ctgram -i 1000000
# Output should show t-statistic < 5 (no significant timing difference)
```

**Interpretation:**

| T-Statistic | Interpretation |
|-------------|----------------|
| < 5 | No detectable leakage |
| 5-10 | Weak evidence of leakage |
| > 10 | Strong evidence of leakage |

---

## 3. Side-Channel Resistance

### 3.1 Cache-Timing Resistance

**Design Principle:** No secret-dependent memory access patterns.

**Anti-Pattern (Cache-Leaky):**

```c
/* DO NOT USE - S-box lookup leaks secret via cache */
uint8_t sbox[256] = { /* ... */ };
uint8_t output = sbox[secret_input];  /* Cache line accessed depends on secret */
```

**Correct Pattern (Cache-Resistant):**

```c
/* TweetNaCl approach - no lookup tables */
/* All operations are arithmetic/logical on registers */
uint64_t result = (input * multiplier) + constant;  /* No table access */
```

**Verification:**

```bash
# Use Intel PIN to trace cache accesses
pin -t source/tools/ManualExamples/obj-intel64/cachesim.so -- ./crypto_test

# Or use Valgrind DHAT (Dynamic Heap Allocation Tracker)
valgrind --tool=dhat ./crypto_test
```

### 3.2 Branch Prediction Safety

**Problem:** Modern CPUs predict branch outcomes; mispredictions leak information.

**CSwap Pattern (Branch-Free):**

```c
/* Instead of: */
if (condition) {
    temp = a; a = b; b = temp;
}

/* Use: */
uint8_t mask = -(uint8_t)condition;  /* 0xFF if true, 0x00 if false */
uint8_t diff = a ^ b;
a ^= mask & diff;
b ^= mask & diff;
/* Same instructions execute regardless of condition */
```

**Assembly Output (x86_64, -O2):**

```asm
; CSwap implementation - no conditional jumps
movzx  eax, BYTE PTR [condition]
neg    al              ; mask = -condition
xor    edx, ebx        ; diff = a ^ b
and    edx, eax        ; mask & diff
xor    ebx, edx        ; b ^= masked_diff
xor    eax, edx        ; a ^= masked_diff
; No 'jcc' (jump conditional) instructions!
```

### 3.3 Speculation Barriers (Compiler Fences)

**Purpose:** Prevent compiler/CPU from speculating past security-critical points.

**Implementation:**

```c
/* Compiler fence - prevents reordering across barrier */
#define COMPILER_BARRIER() __asm__ __volatile__("" ::: "memory")

/* Usage after secret wipe */
secure_zero(secret_key, 32);
COMPILER_BARRIER();  /* Ensures zero completes before function returns */
```

**When Barriers Are Necessary:**

| Location | Barrier Type | Rationale |
|----------|--------------|-----------|
| After `secure_zero()` | Compiler fence | Prevents reordering of wipe |
| Before returning from crypto function | Compiler fence | Ensures all wipes complete |
| After loading secret from memory | Optional | May prevent speculative use |

> ⚠️ **WARNING**: Overuse of barriers harms performance. Only place barriers at security-critical boundaries.

### 3.4 Power Analysis Considerations

**Current Protection Level:** Basic (algorithmic uniformity)

**Not Protected Against:**

- Differential Power Analysis (DPA) with thousands of traces
- Simple Power Analysis (SPA) on distinctive operation sequences
- Electromagnetic Analysis (EMA) with near-field probes

**For High-Security Applications:**

Consider additional countermeasures:

1. **Masking**: Randomize intermediate values with secret shares
2. **Shuffling**: Randomize operation order within algorithm
3. **Hardware Countermeasures**: Noise generation, balanced logic
4. **Protocol-Level**: Limit number of operations with same key

---

## 4. Memory Security

### 4.1 secure_memset Implementation

**Purpose:** Zero memory in a way that cannot be optimized away.

**Implementation:**

```c
void secure_memset(volatile void* dest, int c, size_t n) {
    volatile unsigned char* p = (volatile unsigned char*)dest;
    
    /* Volatile write - compiler must emit store instruction */
    while (n--) {
        *p++ = (unsigned char)c;
    }
    
    /* Compiler memory barrier - prevents reordering */
    __asm__ __volatile__("" ::: "memory");
}
```

**Why `volatile` Works:**

1. **C Standard Guarantee**: Accesses to `volatile` objects are "side effects" that must occur
2. **No Dead Store Elimination**: Compiler cannot remove writes to volatile memory
3. **Ordering Preservation**: Volatile writes occur in program order

**Why `memset_s` Is Preferred When Available:**

```c
#if defined(__STDC_LIB_EXT1__) || defined(__APPLE__)
    /* C11 Annex K / Apple extension */
    memset_s(dest, n, 0, n);  /* Guaranteed non-optimizable */
#else
    secure_memset(dest, 0, n);  /* Fallback to our implementation */
#endif
```

**Advantages of `memset_s`:**

- Part of C11 standard (Annex K)
- Explicitly designed to prevent optimization
- May use platform-specific secure wiping (e.g., `explicit_bzero` on BSD)

### 4.2 Stack Clearing Mechanisms

**Automatic Stack Clearing (Debug Builds):**

```c
#ifdef DEBUG_CLEAR_STACK
#define CLEAR_ON_RETURN(type, var) \
    do { secure_zero(&var, sizeof(var)); } while(0)
#else
#define CLEAR_ON_RETURN(type, var) ((void)0)
#endif

/* Usage in crypto function */
int crypto_box(...) {
    u8 shared_key[32];
    /* ... use shared_key ... */
    CLEAR_ON_RETURN(u8[32], shared_key);
    return result;
}
```

**Manual Clearing (Production):**

Developers must explicitly clear sensitive locals:

```c
int crypto_function(...) {
    u8 secret_nonce[24];
    randombytes(secret_nonce, 24);
    
    /* ... use nonce ... */
    
    secure_zero(secret_nonce, sizeof(secret_nonce));  /* MUST call before return */
    return 0;
}
```

### 4.3 LTO-Safe Wiping

**Problem:** Link Time Optimization can inline across translation units and optimize away "dead" stores.

**Solution:** Use `extern` declaration to prevent inlining of wipe function:

```c
/* In secure_mem.h */
#ifndef SECURE_MEM_H
#define SECURE_MEM_H
#include <stddef.h>

/* Declare as extern to prevent LTO inlining */
extern void secure_zero(volatile void* dest, size_t count);

#endif
```

```c
/* In secure_mem.c */
#include "secure_mem.h"

/* Define with visibility hidden to avoid exporting */
__attribute__((visibility("hidden")))
void secure_zero(volatile void* dest, size_t count) {
    /* ... implementation ... */
}
```

**Verification:**

```bash
# Build with LTO and inspect
gcc -flto -O2 -S secure_mem.c
grep "secure_zero" secure_mem.s
# Should see actual store instructions, not optimized-away calls
```

---

## 5. Input Validation

### 5.1 NULL Pointer Handling

**Policy:** All public API functions validate pointer arguments.

**Implementation Pattern:**

```c
int crypto_box(...) {
    /* Validate all pointers */
    if (!c || !m || !n || !pk || !sk) {
        return -1;  /* Immediate failure */
    }
    
    /* Proceed with cryptographic operation */
    /* ... */
}
```

**Non-Null Annotations:**

```c
/* Inform compiler about non-null expectations */
int crypto_box(
    u8 *c, 
    const u8 *m, 
    const u8 *n, 
    const u8 *pk, 
    const u8 *sk
) __attribute__((nonnull(1, 2, 3, 4, 5)));
```

**Benefits:**

- Compiler can optimize assuming non-null (after validation)
- Static analyzers can detect potential NULL dereferences
- Undefined behavior avoided

### 5.2 Bounds Checking in safe_memcpy

**Purpose:** Prevent buffer overflows during internal copies.

**Implementation:**

```c
int safe_memcpy(void* dest, const void* src, size_t dest_size, size_t count) {
    if (!dest || !src) {
        return -1;
    }
    if (count > dest_size) {
        return -1;  /* Would overflow destination */
    }
    
    memcpy(dest, src, count);
    return 0;
}
```

**Usage:**

```c
u8 buffer[64];
u8 input[128];

/* Safe copy - checks bounds */
if (safe_memcpy(buffer, input, sizeof(buffer), 64) != 0) {
    /* Handle error */
}
```

### 5.3 Integer Overflow Prevention

**Field Arithmetic Example:**

```c
/* Multiplication in GF(2^255-19) */
/* Inputs are reduced modulo 2^255-19, so max value ~2^255 */
/* Product can be up to 2^510, requiring multi-limb arithmetic */

typedef u64 limb_t;

void gf_mul(gf out, const gf a, const gf b) {
    limb_t carry;
    u128 product;  /* 128-bit intermediate to prevent overflow */
    
    /* Multiply with 128-bit intermediates */
    product = (u128)a[0] * b[0];
    carry = product >> 64;
    out[0] = product & MASK_64;
    
    /* ... continue with reduction mod 2^255-19 ... */
}
```

**Safety Checks:**

- Use wider types for intermediate calculations
- Reduce modulo prime after each operation
- Assert bounds in debug builds

---

## 6. Post-Quantum Security

### 6.1 Hybrid Key Exchange Rationale

**Threat:** Quantum computers running Shor's algorithm can break ECDH (Curve25519) and RSA.

**Timeline Estimates:**

| Source | Estimated Quantum Threat |
|--------|-------------------------|
| NIST | 10-30 years |
| NSA/CSS | "Significant progress possible within 10 years" |
| Academic Consensus | Prepare now, deploy gradually |

**Hybrid Approach ("Belt and Suspenders"):**

```
Shared Secret = KDF(Classical_DH_Output || PQC_KEM_Output)
```

**Security Properties:**

- **If classical is broken**: PQC component remains secure
- **If PQC is broken**: Classical component remains secure
- **If both are secure**: Combined security exceeds either alone

### 6.2 Algorithm Security Levels

**NIST Security Categories:**

| Level | Classical Equivalent | PQC Algorithms |
|-------|---------------------|----------------|
| 1 | AES-128 | KYBER512, Dilithium2 |
| 3 | AES-192 | KYBER768, Dilithium3, Falcon-512 |
| 5 | AES-256 | KYBER1024, Dilithium5, Falcon-1024, SPHINCS+ |

**TweetNaCl-Modular Defaults:**

| Primitive | Default Algorithm | Security Level |
|-----------|------------------|----------------|
| KEM | KYBER768 | Level 3 |
| Signature | Dilithium3 | Level 3 |
| Hybrid | X25519 + KYBER768 | Level 3 (both) |

### 6.3 Migration Strategies

**Phase 1: Hybrid Deployment (Current)**

```
Protocol: Send (classical_pk, pqc_pk)
Key Agreement: Compute KDF(ECDH(shared) || KEM(shared))
Signature: Sign with both Ed25519 and Dilithium
```

**Phase 2: PQC-Primary (Future)**

```
Protocol: Send (pqc_pk, classical_pk_as_fallback)
Key Agreement: KEM(shared) primary, ECDH as fallback
Signature: Dilithium primary, Ed25519 as fallback
```

**Phase 3: PQC-Only (Long-term)**

```
Protocol: Send (pqc_pk)
Key Agreement: KEM(shared) only
Signature: PQC signature only
```

### 6.4 Cryptographic Agility

**Design:** Runtime algorithm selection via enum.

```c
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

/* Runtime selection */
pqc_result_t pqc_hybrid_keygen(
    pqc_algorithm_t alg,
    u8* pk, size_t* pk_len,
    u8* sk, size_t* sk_len
);
```

---

## 7. Vulnerability Disclosure Policy

### 7.1 How to Report Security Issues

**Primary Channel:**

- **Email:** security@tweetnacl-modular.org
- **PGP Key:** [Download PGP Key](pgp/security-key.asc)
- **Key Fingerprint:** `XXXX XXXX XXXX XXXX XXXX XXXX XXXX XXXX XXXX XXXX`

**Alternative Channels:**

- **GitHub Security Advisories:** https://github.com/your-org/tweetnacl-modular/security/advisories
- **HackerOne:** (If program is established)

### 7.2 Response Timeline Commitments

| Stage | Target Time | Description |
|-------|-------------|-------------|
| Acknowledgment | 48 hours | Confirm receipt of report |
| Triage | 7 days | Assess severity and validity |
| Fix Development | 30 days | Develop and test patch |
| Coordinated Disclosure | 90 days | Public disclosure after user patch window |

**Severity Classification:**

| Severity | CVSS Score | Response Time |
|----------|------------|---------------|
| Critical | 9.0-10.0 | 7-day patch |
| High | 7.0-8.9 | 30-day patch |
| Medium | 4.0-6.9 | 90-day patch |
| Low | 0.0-3.9 | Next release |

### 7.3 Security Contact Information

**Security Team:**

- Lead: [Name Redacted] <security-lead@...>
- Cryptography: [Name Redacted] <crypto-review@...>
- Engineering: [Name Redacted] <security-eng@...>

**Escalation Path:**

1. Initial report to security@...
2. If no response in 48h, escalate to project maintainer
3. If critical and unresolved, coordinate with CERT/CC

### 7.4 PGP Key for Encrypted Reports

```
-----BEGIN PGP PUBLIC KEY BLOCK-----
Version: OpenPGP.js v4.10.10
Comment: TweetNaCl-Modular Security Team

xsBNBF/... [truncated for template]
-----END PGP PUBLIC KEY BLOCK-----
```

> ⚠️ **NOTE**: Replace with actual PGP key before publication.

---

## 8. Security Audit Checklist

### 8.1 Pre-Release Security Review Procedures

**Mandatory Reviews Before Each Release:**

- [ ] **Code Review**: All changes reviewed by ≥2 cryptography-literate engineers
- [ ] **Diff Audit**: Compare against previous release for unintended changes
- [ ] **Dependency Check**: Verify no new external dependencies introduced
- [ ] **Changelog Review**: Ensure all security-relevant changes documented

**Checklist Template:**

```markdown
## Release X.Y.Z Security Review

### Changes Since Last Release
- [List of commits/PRs]

### Reviewers
- [ ] Reviewer 1: _________________ Date: ___
- [ ] Reviewer 2: _________________ Date: ___

### Findings
- [ ] No security issues identified
- [ ] Issues found and resolved (see below)

### Issues Resolved
| ID | Description | Resolution |
|----|-------------|------------|
|    |             |            |
```

### 8.2 Static Analysis Requirements

**Required Tools (CI Pipeline):**

#### Clang-Tidy

```yaml
# .clang-tidy configuration
Checks: >
  -*,
  bugprone-*,
  cert-*,
  misc-*,
  clang-analyzer-*
WarningsAsErrors: '*'
```

**Run Command:**

```bash
run-clang-tidy -p build/ src/*.c
# Must pass with zero warnings
```

#### GitHub CodeQL

```yaml
# .github/workflows/codeql.yml
name: "CodeQL"
on: [push, pull_request]
jobs:
  analyze:
    runs-on: ubuntu-latest
    steps:
      - uses: github/codeql-action/init@v2
        with:
          languages: cpp
          queries: security-extended
      - uses: github/codeql-action/analyze@v2
```

**Query Focus:**

- Buffer overflows
- Integer overflows
- Use-after-free
- Null pointer dereferences
- Taint analysis (user input → crypto operations)

### 8.3 Fuzzing Requirements

**Continuous Fuzzing Setup:**

#### OSS-Fuzz Integration

```cpp
// fuzz_target.cc
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    // Fuzz crypto_box
    if (size >= 100) {
        u8 pk[32], sk[32], nonce[24], m[100], c[100];
        memcpy(nonce, data, 24);
        memcpy(m, data + 24, 64);
        crypto_box_keypair(pk, sk);
        crypto_box(c, m, 100, nonce, pk, sk);
    }
    return 0;
}
```

**Coverage Goals:**

- ≥90% line coverage for cryptographic primitives
- ≥80% branch coverage for validation code
- Continuous fuzzing (≥10^9 executions/day)

#### Local Fuzzing

```bash
# Using AFL++
afl-clang-fast -O2 -o fuzzer fuzz_target.c libtweetnacl.a
afl-fuzz -i corpus -o findings ./fuzzer

# Using libFuzzer
clang -fsanitize=fuzzer -o fuzzer fuzz_target.c libtweetnacl.a
./fuzzer -max_total_time=3600 corpus/
```

### 8.4 Formal Verification Goals

**Target: Frama-C WP Plugin**

**Verification Goals (Long-term):**

1. **Memory Safety**: Prove no buffer overflows in `crypto_verify_*`
2. **Constant-Time**: Prove no secret-dependent branches in scalar mult
3. **Functional Correctness**: Prove Poly1305 matches RFC 8439 specification

**Example ACSL Annotation:**

```c
/*@ requires \valid(x + (0..15));
  @ requires \valid(y + (0..15));
  @ ensures \result == 0 <==> \forall integer i; 0 <= i < 16 ==> x[i] == y[i];
  @ assigns nothing;
  @*/
int crypto_verify_16(const unsigned char *x, const unsigned char *y);
```

**Current Status:**

- [ ] Proof-of-concept for `crypto_verify_16`
- [ ] Full verification of Poly1305
- [ ] Partial verification of Curve25519
- [ ] Integration into CI pipeline

---

## References

1. Bernstein, D.J., et al. "Ed25519: high-speed high-security signatures." CHES 2011.
2. Bernstein, D.J. "Curve25519: new Diffie-Hellman speed records." PKC 2006.
3. NIST IR 8413. "Status Report on the Third Round of the NIST Post-Quantum Cryptography Standardization Process." 2022.
4. SEI CERT C Coding Standard. Carnegie Mellon University, 2016.
5. Osvik, D.A., et al. "Cache attacks and countermeasures: the case of AES." CT-RSA 2006.
6. Van Goethem, T., et al. "SoK: Practical Foundations for Software Spectre Defenses." S&P 2022.

---

*This document is part of the TweetNaCl-Modular documentation suite. See also: [ARCHITECTURE.md](ARCHITECTURE.md), [CONSTANT_TIME.md](CONSTANT_TIME.md), [API_REFERENCE.md](API_REFERENCE.md).*
