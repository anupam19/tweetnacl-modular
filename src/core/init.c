/*
 * TweetNaCl Library Initialization
 * Performs power-on self-tests and sets up dispatch layer
 */

#include "api/tweetnacl.h"
#include "core/dispatch.h"
#include "drivers/crypto/fips/fips_selftest.h"

/* Library initialization state */
static volatile int nacl_initialized = 0;

int nacl_init(void) {
    if (nacl_initialized) return 0;

    /* Run power-on self-tests */
    if (fips_power_on_selftest() != 0) {
        return -1;
    }

    /* Select best implementation */
    nacl_select_implementation();

    nacl_initialized = 1;
    return 0;
}

int nacl_is_initialized(void) {
    return nacl_initialized;
}
