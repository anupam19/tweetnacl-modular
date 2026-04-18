/*
 * Salsa20 Driver - Operations Interface
 *
 * This driver provides the struct salsa20_ops interface.
 * Actual implementations are in:
 *   - src/arch/x86_64/lib/salsa20_sse2.c (SSE2)
 *   - src/arch/x86_64/lib/salsa20_avx2.c (AVX2)
 *   - src/arch/arm64/lib/salsa20_neon.c (NEON)
 *   - src/arch/generic/lib/software_impl.c (Software fallback)
 */

#include "core/types.h"
#include "drivers/crypto/salsa20.h"

/* Default software implementation ops */
static int salsa20_soft_core(u8 *out, const u8 *in, const u8 *k, const u8 *c) {
    /* Implemented in software_impl.c */
    return -1; /* Stub - actual impl in software_impl.c */
}

static int hsalsa20_soft_core(u8 *out, const u8 *in, const u8 *k, const u8 *c) {
    return -1; /* Stub */
}

struct salsa20_ops salsa20_default_ops = {
    .name = "software",
    .core = salsa20_soft_core,
    .hsalsa20 = hsalsa20_soft_core,
};
