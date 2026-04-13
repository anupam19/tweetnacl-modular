# =============================================================================
# TweetNaCl-Modular — CMake Build System
# Maintainer: Anupam Datta <adbd04@gmail.com>
# =============================================================================
cmake_minimum_required(VERSION 3.16)

project(TweetNaClModular
    VERSION 2.0.0
    DESCRIPTION "Production-grade modular TweetNaCl with Post-Quantum Cryptography"
    LANGUAGES C
    HOMEPAGE_URL "https://github.com/anupam19/tweetnacl-modular"
)

# ─── Options ─────────────────────────────────────────────────────────────────
option(BUILD_SHARED_LIBS "Build shared library" ON)
option(BUILD_STATIC_LIBS "Build static library" ON)
option(BUILD_TESTS       "Build test suite" ON)
option(BUILD_APP         "Build example application" ON)
option(ENABLE_DRNG       "Enable hardware DRNG (RDRAND/RDSEED/ARM RNDR)" ON)
option(ENABLE_PQC        "Enable Post-Quantum Cryptography" ON)
option(ENABLE_COVERAGE   "Enable code coverage (gcov/lcov)" OFF)
option(ENABLE_SANITIZERS "Enable AddressSanitizer + UBSan" OFF)
option(ENABLE_LTO        "Enable Link Time Optimization" OFF)
option(BUILD_CUNIT       "Build CUnit test suite (requires libcunit1-dev)" OFF)
option(BUILD_GTEST       "Build GTest suite (requires libgtest-dev)" OFF)
option(BUILD_BENCHMARK   "Build benchmark suite" OFF)
option(BUILD_FUZZ        "Build fuzz target (requires clang + -fsanitize=fuzzer)" OFF)
option(ENABLE_FIPS_MODE "Enable FIPS 140-3 algorithm compliance" OFF)
option(NACL_RUNTIME_DISPATCH "Enable runtime dispatch (default)" OFF)
option(NACL_STATIC_IMPL  "Use compile-time selection only (no dispatch overhead)" OFF)

# ─── Compiler Requirements ──────────────────────────────────────────────────
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# ─── Hardware Extension Detection ──────────────────────────────────────────
include(CheckCSourceCompiles)

# Check x86 SSE2
if(ARCH_X86_64 OR ARCH_X86_32)
    check_c_source_compiles("
        #include <emmintrin.h>
        int main(void) { __m128i a = _mm_setzero_si128(); return 0; }
    " HAVE_SSE2)
    if(HAVE_SSE2)
        message(STATUS "SSE2: detected")
    endif()

    # Check x86 AVX2
    check_c_source_compiles("
        #include <immintrin.h>
        int main(void) { __m256i a = _mm256_setzero_si256(); return 0; }
    " HAVE_AVX2)
    if(HAVE_AVX2)
        message(STATUS "AVX2: detected")
    endif()

    # Check SHA-NI
    check_c_source_compiles("
        #include <immintrin.h>
        int main(void) {
            __m128i a = _mm_setzero_si128();
            (void)_mm_sha256rnds2_epu32(a, a, a);
            return 0;
        }
    " HAVE_SHANI)
    if(HAVE_SHANI)
        message(STATUS "SHA-NI: detected")
    endif()
endif()

# Check ARM NEON
if(ARCH_ARM64)
    check_c_source_compiles("
        #include <arm_neon.h>
        int main(void) { uint32x4_t a = vdupq_n_u32(0); return 0; }
    " HAVE_NEON)
    if(HAVE_NEON)
        message(STATUS "NEON: detected")
    endif()
endif()

if(NACL_STATIC_IMPL)
    add_compile_definitions(NACL_STATIC_IMPL)
    message(STATUS "Implementation selection: COMPILE-TIME (static)")
elseif(NACL_RUNTIME_DISPATCH)
    message(STATUS "Implementation selection: RUNTIME dispatch")
else()
    message(STATUS "Implementation selection: SOFTWARE ONLY")
endif()

# ─── Architecture Detection ─────────────────────────────────────────────────
if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64|amd64")
    set(ARCH_NAME "x86_64")
    set(ARCH_X86_64 ON)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "i[3-6]86|x86")
    set(ARCH_NAME "x86_32")
    set(ARCH_X86_32 ON)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64|ARM64")
    set(ARCH_NAME "ARM64")
    set(ARCH_ARM64 ON)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "arm|ARM")
    set(ARCH_NAME "ARM32")
    set(ARCH_ARM32 ON)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "riscv64|RISC-V.64")
    set(ARCH_NAME "RISC-V 64")
    set(ARCH_RISCV64 ON)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "riscv32|RISC-V.32")
    set(ARCH_NAME "RISC-V 32")
    set(ARCH_RISCV32 ON)
else()
    set(ARCH_NAME "Generic")
endif()

message(STATUS "Target architecture: ${ARCH_NAME}")

# ─── Security Compiler Flags ────────────────────────────────────────────────
add_compile_options(
    -Wextra
    -Wformat-security
    -Wmissing-declarations
    -Wmissing-prototypes
    -Wnested-externs
    -Wpointer-arith
    -Wshadow
    -Wstrict-prototypes
    -Wswitch-default
    -Wswitch-enum
    -Wunused
    -Wunused-result
    -fstack-protector-strong
    -fstack-clash-protection
    -D_FORTIFY_SOURCE=2
    -fPIC
)

# GCC-only warnings
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-Wlogical-op)
endif()

# Linker security flags
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    add_link_options(-Wl,-z,relro,-z,now)
endif()

# ─── Optional Features ──────────────────────────────────────────────────────
if(ENABLE_LTO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT lto_ok OUTPUT lto_error)
    if(lto_ok)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
        message(STATUS "LTO: ENABLED")
    else()
        message(WARNING "LTO not supported: ${lto_error}")
    endif()
endif()

if(ENABLE_SANITIZERS)
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
    message(STATUS "Sanitizers: ENABLED (ASan + UBSan)")
endif()

if(ENABLE_COVERAGE)
    add_compile_options(--coverage -fprofile-arcs -ftest-coverage)
    add_link_options(--coverage)
    message(STATUS "Coverage: ENABLED")
endif()

# ─── Include Directories ────────────────────────────────────────────────────
# Use include/ as the root for all header lookups

# ─── Source Files ───────────────────────────────────────────────────────────
# TweetNaCl philosophy: single compilation unit with all internal helpers.
# Module directories in src/ mirror include/ for organization.
set(NACL_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tweetnacl/tweetnacl.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/utils.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/secure_utils.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/integrity.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/keygen_validate.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/error.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/fips_selftest.c
    # Drivers
    ${CMAKE_CURRENT_SOURCE_DIR}/src/drivers/rng/randombytes.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/drivers/crypto/pqc.c
)

if(ENABLE_DRNG)
    # Check DRNG support
    list(APPEND NACL_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/drivers/rng/drng.c)
    add_compile_definitions(WITH_DRNG)

    # Check for x86 RDRAND/RDSEED
    if(ARCH_X86_64 OR ARCH_X86_32)
        include(CheckCSourceCompiles)
        check_c_source_compiles("
            #include <immintrin.h>
            int main(void) {
                unsigned long long r;
                return _rdrand64_step(&r) ? 0 : 1;
            }
        " HAVE_RDRAND_COMPILE)
        check_c_source_compiles("
            #include <immintrin.h>
            int main(void) {
                unsigned long long r;
                return _rdseed64_step(&r) ? 0 : 1;
            }
        " HAVE_RDSEED_COMPILE)

        if(HAVE_RDRAND_COMPILE)
            add_compile_definitions(HAS_RDRAND)
            message(STATUS "x86 RDRAND: compile support detected")
        endif()
        if(HAVE_RDSEED_COMPILE)
            add_compile_definitions(HAS_RDSEED)
            message(STATUS "x86 RDSEED: compile support detected")
        endif()
    endif()

    # Check ARM RNDR (ARMv8.5+)
    if(ARCH_ARM64)
        include(CheckCSourceCompiles)
        check_c_source_compiles("
            #include <stdint.h>
            int main(void) {
                #if defined(__ARM_FEATURE_RNG) && __ARM_FEATURE_RNG
                return 0;
                #else
                return 1;
                #endif
            }
        " HAVE_ARM_RNG_FEATURE)
        if(HAVE_ARM_RNG_FEATURE)
            add_compile_definitions(HAS_ARM_RNG)
            message(STATUS "ARM RNDR: compile support detected")
        endif()
    endif()

    message(STATUS "Hardware DRNG: ENABLED")
endif()

if(ENABLE_PQC)
    list(APPEND NACL_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/drivers/crypto/pqc.c)
    add_compile_definitions(WITH_PQC)
    message(STATUS "Post-Quantum Cryptography: ENABLED")
endif()

# ─── FIPS Compliance Sources ─────────────────────────────────────────
if(ENABLE_FIPS_MODE)
    add_compile_definitions(FIPS_MODE=1)
    list(APPEND NACL_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/drivers/crypto/fips/fips1804_sha512.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/drivers/crypto/fips/fips1865_ed25519.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/drivers/crypto/fips/fips_selftest.c
    )
    message(STATUS "FIPS mode: ENABLED")
endif()

# ─── Hardware Acceleration Sources ─────────────────────────────────────
if(NOT NACL_STATIC_IMPL)
    list(APPEND NACL_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/generic/lib/software_impl.c)
endif()

if(HAVE_SSE2 AND NOT NACL_STATIC_IMPL)
    list(APPEND NACL_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/x86_64/lib/salsa20_sse2.c)
    add_compile_definitions(HAVE_SSE2_IMPL)
    message(STATUS "SSE2 implementations: ENABLED")
endif()

if(HAVE_AVX2 AND NOT NACL_STATIC_IMPL)
    list(APPEND NACL_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/x86_64/lib/salsa20_avx2.c)
    add_compile_definitions(HAVE_AVX2_IMPL)
    message(STATUS "AVX2 implementations: ENABLED")
endif()

if(HAVE_NEON AND NOT NACL_STATIC_IMPL)
    list(APPEND NACL_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/arm64/lib/salsa20_neon.c)
    add_compile_definitions(HAVE_NEON_IMPL)
    message(STATUS "NEON implementations: ENABLED")
endif()

if(NACL_RUNTIME_DISPATCH AND NOT NACL_STATIC_IMPL)
    list(APPEND NACL_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/core/dispatch.c)
    if(ARCH_X86_64 OR ARCH_X86_32)
        list(APPEND NACL_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/x86_64/lib/cpu_detect.c)
    elseif(ARCH_ARM64)
        list(APPEND NACL_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/arm64/lib/cpu_detect.c)
    endif()
endif()

# ─── Library Targets ────────────────────────────────────────────────────────
# Shared library
if(BUILD_SHARED_LIBS)
    add_library(tweetnacl SHARED ${NACL_SOURCES})
    target_include_directories(tweetnacl PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include/tweetnacl>
    )
    target_compile_definitions(tweetnacl PRIVATE TWEETNACL_BUILDING)
    set_target_properties(tweetnacl PROPERTIES
        OUTPUT_NAME tweetnacl
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
        POSITION_INDEPENDENT_CODE ON
    )
endif()

# Static library
if(BUILD_STATIC_LIBS)
    add_library(tweetnacl_static STATIC ${NACL_SOURCES})
    target_include_directories(tweetnacl_static PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include/tweetnacl>
    )
    set_target_properties(tweetnacl_static PROPERTIES
        OUTPUT_NAME tweetnacl
        POSITION_INDEPENDENT_CODE ON
    )
endif()

# Alias for internal use
if(BUILD_SHARED_LIBS)
    add_library(TweetNaClModular::tweetnacl ALIAS tweetnacl)
elseif(BUILD_STATIC_LIBS)
    add_library(TweetNaClModular::tweetnacl ALIAS tweetnacl_static)
endif()

# ─── Installation ───────────────────────────────────────────────────────────
include(GNUInstallDirs)

if(BUILD_SHARED_LIBS)
    install(TARGETS tweetnacl
        EXPORT TweetNaClModularTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()

if(BUILD_STATIC_LIBS)
    install(TARGETS tweetnacl_static
        EXPORT TweetNaClModularTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
endif()

# Public headers
# Install headers from new structure
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/tweetnacl
    FILES_MATCHING PATTERN "*.h"
)
if(ENABLE_FIPS_MODE)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/drivers/crypto/fips/fips1865_ctx.h
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/tweetnacl
    )
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/drivers/crypto/fips/fips1804_sha512.h
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/tweetnacl
    )
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/drivers/crypto/fips/fips_selftest.h
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/tweetnacl
    )
endif()
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/arch/generic/fallback.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/tweetnacl
)
if(ENABLE_PQC)
    # pqc.h already installed via include/ directory install
endif()

# Export targets
install(EXPORT TweetNaClModularTargets
    FILE TweetNaClModularConfig.cmake
    NAMESPACE TweetNaClModular::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/tweetnacl
)

# pkg-config
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/tweetnacl.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/tweetnacl.pc
    @ONLY
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/tweetnacl.pc
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)

# Uninstall target
if(NOT TARGET uninstall)
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
        IMMEDIATE @ONLY
    )
    add_custom_target(uninstall
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
    )
endif()

# ─── CUnit Test Suite ─────────────────────────────────────────────────────
if(BUILD_CUNIT)
    find_package(PkgConfig QUIET)
    pkg_check_modules(CUNIT cunit)
    if(CUNIT_FOUND OR CUNIT_FOUND)
        enable_testing()
        add_executable(test_cunit
            tests/cunit/test_runner.c
            tests/cunit/test_tweetnacl_core.c
            tests/cunit/test_tweetnacl_box.c
            tests/cunit/test_tweetnacl_sign.c
            tests/cunit/test_tweetnacl_secretbox.c
            tests/cunit/test_tweetnacl_pqc.c
            tests/cunit/test_secure_mem.c
            tests/cunit/test_arch_detection.c
            tests/cunit/test_constant_time.c
        )
        target_include_directories(test_cunit PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/includes
            ${CMAKE_CURRENT_SOURCE_DIR}/arch
            ${CMAKE_CURRENT_SOURCE_DIR}/pqc
            ${CMAKE_CURRENT_SOURCE_DIR}
        )
        target_link_libraries(test_cunit PRIVATE tweetnacl_static ${CUNIT_LIBRARIES} cunit)
        add_test(NAME CUnitTests COMMAND test_cunit)
        message(STATUS "CUnit tests: ENABLED")
    else()
        message(STATUS "CUnit tests: DISABLED (libcunit1-dev not found)")
    endif()
endif()

# ─── GTest Suite ──────────────────────────────────────────────────────────
if(BUILD_GTEST)
    find_package(GTest QUIET)
    if(GTEST_FOUND)
        enable_language(CXX)
        add_executable(test_gtest tests/gtest/test_tweetnacl.cpp)
        target_include_directories(test_gtest PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/includes
            ${CMAKE_CURRENT_SOURCE_DIR}/arch
            ${CMAKE_CURRENT_SOURCE_DIR}/pqc
            ${CMAKE_CURRENT_SOURCE_DIR}
        )
        target_link_libraries(test_gtest PRIVATE tweetnacl_static GTest::gtest GTest::gtest_main)
        add_test(NAME GTestTests COMMAND test_gtest)
        message(STATUS "GTest tests: ENABLED")
    else()
        message(STATUS "GTest tests: DISABLED (libgtest-dev not found)")
    endif()
endif()

# ─── Benchmark Suite ──────────────────────────────────────────────────────
if(BUILD_BENCHMARK)
    add_executable(bench tests/bench/bench.c)
    target_include_directories(bench PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/includes
        ${CMAKE_CURRENT_SOURCE_DIR}/arch
        ${CMAKE_CURRENT_SOURCE_DIR}/pqc
    )
    target_link_libraries(bench PRIVATE tweetnacl_static)
    add_custom_target(run_bench COMMAND bench DEPENDS bench
        COMMENT "Running benchmarks..."
    )
    message(STATUS "Benchmark suite: ENABLED")
endif()

# ─── Fuzz Target ──────────────────────────────────────────────────────────
if(BUILD_FUZZ)
    if(CMAKE_C_COMPILER_ID MATCHES "Clang")
        add_executable(fuzz_tweetnacl tests/fuzz/fuzz_target.c)
        target_include_directories(fuzz_tweetnacl PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/includes
            ${CMAKE_CURRENT_SOURCE_DIR}/arch
            ${CMAKE_CURRENT_SOURCE_DIR}/pqc
        )
        target_link_libraries(fuzz_tweetnacl PRIVATE tweetnacl_static)
        target_compile_options(fuzz_tweetnacl PRIVATE -fsanitize=fuzzer)
        target_link_options(fuzz_tweetnacl PRIVATE -fsanitize=fuzzer)
        message(STATUS "Fuzz target: ENABLED")
    else()
        message(STATUS "Fuzz target: DISABLED (requires Clang)")
    endif()
endif()

# ─── Known Answer Tests (KATs) ───────────────────────────────────────────
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)

    # KAT self-test executable
    add_executable(test_kat tests/kat/kat_runner.c)
    target_include_directories(test_kat PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/kat
    )
    target_link_libraries(test_kat PRIVATE tweetnacl_static)
    target_compile_definitions(test_kat PRIVATE KAT_STANDALONE)
    add_test(NAME kat_selftest COMMAND test_kat)
    message(STATUS "KAT self-tests: ENABLED")

    # FIPS CAVP test executable
    if(ENABLE_FIPS_MODE)
        add_executable(test_cavp tests/fips/test_cavp.c)
        target_include_directories(test_cavp PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/include
            ${CMAKE_CURRENT_SOURCE_DIR}/src
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/fips
        )
        target_link_libraries(test_cavp PRIVATE tweetnacl_static)
        add_test(NAME FipsCavp COMMAND test_cavp)
        message(STATUS "FIPS CAVP tests: ENABLED")
    endif()
endif()

# ─── Example Application ──────────────────────────────────────────────────
if(BUILD_APP)
    add_executable(tweetnacl_app samples/main.c)
    target_include_directories(tweetnacl_app PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/samples
    )
    target_link_libraries(tweetnacl_app PRIVATE tweetnacl_static)
    target_compile_options(tweetnacl_app PRIVATE
        -Wextra -Wformat-security -Wmissing-declarations -Wmissing-prototypes
        -Wnested-externs -Wpointer-arith -Wshadow -Wstrict-prototypes
        -g -O0
    )
endif()

# ─── Summary ────────────────────────────────────────────────────────────────
message(STATUS "")
message(STATUS "TweetNaCl-Modular ${PROJECT_VERSION} Configuration:")
message(STATUS "  Architecture:      ${ARCH_NAME}")
message(STATUS "  Shared library:    ${BUILD_SHARED_LIBS}")
message(STATUS "  Static library:    ${BUILD_STATIC_LIBS}")
message(STATUS "  Tests:             ${BUILD_TESTS}")
message(STATUS "  Example app:       ${BUILD_APP}")
message(STATUS "  DRNG:              ${ENABLE_DRNG}")
message(STATUS "  PQC:               ${ENABLE_PQC}")
message(STATUS "  Coverage:          ${ENABLE_COVERAGE}")
message(STATUS "  Sanitizers:        ${ENABLE_SANITIZERS}")
message(STATUS "  LTO:               ${ENABLE_LTO}")
message(STATUS "  CUnit:             ${BUILD_CUNIT}")
message(STATUS "  GTest:             ${BUILD_GTEST}")
message(STATUS "  Benchmark:         ${BUILD_BENCHMARK}")
message(STATUS "  Fuzz:              ${BUILD_FUZZ}")
message(STATUS "")
