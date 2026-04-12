/*
 * Implementation Dispatch Interface
 */

#ifndef TWEETNACL_DISPATCH_H
#define TWEETNACL_DISPATCH_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* CPU feature flags */
#define CPU_FEATURE_SSE2   (1u << 0)
#define CPU_FEATURE_AVX2   (1u << 1)
#define CPU_FEATURE_SHANI  (1u << 2)
#define CPU_FEATURE_AVX512 (1u << 3)
#define CPU_FEATURE_SSSE3  (1u << 4)
#define CPU_FEATURE_NEON   (1u << 5)
#define CPU_FEATURE_SHA2   (1u << 6)
#define CPU_FEATURE_SHA512 (1u << 7)
#define CPU_FEATURE_AES    (1u << 8)
#define CPU_FEATURE_SVE    (1u << 9)

/* Query detected CPU features */
uint32_t detect_cpu_features(void);

/* Select best implementation at runtime */
int nacl_select_implementation(void);

/* Force a specific implementation (for testing) */
int nacl_set_implementation(const char *name);

/* Get current implementation name */
const char *nacl_get_implementation_name(void);

/* List all available implementations */
const char **nacl_list_implementations(int *count);

#ifdef __cplusplus
}
#endif

#endif /* TWEETNACL_DISPATCH_H */
