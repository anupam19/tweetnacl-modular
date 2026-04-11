# TweetNaCl-Modular Porting Guide

**Last Updated:** 2025-01-15  
**Version:** 2.0.0

---

## Table of Contents

1. [Porting from Original TweetNaCl](#1-porting-from-original-tweetnacl)
2. [Porting from NaCl/libsodium](#2-porting-from-nacllibsodium)
3. [Embedded Systems Integration](#3-embedded-systems-integration)
4. [Custom Random Number Generators](#4-custom-random-number-generators)
5. [Memory Allocator Integration](#5-memory-allocator-integration)
6. [Assembly Optimizations](#6-assembly-optimizations)

---

## 1. Porting from Original TweetNaCl

### 1.1 API Compatibility

TweetNaCl-Modular maintains **source-level compatibility** with original TweetNaCl for core cryptographic functions.

**Drop-In Replacement Status:**

| Function Category | Compatibility | Notes |
|------------------|---------------|-------|
| `crypto_box*` | ✅ Drop-in | Same signatures, same behavior |
| `crypto_sign*` | ✅ Drop-in | Same signatures, same behavior |
| `crypto_secretbox*` | ✅ Drop-in | Same signatures, same behavior |
| `crypto_hash*` | ✅ Drop-in | Same signatures, same behavior |
| `crypto_scalarmult*` | ✅ Drop-in | Same signatures, same behavior |
| `crypto_onetimeauth*` | ✅ Drop-in | Same signatures, same behavior |
| `crypto_verify*` | ✅ Drop-in | Same signatures, same behavior |
| `randombytes()` | ⚠️ Check | May need to link separately |

### 1.2 Behavioral Differences

#### Secure Memory Zeroing

**Original TweetNaCl:**
```c
/* May be optimized away by compiler */
for (int i = 0; i < 32; i++) {
    sk[i] = 0;
}
```

**TweetNaCl-Modular:**
```c
/* Guaranteed to persist - volatile + barrier */
secure_zero(sk, 32);
```

**Migration Action:** Replace manual zeroing loops with `secure_zero()`.

#### Header Include Paths

**Original TweetNaCl:**
```c
#include "tweetnacl.h"
```

**TweetNaCl-Modular:**
```c
#include <tweetnacl/tweetnacl.h>  /* System installation */
/* OR */
#include "../includes/tweetnacl.h"  /* Source tree */
```

### 1.3 Performance Considerations

| Aspect | Original | Modular | Impact |
|--------|----------|---------|--------|
| Function call overhead | Minimal (monolithic) | Slightly higher (modular) | <1% difference |
| Inlining opportunities | More (single TU) | Less (multiple TUs) | LTO recovers most |
| Cache locality | Good (single object) | Similar | No significant change |
| Stack usage | Same | Same | Identical algorithms |

**Recommendation:** Enable Link Time Optimization (LTO) to recover inlining benefits:

```bash
gcc -flto -O2 -o myapp myapp.c -ltweetnacl
```

---

## 2. Porting from NaCl/libsodium

### 2.1 API Surface Differences

TweetNaCl-Modular implements a **subset** of libsodium's API surface.

**Available in Both:**

| Function | TweetNaCl-Modular | libsodium | Notes |
|----------|------------------|-----------|-------|
| `crypto_box_keypair` | ✅ | ✅ | Identical |
| `crypto_box_easy` | ❌ | ✅ | Use `crypto_box` with padding |
| `crypto_sign_keypair` | ✅ | ✅ | Identical |
| `crypto_sign_ed25519_pk_to_curve25519` | ❌ | ✅ | Not implemented |
| `crypto_secretbox_easy` | ❌ | ✅ | Use `crypto_secretbox` with padding |
| `crypto_aead_xchacha20poly1305_ietf_*` | ❌ | ✅ | XChaCha20 not included |
| `crypto_kdf_*` | ❌ | ✅ | Use `crypto_hash` for KDF |
| `crypto_pwhash_*` | ❌ | ✅ | Use external Argon2 |

### 2.2 Missing Features (Intentionally)

The following features are **not** included by design:

1. **XChaCha20 / X25519 extensions**: Only ChaCha20/Salsa20 variants
2. **Password hashing**: Use dedicated libraries (Argon2, scrypt)
3. **Key derivation functions**: Implement using `crypto_hash`
4. **Secret streams**: Streaming API not included
5. **Short-input authentication**: Use full `crypto_onetimeauth`

### 2.3 Compatibility Shims

For projects migrating from libsodium, consider these shims:

```c
/* Shim for crypto_box_easy */
int crypto_box_easy(
    unsigned char *c,
    const unsigned char *m,
    unsigned long long mlen,
    const unsigned char *n,
    const unsigned char *pk,
    const unsigned char *sk
) {
    /* Allocate buffer with padding */
    unsigned char padded[mlen + crypto_box_ZEROBYTES];
    unsigned char c_padded[mlen + crypto_box_BOXZEROBYTES];
    
    /* Zero-fill padding area */
    memset(padded, 0, crypto_box_ZEROBYTES);
    memcpy(padded + crypto_box_ZEROBYTES, m, mlen);
    
    /* Encrypt with padding */
    if (crypto_box(c_padded, padded, sizeof(padded), n, pk, sk) != 0) {
        return -1;
    }
    
    /* Copy ciphertext (skip padding overhead) */
    memcpy(c, c_padded + crypto_box_BOXZEROBYTES, mlen + 16);
    return 0;
}

/* Shim for crypto_secretbox_easy */
int crypto_secretbox_easy(
    unsigned char *c,
    const unsigned char *m,
    unsigned long long mlen,
    const unsigned char *n,
    const unsigned char *k
) {
    unsigned char padded[mlen + crypto_secretbox_ZEROBYTES];
    unsigned char c_padded[mlen + crypto_secretbox_BOXZEROBYTES];
    
    memset(padded, 0, crypto_secretbox_ZEROBYTES);
    memcpy(padded + crypto_secretbox_ZEROBYTES, m, mlen);
    
    if (crypto_secretbox(c_padded, padded, sizeof(padded), n, k) != 0) {
        return -1;
    }
    
    memcpy(c, c_padded + crypto_secretbox_BOXZEROBYTES, mlen + 16);
    return 0;
}
```

---

## 3. Embedded Systems Integration

### 3.1 Removing libc Dependencies (Optional)

For bare-metal or RTOS environments, minimize libc dependencies:

**Required Functions:**

| Function | Fallback Implementation |
|----------|------------------------|
| `memcpy` | Custom implementation or compiler builtin |
| `memset` | Replaced by `secure_memset` |
| `memcmp` | Replaced by `secure_memcmp` |

**Build Configuration:**

```makefile
# Makefile for freestanding build
CFLAGS += -ffreestanding -nostdlib -fno-builtin

# Provide minimal libc replacements
OBJS += custom_memcpy.o custom_memset.o

# Or use compiler builtins
LDFLAGS += --use-builtins
```

**Example: Minimal memcpy**

```c
void* memcpy(void* dest, const void* src, size_t n) {
    unsigned char* d = dest;
    const unsigned char* s = src;
    while (n--) {
        *d++ = *s++;
    }
    return dest;
}
```

### 3.2 Stack Size Configuration

**Minimum Stack Requirements:**

| Operation | Stack Usage | Recommended Minimum |
|-----------|-------------|---------------------|
| Hash only | 256 bytes | 512 bytes |
| Secretbox | 512 bytes | 1 KB |
| Box (ECDH + encrypt) | 2 KB | 4 KB |
| Sign (Ed25519) | 3 KB | 8 KB |
| PQC Hybrid | 8 KB | 16 KB |

**Configuration for FreeRTOS:**

```c
/* FreeRTOSConfig.h */
#define configMINIMAL_STACK_SIZE 128  /* For idle task */
#define configSTACK_DEPTH_TYPE uint32_t

/* Task creation with adequate stack */
xTaskCreate(
    crypto_task,           /* Task function */
    "CryptoTask",
    4096,                  /* Stack depth in words (16 KB on 32-bit) */
    NULL,
    tskIDLE_PRIORITY + 2,
    NULL
);
```

### 3.3 ROM/RAM Layout Optimization

**Code Placement:**

```ld
/* linker.ld for embedded */
MEMORY
{
    FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 512K
    RAM (rwx)  : ORIGIN = 0x20000000, LENGTH = 128K
}

SECTIONS
{
    .text : {
        *(.text.crypto*)  /* Place crypto code in flash */
        *(.text*)
    } > FLASH
    
    .rodata : {
        *(.rodata*)       /* Constants in flash */
    } > FLASH
    
    .data : {
        _sidata = LOADADDR(.data);
        _sdata = .;
        *(.data*)
        _edata = .;
    } > RAM AT> FLASH
    
    .bss : {
        _sbss = .;
        *(.bss*)
        *(COMMON)
        _ebss = .;
    } > RAM
}
```

**Size Optimization Flags:**

```bash
# Optimize for size
gcc -Os -ffunction-sections -fdata-sections -c src/*.c

# Link with section garbage collection
gcc -Wl,--gc-sections -o libtweetnacl.a *.o

# Strip debug symbols
strip --strip-all libtweetnacl.a
```

### 3.4 Interrupt Safety Considerations

**Problem:** Crypto operations may be interrupted by ISRs that also use crypto.

**Solution Options:**

1. **Disable Interrupts During Critical Sections:**

```c
void crypto_operation_isr_safe(...) {
    __disable_irq();  /* ARM intrinsic */
    
    /* Perform crypto operation */
    crypto_box(...);
    
    __enable_irq();
}
```

2. **Per-Core State (Multi-core Systems):**

```c
typedef struct {
    u8 shared_state[2048];  /* Per-core workspace */
    bool in_use;
} crypto_core_state_t;

crypto_core_state_t core_states[NUM_CORES];

void crypto_with_isolation(...) {
    int core_id = get_core_id();
    crypto_core_state_t* state = &core_states[core_id];
    
    if (state->in_use) {
        /* Handle reentrancy - queue or error */
        return ERROR_REENTRANT;
    }
    
    state->in_use = true;
    /* ... use state->shared_state ... */
    state->in_use = false;
}
```

---

## 4. Custom Random Number Generators

### 4.1 Interface Requirements

The `randombytes()` function must satisfy:

```c
/**
 * Fill buffer with cryptographically secure random bytes.
 * 
 * @param buf  Output buffer (must not be NULL)
 * @param len  Number of bytes to generate (must be > 0)
 * 
 * Requirements:
 * - Must be cryptographically secure (CSPRNG)
 * - Must not fail silently (abort or loop until success)
 * - Must be thread-safe (if used in multi-threaded context)
 * - Should be fork-safe (reseed after fork if applicable)
 */
void randombytes(u8* buf, size_t len);
```

### 4.2 Example: Hardware RNG Integration

**STM32 Hardware RNG:**

```c
#include <stm32f4xx.h>

void randombytes(u8* buf, size_t len) {
    /* Enable HWRNG peripheral */
    RCC->AHB2ENR |= RCC_AHB2ENR_RNGEN;
    RNG->CR |= RNG_CR_RNGEN;
    
    for (size_t i = 0; i < len; i++) {
        /* Wait for random number ready */
        while (!(RNG->SR & RNG_SR_DRDY)) {
            /* Optional: timeout handling */
        }
        
        /* Check for errors (seed loss, clock error) */
        if (RNG->SR & (RNG_SR_SECS | RNG_SR_CECS)) {
            /* Handle error - maybe fall back to SW RNG */
            RNG->SR = 0;  /* Clear error flags */
        }
        
        buf[i] = (u8)(RNG->DR & 0xFF);
    }
    
    /* Disable HWRNG to save power */
    RNG->CR &= ~RNG_CR_RNGEN;
    RCC->AHB2ENR &= ~RNG_CR_RNGEN;
}
```

**ESP32 Hardware RNG:**

```c
#include "driver/rng.h"

void randombytes(u8* buf, size_t len) {
    /* ESP-IDF provides esp_random() */
    esp_fill_random(buf, len);
}
```

### 4.3 Example: RDRAND/RDSEED on x86

```c
#include <immintrin.h>

static inline int get_rdrand64(uint64_t* val) {
    return _rdrand64_step(val);  /* Returns 1 on success, 0 on failure */
}

void randombytes(u8* buf, size_t len) {
    size_t i = 0;
    
    /* Use RDSEED if available (better entropy source) */
    #ifdef __RDSEED__
        while (i + 8 <= len) {
            uint64_t val;
            int retries = 100;
            
            while (!_rdseed64_step(&val) && --retries > 0) {
                _mm_pause();  /* Hint to CPU */
            }
            
            if (retries == 0) {
                break;  /* Fall back to RDRAND */
            }
            
            memcpy(buf + i, &val, 8);
            i += 8;
        }
    #endif
    
    /* Use RDRAND for remaining bytes */
    #ifdef __RDRND__
        while (i + 8 <= len) {
            uint64_t val;
            int retries = 100;
            
            while (!_rdrand64_step(&val) && --retries > 0) {
                _mm_pause();
            }
            
            if (retries == 0) {
                /* Fatal error - cannot generate randomness */
                abort();
            }
            
            memcpy(buf + i, &val, 8);
            i += 8;
        }
        
        /* Handle remaining bytes (< 8) */
        if (i < len) {
            uint64_t val;
            if (_rdrand64_step(&val)) {
                memcpy(buf + i, &val, len - i);
            } else {
                abort();
            }
        }
    #else
        /* Fallback to /dev/urandom if no RDRAND */
        FILE* f = fopen("/dev/urandom", "rb");
        fread(buf + i, 1, len - i, f);
        fclose(f);
    #endif
}
```

### 4.4 Fork Safety Implementation

**Problem:** After `fork()`, child process inherits RNG state, leading to duplicate random numbers.

**Solution for POSIX Systems:**

```c
#include <pthread.h>
#include <unistd.h>

static pthread_once_t randombytes_initialized = PTHREAD_ONCE_INIT;
static pid_t randombytes_fork_pid = 0;
static FILE* randombytes_urandom = NULL;

static void randombytes_init(void) {
    randombytes_fork_pid = getpid();
    randombytes_urandom = fopen("/dev/urandom", "rb");
    if (!randombytes_urandom) {
        abort();
    }
}

static void randombytes_check_fork(void) {
    pid_t current_pid = getpid();
    
    if (current_pid != randombytes_fork_pid) {
        /* We're in a forked child - reopen /dev/urandom */
        if (randombytes_urandom) {
            fclose(randombytes_urandom);
        }
        randombytes_urandom = fopen("/dev/urandom", "rb");
        randombytes_fork_pid = current_pid;
        
        if (!randombytes_urandom) {
            abort();
        }
    }
}

void randombytes(u8* buf, size_t len) {
    pthread_once(&randombytes_initialized, randombytes_init);
    randombytes_check_fork();
    
    size_t offset = 0;
    while (offset < len) {
        size_t read_len = len - offset;
        if (read_len > 256) {
            read_len = 256;  /* Limit individual reads */
        }
        
        size_t actual = fread(buf + offset, 1, read_len, randombytes_urandom);
        if (actual == 0 && feof(randombytes_urandom)) {
            /* EOF - shouldn't happen with /dev/urandom */
            clearerr(randombytes_urandom);
        }
        
        offset += actual;
    }
}
```

---

## 5. Memory Allocator Integration

### 5.1 Why No malloc (Design Decision)

TweetNaCl-Modular explicitly avoids dynamic allocation because:

1. **Timing Side-Channels**: `malloc()` timing varies with heap state and fragmentation
2. **Failure Handling**: Cryptographic operations should not fail due to memory exhaustion
3. **Predictability**: Embedded systems require deterministic memory usage
4. **Security**: Heap corruption vulnerabilities (use-after-free, double-free)
5. **Auditability**: Static analysis is simpler without allocation patterns

### 5.2 Handling Large Messages (Streaming API Workaround)

For messages exceeding stack limits, implement chunking:

```c
/* Example: Chunked encryption for large files */
#define CHUNK_SIZE (64 * 1024)  /* 64 KB chunks */

int encrypt_large_file(const char* input_path, const char* output_path, 
                       const u8* key, const u8* nonce) {
    FILE* in = fopen(input_path, "rb");
    FILE* out = fopen(output_path, "wb");
    
    u8 plaintext[CHUNK_SIZE + 32];   /* +32 for zero padding */
    u8 ciphertext[CHUNK_SIZE + 48];  /* +16 for Poly1305 tag */
    
    size_t total_read = 0;
    u8 chunk_nonce[24];
    memcpy(chunk_nonce, nonce, 24);
    
    while (1) {
        /* Initialize chunk with zero padding */
        memset(plaintext, 0, 32);
        
        /* Read chunk */
        size_t bytes_read = fread(plaintext + 32, 1, CHUNK_SIZE, in);
        if (bytes_read == 0) {
            break;  /* EOF */
        }
        
        /* Pad to block boundary if needed */
        size_t padded_size = ((bytes_read + 31) / 32) * 32 + 32;
        
        /* Encrypt chunk */
        if (crypto_secretbox(ciphertext, plaintext, padded_size, 
                            chunk_nonce, key) != 0) {
            fclose(in);
            fclose(out);
            return -1;
        }
        
        /* Write ciphertext (skip padding overhead) */
        fwrite(ciphertext + 16, 1, bytes_read + 16, out);
        
        /* Increment nonce for next chunk */
        for (int i = 0; i < 8 && ++chunk_nonce[i] == 0; i++);
        
        total_read += bytes_read;
    }
    
    fclose(in);
    fclose(out);
    return 0;
}
```

### 5.3 Static Buffer Pools

For applications requiring multiple concurrent operations:

```c
#define MAX_CONCURRENT_CRYPTO 4
#define CRYPTO_WORKSPACE_SIZE 4096

typedef struct {
    u8 workspace[CRYPTO_WORKSPACE_SIZE];
    bool in_use;
} crypto_buffer_t;

static crypto_buffer_t buffer_pool[MAX_CONCURRENT_CRYPTO];
static pthread_mutex_t pool_mutex = PTHREAD_MUTEX_INITIALIZER;

crypto_buffer_t* crypto_buffer_acquire(void) {
    pthread_mutex_lock(&pool_mutex);
    
    for (int i = 0; i < MAX_CONCURRENT_CRYPTO; i++) {
        if (!buffer_pool[i].in_use) {
            buffer_pool[i].in_use = true;
            pthread_mutex_unlock(&pool_mutex);
            return &buffer_pool[i];
        }
    }
    
    pthread_mutex_unlock(&pool_mutex);
    return NULL;  /* Pool exhausted */
}

void crypto_buffer_release(crypto_buffer_t* buf) {
    pthread_mutex_lock(&pool_mutex);
    
    /* Find and release */
    for (int i = 0; i < MAX_CONCURRENT_CRYPTO; i++) {
        if (&buffer_pool[i] == buf) {
            buffer_pool[i].in_use = false;
            secure_zero(buffer_pool[i].workspace, sizeof(buffer_pool[i].workspace));
            break;
        }
    }
    
    pthread_mutex_unlock(&pool_mutex);
}

/* Usage */
void example_usage(void) {
    crypto_buffer_t* buf = crypto_buffer_acquire();
    if (!buf) {
        /* Handle pool exhaustion - wait or error */
        return;
    }
    
    /* Use buf->workspace for crypto operations */
    /* ... */
    
    crypto_buffer_release(buf);
}
```

---

## 6. Assembly Optimizations

### 6.1 Where to Add Arch-Specific Code

**Directory Structure:**

```
arch/
├── arch_config.h          # Auto-detection and configuration
├── x86_64/
│   ├── poly1305_avx2.S    # AVX2 Poly1305
│   └── salsa20_sse2.S     # SSE2 Salsa20
├── arm64/
│   ├── curve25519_neon.S  # NEON Curve25519
│   └── sha512_armv8.S     # ARMv8 SHA-512
└── riscv64/
    └── poly1305_rv64.S    # RISC-V64 Poly1305
```

### 6.2 Using the arch/ Directory

**Build System Integration:**

```makefile
# Detect architecture
ARCH ?= $(shell gcc -dumpmachine | cut -d'-' -f1)

ifeq ($(ARCH),x86_64)
    ARCH_DIR = arch/x86_64
    ARCH_FLAGS = -mavx2 -msse2
    ARCH_SRCS = $(wildcard $(ARCH_DIR)/*.S)
endif

ifeq ($(ARCH),aarch64)
    ARCH_DIR = arch/arm64
    ARCH_FLAGS = -march=armv8-a+crypto
    ARCH_SRCS = $(wildcard $(ARCH_DIR)/*.S)
endif

# Compile assembly with appropriate flags
$(ARCH_DIR)/%.o: $(ARCH_DIR)/%.S
	$(CC) $(ARCH_FLAGS) -c $< -o $@

OBJS += $(ARCH_SRCS:.S=.o)
```

### 6.3 Maintaining Constant-Time Properties in ASM

**Critical Rules:**

1. **No Conditional Branches on Secrets**: Use `cmov` (conditional move) instead of `jmp`
2. **Uniform Memory Access**: Avoid secret-dependent indices
3. **Fixed Iteration Counts**: Unroll loops or use fixed counters

**Example: Constant-Time CSwap in ARM Assembly**

```asm
/* ARM64 constant-time conditional swap */
/* Inputs: x0 = &a, x1 = &b, w2 = condition (0 or 1) */
.global constant_time_cswap_arm64
constant_time_cswap_arm64:
    /* Negate condition to create mask: 0xFFFFFFFF or 0x00000000 */
    neg     w3, w2
    
    /* Load values */
    ldr     w4, [x0]
    ldr     w5, [x1]
    
    /* Compute XOR */
    eor     w6, w4, w5
    
    /* Apply mask */
    and     w6, w6, w3
    
    /* Swap */
    eor     w4, w4, w6
    eor     w5, w5, w6
    
    /* Store results */
    str     w4, [x0]
    str     w5, [x1]
    
    ret
/* NO conditional branches - constant time! */
```

### 6.4 Build System Integration

**Makefile Example:**

```makefile
# Multi-architecture build with optional assembly optimizations

CC = gcc
AS = $(CC)

# Default C sources
C_SRCS = src/tweetnacl.c src/secure_mem.c src/randombytes.c
C_OBJS = $(C_SRCS:.c=.o)

# Architecture-specific assembly (optional)
ARCH_ASM_OBJS =

ifeq ($(ARCH),x86_64)
    # Check for AVX2 support
    ifneq ($(shell grep avx2 /proc/cpuinfo),)
        ARCH_ASM_OBJS += arch/x86_64/poly1305_avx2.o
        ARCH_CFLAGS = -mavx2
    endif
endif

ifeq ($(ARCH),aarch64)
    # ARM64 always has NEON
    ARCH_ASM_OBJS += arch/arm64/curve25519_neon.o
    ARCH_CFLAGS = -march=armv8-a+crypto
endif

# Combine all objects
ALL_OBJS = $(C_OBJS) $(ARCH_ASM_OBJS)

# Build rule for assembly
arch/%.o: arch/%.S
	$(AS) $(ARCH_CFLAGS) -c $< -o $@

# Final library
libtweetnacl.a: $(ALL_OBJS)
	ar rcs $@ $^

.PHONY: clean
clean:
	rm -f $(ALL_OBJS) libtweetnacl.a
```

---

## References

1. Bernstein, D.J. "TweetNaCl: A crypto library in 100 tweets." 2014.
2. libsodium Documentation. https://libsodium.gitbook.io/
3. ARM Software Standards. "Arm Architecture Reference Manual."
4. Intel Corporation. "Intel® 64 and IA-32 Architectures Optimization Reference Manual."

---

*This document is part of the TweetNaCl-Modular documentation suite. See also: [ARCHITECTURE.md](ARCHITECTURE.md), [BUILD.md](BUILD.md), [API_REFERENCE.md](API_REFERENCE.md).*
