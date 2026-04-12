/*
 * Curve25519 Driver - Operations Interface
 */

#include "core/types.h"
#include "drivers/crypto/curve25519.h"

struct curve25519_ops curve25519_default_ops = {
    .name = "software",
};
