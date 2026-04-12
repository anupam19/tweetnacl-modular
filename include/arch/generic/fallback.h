/*
 * Architecture-specific configuration header
 * Supports: x86, ARM, RISC-V
 */

#ifndef ARCH_CONFIG_H
#define ARCH_CONFIG_H

/* Detect architecture */
#if defined(__x86_64__) || defined(_M_X64)
    #define ARCH_X86_64 1
    #define ARCH_NAME "x86_64"
#elif defined(__i386__) || defined(_M_IX86)
    #define ARCH_X86_32 1
    #define ARCH_NAME "x86_32"
#elif defined(__aarch64__) || defined(_M_ARM64)
    #define ARCH_ARM64 1
    #define ARCH_NAME "ARM64"
#elif defined(__arm__) || defined(_M_ARM)
    #define ARCH_ARM32 1
    #define ARCH_NAME "ARM32"
#elif defined(__riscv)
    #if __riscv_xlen == 64
        #define ARCH_RISCV64 1
        #define ARCH_NAME "RISC-V 64"
    #elif __riscv_xlen == 32
        #define ARCH_RISCV32 1
        #define ARCH_NAME "RISC-V 32"
    #endif
#else
    #define ARCH_GENERIC 1
    #define ARCH_NAME "Generic"
#endif

/* Endianness detection */
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    #define ARCH_BIG_ENDIAN 1
#else
    #define ARCH_LITTLE_ENDIAN 1
#endif

/* Architecture-specific optimizations */
#ifdef ARCH_X86_64
    /* x86-64 specific optimizations can be added here */
    #define HAVE_NATIVE_UINT64 1
#elif defined(ARCH_ARM64)
    /* ARM64 specific optimizations */
    #define HAVE_NATIVE_UINT64 1
    #define HAVE_ARM_CRYPTO 1
#elif defined(ARCH_RISCV64)
    /* RISC-V 64 specific optimizations */
    #define HAVE_NATIVE_UINT64 1
    #ifdef __riscv_vector
        #define HAVE_RISCV_VECTOR 1
    #endif
#else
    /* Generic fallback */
    #define HAVE_NATIVE_UINT64 1
#endif

/* Inline optimization hints */
#if defined(__GNUC__) || defined(__clang__)
    #define FORCE_INLINE inline __attribute__((always_inline))
    #define NO_INLINE __attribute__((noinline))
#elif defined(_MSC_VER)
    #define FORCE_INLINE __forceinline
    #define NO_INLINE __declspec(noinline)
#else
    #define FORCE_INLINE inline
    #define NO_INLINE
#endif

/* Unreachable hint */
#if defined(__GNUC__) || defined(__clang__)
    #define UNREACHABLE() __builtin_unreachable()
#else
    #define UNREACHABLE()
#endif

/* Likely/unlikely hints */
#if defined(__GNUC__) || defined(__clang__)
    #define LIKELY(x) __builtin_expect(!!(x), 1)
    #define UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
    #define LIKELY(x) (x)
    #define UNLIKELY(x) (x)
#endif

#endif /* ARCH_CONFIG_H */
