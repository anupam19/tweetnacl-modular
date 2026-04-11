# TweetNaCl-Modular Frequently Asked Questions

**Last Updated:** 2025-01-15  
**Version:** 2.0.0

---

## Table of Contents

1. [General](#1-general)
2. [Security](#2-security)
3. [Technical](#3-technical)
4. [Build/Integration](#4-buildintegration)

---

## 1. General

### Q: Why not just use libsodium?

**A:** TweetNaCl-Modular serves specific use cases where libsodium may not be ideal:

| Aspect | libsodium | TweetNaCl-Modular |
|--------|-----------|-------------------|
| Code Size | ~500 KB | ~100 KB |
| Lines of Code | ~50,000 | ~8,000 |
| Auditability | Complex | Designed for auditability |
| PQC Integration | Experimental | Built-in hybrid support |
| Embedded Focus | General purpose | Optimized for constraints |
| Header Structure | Single header | Modular internal headers |

**Choose TweetNaCl-Modular when:**

- You need maximum auditability (fewer lines of code)
- Binary size is critical (embedded systems)
- You want built-in post-quantum hybrid support
- You prefer modular architecture for selective compilation
- You need fine-grained control over cryptographic primitives

**Choose libsodium when:**

- You need the full feature set (password hashing, secret streams, etc.)
- You want battle-tested production code with wide deployment
- You need platform-specific optimizations out of the box
- You prefer a single-library solution

### Q: Is this production ready?

**A:** TweetNaCl-Modular version 2.0.0 has the following status:

| Component | Status | Notes |
|-----------|--------|-------|
| Core Primitives (box, sign, secretbox) | ✅ Production Ready | Based on audited TweetNaCl |
| Secure Memory Functions | ✅ Production Ready | Tested across platforms |
| PQC Components | ⚠️ Beta | NIST standards finalized 2024 |
| Hybrid Mode | ⚠️ Beta | Under active development |
| Assembly Optimizations | 🧪 Experimental | Architecture-specific |

**Recommendations:**

- **Classical cryptography**: Safe for production use
- **PQC components**: Suitable for early adopters, testing, and hybrid deployments
- **Critical infrastructure**: Wait for independent security audit completion

**Audit Status:**

- Internal review: Complete
- External audit: In progress (expected Q2 2025)
- Fuzzing coverage: >90% for core primitives

### Q: What platforms are tested?

**A:** Continuous Integration covers the following platforms:

| Platform | Architecture | Compiler | CI Status |
|----------|--------------|----------|-----------|
| Ubuntu 22.04 | x86_64 | GCC 11, Clang 14 | ✅ Full |
| Ubuntu 22.04 | x86_64 | GCC 9, GCC 10 | ✅ Full |
| Ubuntu 22.04 | ARM64 (QEMU) | GCC AArch64 | ✅ Tests |
| macOS 13+ | x86_64, ARM64 | Clang 14 | ✅ Full |
| Windows Server 2022 | x86_64 | MSVC 2022 | ✅ Build |
| Alpine Linux | x86_64 | musl-gcc | ✅ Build |

**Manual Testing:**

- FreeBSD 13.x (x86_64)
- OpenBSD 7.x (x86_64)
- ESP32 (Xtensa LX6)
- STM32F4 (ARM Cortex-M4)

---

## 2. Security

### Q: Has this been formally verified?

**A:** Current formal verification status:

| Component | Tool | Status | Coverage |
|-----------|------|--------|----------|
| `crypto_verify_16` | Frama-C WP | 🟡 In Progress | Partial |
| `crypto_verify_32` | Frama-C WP | 🟡 Planned | - |
| Poly1305 | Frama-C WP | 🔴 Not Started | - |
| Curve25519 | Frama-C WP | 🔴 Not Started | - |
| Memory Safety | CBMC | 🟡 In Progress | Partial |

**Goals:**

- Short-term (2025): Complete verification of constant-time comparison functions
- Medium-term (2026): Verify Poly1305 and Salsa20 implementations
- Long-term (2027+): Full functional correctness of Curve25519

**Alternative Approaches:**

While formal verification is in progress, we rely on:

1. **Extensive testing**: Unit tests, property-based tests, fuzzing
2. **Constant-time verification**: dudect, ctgrind analysis
3. **Code review**: Multiple cryptography experts have reviewed the codebase
4. **Heritage**: Original TweetNaCl has been extensively studied since 2014

### Q: How do I report a vulnerability?

**A:** See our [Security Policy](SECURITY.md#7-vulnerability-disclosure-policy) for complete details.

**Quick Summary:**

- **Email:** security@tweetnacl-modular.org
- **PGP Key:** Available at `/pgp/security-key.asc`
- **Response Time:** 48 hours acknowledgment, 90 days coordinated disclosure
- **Bug Bounty:** Not currently offered (planned for 2025)

**What to Include:**

1. Description of the vulnerability
2. Steps to reproduce
3. Impact assessment (if known)
4. Suggested fix (optional)
5. Your contact information

### Q: Is timing attack prevention guaranteed?

**A:** Timing attack resistance depends on multiple factors:

| Factor | Our Control | Platform Dependency |
|--------|-------------|---------------------|
| Algorithm design | ✅ Yes | N/A |
| Implementation patterns | ✅ Yes | N/A |
| Compiler behavior | ⚠️ Partial | Compiler version, flags |
| CPU microarchitecture | ❌ No | Intel vs AMD vs ARM |
| Cache behavior | ⚠️ Partial | Cache size, associativity |
| Branch prediction | ⚠️ Partial | CPU-specific |

**What We Guarantee:**

- All cryptographic operations use constant-time algorithms by design
- No secret-dependent branches in critical paths
- Constant-time comparison functions (`crypto_verify_*`)
- Montgomery ladder for scalar multiplication

**What We Cannot Guarantee:**

- Identical timing across all CPU architectures
- Resistance to sophisticated cache-timing attacks on shared systems
- Protection against speculative execution vulnerabilities (Spectre, Meltdown)

**Best Practices for Users:**

```bash
# Compile with appropriate flags
gcc -O2 -fno-tree-vectorize -mno-avx2 crypto.c

# Avoid running crypto operations on shared systems
# Use dedicated hardware for high-security applications
```

---

## 3. Technical

### Q: Why no dynamic allocation?

**A:** This is an intentional design decision based on several factors:

**1. Predictability:**

```c
/* With malloc: May fail unpredictably */
u8* buffer = malloc(size);
if (!buffer) {
    /* Handle failure - but when does this happen? */
}

/* Without malloc: Failure mode is clear */
u8 buffer[MAX_SIZE];  /* Fails at compile time if too large */
```

**2. Timing Side-Channels:**

- `malloc()` timing varies with heap state and fragmentation
- Allocation patterns can leak information about data sizes
- Free list manipulation may have observable timing

**3. Embedded Systems:**

- Many embedded environments have limited or no heap
- Stack allocation is more predictable for RTOS
- No risk of heap corruption vulnerabilities

**4. Security:**

- Eliminates use-after-free vulnerabilities
- No double-free risks
- Simpler memory ownership semantics

**Workaround for Large Data:**

See [PORTING_GUIDE.md](PORTING_GUIDE.md#52-handling-large-messages-streaming-api-workaround) for chunking strategies.

### Q: Why no forward declarations?

**A:** TweetNaCl-Modular uses a topological include ordering instead:

**Traditional Approach (with forward declarations):**

```c
/* tweetnacl.h */
int func_a(void);
int func_b(void);

/* Forward declaration needed for mutual recursion */
int func_c(void);

int func_a(void) {
    return func_b();
}

int func_b(void) {
    return func_c();
}

int func_c(void) {
    return func_a();  /* Needs forward declaration */
}
```

**TweetNaCl-Modular Approach:**

```c
/* Each .c includes its .h first */
/* Headers are ordered so dependencies come first */

/* src/module_a.c */
#include "module_a.h"  /* Declares func_a */
#include "module_b.h"  /* Declares func_b - included before use */

int func_a(void) {
    return func_b();  /* func_b already declared */
}
```

**Benefits:**

1. **Compile-Time Verification**: If header doesn't declare what implementation needs, compilation fails immediately
2. **No Hidden Dependencies**: All dependencies are explicit in includes
3. **Better IDE Support**: Type information is complete when parsing
4. **Clearer Module Boundaries**: Forces thoughtful API design

### Q: How do I add a new architecture?

**A:** See [PORTING_GUIDE.md](PORTING_GUIDE.md#6-assembly-optimizations) for detailed instructions.

**Quick Start:**

1. **Create architecture directory:**

```bash
mkdir -p arch/myarch
```

2. **Add architecture detection:**

```c
/* arch/arch_config.h */
#if defined(__MYARCH__)
    #define ARCH_MYARCH 1
    #define ARCH_NAME "myarch"
#endif
```

3. **Implement optimized primitives:**

```asm
/* arch/myarch/poly1305_myarch.S */
.global poly1305_blocks_myarch
poly1305_blocks_myarch:
    /* Your optimized implementation */
    ret
```

4. **Update Makefile:**

```makefile
ifeq ($(ARCH),myarch)
    ARCH_DIR = arch/myarch
    ARCH_SRCS = $(wildcard $(ARCH_DIR)/*.S)
endif
```

5. **Test thoroughly:**

```bash
make ARCH=myarch test
```

---

## 4. Build/Integration

### Q: Can I use this in a kernel module?

**A:** Yes, with modifications:

**Requirements:**

1. **Remove libc dependencies:**

```c
/* Replace standard library calls */
memcpy → __builtin_memcpy or custom implementation
memset → secure_memset (already provided)
```

2. **Kernel-compatible types:**

```c
/* Use kernel types */
#include <linux/types.h>
typedef u8 uint8_t;  /* Or use kernel's u8 */
```

3. **No floating point:**

TweetNaCl already avoids floating point, so this is compatible.

**Example Kernel Module:**

```c
#include <linux/module.h>
#include <linux/kernel.h>
#include "../includes/tweetnacl_types.h"
#include "../includes/tweetnacl.h"

static int __init mycrypto_init(void) {
    u8 pk[32], sk[32];
    
    /* Note: randombytes() must be replaced with get_random_bytes() */
    get_random_bytes(sk, 32);
    crypto_scalarmult_base(pk, sk);
    
    printk(KERN_INFO "Generated keypair\n");
    return 0;
}

module_init(mycrypto_init);
MODULE_LICENSE("GPL");
```

**Caveats:**

- Replace `randombytes()` with kernel's `get_random_bytes()`
- Ensure stack usage fits within kernel limits (typically 8 KB)
- No userspace headers in kernel code

### Q: How small can the binary be?

**A:** With aggressive optimization:

| Configuration | Library Size | Notes |
|---------------|--------------|-------|
| Default build | ~100 KB | Balanced |
| `-Os` | ~80 KB | Size optimized |
| `-Oz` + LTO | ~65 KB | Aggressive |
| Selective primitives | ~30 KB | Only hash + verify |
| Amalgamated + stripped | ~25 KB | Minimal configuration |

**Minimal Build Example:**

```bash
# Only include required primitives
gcc -Oz -flto -ffunction-sections -fdata-sections \
    -DCRYPTO_ONLY_HASH -DCRYPTO_ONLY_VERIFY \
    -c src/tweetnacl.c src/secure_mem.c

# Link with section garbage collection
gcc -Wl,--gc-sections -Wl,--strip-all -o libtweetnacl-min.a *.o

# Result: ~25 KB
```

### Q: Does it work on Arduino?

**A:** Yes, with considerations:

**Arduino Uno (ATmega328P):**

| Resource | Available | TweetNaCl Usage | Verdict |
|----------|-----------|-----------------|---------|
| Flash | 32 KB | ~20 KB (minimal) | ⚠️ Tight |
| SRAM | 2 KB | ~1.5 KB (stack) | ⚠️ Tight |
| Clock | 16 MHz | N/A | ✅ OK |

**Arduino Due (SAM3X8E, ARM Cortex-M3):**

| Resource | Available | TweetNaCl Usage | Verdict |
|----------|-----------|-----------------|---------|
| Flash | 512 KB | ~100 KB | ✅ Comfortable |
| SRAM | 96 KB | ~8 KB | ✅ Comfortable |
| Clock | 84 MHz | N/A | ✅ OK |

**Example Arduino Sketch:**

```cpp
#include <tweetnacl.h>

void setup() {
    Serial.begin(9600);
    
    u8 pk[32], sk[32];
    
    /* Generate keypair */
    if (crypto_box_keypair(pk, sk) == 0) {
        Serial.println("Keypair generated!");
        
        /* Print public key */
        Serial.print("PK: ");
        for (int i = 0; i < 32; i++) {
            Serial.print(pk[i], HEX);
            Serial.print(" ");
        }
        Serial.println();
    } else {
        Serial.println("Key generation failed!");
    }
    
    /* Wipe secret key */
    secure_zero(sk, sizeof(sk));
}

void loop() {
    /* Nothing here */
}
```

**Tips for Arduino:**

- Use Arduino Due or ESP32 for better performance
- Consider external RNG for better randomness
- Be mindful of stack usage in recursive functions
- Use `-Os` flag for size optimization

---

## Additional Resources

- **Architecture Details:** [ARCHITECTURE.md](ARCHITECTURE.md)
- **Security Information:** [SECURITY.md](SECURITY.md)
- **API Documentation:** [API_REFERENCE.md](API_REFERENCE.md)
- **Build Instructions:** [BUILD.md](BUILD.md)
- **Porting Guide:** [PORTING_GUIDE.md](PORTING_GUIDE.md)
- **Constant-Time Details:** [CONSTANT_TIME.md](CONSTANT_TIME.md)
- **Post-Quantum Guide:** [PQC.md](PQC.md)

---

*This document is part of the TweetNaCl-Modular documentation suite.*
