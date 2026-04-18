/*
 * TweetNaCl Library Initialization
 * Performs power-on self-tests and sets up dispatch layer
 */

#include "core/dispatch.h"
#include "core/fips_selftest.h"
#include "tweetnacl/tweetnacl.h"

/* Library initialization state */
static volatile int nacl_initialized = 0;

int nacl_init(void) {
    if (nacl_initialized)
        return 0;

    /* Run power-on self-tests */
    if (fips_power_up_self_tests() != 0) {
        return -1;
    }

    /* Select best implementation */
    nacl_select_implementation();

    nacl_initialized = 1;
    return 0;
}

int nacl_is_initialized(void) { return nacl_initialized; }
