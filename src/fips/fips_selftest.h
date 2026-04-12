/*
 * FIPS Self-Test Public Interface
 */

#ifndef FIPS_SELFTEST_H
#define FIPS_SELFTEST_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    FIPS_STATE_INITIAL = 0,
    FIPS_STATE_SELFTEST = 1,
    FIPS_STATE_ERROR = 2,
    FIPS_STATE_APPROVED = 3
} fips_state_t;

int fips_power_on_selftest(void);
int nacl_fips_mode_enabled(void);
const char *nacl_fips_version(void);
fips_state_t nacl_get_fips_state(void);

#ifdef __cplusplus
}
#endif

#endif /* FIPS_SELFTEST_H */
