/*
 * ARM64 CPU Feature Detection
 * Reads ID registers to detect available extensions
 */

#include <stdint.h>

#ifdef __aarch64__

static uint64_t read_id_aa64isar0_el1(void) {
    uint64_t val;
    __asm__ __volatile__("mrs %0, id_aa64isar0_el1" : "=r"(val));
    return val;
}

static uint64_t read_id_aa64pfr0_el1(void) {
    uint64_t val;
    __asm__ __volatile__("mrs %0, id_aa64pfr0_el1" : "=r"(val));
    return val;
}

uint32_t detect_cpu_features(void) {
    uint32_t features = 0;

    /* NEON is always available on AArch64 */
    features |= (1 << 5);  /* CPU_FEATURE_NEON */

    uint64_t isar0 = read_id_aa64isar0_el1();

    /* SHA1/SHA256 extensions: bits [11:8] */
    if ((isar0 >> 8) & 0xf) features |= (1 << 6);  /* CPU_FEATURE_SHA2 */

    /* SHA512 extensions: bits [15:12] */
    if ((isar0 >> 12) & 0xf) features |= (1 << 7);  /* CPU_FEATURE_SHA512 */

    /* AES: bits [7:4] */
    if ((isar0 >> 4) & 0xf) features |= (1 << 8);  /* CPU_FEATURE_AES */

    uint64_t pfr0 = read_id_aa64pfr0_el1();
    /* SVE: bits [35:32] */
    if ((pfr0 >> 32) & 0xf) features |= (1 << 9);  /* CPU_FEATURE_SVE */

    return features;
}

#else

uint32_t detect_cpu_features(void) {
    return 0;
}

#endif
