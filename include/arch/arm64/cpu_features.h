/*
 * ARM64 CPU Feature Detection
 * Detects: NEON, SVE, SVE2, AES, SHA1, SHA256, SHA512
 */

#ifndef ARCH_ARM64_CPU_FEATURES_H
#define ARCH_ARM64_CPU_FEATURES_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* CPU feature flags */
#define CPU_ARM_NEON (1u << 0)
#define CPU_ARM_SVE (1u << 1)
#define CPU_ARM_SVE2 (1u << 2)
#define CPU_ARM_AES (1u << 3)
#define CPU_ARM_SHA1 (1u << 4)
#define CPU_ARM_SHA256 (1u << 5)
#define CPU_ARM_SHA512 (1u << 6)

/* Detect available CPU features via ID registers */
uint32_t arm64_detect_features(void);

/* Check if a specific feature is available */
static inline int arm64_has_feature(uint32_t feature) {
    static uint32_t cached = 0;
    if (!cached)
        cached = arm64_detect_features();
    return (cached & feature) != 0;
}

#ifdef __cplusplus
}
#endif

#endif /* ARCH_ARM64_CPU_FEATURES_H */
