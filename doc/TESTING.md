# TweetNaCl-Modular Testing Guide

**Author:** Anupam Datta <adbd04@gmail.com>  
**Version:** 2.0  
**Last Updated:** 2024

## Table of Contents

1. [Testing Philosophy](#testing-philosophy)
2. [Quick Start](#quick-start)
3. [Test Suite Overview](#test-suite-overview)
4. [Running Tests](#running-tests)
5. [Test Coverage Analysis](#test-coverage-analysis)
6. [Adding New Tests](#adding-new-tests)
7. [Continuous Integration](#continuous-integration)
8. [Fuzzing Guide](#fuzzing-guide)
9. [Memory Safety Tools](#memory-safety-tools)
10. [Performance Regression Testing](#performance-regression-testing)

---

## Testing Philosophy

TweetNaCl-Modular employs a **dual-framework testing strategy** using both CUnit (for C developers) and Google Test (for C++ developers):

### Why Both CUnit and GTest?

| Framework | Purpose | Audience |
|-----------|---------|----------|
| **CUnit** | Lightweight, pure C testing | Embedded developers, C purists |
| **GTest** | Feature-rich, C++ testing | Enterprise adopters, C++ projects |

### Testing Principles

1. **Determinism**: All tests produce identical results across runs (except timing tests)
2. **Isolation**: No shared state between tests
3. **Coverage**: Target >90% line coverage for core crypto functions
4. **Speed**: Complete test suite runs in <30 seconds
5. **Portability**: Tests run on x86, ARM, and RISC-V architectures

---

## Quick Start

### Prerequisites

```bash
# Ubuntu/Debian
sudo apt-get install libcunit1-dev libgtest-dev valgrind gcovr

# Or build GTest from source (recommended for latest version)
```

### Run All Tests

```bash
# Build library first
make

# Run original tests
make test

# Run CUnit tests
make test-cunit

# Run GTest tests
make test-gtest

# Run all tests
make test-all
```

---

## Test Suite Overview

### CUnit Test Suites (`tests/cunit/`)

| Suite | Tests | Description |
|-------|-------|-------------|
| **CoreSuite** | 8 | Hash, verify, scalarmult primitives |
| **BoxSuite** | 4 | Public-key encryption (crypto_box) |
| **SignSuite** | 5 | Digital signatures (crypto_sign) |
| **SecretBoxSuite** | 5 | Secret-key encryption |
| **PqCSuite** | 9 | Post-quantum cryptography interface |
| **SecureMemSuite** | 13 | Memory safety operations |
| **ArchSuite** | 4 | Architecture detection |
| **ConstantTimeSuite** | 3 | Timing attack resistance |

### GTest Test Fixtures (`tests/gtest/`)

| Fixture | Description |
|---------|-------------|
| **TweetNaClTest** | Base fixture with utility methods |
| **PqcTest** | PQC-specific setup |
| **SecureMemTest** | Memory operation tests |
| **MessageSizeTest** | Parameterized size tests |

---

## Running Tests

### Individual Test Suites

```bash
# CUnit tests
cd tests/cunit
make && make run

# GTest tests
cd tests/gtest
make && make run

# With Valgrind
make valgrind
```

### Test Output Examples

**CUnit Verbose Mode:**
```
========================================
TweetNaCl-Modular CUnit Test Suite
========================================

Running suite(s): CoreSuite
  test_hash_deterministic: PASS
  test_hash_different: PASS
  ...

Total tests run: 51
Failures: 0
========================================
```

**GTest Output:**
```
[==========] Running 15 tests from 3 test suites.
[----------] Global test environment set-up.
[----------] 5 tests from TweetNaClTest
[ RUN      ] TweetNaClTest.HashDeterministic
[       OK ] TweetNaClTest.HashDeterministic (0 ms)
...
[==========] 15 tests from 3 test suites ran. (2 ms total)
[  PASSED  ] 15 tests.
```

---

## Test Coverage Analysis

### Generate Coverage Report

```bash
# Clean build
make clean

# Build with coverage flags
make test-coverage

# View HTML report
firefox coverage_html/index.html
```

### Coverage Targets

| Component | Target | Current |
|-----------|--------|---------|
| Core Crypto (tweetnacl.c) | >95% | - |
| Secure Memory | >90% | - |
| PQC Interface | >85% | - |
| Architecture Code | >80% | - |

### Interpreting Coverage

- **Green (>90%)**: Excellent coverage
- **Yellow (70-90%)**: Acceptable, consider additional tests
- **Red (<70%)**: Needs immediate attention

---

## Adding New Tests

### CUnit Test Template

```c
/* tests/cunit/test_my_feature.c */
#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>

static int my_setup(void) { return 0; }
static int my_teardown(void) { return 0; }

static void test_my_feature(void) {
    /* Arrange */
    uint8_t buffer[32];
    
    /* Act */
    my_function(buffer, sizeof(buffer));
    
    /* Assert */
    CU_ASSERT(0 == buffer[0]);
}

CU_ErrorCode create_my_suite(void) {
    CU_pSuite suite = CU_create_suite("MySuite", my_setup, my_teardown);
    if (NULL == suite) return CU_get_error();
    
    CU_add_test(suite, "test_my_feature", test_my_feature);
    return CUE_SUCCESS;
}
```

### GTest Test Template

```cpp
// tests/gtest/test_my_feature.cpp
#include "test_fixture.h"

TEST_F(TweetNaClTest, MyFeatureTest) {
    // Arrange
    std::vector<uint8_t> buffer = randomBytes(32);
    
    // Act
    my_function(buffer.data(), buffer.size());
    
    // Assert
    EXPECT_EQ(0, buffer[0]);
}
```

### Test Naming Conventions

- Use descriptive names: `test_<feature>_<condition>_<expected>`
- Example: `test_box_wrong_key_fails_decryption`

---

## Continuous Integration

### GitHub Actions Workflow

Create `.github/workflows/ci.yml`:

```yaml
name: CI

on: [push, pull_request]

jobs:
  test:
    runs-on: ubuntu-latest
    strategy:
      matrix:
        arch: [native, x86_64]
        cc: [gcc, clang]
    
    steps:
    - uses: actions/checkout@v3
    
    - name: Install dependencies
      run: |
        sudo apt-get update
        sudo apt-get install -y libcunit1-dev valgrind
    
    - name: Build
      run: make ARCH=${{ matrix.arch }} CC=${{ matrix.cc }}
    
    - name: Run tests
      run: make test-all
    
    - name: Valgrind check
      run: make test-valgrind
    
    - name: Upload coverage
      uses: codecov/codecov-action@v3
      with:
        files: ./coverage.info
```

### Required CI Checks

- ✅ All tests pass
- ✅ No Valgrind errors
- ✅ Coverage >90%
- ✅ No compiler warnings

---

## Fuzzing Guide

### AFL++ Fuzzing

```bash
# Install AFL++
sudo apt-get install afl++

# Build with AFL instrumentation
CC=afl-gcc make clean all

# Run fuzzer
afl-fuzz -i tests/vectors/fuzz_corpus -o fuzz_output ./app/app @@
```

### libFuzzer Integration

```bash
# Build with libFuzzer
clang -fsanitize=fuzzer -DFUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION \
    -g -O2 src/*.c tests/fuzz/fuzz_target.c -o fuzz_target

# Run fuzzer
./fuzz_target
```

### Fuzz Corpus

Seed corpus located in `tests/vectors/fuzz_corpus/`:
- Valid cryptographic inputs
- Edge cases (empty, max-size)
- Malformed inputs

---

## Memory Safety Tools

### Valgrind

```bash
# Memory leak detection
make test-valgrind

# Detailed analysis
valgrind --tool=memcheck --leak-check=full \
         --show-leak-kinds=all --track-origins=yes \
         ./tests/test_all
```

### AddressSanitizer (ASan)

```bash
# Build with ASan
export CFLAGS="-fsanitize=address -fno-omit-frame-pointer -g"
export LDFLAGS="-fsanitize=address"
make clean all test

# Run tests (ASan reports on first error)
./tests/test_all
```

### MemorySanitizer (MSan)

```bash
# Requires Clang
export CC=clang
export CFLAGS="-fsanitize=memory -fno-omit-frame-pointer -g"
make clean all test
```

### UndefinedBehaviorSanitizer (UBSan)

```bash
export CFLAGS="-fsanitize=undefined -fno-sanitize-recover=all"
make clean all test
```

---

## Performance Regression Testing

### Benchmark Suite

```bash
# Build benchmarks
cd tests/bench
make && ./bench

# Output example:
Operation           Cycles      Time (μs)
SHA-512 (1KB)       45000       15.2
Ed25519 Sign        2500000     833.4
Kyber768 Keygen     1500000     500.1
```

### Performance Tracking

1. Run benchmarks on each commit
2. Compare against baseline
3. Alert if regression >5%

### Statistical Methods

Use multiple runs and report:
- Mean execution time
- Standard deviation
- 95% confidence interval

```bash
# Multiple runs
for i in {1..10}; do ./bench >> results.txt; done
python3 analyze_performance.py results.txt
```

---

## Troubleshooting

### Common Issues

**CUnit not found:**
```bash
sudo apt-get install libcunit1-dev
# Or download from: http://cunit.sourceforge.net/
```

**GTest compilation errors:**
```bash
# Ensure C++11 or later
export CXXFLAGS="-std=c++11"
```

**Valgrind false positives:**
- Some may be from system libraries
- Focus on errors in TweetNaCl code paths

### Getting Help

- Check existing issues on GitHub
- Contact: adbd04@gmail.com
- See doc/FAQ.md for common questions

---

## References

- [CUnit Documentation](http://cunit.sourceforge.net/doc/index.html)
- [Google Test Primer](https://google.github.io/googletest/primer.html)
- [Valgrind User Guide](https://valgrind.org/docs/manual/manual.html)
- [AFL++ Documentation](https://aflplus.plus/docs/)
