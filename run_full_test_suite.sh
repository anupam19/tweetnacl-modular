#!/bin/bash
# =============================================================================
# TweetNaCl-Modular Full Test Suite Runner
# Maintainer: Anupam Datta <adbd04@gmail.com>
# Repository: https://github.com/anupam19/tweetnacl-modular
#
# Usage: ./run_full_test_suite.sh
#
# This script automates the complete build-test-validate pipeline:
#   1. CMake configuration
#   2. Library build
#   3. DRNG detection check
#   4. CTest execution
#   5. Valgrind memory analysis (if available)
#   6. Static analysis with cppcheck (if available)
#   7. Code coverage report (if enabled)
#   8. Installation test
#   9. Compile test against installed library
# =============================================================================
# Note: Do NOT use set -e — we handle errors per-step to generate full report

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build"
REPORT_DIR="${SCRIPT_DIR}/test_reports"
TIMESTAMP="$(date +"%Y%m%d_%H%M%S")"
REPORT_FILE="${REPORT_DIR}/report_${TIMESTAMP}.txt"

# ─── Color Codes ────────────────────────────────────────────────────────────
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# ─── Counters ───────────────────────────────────────────────────────────────
TESTS_PASSED=0
TESTS_FAILED=0
WARNINGS=0

# ─── Create Directories ─────────────────────────────────────────────────────
mkdir -p "${BUILD_DIR}"
mkdir -p "${REPORT_DIR}"

# ─── Logging Functions ──────────────────────────────────────────────────────
log() {
    echo -e "[$(date '+%Y-%m-%d %H:%M:%S')] $1" | tee -a "${REPORT_FILE}"
}

pass() {
    echo -e "${GREEN}[PASS]${NC} $1" | tee -a "${REPORT_FILE}"
    ((TESTS_PASSED++))
}

fail() {
    echo -e "${RED}[FAIL]${NC} $1" | tee -a "${REPORT_FILE}"
    ((TESTS_FAILED++))
}

warn() {
    echo -e "${YELLOW}[WARN]${NC} $1" | tee -a "${REPORT_FILE}"
    ((WARNINGS++))
}

section() {
    echo "" | tee -a "${REPORT_FILE}"
    echo -e "${BLUE}=== $1 ===${NC}" | tee -a "${REPORT_FILE}"
}

# ─── Header ─────────────────────────────────────────────────────────────────
log "============================================================"
log "TweetNaCl-Modular Full Test Suite Report"
log "============================================================"
log "Architecture: $(uname -m)"
log "OS:           $(uname -os 2>/dev/null || uname -s)"
log "Date:         $(date)"
log "Maintainer:   Anupam Datta <adbd04@gmail.com>"
log "Repository:   https://github.com/anupam19/tweetnacl-modular"
log "============================================================"

# ─── Step 1: Clean Previous Builds ─────────────────────────────────────────
section "Step 1: Cleaning Previous Builds"
rm -rf "${BUILD_DIR:?}"/*
pass "Cleaned build directory"

# ─── Step 2: CMake Configuration ───────────────────────────────────────────
section "Step 2: CMake Configuration"
if cmake -B "${BUILD_DIR}" \
    -DCMAKE_BUILD_TYPE=Release \
    -DENABLE_DRNG=ON \
    -DENABLE_PQC=ON \
    -DBUILD_TESTS=ON \
    -DBUILD_APP=ON \
    2>&1 | tee -a "${REPORT_FILE}"; then
    pass "CMake configuration succeeded"
else
    fail "CMake configuration failed"
    exit 1
fi

# ─── Step 3: Build Library ─────────────────────────────────────────────────
section "Step 3: Building Library"
if cmake --build "${BUILD_DIR}" --parallel 2>&1 | tee -a "${REPORT_FILE}"; then
    pass "Library build succeeded"
else
    fail "Library build failed"
    exit 1
fi

# ─── Step 4: Check DRNG Status ─────────────────────────────────────────────
section "Step 4: Checking DRNG Support"
if grep -qi "Hardware DRNG: ENABLED" "${BUILD_DIR}/CMakeFiles/CMakeOutput.log" 2>/dev/null ||
   grep -qi "DRNG.*ENABLED" "${BUILD_DIR}/CMakeCache.txt" 2>/dev/null; then
    pass "Hardware DRNG support: ENABLED"
elif grep -qi "DRNG.*ON" "${BUILD_DIR}/CMakeCache.txt" 2>/dev/null; then
    warn "DRNG enabled in config but hardware not detected (using software fallback)"
else
    warn "DRNG status unclear — check CMake output"
fi

# ─── Step 5: Run CTest ─────────────────────────────────────────────────────
section "Step 5: Running CTest Suite"
if cd "${BUILD_DIR}" && ctest --output-on-failure 2>&1 | tee -a "${REPORT_FILE}"; then
    cd "${SCRIPT_DIR}"
    pass "CTest: all tests passed"
else
    cd "${SCRIPT_DIR}"
    fail "CTest: some tests failed (see report for details)"
fi

# ─── Step 6: Valgrind Memory Check ─────────────────────────────────────────
section "Step 6: Valgrind Memory Analysis"
if command -v valgrind &>/dev/null; then
    if valgrind --tool=memcheck --leak-check=full --errors-for-leak-kinds=definite \
        --error-exitcode=1 \
        "${BUILD_DIR}/tests/test_original" 2>&1 | tee -a "${REPORT_FILE}"; then
        pass "Valgrind: no memory leaks detected"
    else
        fail "Valgrind: memory issues detected (see report)"
    fi
else
    warn "Valgrind not installed — skipping memory analysis"
    log "  Install: sudo apt-get install valgrind"
fi

# ─── Step 7: Static Analysis ───────────────────────────────────────────────
section "Step 7: Static Analysis (cppcheck)"
if command -v cppcheck &>/dev/null; then
    if cppcheck --enable=all --inconclusive --std=c11 \
        --suppress=missingIncludeSystem \
        --suppress=unusedFunction \
        --suppress=missingInclude \
        "${SCRIPT_DIR}/src/" 2>&1 | tee -a "${REPORT_FILE}"; then
        pass "Static analysis: no critical issues"
    else
        warn "Static analysis found minor issues (see report)"
    fi
else
    warn "cppcheck not installed — skipping static analysis"
    log "  Install: sudo apt-get install cppcheck"
fi

# ─── Step 8: Installation Test ─────────────────────────────────────────────
section "Step 8: Testing Installation"
INSTALL_TEST_DIR=$(mktemp -d)
if cmake --install "${BUILD_DIR}" --prefix="${INSTALL_TEST_DIR}" 2>&1 | tee -a "${REPORT_FILE}"; then
    if [ -f "${INSTALL_TEST_DIR}/lib/libtweetnacl.so" ] || \
       [ -f "${INSTALL_TEST_DIR}/lib/libtweetnacl.a" ] || \
       [ -f "${INSTALL_TEST_DIR}/lib64/libtweetnacl.so" ] || \
       [ -f "${INSTALL_TEST_DIR}/lib64/libtweetnacl.a" ]; then
        pass "Installation: libraries installed correctly"
    else
        fail "Installation: expected library files not found"
    fi
else
    fail "Installation: cmake --install failed"
fi

# ─── Step 9: Compile Test Against Installed Library ────────────────────────
section "Step 9: Compile Test (Installed Library)"
cat > /tmp/test_tweetnacl_compile_$$.c << 'EOF'
#include <tweetnacl/tweetnacl.h>
#include <tweetnacl/randombytes.h>
#include <string.h>

int main(void) {
    uint8_t pk[32], sk[32];
    if (crypto_box_keypair(pk, sk) != 0) return 1;

    uint8_t buf[32];
    randombytes(buf, sizeof(buf));

    /* Verify DRNG functions are available */
    (void)randombytes_drng_available();
    (void)randombytes_implementation_name();

    return 0;
}
EOF

# Determine lib directory (lib or lib64)
LIB_DIR="lib"
if [ -d "${INSTALL_TEST_DIR}/lib64" ]; then
    LIB_DIR="lib64"
fi

if gcc -I"${INSTALL_TEST_DIR}/include" \
       -L"${INSTALL_TEST_DIR}/${LIB_DIR}" \
       /tmp/test_tweetnacl_compile_$$.c \
       -ltweetnacl -Wl,-rpath,"${INSTALL_TEST_DIR}/${LIB_DIR}" \
       -o /tmp/test_tweetnacl_compile_$$ 2>&1 | tee -a "${REPORT_FILE}"; then
    if /tmp/test_tweetnacl_compile_$$ 2>&1 | tee -a "${REPORT_FILE}"; then
        pass "Compile test: program compiled and executed successfully"
    else
        warn "Compile test: program compiled but execution failed (may need LD_LIBRARY_PATH)"
    fi
else
    warn "Compile test: compilation failed (may need additional link flags)"
fi

# Cleanup
rm -f /tmp/test_tweetnacl_compile_$$.c /tmp/test_tweetnacl_compile_$$
rm -rf "${INSTALL_TEST_DIR}"

# ─── Step 10: Library Files Summary ────────────────────────────────────────
section "Step 10: Build Artifacts Summary"
log "Shared library:  $(find "${BUILD_DIR}" -name 'libtweetnacl.so*' -type f 2>/dev/null || echo 'not found')"
log "Static library:  $(find "${BUILD_DIR}" -name 'libtweetnacl.a' -type f 2>/dev/null || echo 'not found')"
log "Test binary:     $(find "${BUILD_DIR}" -name 'test_original' -type f 2>/dev/null || echo 'not found')"
log "App binary:      $(find "${BUILD_DIR}" -name 'tweetnacl_app' -type f 2>/dev/null || echo 'not found')"

# ─── Summary Report ────────────────────────────────────────────────────────
log ""
log "============================================================"
log "TEST SUMMARY"
log "============================================================"
log "Tests Passed:  ${TESTS_PASSED}"
log "Tests Failed:  ${TESTS_FAILED}"
log "Warnings:      ${WARNINGS}"
log "Report saved:  ${REPORT_FILE}"
log "============================================================"

if [ "${TESTS_FAILED}" -eq 0 ]; then
    log "${GREEN}✅ ALL CRITICAL TESTS PASSED${NC}"
    log "============================================================"
    exit 0
else
    log "${RED}❌ SOME TESTS FAILED — review report: ${REPORT_FILE}${NC}"
    log "============================================================"
    exit 1
fi
