# TweetNaCl-Modular Testing Guide

## Testing Philosophy

TweetNaCl-Modular uses a **three-tier testing strategy**:

| Tier | Framework | Purpose |
|------|-----------|---------|
| **Original** | Custom C framework | Regression tests, end-to-end crypto verification |
| **CUnit** | CUnit 2.1+ | Structured unit tests, CI-friendly, known-answer tests |
| **GTest** | Google Test 1.10+ | C++ RAII wrappers, parameterized tests, death tests |

### Why Both CUnit and GTest?

- **CUnit** — Lightweight C framework, no C++ dependency, ideal for embedded CI environments
- **GTest** — Rich assertion macros, parameterized tests, fixtures, death tests for C++ consumers

## Running Tests

### Quick Start
```bash
# Build everything
cmake -B build -DENABLE_DRNG=ON -DENABLE_PQC=ON -DBUILD_TESTS=ON
cmake --build build --parallel

# Run original tests (56 tests)
ctest --output-on-failure

# Run full automated test suite with report
./run_full_test_suite.sh
```

### CUnit Tests
```bash
# Requires: libcunit1-dev (Debian/Ubuntu) or CUnit (macOS)
cmake -B build -DBUILD_CUNIT=ON
cmake --build build --parallel
./build/tests/cunit/test_cunit
```

### GTest Tests
```bash
# Requires: libgtest-dev or CMake FetchContent
cmake -B build -DBUILD_GTEST=ON
cmake --build build --parallel
./build/tests/gtest/test_gtest
```

## Test Coverage

| Suite | Tests | Coverage |
|-------|-------|----------|
| Original | 56 | Core crypto, PQC, secure memory |
| CUnit | ~30 | Known-answer vectors, edge cases |
| GTest | ~15 | Parameterized sizes, RAII, fixtures |
| Fuzzing | Continuous | OSS-Fuzz integration |

### Generate Coverage Report
```bash
cmake -B build -DENABLE_COVERAGE=ON
cmake --build build --parallel
./run_full_test_suite.sh  # or: make test
lcov --capture --directory build --output-file coverage.info
genhtml coverage.info --output-directory coverage_html
# Open coverage_html/index.html
```

## Adding New Tests

### CUnit
1. Create `tests/cunit/test_<module>.c`
2. Write test functions (no return value, use `CU_ASSERT_*`)
3. Create `int init_<module>_tests(void)` that adds tests to a suite
4. Declare `extern int init_<module>_tests(void);` in `test_runner.c`
5. Call it in `main()` before `CU_basic_run_tests()`

### GTest
1. Add `TEST()` or `TEST_F()` macros to `tests/gtest/test_tweetnacl.cpp`
2. Use `CU_ASSERT_*` → `EXPECT_*` / `ASSERT_*`
3. For parameterized tests, use `TEST_P()` + `INSTANTIATE_TEST_SUITE_P()`

## Continuous Integration

GitHub Actions runs on every push/PR:
- **build-native**: gcc + clang, CTest
- **build-native-valgrind**: Memory leak detection
- **build-cross-arm64**: ARM64 cross-compilation
- **build-cross-x86-32**: x86 32-bit cross-compilation
- **static-analysis**: cppcheck
- **full-test-suite**: Automated script with report

## Fuzzing

### OSS-Fuzz
```bash
# Build fuzz target
clang -fsanitize=fuzzer -Iincludes -Iarch -Ipqc \
    src/tweetnacl.c src/randombytes.c src/secure_mem.c \
    src/secure_utils.c src/randombytes_drng.c pqc/pqc.c \
    tests/fuzz/fuzz_target.c -o fuzz_tweetnacl

# Run
./fuzz_tweetnacl -max_total_time=3600
```

### AFL++
```bash
afl-gcc -O2 -Iincludes -Iarch -Ipqc \
    src/*.c pqc/pqc.c tests/fuzz/fuzz_target.c -o fuzz_afl
afl-fuzz -i tests/vectors/fuzz_corpus -o findings -- ./fuzz_afl
```

## Valgrind / ASan / MSan

```bash
# Valgrind (detects memory leaks)
valgrind --leak-check=full --error-exitcode=1 build/tests/test_original

# AddressSanitizer (detects buffer overflows, UAF)
cmake -B build -DENABLE_SANITIZERS=ON
cmake --build build
./build/tests/test_original

# MemorySanitizer (detects uninitialized reads, clang only)
CC=clang cmake -B build -DCMAKE_C_FLAGS="-fsanitize=memory -fno-omit-frame-pointer"
cmake --build build
./build/tests/test_original
```

## Performance Regression Testing

```bash
# Run benchmarks
cmake --build build --target benchmark
./build/tests/bench/bench

# Compare outputs across versions to detect regressions
```
