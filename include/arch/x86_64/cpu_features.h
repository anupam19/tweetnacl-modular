/*
 * x86_64 CPU Feature Detection
 * Detects: SSE2, AVX2, AVX-512, SHA-NI
 */

#ifndef ARCH_X86_64_CPU_FEATURES_H
#define ARCH_X86_64_CPU_FEATURES_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* CPU feature flags */
#define CPU_X86_SSE2 (1u << 0)
#define CPU_X86_AVX (1u << 1)
#define CPU_X86_AVX2 (1u << 2)
#define CPU_X86_AVX512F (1u << 3)
#define CPU_X86_SHANI (1u << 4)

/* Detect available CPU features via CPUID */
uint32_t x86_64_detect_features(void);

/* Check if a specific feature is available */
static inline int x86_64_has_feature(uint32_t feature) {
    static uint32_t cached = 0;
    if (!cached)
        cached = x86_64_detect_features();
    return (cached & feature) != 0;
}

#ifdef __cplusplus
}
#endif

#endif /* ARCH_X86_64_CPU_FEATURES_H */
