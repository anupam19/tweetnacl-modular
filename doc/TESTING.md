# TweetNaCl-Modular Testing Guide

## Testing Philosophy

TweetNaCl-Modular employs a dual testing framework approach using **CUnit** for C-based tests and **Google Test (GTest)** for C++ integration tests. This ensures maximum compatibility across different project environments and provides comprehensive coverage of all cryptographic primitives.

### Why CUnit + GTest?

- **CUnit**: Lightweight, pure C testing framework ideal for embedded systems and C-only projects
- **GTest**: Feature-rich C++ framework with advanced assertions, parameterized tests, and death tests
- **Dual Support**: Ensures the library works seamlessly in both C and C++ environments

---

## Quick Start

### Running All Tests

```bash
# Build and run all tests
make test-all

# Run only CUnit tests
make test-cunit

# Run only GTest tests
make test-gtest

# Run original custom tests
make test-original
```

### Prerequisites

```bash
# Install CUnit (Ubuntu/Debian)
sudo apt-get install libcunit1-dev

# Install Google Test (Ubuntu/Debian)
sudo apt-get install libgtest-dev cmake

# Install Valgrind for memory checking
sudo apt-get install valgrind

# Install coverage tools
sudo apt-get install gcov lcov
```

---

## Test Suite Organization

### CUnit Test Suites (`tests/cunit/`)

| Suite | Purpose | Functions Tested |
|-------|---------|------------------|
| **CoreSuite** | Core cryptographic primitives | `crypto_hash`, `crypto_verify`, `crypto_scalarmult` |
| **BoxSuite** | Public-key encryption | `crypto_box`, `crypto_box_open`, `crypto_box_keypair` |
| **SignSuite** | Digital signatures | `crypto_sign`, `crypto_sign_open`, `crypto_sign_keypair` |
| **SecretBoxSuite** | Secret-key encryption | `crypto_secretbox`, `crypto_secretbox_open` |
| **PqCSuite** | Post-quantum cryptography | All `pqc_*` functions (Kyber, Dilithium, Falcon) |
| **SecureMemSuite** | Memory safety | `secure_memset`, `secure_zero`, `secure_memcmp` |
| **ArchSuite** | Architecture detection | Endianness, CPU feature detection |
| **ConstantTimeSuite** | Timing attack resistance | Statistical timing analysis |

### GTest Test Suites (`tests/gtest/`)

| Test Class | Type | Features |
|------------|------|----------|
| **TweetNaClTest** | Base fixture | Common setup/teardown, key generation |
| **PqcTest** | Parameterized | Different PQC algorithms (Kyber512/768/1024, Dilithium2/3/5) |
| **SecureMemTest** | Death tests | Memory wiping verification, NULL pointer handling |
| **CrossArchTest** | Parameterized | Multi-architecture verification |

---

## Test Coverage Analysis

### Coverage Requirements

- **Target**: >90% line coverage for core crypto functions
- **Critical paths**: 100% coverage for security-sensitive code
- **Boundary conditions**: All edge cases tested (0-byte, max-size messages)

### Generating Coverage Reports

```bash
# Generate coverage report
make test-coverage

# View HTML report
firefox coverage_html/index.html
```

### Coverage Report Structure

```
coverage_html/
├── index.html          # Overall coverage summary
├── src/
│   ├── tweetnacl.c.gcov.html
│   ├── secure_mem.c.gcov.html
│   └── ...
└── tests/
    └── ...
```

---

## Adding New Tests

### CUnit Test Example

```c
#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include "tweetnacl.h"

static int setup(void) {
    /* Initialize test state */
    return 0;
}

static int teardown(void) {
    /* Clean up test state */
    return 0;
}

static void test_crypto_box_basic(void) {
    uint8_t pk[32], sk[32];
    uint8_t nonce[24];
    uint8_t m[64], c[64], d[64];
    
    /* Generate keypair */
    CU_ASSERT_EQUAL(crypto_box_keypair(pk, sk), 0);
    
    /* Generate nonce */
    randombytes(nonce, 24);
    
    /* Encrypt */
    CU_ASSERT_EQUAL(crypto_box(c, m, sizeof(m), nonce, pk, sk), 0);
    
    /* Decrypt */
    CU_ASSERT_EQUAL(crypto_box_open(d, c, sizeof(c), nonce, pk, sk), 0);
    
    /* Verify plaintext matches */
    CU_ASSERT_EQUAL(memcmp(m, d, sizeof(m)), 0);
}

void register_core_suite(CU_pRegistry registry) {
    CU_pSuite suite = CU_add_suite(registry, "CoreSuite", setup, teardown);
    CU_add_test(suite, "test_crypto_box_basic", test_crypto_box_basic);
}
```

### GTest Test Example

```cpp
#include <gtest/gtest.h>
#include "tweetnacl.h"

class TweetNaClTest : public ::testing::Test {
protected:
    void SetUp() override {
        /* Initialize test state */
    }
    
    void TearDown() override {
        /* Clean up test state */
    }
    
    std::vector<uint8_t> generate_random(size_t len) {
        std::vector<uint8_t> buf(len);
        randombytes(buf.data(), len);
        return buf;
    }
};

TEST_F(TweetNaClTest, CryptoBoxRoundTrip) {
    uint8_t pk[32], sk[32];
    ASSERT_EQ(crypto_box_keypair(pk, sk), 0);
    
    auto nonce = generate_random(24);
    auto msg = generate_random(64);
    
    std::vector<uint8_t> ct(msg.size());
    std::vector<uint8_t> pt(msg.size());
    
    ASSERT_EQ(crypto_box(ct.data(), msg.data(), msg.size(), 
                         nonce.data(), pk, sk), 0);
    ASSERT_EQ(crypto_box_open(pt.data(), ct.data(), ct.size(),
                              nonce.data(), pk, sk), 0);
    
    EXPECT_EQ(msg, pt);
}

/* Parameterized test for different message sizes */
class BoxSizeTest : public TweetNaClTest,
                    public ::testing::WithParamInterface<size_t> {};

TEST_P(BoxSizeTest, VariableLengthMessages) {
    size_t msg_size = GetParam();
    /* Test implementation */
}

INSTANTIATE_TEST_SUITE_P(
    MessageSizes,
    BoxSizeTest,
    ::testing::Values(0, 1, 32, 1024, 65536));
```

---

## Continuous Integration

### GitHub Actions Configuration

The repository includes `.github/workflows/ci.yml` which runs:

- **Build Matrix**: Multiple architectures (native, x86_32, ARM64)
- **Compiler Matrix**: GCC and Clang
- **Test Execution**: All test suites
- **Memory Checking**: Valgrind memcheck
- **Coverage Reporting**: Automatic upload to Codecov

### CI Workflow Steps

```yaml
name: CI

on: [push, pull_request]

jobs:
  build:
    runs-on: ubuntu-latest
    steps:
    - uses: actions/checkout@v3
    
    - name: Install dependencies
      run: sudo apt-get install -y libcunit1-dev valgrind
    
    - name: Build
      run: make
    
    - name: Run tests
      run: make test-all
    
    - name: Valgrind check
      run: make test-valgrind
    
    - name: Upload coverage
      uses: codecov/codecov-action@v3
```

---

## Fuzzing Guide

### AFL++ Fuzzing

```bash
# Install AFL++
sudo apt-get install afl++

# Build with AFL instrumentation
make clean
CC=afl-gcc make

# Run fuzzer on crypto_sign_open
afl-fuzz -i tests/vectors/fuzz_corpus -o fuzz_output \
    ./tests/fuzz/fuzz_sign_open
```

### libFuzzer Integration

```bash
# Build with libFuzzer support
clang -fsanitize=fuzzer -DFUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION \
    -Iincludes -c tests/fuzz/fuzz_target.c

# Run fuzzer
./fuzz_target -max_total_time=60
```

### OSS-Fuzz Integration

For continuous fuzzing via OSS-Fuzz, see `tests/fuzz/oss_fuzz_build.sh`.

---

## Memory Safety Tools

### Valgrind

```bash
# Full memory check
make test-valgrind

# Manual Valgrind run
valgrind --tool=memcheck --leak-check=full \
         --show-leak-kinds=all --track-origins=yes \
         ./tests/cunit/test_runner
```

### AddressSanitizer (ASan)

```bash
# Build with ASan
make clean
CFLAGS="-fsanitize=address -g -O1" make

# Run tests (ASan will report errors automatically)
make test
```

### MemorySanitizer (MSan)

```bash
# Build with MSan (requires clang)
make clean
CC=clang CFLAGS="-fsanitize=memory -g -O1" make

# Run tests
make test
```

### UndefinedBehaviorSanitizer (UBSan)

```bash
# Build with UBSan
make clean
CFLAGS="-fsanitize=undefined -g -O1" make

# Run tests
make test
```

---

## Performance Regression Testing

### Benchmark Suite

```bash
# Run benchmarks
cd tests/bench && make && ./bench

# Output format:
# Operation          Cycles      Time (μs)
# SHA-512 (1KB)      45000       15.0
# Ed25519 Sign       2500000     833.3
# Kyber768 Keygen    1500000     500.0
```

### Performance Baseline

Maintain performance baselines in `tests/bench/baseline.txt`. CI will flag regressions >5%.

---

## Test Vector Sources

### NIST/NaCl Vectors

Located in `tests/vectors/nacl_test_vectors.h`:
- Original TweetNaCl test vectors
- SUPERCOP reference vectors

### Wycheproof Vectors

Located in `tests/vectors/wycheproof/`:
- X25519 key exchange tests
- Ed25519 signature tests
- Converted from JSON to C arrays

### PQC Test Vectors

Located in `tests/vectors/wycheproof/kyber_test.json.h`:
- NIST PQC standardization vectors
- KAT (Known Answer Test) files

---

## Troubleshooting

### Common Issues

**Q: CUnit tests fail with "CU_initialize_registry failed"**
```bash
# Ensure CUnit is installed
sudo apt-get install libcunit1-dev

# Check library linking
ldd tests/cunit/test_runner | grep cunit
```

**Q: GTest compilation fails with "gtest/gtest.h not found"**
```bash
# Install Google Test
sudo apt-get install libgtest-dev cmake

# Or build from source
cd /usr/src/googletest
cmake .
make
sudo make install
```

**Q: Valgrind reports false positives in secure_zero**
```bash
# This is expected - secure_zero intentionally writes zeros
# Add suppression file if needed:
valgrind --suppressions=valgrind.supp ./tests/cunit/test_runner
```

**Q: Coverage shows <90% for critical functions**
```bash
# Identify uncovered lines
lcov --list coverage.info

# Add tests for missing branches
# Re-run coverage
make test-coverage
```

---

## Security Contact

Report test-related security issues to:
- **Email**: adbd04@gmail.com
- **Subject Prefix**: [SECURITY] [TESTING]

See [doc/SECURITY.md](SECURITY.md) for full disclosure policy.

---

## References

- [CUnit Documentation](http://cunit.sourceforge.net/)
- [Google Test Primer](https://google.github.io/googletest/primer.html)
- [Valgrind User Guide](https://valgrind.org/docs/manual/manual.html)
- [AFL++ Documentation](https://aflplus.plus/docs/)
- [NIST CVP](https://csrc.nist.gov/Projects/cryptographic-algorithm-validation-program)
