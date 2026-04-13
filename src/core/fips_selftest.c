/**
 * @file fips_selftest.c
 * @brief FIPS 140-3 Self-Test Implementation
 * @details Implements power-up self-tests and conditional self-tests
 *          as required by FIPS 140-3 Section 9.3 and NIST SP 800-193
 * 
 * @copyright MIT License
 * @author Anupam Datta
 * @version 1.0.0
 * @date 2024
 */

#include "core/fips_selftest.h"
#include "core/error.h"
#include "drivers/rng/randombytes.h"
#include "tweetnacl/tweetnacl.h"
#include <string.h>
#include <stdint.h>

/* FIPS 140-3 State Management */
typedef struct {
    int power_up_tests_passed;
    int conditional_tests_enabled;
    uint64_t keygen_counter;
    int last_test_result;
} fips_state_t;

static volatile fips_state_t fips_state = {
    .power_up_tests_passed = 0,
    .conditional_tests_enabled = 1,
    .keygen_counter = 0,
    .last_test_result = 0
};

/* Helper: Zeroize sensitive data (NIST SP 800-88) */
static void secure_zeroize(void *buf, size_t len) {
    volatile uint8_t *p = (volatile uint8_t *)buf;
    while (len--) {
        *p++ = 0;
    }
}

/* Helper: Constant-time comparison */
static int constant_time_compare(const void *a, const void *b, size_t len) {
    const volatile uint8_t *va = (const volatile uint8_t *)a;
    const volatile uint8_t *vb = (const volatile uint8_t *)b;
    uint8_t result = 0;
    
    for (size_t i = 0; i < len; i++) {
        result |= va[i] ^ vb[i];
    }
    
    return result;
}

/* RNG Self-Test */
static int fips_selftest_rng(void) {
    uint8_t test_buf[64];
    uint8_t test_buf2[64];
    
    memset(test_buf, 0, sizeof(test_buf));
    memset(test_buf2, 0, sizeof(test_buf2));
    
    if (randombytes_safe(test_buf, sizeof(test_buf)) != NACL_SUCCESS) {
        return NACL_ERROR_SELF_TEST_FAILED;
    }
    
    if (randombytes_safe(test_buf2, sizeof(test_buf2)) != NACL_SUCCESS) {
        return NACL_ERROR_SELF_TEST_FAILED;
    }
    
    if (constant_time_compare(test_buf, test_buf2, sizeof(test_buf)) == 0) {
        secure_zeroize(test_buf, sizeof(test_buf));
        secure_zeroize(test_buf2, sizeof(test_buf2));
        return NACL_ERROR_SELF_TEST_FAILED;
    }
    
    int all_zeros = 1;
    for (size_t i = 0; i < sizeof(test_buf); i++) {
        if (test_buf[i] != 0) {
            all_zeros = 0;
            break;
        }
    }
    
    secure_zeroize(test_buf, sizeof(test_buf));
    secure_zeroize(test_buf2, sizeof(test_buf2));
    
    if (all_zeros) {
        return NACL_ERROR_SELF_TEST_FAILED;
    }
    
    return NACL_SUCCESS;
}

/* SHA-512 KAT */
static int fips_selftest_sha512(void) {
    static const uint8_t empty_msg[1] = {0};
    static const uint8_t expected_hash[64] = {
        0xcf, 0x83, 0xe1, 0x35, 0x7e, 0xef, 0xb8, 0xbd,
        0xf1, 0x54, 0x28, 0x50, 0xd6, 0x6d, 0x80, 0x07,
        0xd6, 0x20, 0xe4, 0x05, 0x0b, 0x57, 0x15, 0xdc,
        0x83, 0xf4, 0xa9, 0x21, 0xd3, 0x6c, 0xe9, 0xce,
        0x47, 0xd0, 0xd1, 0x3c, 0x5d, 0x85, 0xf2, 0xb0,
        0xff, 0x83, 0x18, 0xd2, 0x87, 0x7e, 0xec, 0x2f,
        0x63, 0xb9, 0x31, 0xbd, 0x47, 0x41, 0x7a, 0x81,
        0xa5, 0x38, 0x32, 0x7a, 0xf9, 0x27, 0xda, 0x3e
    };
    
    uint8_t hash[64];
    int ret = crypto_hash(hash, empty_msg, 0);
    
    if (ret != 0) {
        return NACL_ERROR_SELF_TEST_FAILED;
    }
    
    if (constant_time_compare(hash, expected_hash, 64) != 0) {
        secure_zeroize(hash, sizeof(hash));
        return NACL_ERROR_SELF_TEST_FAILED;
    }
    
    secure_zeroize(hash, sizeof(hash));
    return NACL_SUCCESS;
}

/* Ed25519 Self-Test */
static int fips_selftest_ed25519(void) {
    uint8_t pk[32], sk[64];
    uint8_t msg[16] = "Test message";
    uint8_t sig[64];
    
    if (crypto_sign_keypair(pk, sk) != 0) {
        return NACL_ERROR_SELF_TEST_FAILED;
    }
    
    unsigned long long sig_len;
    if (crypto_sign_signature(sig, &sig_len, msg, sizeof(msg), sk) != 0) {
        secure_zeroize(pk, sizeof(pk));
        secure_zeroize(sk, sizeof(sk));
        return NACL_ERROR_SELF_TEST_FAILED;
    }
    
    if (crypto_sign_verify(sig, sig_len, msg, sizeof(msg), pk) != 0) {
        secure_zeroize(pk, sizeof(pk));
        secure_zeroize(sk, sizeof(sk));
        return NACL_ERROR_SELF_TEST_FAILED;
    }
    
    secure_zeroize(pk, sizeof(pk));
    secure_zeroize(sk, sizeof(sk));
    return NACL_SUCCESS;
}

/* X25519 Self-Test */
static int fips_selftest_x25519(void) {
    uint8_t pk1[32], sk1[32];
    uint8_t pk2[32], sk2[32];
    uint8_t ss1[32], ss2[32];
    
    if (crypto_box_keypair(pk1, sk1) != 0) {
        return NACL_ERROR_SELF_TEST_FAILED;
    }
    
    if (crypto_box_keypair(pk2, sk2) != 0) {
        secure_zeroize(pk1, sizeof(pk1));
        secure_zeroize(sk1, sizeof(sk1));
        return NACL_ERROR_SELF_TEST_FAILED;
    }
    
    if (crypto_scalarmult(ss1, sk1, pk2) != 0) {
        secure_zeroize(pk1, sizeof(pk1));
        secure_zeroize(sk1, sizeof(sk1));
        secure_zeroize(pk2, sizeof(pk2));
        secure_zeroize(sk2, sizeof(sk2));
        return NACL_ERROR_SELF_TEST_FAILED;
    }
    
    if (crypto_scalarmult(ss2, sk2, pk1) != 0) {
        secure_zeroize(pk1, sizeof(pk1));
        secure_zeroize(sk1, sizeof(sk1));
        secure_zeroize(pk2, sizeof(pk2));
        secure_zeroize(sk2, sizeof(sk2));
        secure_zeroize(ss1, sizeof(ss1));
        return NACL_ERROR_SELF_TEST_FAILED;
    }
    
    if (constant_time_compare(ss1, ss2, 32) != 0) {
        secure_zeroize(pk1, sizeof(pk1));
        secure_zeroize(sk1, sizeof(sk1));
        secure_zeroize(pk2, sizeof(pk2));
        secure_zeroize(sk2, sizeof(sk2));
        secure_zeroize(ss1, sizeof(ss1));
        secure_zeroize(ss2, sizeof(ss2));
        return NACL_ERROR_SELF_TEST_FAILED;
    }
    
    secure_zeroize(pk1, sizeof(pk1));
    secure_zeroize(sk1, sizeof(sk1));
    secure_zeroize(pk2, sizeof(pk2));
    secure_zeroize(sk2, sizeof(sk2));
    secure_zeroize(ss1, sizeof(ss1));
    secure_zeroize(ss2, sizeof(ss2));
    
    return NACL_SUCCESS;
}

/* Power-Up Self-Tests */
int fips_power_up_self_tests(void) {
    int ret;
    
    ret = fips_selftest_rng();
    if (ret != NACL_SUCCESS) {
        fips_state.power_up_tests_passed = 0;
        fips_state.last_test_result = ret;
        return ret;
    }
    
    ret = fips_selftest_sha512();
    if (ret != NACL_SUCCESS) {
        fips_state.power_up_tests_passed = 0;
        fips_state.last_test_result = ret;
        return ret;
    }
    
    ret = fips_selftest_ed25519();
    if (ret != NACL_SUCCESS) {
        fips_state.power_up_tests_passed = 0;
        fips_state.last_test_result = ret;
        return ret;
    }
    
    ret = fips_selftest_x25519();
    if (ret != NACL_SUCCESS) {
        fips_state.power_up_tests_passed = 0;
        fips_state.last_test_result = ret;
        return ret;
    }
    
    fips_state.power_up_tests_passed = 1;
    fips_state.last_test_result = NACL_SUCCESS;
    
    return NACL_SUCCESS;
}

/* Conditional Self-Test */
int fips_conditional_selftest_keygen(void) {
    if (!fips_state.conditional_tests_enabled) {
        return NACL_SUCCESS;
    }
    
    fips_state.keygen_counter++;
    
    if ((fips_state.keygen_counter % 100) != 0) {
        return NACL_SUCCESS;
    }
    
    uint8_t pk[32], sk[64];
    uint8_t msg[16] = "Consistency test";
    uint8_t sig[64];
    
    if (crypto_sign_keypair(pk, sk) != 0) {
        fips_state.last_test_result = NACL_ERROR_SELF_TEST_FAILED;
        return NACL_ERROR_SELF_TEST_FAILED;
    }
    
    unsigned long long sig_len;
    if (crypto_sign_signature(sig, &sig_len, msg, sizeof(msg), sk) != 0) {
        secure_zeroize(pk, sizeof(pk));
        secure_zeroize(sk, sizeof(sk));
        fips_state.last_test_result = NACL_ERROR_SELF_TEST_FAILED;
        return NACL_ERROR_SELF_TEST_FAILED;
    }
    
    if (crypto_sign_verify(sig, sig_len, msg, sizeof(msg), pk) != 0) {
        secure_zeroize(pk, sizeof(pk));
        secure_zeroize(sk, sizeof(sk));
        fips_state.last_test_result = NACL_ERROR_SELF_TEST_FAILED;
        return NACL_ERROR_SELF_TEST_FAILED;
    }
    
    secure_zeroize(pk, sizeof(pk));
    secure_zeroize(sk, sizeof(sk));
    
    return NACL_SUCCESS;
}

/* Public API */
int fips_get_power_up_status(void) {
    return fips_state.power_up_tests_passed;
}

int fips_get_last_test_result(void) {
    return fips_state.last_test_result;
}

uint64_t fips_get_keygen_counter(void) {
    return fips_state.keygen_counter;
}

void fips_enable_conditional_tests(int enable) {
    fips_state.conditional_tests_enabled = enable;
}

int fips_are_self_tests_passed(void) {
    return fips_state.power_up_tests_passed;
}
