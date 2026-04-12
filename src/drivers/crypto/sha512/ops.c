/*
 * SHA-512 Driver - Operations Interface
 */

#include "core/types.h"
#include "drivers/crypto/sha512.h"

struct sha512_ops sha512_default_ops = {
    .name = "software",
};
