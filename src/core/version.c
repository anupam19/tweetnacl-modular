/*
 * TweetNaCl Version Information
 * Provides runtime version query functions.
 */

#include "tweetnacl/tweetnacl.h"

const char* tweetnacl_version(void) {
    return "2.0.0";
}

void tweetnacl_version_numbers(int *major, int *minor, int *patch) {
    if (major) *major = 2;
    if (minor) *minor = 0;
    if (patch) *patch = 0;
}
