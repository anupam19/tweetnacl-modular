# TweetNaCl-Modular Constant-Time Implementation Guide

**Last Updated:** 2025-01-15  
**Version:** 2.0.0

---

## Table of Contents

1. [Theory of Constant-Time Cryptography](#1-theory-of-constant-time-cryptography)
2. [Verification Methodology](#2-verification-methodology)
3. [Function-by-Function Analysis](#3-function-by-function-analysis)
4. [Compiler Barriers](#4-compiler-barriers)
5. [Common Pitfalls](#5-common-pitfalls)
6. [Testing Constant-Time Behavior](#6-testing-constant-time-behavior)

---

## 1. Theory of Constant-Time Cryptography

### 1.1 What is Timing Side-Channel Leakage?

**Definition:** A timing side-channel occurs when the execution time of a cryptographic operation varies based on secret data, allowing an attacker to infer information about those secrets by measuring operation durations.

**Historical Examples:**

| Attack | Year | Target | Impact |
|--------|------|--------|--------|
| Kocher's Timing Attack | 1996 | RSA, Diffie-Hellman | Private key recovery |
| Cache-Timing Attack on AES | 2005 | OpenSSL AES | Full key recovery in 65ms |
| Lucky Thirteen | 2013 | TLS CBC mode | Plaintext recovery |
| Minerva | 2020 | ECDSA (Edwards curves) | Nonce leakage |

**Mathematical Foundation:**

If execution time `T` is a function of secret `s`:

```
T = f(s, public_inputs, noise)
```

An attacker who can measure `T` precisely enough may be able to recover `s` through statistical analysis.

### 1.2 Why Compilers Make This Hard

**Optimization Dangers:**

1. **Dead Store Elimination:**

```c
/* Programmer intends to wipe key */
u8 key[32];
/* ... use key ... */
memset(key, 0, 32);  /* Compiler may remove this! */

/* Compiler reasoning: 'key' goes out of scope immediately,
   and no one reads it after the memset, so the memset is dead code */
```

2. **Loop Unrolling with Early Exit:**

```c
/* Variable-time comparison (BAD) */
for (int i = 0; i < len; i++) {
    if (x[i] != y[i]) {
        return -1;  /* Compiler might optimize to branch-on-first-difference */
    }
}
```

3. **Conditional Move to Branch Conversion:**

```c
/* Might compile to conditional branch on some architectures */
result = condition ? a : b;

/* Compiler output (hypothetical):
   test condition
   jne  use_a      ; Branch leaks condition!
   mov  result, b
   jmp  done
use_a:
   mov  result, a
done:
*/
```

### 1.3 Architecture Considerations

**Variable-Time Instructions:**

| Architecture | Instruction | Issue |
|--------------|-------------|-------|
| x86 | `DIV`, `IDIV` | Division latency depends on operand values |
| x86 | `MULX` | Some implementations vary by operand |
| ARM | `LDREX`/`STREX` | May retry on contention |
| All | Cache misses | Memory access time varies by address |

**Safe Instructions:**

- Integer addition/subtraction (`ADD`, `SUB`)
- Bitwise operations (`AND`, `OR`, `XOR`, `NOT`)
- Shifts by constant amounts (`SHL`, `SHR`)
- Multiplication (on most modern CPUs)

**Recommendation:** Avoid division, modulo, and variable shifts in secret-dependent code paths.

---

## 2. Verification Methodology

### 2.1 Manual Code Audit Checklist

**Pre-Commit Review:**

- [ ] No `if` statements where condition depends on secret data
- [ ] No `switch` statements with secret selector
- [ ] No array indexing with secret indices
- [ ] No loops with secret-dependent iteration counts
- [ ] No early returns based on secret comparisons
- [ ] All comparisons use XOR-accumulate pattern
- [ ] Memory wipes use `volatile` or `secure_zero()`
- [ ] No floating-point operations on secret data

**Checklist Example:**

```c
/* REVIEW: Is this constant-time? */
int check_secret(u8* data, u8 secret_byte) {
    /* BAD: Branch on secret */
    if (data[0] == secret_byte) {  /* ❌ NOT CONSTANT-TIME */
        return 1;
    }
    return 0;
}

/* GOOD: Accumulate without branching */
int check_secret_ct(u8* data, u8 secret_byte) {
    u8 diff = 0;
    for (int i = 0; i < 32; i++) {
        diff |= data[i] ^ secret_byte;  /* ✅ CONSTANT-TIME */
    }
    return (diff == 0) ? 1 : 0;
}
```

### 2.2 Automated Tools

#### dudect (Dual-Mode T-Test)

**Installation:**

```bash
git clone https://github.com/oreparni/dudect.git
cd dudect
make
```

**Usage:**

```c
/* dudect test harness */
#include "dudect/constant_time.h"

static u8 input_data[2][64];
static u8 secret_key[32];

void init(void) {
    randombytes(secret_key, 32);
}

void prepare_inputs(u8* input_data, u8* classes) {
    randombytes(input_data, 64);
    *classes = random() % 2;  /* Class 0 or 1 */
    
    /* Modify input based on class to create distinguishable cases */
    if (*classes == 1) {
        input_data[0] ^= 0xFF;
    }
}

char is_constant_time(void) {
    u8 output[64];
    
    /* Run crypto operation */
    crypto_hash(output, input_data, 64);
    
    /* Measure cycle count (dudect provides instrumentation) */
    return 1;  /* Claim constant-time */
}
```

**Run Test:**

```bash
./ducbench  # Runs 1M+ measurements

# Output interpretation:
# max t = 0.45  → PASS (< 5 threshold)
# max t = 12.3  → FAIL (> 10 strong evidence of leakage)
```

#### ctgrind (Valgrind-based)

```bash
# Patch Valgrind with ctgrind support
git clone https://github.com/presbrey/ctgrind.git
cd ctgrind && make

# Compile code with ctgrind annotations
gcc -g -O2 -I../ctgrind mycode.c -o mycode

# Run with ctgrind-enabled Valgrind
valgrind --tool=ctgrind ./mycode
```

### 2.3 Statistical Testing Methodology

**Sample Size Requirements:**

| Confidence Level | Minimum Samples | Recommended |
|-----------------|-----------------|-------------|
| Preliminary | 10,000 | 100,000 |
| Production | 100,000 | 1,000,000 |
| Security-Critical | 1,000,000 | 10,000,000 |

**T-Test Interpretation:**

```
t-statistic < 5   : No detectable leakage (PASS)
t-statistic 5-10  : Weak evidence (INVESTIGATE)
t-statistic > 10  : Strong evidence of leakage (FAIL)
```

**Measurement Setup:**

1. Disable CPU frequency scaling: `cpufreq-set -g performance`
2. Pin to specific CPU core: `taskset -c 0 ./test`
3. Warm up CPU caches before measurement
4. Use `rdtsc` for cycle-accurate timing (x86)

---

## 3. Function-by-Function Analysis

### 3.1 crypto_scalarmult: Montgomery Ladder Analysis

**Algorithm Structure:**

```c
int crypto_scalarmult(u8 *q, const u8 *n, const u8 *p) {
    gf x0, x1, z0, z1;  /* Projective coordinates */
    
    /* Initialize ladder state */
    fe_frombytes(x0, p);
    fe_1(x1);
    fe_0(z0);
    fe_1(z1);
    
    /* Montgomery ladder: 255 iterations, always */
    for (int pos = 254; pos >= 0; pos--) {
        /* Extract bit without branching */
        u8 bit = (n[pos >> 3] >> (pos & 7)) & 1;
        
        /* Conditional swap - same instructions regardless of bit */
        cswap(x0, x1, bit);
        cswap(z0, z1, bit);
        
        /* Montgomery step - fixed sequence of field operations */
        montgomery_step(x0, x1, z0, z1);
        
        /* Swap back */
        cswap(x0, x1, bit);
        cswap(z0, z1, bit);
    }
    
    /* Pack result */
    fe_tobytes(q, x0);
    
    return 0;
}
```

**Constant-Time Properties:**

| Property | Status | Verification |
|----------|--------|--------------|
| Fixed iteration count | ✅ | Loop always runs 255 times |
| No secret branches | ✅ | Bit extraction uses shifts/masks |
| Uniform memory access | ✅ | Array indices are public |
| CSwap is constant-time | ✅ | Uses XOR-mask pattern |
| Field ops are constant-time | ✅ | No divisions, only add/mul |

**CSwap Implementation:**

```c
static void cswap(gf p, gf q, u8 b) {
    u8 mask = -b;  /* 0xFF if b=1, 0x00 if b=0 */
    
    for (int i = 0; i < 10; i++) {
        u8 x0 = p[i], x1 = q[i];
        u8 diff = x0 ^ x1;
        u8 masked_diff = mask & diff;
        p[i] = x0 ^ masked_diff;
        q[i] = x1 ^ masked_diff;
    }
}
```

**Assembly Verification (x86_64, -O2):**

```asm
; CSwap inner loop - note NO conditional jumps
.L_cswap_loop:
    movzx  eax, BYTE PTR [rdi+rcx]    ; load p[i]
    movzx  edx, BYTE PTR [rsi+rcx]    ; load q[i]
    xor    eax, edx                   ; diff = p[i] ^ q[i]
    and    eax, r8d                   ; mask & diff (r8d = mask)
    xor    DWORD PTR [rdi+rcx], eax   ; p[i] ^= masked_diff
    xor    DWORD PTR [rsi+rcx], eax   ; q[i] ^= masked_diff
    inc    rcx
    cmp    rcx, 10
    jne    .L_cswap_loop              ; Only loop counter branch
```

### 3.2 crypto_verify_*: XOR-Accumulate Pattern

**Implementation:**

```c
int crypto_verify_32(const unsigned char *x, const unsigned char *y) {
    unsigned int d = 0;
    
    for (int i = 0; i < 32; i++) {
        d |= x[i] ^ y[i];  /* Accumulate differences */
    }
    
    /* Compiler barrier */
    __asm__ __volatile__("" ::: "memory");
    
    /* Convert to return value: 0 if equal, -1 if different */
    return (1 & ((d - 1) >> 8)) - 1;
}
```

**Why This Works:**

1. **XOR Property:** `a ^ b = 0` iff `a = b`
2. **OR Accumulation:** Once any bit is set, it stays set
3. **Final Conversion:** `(d - 1) >> 8` maps:
   - `d = 0` → `(0-1) >> 8 = 0xFFFFFFFF >> 8 = 0x00FFFFFF & 1 = 1` → `1 - 1 = 0`
   - `d > 0` → `(d-1) >> 8 >= 0` → `1 & ... = 0 or 1` → result is `-1 or 0`

**Timing Analysis:**

```
For equal inputs:    32 XORs + 32 ORs + final conversion
For different inputs: 32 XORs + 32 ORs + final conversion
                     (SAME number of instructions!)
```

### 3.3 crypto_onetimeauth: Poly1305 Clamping

**Key Clamping (Constant-Time):**

```c
void poly1305_key_clamp(u8 key[32]) {
    /* Clamp r (first 16 bytes) */
    key[3]  &= 15;   /* Clear top 4 bits */
    key[4]  &= 15;   /* Clear top 4 bits */
    key[8]  &= 15;   /* Clear top 4 bits */
    key[9]  &= 15;   /* Clear top 4 bits */
    key[12] &= 15;   /* Clear top 4 bits */
    key[13] &= 15;   /* Clear top 4 bits */
    
    /* Clear bottom 2 bits of r (mod 2^130-5 requirement) */
    key[0]  &= 252;
    key[4]  &= 252;
    key[8]  &= 252;
    key[12] &= 252;
    
    /* Note: No conditionals - always executes same operations */
}
```

**Accumulator Update (Radix-2^28):**

```c
/* Process one 16-byte block */
void poly1305_block(poly1305_state* st, const u8* block) {
    u128 h0, h1, h2;  /* 29-bit limbs */
    u128 r0, r1, r2, r3, r4;
    u128 s0, s1, s2, s3, s4;
    
    /* Load message block - little-endian */
    /* Same operations regardless of content */
    h0 = st->h[0] + ((u128)LOAD32_LE(block) & 0x3ffffff);
    h1 = st->h[1] + ((u128)(LOAD32_LE(block+3) >> 2) & 0x3ffffff);
    /* ... continue for all limbs ... */
    
    /* Multiply-accumulate with r */
    /* Fixed sequence of operations */
    s0 = h0*r0 + h1*r4 + h2*r3 + h3*r2 + h4*r1;
    s1 = h0*r1 + h1*r0 + h2*r4 + h3*r3 + h4*r2;
    /* ... */
    
    /* Reduce mod 2^130-5 */
    /* Always executes same reduction steps */
}
```

### 3.4 crypto_sign: Ed25519 Double-Scalar Multiply

**Signature Verification:**

```c
int crypto_sign_verify(const u8* sm, size_t smlen, const u8* pk) {
    ge_p3 A;           /* Public key point */
    ge_p2 R_check;     /* Computed R point */
    
    /* Decode public key */
    if (ge_frombytes_negate_vartime(&A, pk) != 0) {
        return -1;
    }
    
    /* Compute h = H(R || A || M) */
    u8 h[64];
    crypto_hash(h, sm, 32 + smlen);  /* R || A || M */
    
    /* Double-scalar multiplication: R_check = h*A + B*R */
    /* This is the sensitive part - must be constant-time */
    ge_double_scalarmult_vartime(&R_check, h, &A, sm);  /* sm starts with R */
    
    /* Encode and compare */
    u8 R_check_bytes[32];
    ge_tobytes(R_check_bytes, &R_check);
    
    /* Constant-time comparison */
    return crypto_verify_32(sm, R_check_bytes);
}
```

**Note:** The `_vartime` suffix indicates this function has some variable-time components. For highest security applications, consider using constant-time alternatives.

---

## 4. Compiler Barriers

### 4.1 Why volatile is Used

**C Standard Guarantee (C11 6.7.3.7):**

> "Accessing a volatile object is a side effect. The implementation shall not remove or reorder such accesses."

**Practical Effect:**

```c
volatile u8* p = secret_buffer;
while (len--) {
    *p++ = 0;  /* Compiler MUST emit a store instruction */
}
```

**Without volatile:**

```c
u8* p = secret_buffer;
while (len--) {
    *p++ = 0;  /* Compiler may optimize away if buffer not read again */
}
```

### 4.2 __asm__ __volatile__("" ::: "memory") Explanation

**Breakdown:**

```c
__asm__ __volatile__(    /* Inline assembly, don't optimize */
    ""                   /* Empty instruction string */
    ::: 
    "memory"             /* Clobber list: memory is affected */
);
```

**Effect:**

- Tells compiler that memory may be modified
- Prevents reordering of memory operations across the barrier
- Does NOT emit any actual machine code

**Usage Pattern:**

```c
void secure_function(u8* secret) {
    /* ... use secret ... */
    
    /* Wipe secret */
    secure_zero(secret, 32);
    
    /* Ensure wipe completes before any subsequent operations */
    __asm__ __volatile__("" ::: "memory");
    
    /* Function returns - secret is definitely wiped */
}
```

### 4.3 When Barriers Are Necessary vs Harmful

**Necessary:**

| Location | Reason |
|----------|--------|
| After `secure_zero()` | Prevent reordering of wipe |
| Before returning from crypto function | Ensure all wipes complete |
| Between secret load and use | Prevent speculative use (optional) |

**Harmful (Performance Impact):**

```c
/* DON'T: Excessive barriers slow down computation */
for (int i = 0; i < 255; i++) {
    do_something();
    __asm__ __volatile__("" ::: "memory");  /* UNNECESSARY - hurts performance */
}

/* DO: Barrier only at critical boundaries */
for (int i = 0; i < 255; i++) {
    do_something();  /* No barrier inside loop */
}
__asm__ __volatile__("" ::: "memory");  /* One barrier at end */
```

**Performance Measurement:**

```bash
# With excessive barriers
time ./crypto_bench --barriers=every
# Result: ~5000 cycles per operation

# With minimal barriers
time ./crypto_bench --barriers=minimal
# Result: ~500 cycles per operation
```

---

## 5. Common Pitfalls

### 5.1 Memory Access Patterns (Cache Lines)

**Problem:** Even without secret-dependent branches, cache behavior can leak information.

**Anti-Pattern:**

```c
/* Lookup table indexed by secret - LEAKS via cache timing */
u8 sbox[256] = { /* ... */ };
u8 output = sbox[secret_input];  /* Cache line accessed depends on secret */
```

**Solution:**

```c
/* No lookup tables - compute directly */
/* TweetNaCl approach: use only arithmetic/logical operations */
u64 result = (input * constant1) ^ constant2;
```

### 5.2 Branch Predictor Training

**Attack:** Attacker can "train" the branch predictor to mispredict on secret-dependent branches, causing measurable timing differences.

**Vulnerable Code:**

```c
/* Branch depends on secret bit */
if (secret_byte & 0x01) {
    do_a();
} else {
    do_b();
}
```

**Fix:**

```c
/* Convert to branch-free selection */
void (*funcs[2])(void) = {do_b, do_a};
funcs[secret_byte & 0x01]();  /* Still has indirect branch - better but not perfect */

/* Best: inline both paths */
u8 mask = -(secret_byte & 0x01);
result_a = compute_a();
result_b = compute_b();
final_result = (mask & result_a) | (~mask & result_b);
```

### 5.3 Speculative Execution Leaks

**Problem:** CPU may speculatively execute past bounds checks (Spectre variant 1).

**Vulnerable Pattern:**

```c
if (index < array_size) {
    secret = array[index];  /* Speculative execution may access out-of-bounds */
}
```

**Mitigation:**

```c
/* LFENCE serialization (x86) */
if (index >= array_size) {
    return error;
}
__asm__ __volatile__("lfence" ::: "memory");  /* Block speculation */
secret = array[index];

/* Or use index masking */
size_t safe_index = index & (array_size - 1);  /* Assumes power-of-2 size */
secret = array[safe_index];
```

### 5.4 Floating Point Non-Determinism

**Why We Avoid Float:**

1. **Variable Latency:** FP operations may take different cycles based on values
2. **Precision Differences:** Results may vary between CPU generations
3. **Denormal Handling:** Subnormal numbers cause massive slowdowns

**TweetNaCl Approach:**

```c
/* All integer arithmetic - no floating point */
typedef u64 limb_t;

void field_add(limb_t* out, const limb_t* a, const limb_t* b) {
    /* Pure integer addition with carry */
    u128 sum = (u128)a[0] + b[0];
    out[0] = sum & MASK;
    limb_t carry = sum >> LIMB_BITS;
    /* ... */
}
```

---

## 6. Testing Constant-Time Behavior

### 6.1 Unit Tests for Constant-Time Helpers

**Test Suite Example:**

```c
#include <assert.h>
#include <time.h>

/* Test that secure_memcmp always takes same time */
void test_secure_memcmp_constant_time(void) {
    u8 buf1[32], buf2[32];
    clock_t time_equal = 0, time_different = 0;
    const int iterations = 100000;
    
    /* Initialize buffers */
    memset(buf1, 0x42, sizeof(buf1));
    memset(buf2, 0x42, sizeof(buf2));
    
    /* Time equal comparison */
    clock_t start = clock();
    for (int i = 0; i < iterations; i++) {
        secure_memcmp(buf1, buf2, 32);
    }
    time_equal = clock() - start;
    
    /* Make buffers different */
    buf2[31] ^= 0x01;
    
    /* Time different comparison */
    start = clock();
    for (int i = 0; i < iterations; i++) {
        secure_memcmp(buf1, buf2, 32);
    }
    time_different = clock() - start;
    
    /* Times should be similar (within 20% tolerance) */
    double ratio = (double)time_different / time_equal;
    assert(ratio > 0.8 && ratio < 1.2);
    
    printf("secure_memcmp timing ratio: %.3f (PASS)\n", ratio);
}
```

### 6.2 Statistical Timing Tests

**Setup:**

```c
#define SAMPLE_COUNT 1000000

typedef struct {
    u64 samples[SAMPLE_COUNT];
    size_t count;
} timing_buffer_t;

static inline u64 rdtsc(void) {
    unsigned int lo, hi;
    __asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
    return ((u64)hi << 32) | lo;
}

void measure_timing(timing_buffer_t* buf, void (*func)(void)) {
    /* Warmup */
    for (int i = 0; i < 1000; i++) {
        func();
    }
    
    /* Measurement */
    buf->count = 0;
    for (int i = 0; i < SAMPLE_COUNT && buf->count < SAMPLE_COUNT; i++) {
        u64 start = rdtsc();
        func();
        u64 end = rdtsc();
        
        /* Filter outliers (e.g., interrupts) */
        if (end - start < 10000) {  /* Threshold depends on function */
            buf->samples[buf->count++] = end - start;
        }
    }
}
```

### 6.3 CI Integration for Regression Detection

**GitHub Actions Workflow:**

```yaml
name: Constant-Time Verification

on: [push, pull_request]

jobs:
  dudect-test:
    runs-on: ubuntu-latest
    
    steps:
    - uses: actions/checkout@v3
    
    - name: Install dependencies
      run: |
        sudo apt-get update
        sudo apt-get install -y cmake build-essential
    
    - name: Build dudect
      run: |
        git clone https://github.com/oreparni/dudect.git
        cd dudect && make
    
    - name: Run constant-time tests
      run: |
        cd tests/constant_time
        make
        ./run_dudect_test --samples=100000
    
    - name: Check results
      run: |
        if grep -q "FAIL" dudect_output.txt; then
          echo "Constant-time verification FAILED"
          exit 1
        fi
        echo "All constant-time tests PASSED"
```

**Threshold Monitoring:**

```python
# analyze_timing.py
import sys
import statistics

def analyze(results_file):
    with open(results_file) as f:
        timings = [float(line) for line in f]
    
    mean = statistics.mean(timings)
    stdev = statistics.stdev(timings)
    cv = stdev / mean  # Coefficient of variation
    
    print(f"Mean: {mean:.2f} cycles")
    print(f"StdDev: {stdev:.2f} cycles")
    print(f"Coefficient of Variation: {cv:.4f}")
    
    # CV should be < 0.01 for constant-time code
    if cv > 0.01:
        print("WARNING: High variance detected - possible timing leak")
        return 1
    return 0

if __name__ == '__main__':
    sys.exit(analyze(sys.argv[1]))
```

---

## References

1. Bernstein, D.J. "Curve25519: new Diffie-Hellman speed records." PKC 2006.
2. Osvik, D.A., et al. "Cache attacks and countermeasures: the case of AES." CT-RSA 2006.
3. Percival, C. "Cache missing for fun and profit." BSDCan 2005.
4. Van Goethem, T., et al. "SoK: Practical Foundations for Software Spectre Defenses." S&P 2022.
5. Reparaz, O., et al. "Masking the GLP Lattice-Based Signature Scheme at Arbitrary Order." CHES 2018.

---

*This document is part of the TweetNaCl-Modular documentation suite. See also: [ARCHITECTURE.md](ARCHITECTURE.md), [SECURITY.md](SECURITY.md), [API_REFERENCE.md](API_REFERENCE.md).*
