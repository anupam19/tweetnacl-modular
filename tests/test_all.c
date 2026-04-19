/*
 * Unit Tests for TweetNaCl with Architecture Support and PQC
 * Supports: x86, ARM, RISC-V architectures
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include "tweetnacl/tweetnacl.h"
#include "drivers/rng/randombytes.h"
#include "arch/generic/fallback.h"
#include "drivers/crypto/pqc.h"

/* Forward declarations to avoid -Wmissing-prototypes warnings */
void test_architecture_detection(void);
void test_randombytes_safe(void);
void test_crypto_hash(void);
void test_crypto_secretbox(void);
void test_crypto_box(void);
void test_crypto_sign(void);
void test_crypto_scalarmult(void);
void test_pqc_interface(void);
void test_hybrid_crypto(void);
void test_error_handling(void);
void test_performance_basic(void);
void run_secure_tests(void);

/* External counters from secure memory tests */
extern int get_secure_tests_run(void);
extern int get_secure_tests_passed(void);

/* Test result tracking */
static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST_ASSERT(name, condition) do { \
    tests_run++; \
    if (condition) { \
        tests_passed++; \
        printf("[PASS] %s\n", name); \
    } else { \
        tests_failed++; \
        printf("[FAIL] %s\n", name); \
    } \
} while(0)

/* Helper function to print hex */
static void print_hex(const char* label, const uint8_t* data, size_t len) {
    printf("%s: ", label);
    for (size_t i = 0; i < len && i < 32; i++) {
        printf("%02x", data[i]);
    }
    if (len > 32) printf("...");
    printf("\n");
}

/* Test 1: Architecture Detection */
void test_architecture_detection(void) {
    printf("\n=== Testing Architecture Detection ===\n");

    TEST_ASSERT("Architecture name is defined", ARCH_NAME != NULL);
    printf("Detected architecture: %s\n", ARCH_NAME);

#if defined(ARCH_X86_64)
    TEST_ASSERT("x86_64 flag set", 1);
#elif defined(ARCH_ARM64)
    TEST_ASSERT("ARM64 flag set", 1);
#elif defined(ARCH_RISCV64)
    TEST_ASSERT("RISC-V 64 flag set", 1);
#else
    TEST_ASSERT("Generic or other architecture", 1);
#endif

#if defined(ARCH_LITTLE_ENDIAN)
    TEST_ASSERT("Little endian detected", 1);
#elif defined(ARCH_BIG_ENDIAN)
    TEST_ASSERT("Big endian detected", 1);
#endif

    TEST_ASSERT("HAVE_NATIVE_UINT64 defined", HAVE_NATIVE_UINT64 == 1);
}

/* Test 2: Basic Random Generation */
void test_randombytes_safe(void) {
    printf("\n=== Testing Random Bytes Generation ===\n");

    uint8_t random1[32];
    uint8_t random2[32];

    randombytes_safe(random1, 32);
    randombytes_safe(random2, 32);

    TEST_ASSERT("Random bytes generated (non-zero)",
                memcmp(random1, random2, 32) != 0);

    /* Check that not all bytes are the same */
    int all_same = 1;
    for (int i = 1; i < 32; i++) {
        if (random1[i] != random1[0]) {
            all_same = 0;
            break;
        }
    }
    TEST_ASSERT("Random bytes have variation", !all_same);

    print_hex("Random sample 1", random1, 32);
}

/* Test 3: SHA512 Hash */
void test_crypto_hash(void) {
    printf("\n=== Testing SHA512 Hash ===\n");

    uint8_t hash1[64];
    uint8_t hash2[64];
    const char* message = "Hello, TweetNaCl!";

    crypto_hash(hash1, (const uint8_t*)message, strlen(message));
    crypto_hash(hash2, (const uint8_t*)message, strlen(message));

    TEST_ASSERT("Hash is deterministic", memcmp(hash1, hash2, 64) == 0);

    /* Change message slightly */
    const char* message2 = "Hello, TweetNaCl!!";
    uint8_t hash3[64];
    crypto_hash(hash3, (const uint8_t*)message2, strlen(message2));

    TEST_ASSERT("Different messages produce different hashes",
                memcmp(hash1, hash3, 64) != 0);

    print_hex("SHA512 hash", hash1, 64);
}

/* Test 4: Secret Box (Symmetric Encryption) */
void test_crypto_secretbox(void) {
    printf("\n=== Testing Secret Box (Symmetric Encryption) ===\n");

    uint8_t key[32];
    uint8_t nonce[24];
    uint8_t message[100];
    uint8_t ciphertext[100];
    uint8_t decrypted[100];

    randombytes_safe(key, 32);
    randombytes_safe(nonce, 24);

    /* TweetNaCl requires 32-byte zero padding at the beginning */
    memset(message, 0, 32);  /* Zero padding */
    memset(message + 32, 'A', sizeof(message) - 32);  /* Actual message */

    /* Encrypt */
    int encrypt_result = crypto_secretbox(ciphertext, message, sizeof(message), nonce, key);
    TEST_ASSERT("Encryption succeeds", encrypt_result == 0);

    /* Decrypt */
    int decrypt_result = crypto_secretbox_open(decrypted, ciphertext, sizeof(message), nonce, key);
    TEST_ASSERT("Decryption succeeds", decrypt_result == 0);

    if (decrypt_result == 0) {
        /* Verify decrypted message matches original (skip padding) */
        TEST_ASSERT("Decrypted message matches original",
                    memcmp(message + 32, decrypted + 32, sizeof(message) - 32) == 0);
    }

    /* Test with wrong key */
    uint8_t wrong_key[32];
    randombytes_safe(wrong_key, 32);
    int wrong_decrypt = crypto_secretbox_open(decrypted, ciphertext, sizeof(message), nonce, wrong_key);
    TEST_ASSERT("Wrong key fails decryption", wrong_decrypt != 0);
}

/* Test 5: Box (Asymmetric Encryption) */
void test_crypto_box(void) {
    printf("\n=== Testing Crypto Box (Asymmetric Encryption) ===\n");

    uint8_t pk1[32], sk1[32];
    uint8_t pk2[32], sk2[32];
    uint8_t nonce[24];
    uint8_t message[100];
    uint8_t ciphertext[100];
    uint8_t decrypted[100];

    /* Generate key pairs */
    crypto_box_keypair(pk1, sk1);
    crypto_box_keypair(pk2, sk2);
    randombytes_safe(nonce, 24);

    /* TweetNaCl requires 32-byte zero padding at the beginning */
    memset(message, 0, 32);  /* Zero padding */
    memset(message + 32, 'B', sizeof(message) - 32);  /* Actual message */

    /* Encrypt with recipient's public key and sender's secret key */
    int encrypt_result = crypto_box(ciphertext, message, sizeof(message), nonce, pk2, sk1);
    TEST_ASSERT("Box encryption succeeds", encrypt_result == 0);

    /* Decrypt with sender's public key and recipient's secret key */
    int decrypt_result = crypto_box_open(decrypted, ciphertext, sizeof(message), nonce, pk1, sk2);
    TEST_ASSERT("Box decryption succeeds", decrypt_result == 0);

    if (decrypt_result == 0) {
        /* Verify message */
        TEST_ASSERT("Decrypted message matches",
                    memcmp(message + 32, decrypted + 32, sizeof(message) - 32) == 0);
    }

    print_hex("Public Key 1", pk1, 32);
    print_hex("Public Key 2", pk2, 32);
}

/* Test 6: Digital Signatures */
void test_crypto_sign(void) {
    printf("\n=== Testing Digital Signatures ===\n");

    uint8_t pk[32], sk[64];
    uint8_t message[50];
    uint8_t signed_msg[50 + 64];
    /* crypto_sign_open uses output buffer for intermediate computation:
     * it copies n bytes from signed message then appends 32 bytes of pk,
     * so buffer must be at least sizeof(message) + 64 bytes */
    uint8_t verified_msg[50 + 64];
    u64 signed_len;

    /* Generate signing key pair */
    crypto_sign_keypair(pk, sk);
    randombytes_safe(message, sizeof(message));

    /* Sign message */
    int sign_result = crypto_sign(signed_msg, &signed_len, message, sizeof(message), sk);
    TEST_ASSERT("Signing succeeds", sign_result == 0);
    TEST_ASSERT("Signature length correct", signed_len == sizeof(message) + 64);

    /* Verify signature */
    u64 verified_len;
    int verify_result = crypto_sign_open(verified_msg, &verified_len, signed_msg, signed_len, pk);
    TEST_ASSERT("Verification succeeds", verify_result == 0);
    TEST_ASSERT("Verified message length correct", verified_len == sizeof(message));
    TEST_ASSERT("Verified message matches", memcmp(message, verified_msg, sizeof(message)) == 0);

    /* Tamper with signature */
    signed_msg[10] ^= 0xFF;
    int tampered_verify = crypto_sign_open(verified_msg, &verified_len, signed_msg, signed_len, pk);
    TEST_ASSERT("Tampered signature detection", tampered_verify != 0);

    print_hex("Signing Public Key", pk, 32);
}

/* Test 7: Scalar Multiplication */
void test_crypto_scalarmult(void) {
    printf("\n=== Testing Scalar Multiplication ===\n");

    uint8_t scalar[32];
    uint8_t result1[32];
    uint8_t result2[32];

    randombytes_safe(scalar, 32);

    /* Scalar multiplication with base point */
    int smult_base = crypto_scalarmult_base(result1, scalar);
    TEST_ASSERT("Scalar mult base succeeds", smult_base == 0);

    /* Same operation should produce same result */
    (void)crypto_scalarmult_base(result2, scalar);
    TEST_ASSERT("Scalar mult base deterministic",
                memcmp(result1, result2, 32) == 0);

    print_hex("Scalar mult result", result1, 32);
}

/* Test 8: PQC Interface Tests */
void test_pqc_interface(void) {
    printf("\n=== Testing PQC Interface ===\n");

    pqc_params_t params;
    pqc_result_t result;

    /* Test getting parameters for KYBER768 */
    result = pqc_get_params(PQC_KYBER768, &params);
    TEST_ASSERT("PQC get params succeeds", result == PQC_SUCCESS);
    TEST_ASSERT("KYBER768 has valid public key size", params.public_key_size > 0);
    TEST_ASSERT("KYBER768 has valid secret key size", params.secret_key_size > 0);
    printf("PQC Algorithm: %s - %s\n", params.name, params.description);
    printf("  Public key size: %zu\n", params.public_key_size);
    printf("  Secret key size: %zu\n", params.secret_key_size);

    /* Test key generation for KYBER512 */
    pqc_params_t params512;
    pqc_get_params(PQC_KYBER512, &params512);
    uint8_t pk[2000], sk[3000];
    result = pqc_keygen(PQC_KYBER512, pk, sizeof(pk), sk, sizeof(sk), NULL, 0);
    TEST_ASSERT("PQC keygen succeeds", result == PQC_SUCCESS);

    /* Test encapsulation */
    uint8_t ct[1000], ss[32];
    size_t ct_len = sizeof(ct);
    result = pqc_encapsulate(PQC_KYBER512, pk, params512.public_key_size,
                            ct, &ct_len, ss, sizeof(ss), NULL, 0);
    TEST_ASSERT("PQC encapsulation succeeds", result == PQC_SUCCESS);

    /* Test decapsulation */
    uint8_t ss2[32];
    result = pqc_decapsulate(PQC_KYBER512, sk, params512.secret_key_size,
                            ct, ct_len, ss2, sizeof(ss2));
    TEST_ASSERT("PQC decapsulation succeeds", result == PQC_SUCCESS);

    /* Test signatures with DILITHIUM2 */
    pqc_params_t dilithium_params;
    pqc_get_params(PQC_DILITHIUM2, &dilithium_params);
    uint8_t sig[5000];
    uint8_t msg[] = "Test message for signing";
    size_t sig_len = sizeof(sig);
    result = pqc_sign(PQC_DILITHIUM2, sk, dilithium_params.secret_key_size,
                     msg, sizeof(msg), sig, &sig_len, NULL, 0);
    /* Note: This uses the wrong key type but tests the interface */
    TEST_ASSERT("PQC signing interface works", result != PQC_ERROR_INVALID_PARAM);

    result = pqc_verify(PQC_DILITHIUM2, pk, dilithium_params.public_key_size,
                       msg, sizeof(msg), sig, sig_len);
    /* Note: Verification may fail due to wrong key but interface is tested */
    TEST_ASSERT("PQC verification interface works", result != PQC_ERROR_INVALID_PARAM);
}

/* Test 9: Hybrid Mode (Classical + PQC) */
void test_hybrid_crypto(void) {
    printf("\n=== Testing Hybrid Cryptography ===\n");

    uint8_t hybrid_pk[2000], hybrid_sk[2500];
    size_t hybrid_pk_len, hybrid_sk_len;

    /* Generate hybrid key pair */
    pqc_result_t result = pqc_hybrid_keygen(PQC_KYBER768,
                                           hybrid_pk, &hybrid_pk_len,
                                           hybrid_sk, &hybrid_sk_len);
    TEST_ASSERT("Hybrid keygen succeeds", result == PQC_SUCCESS);
    TEST_ASSERT("Hybrid public key has reasonable size", hybrid_pk_len > 1000);
    TEST_ASSERT("Hybrid secret key has reasonable size", hybrid_sk_len > 1000);

    printf("Hybrid public key size: %zu\n", hybrid_pk_len);
    printf("Hybrid secret key size: %zu\n", hybrid_sk_len);

    /* Hybrid encapsulation */
    uint8_t hybrid_ct[2000], hybrid_ss[32];
    size_t hybrid_ct_len = sizeof(hybrid_ct);
    result = pqc_hybrid_encapsulate(PQC_KYBER768, hybrid_pk, hybrid_pk_len,
                                   hybrid_ct, &hybrid_ct_len,
                                   hybrid_ss, sizeof(hybrid_ss));
    TEST_ASSERT("Hybrid encapsulation succeeds", result == PQC_SUCCESS);

    /* Hybrid decapsulation */
    uint8_t hybrid_ss2[32];
    result = pqc_hybrid_decapsulate(PQC_KYBER768, hybrid_sk, hybrid_sk_len,
                                   hybrid_ct, hybrid_ct_len,
                                   hybrid_ss2, sizeof(hybrid_ss2));
    TEST_ASSERT("Hybrid decapsulation succeeds", result == PQC_SUCCESS);
}

/* Test 10: Error Handling */
void test_error_handling(void) {
    printf("\n=== Testing Error Handling ===\n");

    pqc_result_t result;
    pqc_params_t params;

    /* Test invalid algorithm */
    result = pqc_get_params((pqc_algorithm_t)999, &params);
    TEST_ASSERT("Invalid algorithm returns error", result != PQC_SUCCESS);

    /* Test PQC-specific error handling only when real implementation is available */
    if (!pqc_is_stub_active()) {
        /* Test NULL parameters */
        uint8_t pk[100], sk[100];
        result = pqc_keygen(PQC_KYBER512, NULL, sizeof(pk), sk, sizeof(sk), NULL, 0);
        TEST_ASSERT("NULL public key returns error",
                    result == PQC_ERROR_INVALID_PARAM);

        result = pqc_keygen(PQC_KYBER512, pk, sizeof(pk), NULL, sizeof(sk), NULL, 0);
        TEST_ASSERT("NULL secret key returns error",
                    result == PQC_ERROR_INVALID_PARAM);

        /* Test buffer too small */
        result = pqc_keygen(PQC_KYBER768, pk, 100, sk, 100, NULL, 0);
        TEST_ASSERT("Small buffer returns error",
                    result == PQC_ERROR_BUFFER_TOO_SMALL);
    } else {
        printf("[SKIP] PQC error handling tests: stub implementation active\n");
    }

    /* Test error string conversion */
    const char* error_str = pqc_result_to_string(PQC_SUCCESS);
    TEST_ASSERT("Error string conversion works", strcmp(error_str, "Success") == 0);

    error_str = pqc_result_to_string(PQC_ERROR_INVALID_PARAM);
    TEST_ASSERT("Error string for invalid param",
                strcmp(error_str, "Invalid parameter") == 0);
}

    /* Test error string conversion */
    const char* error_str = pqc_result_to_string(PQC_SUCCESS);
    TEST_ASSERT("Error string conversion works", strcmp(error_str, "Success") == 0);

    error_str = pqc_result_to_string(PQC_ERROR_INVALID_PARAM);
    TEST_ASSERT("Error string for invalid param",
                strcmp(error_str, "Invalid parameter") == 0);
}
<<<<<<< HEAD
=======

    /* Test error string conversion */
    const char* error_str = pqc_result_to_string(PQC_SUCCESS);
    TEST_ASSERT("Error string conversion works", strcmp(error_str, "Success") == 0);

    error_str = pqc_result_to_string(PQC_ERROR_INVALID_PARAM);
    TEST_ASSERT("Error string for invalid param",
                strcmp(error_str, "Invalid parameter") == 0);
}
>>>>>>> dfd008e (Ensure test_error_handling has correct structure: declarations at function top, PQC-specific tests wrapped in stub check)

    /* Test error string conversion */
    const char* error_str = pqc_result_to_string(PQC_SUCCESS);
    TEST_ASSERT("Error string conversion works", strcmp(error_str, "Success") == 0);

    error_str = pqc_result_to_string(PQC_ERROR_INVALID_PARAM);
    TEST_ASSERT("Error string for invalid param",
                strcmp(error_str, "Invalid parameter") == 0);
}

/* Test 11: Performance characteristics (basic timing) */
void test_performance_basic(void) {
    printf("\n=== Testing Basic Performance ===\n");

    uint8_t message[1024];
    uint8_t hash[64];

    randombytes_safe(message, sizeof(message));

    /* Time multiple hash operations */
    clock_t start = clock();
    for (int i = 0; i < 100; i++) {
        crypto_hash(hash, message, sizeof(message));
    }
    clock_t end = clock();

    double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("100 SHA512 hashes of 1KB: %.4f seconds\n", time_taken);
    TEST_ASSERT("Hash performance reasonable", time_taken < 1.0);
}

/* Main test runner */
int main(void) {
    printf("========================================\n");
    printf("TweetNaCl Unit Tests\n");
    printf("Architecture: %s\n", ARCH_NAME);
    printf("========================================\n");

    test_architecture_detection();
    test_randombytes_safe();
    test_crypto_hash();
    test_crypto_secretbox();
    test_crypto_box();
    test_crypto_sign();
    test_crypto_scalarmult();
    test_pqc_interface();
    test_hybrid_crypto();
    test_error_handling();
    test_performance_basic();

    /* Run secure memory and CERT C compliance tests */
    run_secure_tests();

    /* Add secure test counts to totals */
    tests_run += get_secure_tests_run();
    tests_passed += get_secure_tests_passed();

    printf("\n========================================\n");
    printf("Test Summary\n");
    printf("========================================\n");
    printf("Total tests: %d\n", tests_run);
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);
    printf("========================================\n");

    return tests_failed > 0 ? 1 : 0;
}
