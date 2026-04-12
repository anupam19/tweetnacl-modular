/*
 * Implementation Dispatch Layer
 * Runtime selection of best available implementation
 *
 * Falls back to software if hardware extensions unavailable.
 * Selection happens once at first call (lazy initialization).
 */

#include "tweetnacl.h"
#include "tweetnacl_dispatch.h"
#include "impl/impl_salsa20.h"
#include <string.h>

/* Implementation function pointers */
typedef struct {
    const char *name;
    int (*salsa20)(u8 *out, const u8 *in, const u8 *k, const u8 *c);
    int (*hsalsa20)(u8 *out, const u8 *in, const u8 *k, const u8 *c);
    int priority;
    uint32_t cpu_features_required;
} nacl_impl_t;

/* Forward declarations for software impl */
extern int salsa20_soft(u8 *out, const u8 *in, const u8 *k, const u8 *c);
extern int hsalsa20_soft(u8 *out, const u8 *in, const u8 *k, const u8 *c);

/* Implementation table */
static const nacl_impl_t implementations[] = {
    /* Software fallback — always first, always available */
    {
        .name = "software",
        .salsa20 = salsa20_soft,
        .hsalsa20 = hsalsa20_soft,
        .priority = 0,
        .cpu_features_required = 0
    },

#ifdef HAVE_SSE2_IMPL
    {
        .name = "sse2",
        .salsa20 = salsa20_sse2,
        .hsalsa20 = hsalsa20_sse2,
        .priority = 10,
        .cpu_features_required = CPU_FEATURE_SSE2
    },
#endif

#ifdef HAVE_AVX2_IMPL
    {
        .name = "avx2",
        .salsa20 = salsa20_avx2,
        .hsalsa20 = hsalsa20_avx2,
        .priority = 20,
        .cpu_features_required = CPU_FEATURE_AVX2
    },
#endif

#ifdef HAVE_NEON_IMPL
    {
        .name = "neon",
        .salsa20 = salsa20_neon,
        .hsalsa20 = hsalsa20_neon,
        .priority = 15,
        .cpu_features_required = CPU_FEATURE_NEON
    },
#endif

    { NULL, NULL, NULL, 0, 0 }  /* Sentinel */
};

static const nacl_impl_t *current_impl = NULL;

#ifdef NACL_STATIC_IMPL
/* Compile-time selection — no dispatch overhead */
int nacl_select_implementation(void) { return 0; }
const char *nacl_get_implementation_name(void) { return "static"; }
int nacl_set_implementation(const char *name) { (void)name; return 0; }
const char **nacl_list_implementations(int *count) { *count = 0; return NULL; }
#else
/* Runtime selection */
int nacl_select_implementation(void) {
    if (current_impl != NULL) return 0;  /* Already selected */

    uint32_t features = detect_cpu_features();
    const nacl_impl_t *best = &implementations[0];

    for (const nacl_impl_t *impl = implementations; impl->name != NULL; impl++) {
        if ((features & impl->cpu_features_required) == impl->cpu_features_required) {
            if (impl->priority > best->priority) {
                best = impl;
            }
        }
    }

    current_impl = best;
    return 0;
}

const char *nacl_get_implementation_name(void) {
    nacl_select_implementation();
    return current_impl ? current_impl->name : "software";
}

int nacl_set_implementation(const char *name) {
    for (const nacl_impl_t *impl = implementations; impl->name != NULL; impl++) {
        if (strcmp(impl->name, name) == 0) {
            current_impl = impl;
            return 0;
        }
    }
    return -1;
}

const char **nacl_list_implementations(int *count) {
    static const char *names[8];
    int n = 0;
    for (const nacl_impl_t *impl = implementations; impl->name != NULL; impl++) {
        names[n++] = impl->name;
    }
    *count = n;
    return names;
}
#endif /* NACL_STATIC_IMPL */

/* ─── Dispatch wrappers ─────────────────────────────────────────────────── */

/* Only define these when runtime dispatch is enabled and we have alternatives */
#if defined(NACL_RUNTIME_DISPATCH) && !defined(NACL_STATIC_IMPL)

int crypto_core_salsa20(u8 *out, const u8 *in, const u8 *k, const u8 *c) {
    nacl_select_implementation();
    return current_impl->salsa20(out, in, k, c);
}

int crypto_core_hsalsa20(u8 *out, const u8 *in, const u8 *k, const u8 *c) {
    nacl_select_implementation();
    return current_impl->hsalsa20(out, in, k, c);
}

#endif /* NACL_RUNTIME_DISPATCH && !NACL_STATIC_IMPL */
