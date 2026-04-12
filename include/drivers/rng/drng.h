/*
 * RNG Driver Interface
 */
#ifndef DRNG_H
#define DRNG_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

int randombytes_drng_available(void);
const char *randombytes_implementation_name(void);

#ifdef __cplusplus
}
#endif

#endif /* DRNG_H */
