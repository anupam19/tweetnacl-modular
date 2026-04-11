# TweetNaCl-Modular Build Guide

**Last Updated:** 2025-01-15  
**Version:** 2.0.0

---

## Table of Contents

1. [Quick Start](#1-quick-start)
2. [Compiler Requirements](#2-compiler-requirements)
3. [Cross-Compilation Matrix](#3-cross-compilation-matrix)
4. [Build Options](#4-build-options)
5. [Integration Methods](#5-integration-methods)
6. [Testing](#6-testing)
7. [Installation](#7-installation)

---

## 1. Quick Start

### 1.1 Dependencies

**Required:**

| Component | Minimum Version | Installation (Ubuntu/Debian) |
|-----------|-----------------|------------------------------|
| GCC | 9.0+ | `sudo apt install build-essential` |
| Make | 3.81+ | `sudo apt install make` |
| libc headers | Any POSIX | Included in `build-essential` |

**Optional:**

| Component | Purpose | Installation |
|-----------|---------|--------------|
| Clang | Alternative compiler | `sudo apt install clang` |
| Valgrind | Memory testing | `sudo apt install valgrind` |
| QEMU | Cross-architecture testing | `sudo apt install qemu-user` |

### 1.2 Standard Build Commands

```bash
# Clone repository
git clone https://github.com/your-org/tweetnacl-modular.git
cd tweetnacl-modular

# Build everything (library + tests + example app)
make

# Expected output:
# CC      src/tweetnacl.c
# CC      src/secure_mem.c
# CC      src/randombytes.c
# CC      src/secure_utils.c
# AR      libtweetnacl.a
# LD      app/app
# TEST    Running unit tests...
# All tests passed!

# Run tests
make test

# Clean build artifacts
make clean
```

---

## 2. Compiler Requirements

### 2.1 Supported Compilers

| Compiler | Minimum Version | Recommended Version | Notes |
|----------|-----------------|---------------------|-------|
| GCC | 9.0 | 11.x or later | Full C99 support required |
| Clang | 10.0 | 13.x or later | Better diagnostics |
| ICC | 19.0 | 2021.x | Intel optimizations |

**Verification:**

```bash
# Check GCC version
gcc --version

# Check Clang version
clang --version

# Verify C99 support
echo 'int main(void) { return 0; }' | gcc -std=c99 -xc - -o /dev/null && echo "C99 OK"
```

### 2.2 Required Flags Explanation

The Makefile applies these flags by default:

| Flag | Purpose | Rationale |
|------|---------|-----------|
| `-Wall` | Enable all warnings | Catch potential issues early |
| `-Wextra` | Additional warnings | More thorough checking |
| `-Werror` | Treat warnings as errors | Enforce clean compilation |
| `-std=c99` | C99 standard | Required for fixed-width types |
| `-pedantic` | Strict ISO compliance | Portability assurance |

**Additional Warning Flags:**

```makefile
CFLAGS += -Wformat-security
CFLAGS += -Wpointer-arith
CFLAGS += -Wshadow
CFLAGS += -Wstrict-prototypes
CFLAGS += -Wmissing-prototypes
```

### 2.3 Security Flags

**Recommended Hardening:**

```bash
# Stack protection
-fstack-protector-strong

# Buffer overflow detection (glibc)
-D_FORTIFY_SOURCE=2

# Position-independent code (ASLR compatibility)
-fPIC

# Read-only relocations
-Wl,-z,relro,-z,now

# Bind now (immediate symbol resolution)
-Wl,-z,now
```

**Complete Secure Build:**

```bash
export CFLAGS="-Wall -Wextra -Werror -O2 \
    -fstack-protector-strong \
    -D_FORTIFY_SOURCE=2 \
    -fPIC \
    -fPIE"

export LDFLAGS="-Wl,-z,relro,-z,now -pie"

make clean
make
```

### 2.4 Optimization Levels

| Level | Flag | Use Case | Performance | Size |
|-------|------|----------|-------------|------|
| Debug | `-O0 -g` | Development, debugging | Slowest | Largest |
| Balanced | `-O2` | Default production | Good | Medium |
| Aggressive | `-O3` | Performance-critical | Best | Large |
| Size | `-Os` | Embedded, limited flash | Good | Small |
| Extreme Size | `-Oz` | Severe size constraints | Moderate | Smallest |

**Recommendations:**

- **Desktop/Server**: `-O2` (default)
- **Embedded with space**: `-Os`
- **Performance benchmarking**: `-O3 -march=native`
- **Debugging**: `-O0 -g`

---

## 3. Cross-Compilation Matrix

### 3.1 Supported Architectures

| Architecture | Target Triplet | Status | Tested |
|--------------|---------------|--------|--------|
| x86_64 | `x86_64-linux-gnu` | ✅ Native | CI |
| x86_32 | `i686-linux-gnu` | ✅ Supported | CI |
| ARM64 | `aarch64-linux-gnu` | ✅ Supported | CI |
| ARM32 | `arm-linux-gnueabihf` | ✅ Supported | Manual |
| RISC-V 64 | `riscv64-linux-gnu` | ✅ Supported | Manual |
| RISC-V 32 | `riscv32-linux-gnu` | ⚠️ Experimental | Community |

### 3.2 Required Cross-Compilers

**Ubuntu/Debian Installation:**

```bash
# x86 32-bit (multilib)
sudo apt install gcc-multilib g++-multilib

# ARM 64-bit
sudo apt install gcc-aarch64-linux-gnu

# ARM 32-bit
sudo apt install gcc-arm-linux-gnueabihf

# RISC-V 64-bit
sudo apt install gcc-riscv64-linux-gnu

# RISC-V 32-bit
sudo apt install gcc-riscv32-linux-gnu
```

**macOS (with Homebrew):**

```bash
# Install cross-compilers
brew install filosottile/musl-cross/musl-cross
brew install messense/macos-cross-toolchains/aarch64-unknown-linux-gnu
```

### 3.3 Cross-Compilation Commands

```bash
# x86 64-bit (native on most systems)
make ARCH=x86_64 CC=gcc

# x86 32-bit
make ARCH=x86_32 CC="gcc -m32"

# ARM 64-bit
make ARCH=arm64 CC=aarch64-linux-gnu-gcc

# ARM 32-bit (hard-float)
make ARCH=arm32 CC=arm-linux-gnueabihf-gcc

# RISC-V 64-bit
make ARCH=riscv64 CC=riscv64-linux-gnu-gcc

# RISC-V 32-bit
make ARCH=riscv32 CC=riscv32-linux-gnu-gcc
```

### 3.4 QEMU User-Mode Testing

**Setup:**

```bash
# Install QEMU and architecture support
sudo apt install qemu-user-static

# Register binary formats (run once)
sudo update-binfmts --enable
```

**Testing Cross-Compiled Binaries:**

```bash
# Build for ARM64
make ARCH=arm64 CC=aarch64-linux-gnu-gcc clean all

# Run tests via QEMU
qemu-aarch64-static ./tests/test_all

# Or set QEMU_LD_PREFIX for library path
QEMU_LD_PREFIX=/usr/aarch64-linux-gnu qemu-aarch64-static ./app/app
```

### 3.5 Docker-Based Cross-Compilation

**Dockerfile Example:**

```dockerfile
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    build-essential \
    gcc-aarch64-linux-gnu \
    gcc-arm-linux-gnueabihf \
    gcc-riscv64-linux-gnu \
    qemu-user-static \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace
COPY . .

# Build for multiple architectures
RUN make ARCH=x86_64 && \
    make ARCH=arm64 CC=aarch64-linux-gnu-gcc && \
    make ARCH=arm32 CC=arm-linux-gnueabihf-gcc
```

**Usage:**

```bash
docker build -t tweetnacl-cross .
docker run --rm tweetnacl-cross make test
```

---

## 4. Build Options

### 4.1 Architecture Selection

**Via Make Variable:**

```bash
# Auto-detect (default)
make

# Explicit architecture
make ARCH=x86_64
make ARCH=arm64
```

**Via Environment Variable:**

```bash
export ARCH=arm64
make
```

### 4.2 Debug vs Release

**Debug Build:**

```bash
# Enables debug symbols, disables optimizations
make DEBUG=1

# Equivalent to:
CFLAGS="-g -O0 -DDEBUG" make
```

**Release Build:**

```bash
# Default - optimized, no debug info
make

# Explicit release
make NDEBUG=1

# Equivalent to:
CFLAGS="-O2 -DNDEBUG" make
```

**Comparison:**

| Mode | Flags | Binary Size | Speed | Debug Info |
|------|-------|-------------|-------|------------|
| Debug | `-g -O0` | ~500 KB | 1x | Full |
| Release | `-O2` | ~100 KB | 10x | None |
| Release + LTO | `-O2 -flto` | ~80 KB | 12x | None |

### 4.3 Sanitizers

**AddressSanitizer (Memory Errors):**

```bash
make SANITIZE=address

# Or manually:
CFLAGS="-fsanitize=address -fno-omit-frame-pointer" \
LDFLAGS="-fsanitize=address" \
make

# Run tests
ASAN_OPTIONS=detect_leaks=1 ./tests/test_all
```

**MemorySanitizer (Uninitialized Reads):**

```bash
# Requires Clang
make CC=clang SANITIZE=memory

CFLAGS="-fsanitize=memory -fno-omit-frame-pointer" \
LDFLAGS="-fsanitize=memory" \
make CC=clang
```

**UndefinedBehaviorSanitizer (UB):**

```bash
make SANITIZE=undefined

# Common UB detections:
# - Integer overflow
# - Null pointer dereference
# - Misaligned access
# - Signed integer overflow
```

**ThreadSanitizer (Data Races):**

```bash
make SANITIZE=thread

# Note: May have false positives with atomic operations
```

### 4.4 Link Time Optimization (LTO)

**Enable LTO:**

```bash
# GCC LTO
make LTO=1

# Or manually:
CFLAGS="-flto -O2" \
AR="gcc-ar" \
make
```

**Thin LTO (Clang):**

```bash
make CC=clang LTO=thin

# Thin LTO is faster but may miss some optimizations
```

**Expected Benefits:**

- 10-20% performance improvement
- 5-10% size reduction (dead code elimination)
- Longer compile times (2-3x)

---

## 5. Integration Methods

### 5.1 Shared Library (.so)

**Build:**

```bash
make shared

# Creates: libtweetnacl.so.2.0.0
# Symlinks: libtweetnacl.so.2, libtweetnacl.so
```

**Usage:**

```bash
# Compile against shared library
gcc -o myapp myapp.c -L. -ltweetnacl -Wl,-rpath,.

# Or install system-wide first
sudo make install
gcc -o myapp myapp.c -ltweetnacl
```

**Version Information:**

```bash
# Check library version
ldd --version libtweetnacl.so

# Inspect symbols
nm -D libtweetnacl.so | grep crypto_
```

### 5.2 Static Library (.a)

**Build:**

```bash
make static

# Creates: libtweetnacl.a
```

**Usage:**

```bash
# Link statically
gcc -o myapp myapp.c libtweetnacl.a

# Or specify library path
gcc -o myapp myapp.c -L. -ltweetnacl
```

**Advantages:**

- No runtime dependency
- Easier distribution
- Potentially faster (no PLT overhead)

**Disadvantages:**

- Larger binary
- No automatic security updates

### 5.3 Source Inclusion (Amalgamation)

**Create Amalgamated Source:**

```bash
make amalgamation

# Creates:
# - tweetnacl_amalgamated.c (all source combined)
# - tweetnacl_amalgamated.h (public headers)
```

**Usage:**

```c
/* In your project */
#include "tweetnacl_amalgamated.h"

/* Compile directly with your code */
gcc -o myapp myapp.c tweetnacl_amalgamated.c
```

**Benefits:**

- Single file integration
- No library linking needed
- Easy vendor-in approach

### 5.4 CMake Integration

**Example CMakeLists.txt:**

```cmake
cmake_minimum_required(VERSION 3.10)
project(MyCryptoApp C)

set(CMAKE_C_STANDARD 99)

# Add TweetNaCl as subdirectory
add_subdirectory(tweetnacl-modular)

# Create executable
add_executable(myapp main.c)

# Link library
target_link_libraries(myapp PRIVATE tweetnacl)

# Include headers
target_include_directories(myapp PRIVATE 
    ${CMAKE_SOURCE_DIR}/tweetnacl-modular/includes)
```

**Alternative: FetchContent (CMake 3.11+)**

```cmake
include(FetchContent)

FetchContent_Declare(
    tweetnacl
    GIT_REPOSITORY https://github.com/your-org/tweetnacl-modular.git
    GIT_TAG        v2.0.0
)

FetchContent_MakeAvailable(tweetnacl)

target_link_libraries(myapp PRIVATE tweetnacl)
```

### 5.5 pkg-config File Generation

**Create pkg-config file:**

```bash
# Generate during install
make install

# Creates: /usr/local/lib/pkgconfig/tweetnacl.pc
```

**Content (`tweetnacl.pc`):**

```ini
prefix=/usr/local
exec_prefix=${prefix}
libdir=${exec_prefix}/lib
includedir=${prefix}/include

Name: TweetNaCl-Modular
Description: Modular TweetNaCl with PQC support
Version: 2.0.0
Libs: -L${libdir} -ltweetnacl
Cflags: -I${includedir}
```

**Usage:**

```bash
# Get compiler flags
pkg-config --cflags tweetnacl
# Output: -I/usr/local/include

# Get linker flags
pkg-config --libs tweetnacl
# Output: -L/usr/local/lib -ltweetnacl

# Compile using pkg-config
gcc -o myapp myapp.c $(pkg-config --cflags --libs tweetnacl)
```

---

## 6. Testing

### 6.1 Running Tests

**Full Test Suite:**

```bash
make test

# Output:
# ========================================
# TweetNaCl Unit Tests
# Architecture: x86_64
# ========================================
# test_crypto_verify_16... PASS
# test_crypto_verify_32... PASS
# test_crypto_box... PASS
# ...
# Total tests: 24
# Passed: 24
# Failed: 0
# ========================================
```

**Individual Test Suites:**

```bash
# Build specific test
make tests/test_secure_mem

# Run individual test
./tests/test_secure_mem

# Verbose output
./tests/test_all --verbose
```

### 6.2 Valgrind Testing

**Memory Check:**

```bash
valgrind --tool=memcheck \
    --leak-check=full \
    --show-leak-kinds=all \
    --track-origins=yes \
    ./tests/test_all

# Look for:
# - Invalid reads/writes
# - Uninitialized value usage
# - Memory leaks
```

**Cache Analysis:**

```bash
valgrind --tool=cachegrind \
    --cachegrind-out-file=cg.out \
    ./tests/test_all

# Analyze results
cg_annotate cg.out | head -50
```

**Callgrind (Performance Profiling):**

```bash
valgrind --tool=callgrind \
    --callgrind-out-file=callgrind.out \
    ./tests/test_all

# Visualize with KCachegrind
kcachegrind callgrind.out
```

### 6.3 Fuzzing Corpus Generation

**Using AFL++:**

```bash
# Install AFL++
sudo apt install afl++

# Compile with AFL++ instrumentation
CC=afl-clang-fast make clean all

# Generate initial corpus
mkdir -p fuzz/corpus
python3 generate_corpus.py fuzz/corpus

# Start fuzzing
afl-fuzz -i fuzz/corpus -o fuzz/findings ./tests/fuzz_target
```

**Using libFuzzer:**

```bash
# Compile with libFuzzer support
clang -fsanitize=fuzzer -o fuzz_target fuzz_target.c libtweetnacl.a

# Run with existing corpus
./fuzz_target fuzz/corpus

# Run until crash or timeout
./fuzz_target -max_total_time=3600 fuzz/corpus
```

**Corpus Generation Script:**

```python
#!/usr/bin/env python3
# generate_corpus.py

import os
import random
import sys

def generate_valid_message():
    """Generate a valid encrypted message structure."""
    length = random.randint(32, 1024)
    return bytes(random.randint(0, 255) for _ in range(length))

def generate_key():
    """Generate a 32-byte key."""
    return bytes(random.randint(0, 255) for _ in range(32))

def generate_nonce():
    """Generate a 24-byte nonce."""
    return bytes(random.randint(0, 255) for _ in range(24))

if __name__ == '__main__':
    output_dir = sys.argv[1] if len(sys.argv) > 1 else 'corpus'
    os.makedirs(output_dir, exist_ok=True)
    
    for i in range(100):
        # Valid inputs
        with open(f'{output_dir}/valid_{i:03d}', 'wb') as f:
            f.write(generate_key())
            f.write(generate_nonce())
            f.write(generate_valid_message())
        
        # Edge cases
        with open(f'{output_dir}/edge_{i:03d}', 'wb') as f:
            f.write(b'\x00' * 80)  # All zeros
    
    print(f"Generated 200 corpus files in {output_dir}")
```

---

## 7. Installation

### 7.1 System-Wide Installation

**Default Installation:**

```bash
sudo make install

# Installs to:
# - Library: /usr/local/lib/libtweetnacl.so
# - Static:  /usr/local/lib/libtweetnacl.a
# - Headers: /usr/local/include/tweetnacl/
# - pkg-config: /usr/local/lib/pkgconfig/tweetnacl.pc
```

**Update Library Cache:**

```bash
# Linux
sudo ldconfig

# Verify installation
ldconfig -p | grep tweetnacl
```

### 7.2 Local/Prefix Installation

**Custom Prefix:**

```bash
# Install to custom location
make install PREFIX=$HOME/local

# Or via configure-style variable
make install prefix=/opt/crypto
```

**Resulting Structure:**

```
$HOME/local/
├── bin/
├── lib/
│   ├── libtweetnacl.so
│   ├── libtweetnacl.so.2
│   ├── libtweetnacl.so.2.0.0
│   └── libtweetnacl.a
├── include/
│   └── tweetnacl/
│       ├── tweetnacl.h
│       ├── secure_mem.h
│       ├── randombytes.h
│       └── pqc.h
└── lib/
    └── pkgconfig/
        └── tweetnacl.pc
```

**Usage After Local Install:**

```bash
# Set environment variables
export PATH=$HOME/local/bin:$PATH
export LD_LIBRARY_PATH=$HOME/local/lib:$LD_LIBRARY_PATH
export PKG_CONFIG_PATH=$HOME/local/lib/pkgconfig:$PKG_CONFIG_PATH

# Compile
gcc -o myapp myapp.c \
    -I$HOME/local/include \
    -L$HOME/local/lib \
    -ltweetnacl
```

### 7.3 Uninstallation Procedure

**System-Wide:**

```bash
sudo make uninstall

# Manually verify removal:
rm -f /usr/local/lib/libtweetnacl.*
rm -rf /usr/local/include/tweetnacl
rm -f /usr/local/lib/pkgconfig/tweetnacl.pc
sudo ldconfig
```

**Local Installation:**

```bash
make uninstall PREFIX=$HOME/local

# Or manually:
rm -rf $HOME/local/lib/libtweetnacl.*
rm -rf $HOME/local/include/tweetnacl
rm -f $HOME/local/lib/pkgconfig/tweetnacl.pc
```

### 7.4 Header Organization

**Installed Header Structure:**

```
/usr/local/include/tweetnacl/
├── tweetnacl.h           # Main public API
├── secure_mem.h          # Secure memory operations
├── secure_utils.h        # Utility functions
├── randombytes.h         # RNG interface
├── pqc.h                 # Post-quantum cryptography
└── arch/
    └── arch_config.h     # Architecture configuration (internal)
```

**Include in User Code:**

```c
/* Recommended style */
#include <tweetnacl/tweetnacl.h>
#include <tweetnacl/secure_mem.h>
#include <tweetnacl/pqc.h>

/* Alternative (less preferred) */
#include "tweetnacl.h"
```

---

## References

1. GNU Make Manual. https://www.gnu.org/software/make/manual/
2. GCC Options. https://gcc.gnu.org/onlinedocs/gcc/Option-Summary.html
3. Clang Documentation. https://clang.llvm.org/docs/
4. CMake Documentation. https://cmake.org/documentation/

---

*This document is part of the TweetNaCl-Modular documentation suite. See also: [ARCHITECTURE.md](ARCHITECTURE.md), [API_REFERENCE.md](API_REFERENCE.md), [PORTING_GUIDE.md](PORTING_GUIDE.md).*
