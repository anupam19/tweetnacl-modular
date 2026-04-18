/*
 * x86_64 CPU Feature Detection
 * Uses CPUID instruction to detect available instruction set extensions
 */

#include <stdint.h>

/* Forward declaration */
uint32_t detect_cpu_features(void);

#ifdef __x86_64__

static void cpuid(uint32_t leaf, uint32_t subleaf, uint32_t *eax, uint32_t *ebx, uint32_t *ecx,
                  uint32_t *edx) {
    __asm__ __volatile__("cpuid"
                         : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
                         : "a"(leaf), "c"(subleaf));
}

uint32_t detect_cpu_features(void) {
    uint32_t eax, ebx, ecx, edx;
    uint32_t features = 0;

    /* Leaf 1: SSE2, SSE3, etc. */
    cpuid(1, 0, &eax, &ebx, &ecx, &edx);
    if (edx & (1 << 26))
        features |= (1 << 0); /* SSE2 */
    if (ecx & (1 << 0))
        features |= (1 << 4); /* SSE3 */

    /* Leaf 7: AVX2, SHA-NI, etc. */
    cpuid(7, 0, &eax, &ebx, &ecx, &edx);
    if (ebx & (1 << 5))
        features |= (1 << 1); /* AVX2 */
    if (ebx & (1 << 29))
        features |= (1 << 2); /* SHA-NI */
    if (ebx & (1 << 16))
        features |= (1 << 3); /* AVX-512F */

    return features;
}

#else

uint32_t detect_cpu_features(void) { return 0; }

#endif
