/**
 * Google Test Tests for TweetNaCl-Modular
 * 
 * C++ test suite using Google Test framework.
 */

#include <gtest/gtest.h>
#include <cstdint>
#include <cstring>
#include <vector>
#include "../../includes/tweetnacl.h"
#include "../../includes/randombytes.h"
#include "../../includes/secure_mem.h"

/**
 * @brief Base test fixture for TweetNaCl tests
 */
class TweetNaClTest : public ::testing::Test {
protected:
    void SetUp() override {
        /* Initialize test state if needed */
    }
    
    void TearDown() override {
        /* Clean up sensitive data */
    }
    
    std::vector<uint8_t> generate_random(size_t len) {
        std::vector<uint8_t> buf(len);
        randombytes(buf.data(), len);
        return buf;
    }
};

/**
 * @brief Test crypto_hash function
 */
TEST_F(TweetNaClTest, CryptoHashBasic) {
    const uint8_t message[] = "test message";
    uint8_t hash[64];
    
    int result = crypto_hash(hash, message, sizeof(message) - 1);
    
    EXPECT_EQ(result, 0);
    
    /* Verify hash is not all zeros */
    bool all_zero = true;
    for (size_t i = 0; i < 64; i++) {
        if (hash[i] != 0) {
            all_zero = false;
            break;
        }
    }
    EXPECT_FALSE(all_zero);
}

/**
 * @brief Test crypto_verify_32 function with equal buffers
 */
TEST_F(TweetNaClTest, CryptoVerify32Equal) {
    uint8_t buf1[32], buf2[32];
    
    randombytes(buf1, 32);
    memcpy(buf2, buf1, 32);
    
    int result = crypto_verify_32(buf1, buf2);
    EXPECT_EQ(result, 0);
}

/**
 * @brief Test crypto_verify_32 function with different buffers
 */
TEST_F(TweetNaClTest, CryptoVerify32Different) {
    uint8_t buf1[32], buf2[32];
    
    randombytes(buf1, 32);
    randombytes(buf2, 32);
    
    /* Ensure they're different */
    buf2[0] = ~buf1[0];
    
    int result = crypto_verify_32(buf1, buf2);
    EXPECT_NE(result, 0);
}

/**
 * @brief Test secure_zero function
 */
TEST_F(TweetNaClTest, SecureZeroBasic) {
    uint8_t buffer[64];
    
    randombytes(buffer, 64);
    
    /* Verify buffer has non-zero data */
    bool has_nonzero = false;
    for (size_t i = 0; i < 64; i++) {
        if (buffer[i] != 0) {
            has_nonzero = true;
            break;
        }
    }
    EXPECT_TRUE(has_nonzero);
    
    /* Zero the buffer */
    secure_zero(buffer, sizeof(buffer));
    
    /* Verify buffer is now zero */
    for (size_t i = 0; i < 64; i++) {
        EXPECT_EQ(buffer[i], 0);
    }
}

/**
 * @brief Test secure_memcmp function
 */
TEST_F(TweetNaClTest, SecureMemcmpEqual) {
    uint8_t buf1[32], buf2[32];
    
    randombytes(buf1, 32);
    memcpy(buf2, buf1, 32);
    
    int result = secure_memcmp(buf1, buf2, 32);
    EXPECT_EQ(result, 0);
}

TEST_F(TweetNaClTest, SecureMemcmpDifferent) {
    uint8_t buf1[32], buf2[32];
    
    randombytes(buf1, 32);
    randombytes(buf2, 32);
    
    buf2[0] = ~buf1[0];
    
    int result = secure_memcmp(buf1, buf2, 32);
    EXPECT_NE(result, 0);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
