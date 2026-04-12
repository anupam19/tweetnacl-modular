/*
 * Ed25519 Driver - Operations Interface
 */

#include "core/types.h"
#include "drivers/crypto/ed25519.h"

struct ed25519_ops ed25519_default_ops = {
    .name = "software",
};
