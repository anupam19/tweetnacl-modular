/*
 * TweetNaCl-Modular GTest Main Test File
 * Core cryptographic tests using Google Test framework
 * 
 * Author: Anupam Datta <adbd04@gmail.com>
 */

#include "test_fixture.h"
#include <gtest/gtest.h>

/**
 * @test Test SHA-512 hash determinism
 */
TEST_F(TweetNaClTest, HashDeterministic) {
    const char* message = "Hello, TweetNaCl!";
    uint8_t hash1[64], hash2[64];
    
    ASSERT_EQ(0, crypto_hash(hash1, (const uint8_t*)message, strlen(message)));
    ASSERT_EQ(0, crypto_hash(hash2, (const uint8_t*)message, strlen(message)));
    ASSERT_EQ(0, memcmp(hash1, hash2, 64));
}

/**
 * @test Test SHA-512 with different messages
 */
TEST_F(TweetNaClTest, HashDifferentMessages) {
    const char* msg1 = "Message 1";
    const char* msg2 = "Message 2";
    uint8_t hash1[64], hash2[64];
    
    ASSERT_EQ(0, crypto_hash(hash1, (const uint8_t*)msg1, strlen(msg1)));
    ASSERT_EQ(0, crypto_hash(hash2, (const uint8_t*)msg2, strlen(msg2)));
    EXPECT_NE(0, memcmp(hash1, hash2, 64));
}

/**
 * @test Test empty message hash
 */
TEST_F(TweetNaClTest, HashEmptyMessage) {
    uint8_t hash[64];
    ASSERT_EQ(0, crypto_hash(hash, nullptr, 0));
}

/**
 * @test Test scalar multiplication base point
 */
TEST_F(TweetNaClTest, ScalarMultBase) {
    uint8_t scalar[32];
    uint8_t result1[32], result2[32];
    
    randombytes(scalar, 32);
    
    ASSERT_EQ(0, crypto_scalarmult_base(result1, scalar));
    ASSERT_EQ(0, crypto_scalarmult_base(result2, scalar));
    ASSERT_EQ(0, memcmp(result1, result2, 32));
}

/**
 * @test Test verify function with equal buffers
 */
TEST_F(TweetNaClTest, VerifyEqual) {
    uint8_t a[32], b[32];
    memset(a, 0x42, sizeof(a));
    memcpy(b, a, sizeof(b));
    
    ASSERT_EQ(0, crypto_verify_32(a, b));
}

/**
 * @test Test verify function with different buffers
 */
TEST_F(TweetNaClTest, VerifyDifferent) {
    uint8_t a[32], b[32];
    memset(a, 0x42, sizeof(a));
    memset(b, 0x43, sizeof(b));
    
    EXPECT_EQ(-1, crypto_verify_32(a, b));
}

/**
 * @test Test crypto_box encryption/decryption
 */
TEST_F(TweetNaClTest, BoxBasic) {
    uint8_t pk1[32], sk1[32];
    uint8_t pk2[32], sk2[32];
    uint8_t nonce[24];
    uint8_t message[100];
    uint8_t ciphertext[100];
    uint8_t decrypted[100];
    
    ASSERT_EQ(0, crypto_box_keypair(pk1, sk1));
    ASSERT_EQ(0, crypto_box_keypair(pk2, sk2));
    randombytes(nonce, 24);
    
    memset(message, 0, 32);
    memset(message + 32, 'B', sizeof(message) - 32);
    
    ASSERT_EQ(0, crypto_box(ciphertext, message, sizeof(message), nonce, pk2, sk1));
    ASSERT_EQ(0, crypto_box_open(decrypted, ciphertext, sizeof(message), nonce, pk1, sk2));
    ASSERT_EQ(0, memcmp(message + 32, decrypted + 32, sizeof(message) - 32));
}

/**
 * @test Test crypto_box with wrong key fails
 */
TEST_F(TweetNaClTest, BoxWrongKey) {
    uint8_t pk1[32], sk1[32];
    uint8_t pk2[32], sk2[32];
    uint8_t pk3[32], sk3[32];
    uint8_t nonce[24];
    uint8_t message[100];
    uint8_t ciphertext[100];
    uint8_t decrypted[100];
    
    crypto_box_keypair(pk1, sk1);
    crypto_box_keypair(pk2, sk2);
    crypto_box_keypair(pk3, sk3);
    randombytes(nonce, 24);
    
    memset(message, 0, 32);
    memcpy(message + 32, "Test", 5);
    
    ASSERT_EQ(0, crypto_box(ciphertext, message, sizeof(message), nonce, pk2, sk1));
    EXPECT_EQ(-1, crypto_box_open(decrypted, ciphertext, sizeof(message), nonce, pk3, sk3));
}

/**
 * @test Test crypto_sign basic signing and verification
 */
TEST_F(TweetNaClTest, SignBasic) {
    uint8_t pk[32], sk[64];
    uint8_t message[50];
    uint8_t signed_msg[50 + 64];
    uint8_t verified_msg[50];
    unsigned long long signed_len, verified_len;
    
    ASSERT_EQ(0, crypto_sign_keypair(pk, sk));
    randombytes(message, sizeof(message));
    
    ASSERT_EQ(0, crypto_sign(signed_msg, &signed_len, message, sizeof(message), sk));
    EXPECT_EQ(sizeof(message) + 64, signed_len);
    
    ASSERT_EQ(0, crypto_sign_open(verified_msg, &verified_len, signed_msg, signed_len, pk));
    EXPECT_EQ(sizeof(message), verified_len);
    EXPECT_EQ(0, memcmp(message, verified_msg, sizeof(message)));
}

/**
 * @test Test crypto_sign tampered signature detection
 */
TEST_F(TweetNaClTest, SignTampered) {
    uint8_t pk[32], sk[64];
    uint8_t message[50];
    uint8_t signed_msg[50 + 64];
    uint8_t verified_msg[50];
    unsigned long long signed_len, verified_len;
    
    crypto_sign_keypair(pk, sk);
    randombytes(message, sizeof(message));
    crypto_sign(signed_msg, &signed_len, message, sizeof(message), sk);
    
    /* Tamper with signature */
    signed_msg[10] ^= 0xFF;
    
    EXPECT_EQ(-1, crypto_sign_open(verified_msg, &verified_len, signed_msg, signed_len, pk));
}

/**
 * @test Test crypto_secretbox encryption/decryption
 */
TEST_F(TweetNaClTest, SecretBoxBasic) {
    uint8_t key[32];
    uint8_t nonce[24];
    uint8_t message[100];
    uint8_t ciphertext[100];
    uint8_t decrypted[100];
    
    randombytes(key, 32);
    randombytes(nonce, 24);
    
    memset(message, 0, 32);
    memset(message + 32, 'A', sizeof(message) - 32);
    
    ASSERT_EQ(0, crypto_secretbox(ciphertext, message, sizeof(message), nonce, key));
    ASSERT_EQ(0, crypto_secretbox_open(decrypted, ciphertext, sizeof(message), nonce, key));
    ASSERT_EQ(0, memcmp(message + 32, decrypted + 32, sizeof(message) - 32));
}

/**
 * @test Test crypto_secretbox with wrong key fails
 */
TEST_F(TweetNaClTest, SecretBoxWrongKey) {
    uint8_t key1[32], key2[32];
    uint8_t nonce[24];
    uint8_t message[100];
    uint8_t ciphertext[100];
    uint8_t decrypted[100];
    
    randombytes(key1, 32);
    randombytes(key2, 32);
    randombytes(nonce, 24);
    
    memset(message, 0, 32);
    memcpy(message + 32, "Test", 5);
    
    ASSERT_EQ(0, crypto_secretbox(ciphertext, message, sizeof(message), nonce, key1));
    EXPECT_EQ(-1, crypto_secretbox_open(decrypted, ciphertext, sizeof(message), nonce, key2));
}

/**
 * @test Parameterized test for different message sizes
 */
TEST_P(MessageSizeTest, HashVariousSizes) {
    size_t msg_size = GetParam();
    std::vector<uint8_t> message = randomBytes(msg_size);
    uint8_t hash1[64], hash2[64];
    
    ASSERT_EQ(0, crypto_hash(hash1, message.data(), msg_size));
    ASSERT_EQ(0, crypto_hash(hash2, message.data(), msg_size));
    ASSERT_EQ(0, memcmp(hash1, hash2, 64));
}

/* Message sizes to test */
INSTANTIATE_TEST_SUITE_P(
    VariousSizes,
    MessageSizeTest,
    ::testing::Values(0, 1, 32, 1024, 65536)
);

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
