/*
 * Poly1305 Driver - Operations Interface
 */

#include "core/types.h"
#include "drivers/crypto/poly1305.h"

struct poly1305_ops poly1305_default_ops = {
    .name = "software",
};
