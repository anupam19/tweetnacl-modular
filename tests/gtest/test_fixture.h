/*
 * TweetNaCl-Modular GTest Test Fixture
 * Common test fixture class for Google Test
 * 
 * Author: Anupam Datta <adbd04@gmail.com>
 */

#ifndef TEST_FIXTURE_H
#define TEST_FIXTURE_H

#include <gtest/gtest.h>
#include <stdint.h>
#include <string.h>
#include <vector>

extern "C" {
#include "../../includes/tweetnacl.h"
#include "../../includes/randombytes.h"
#include "../../includes/secure_mem.h"
#include "../../pqc/pqc.h"
}

/**
 * @brief Base test fixture for TweetNaCl tests
 * Provides common setup and utility functions
 */
class TweetNaClTest : public ::testing::Test {
protected:
    void SetUp() override {
        /* Initialize any required state */
    }
    
    void TearDown() override {
        /* Clean up sensitive data */
    }
    
    /**
     * @brief Generate random bytes for testing
     */
    std::vector<uint8_t> randomBytes(size_t len) {
        std::vector<uint8_t> buffer(len);
        randombytes(buffer.data(), len);
        return buffer;
    }
    
    /**
     * @brief Securely zero a buffer
     */
    void secureZero(std::vector<uint8_t>& buffer) {
        secure_zero(buffer.data(), buffer.size());
    }
};

/**
 * @brief PQC-specific test fixture
 * Extends TweetNaClTest with PQC initialization
 */
class PqcTest : public TweetNaClTest {
protected:
    pqc_params_t params_{};
    
    void SetUp() override {
        TweetNaClTest::SetUp();
        /* Default to KYBER768 */
        pqc_get_params(PQC_KYBER768, &params_);
    }
};

/**
 * @brief Secure memory test fixture
 */
class SecureMemTest : public ::testing::Test {
protected:
    std::vector<uint8_t> buffer_;
    
    void SetUp() override {
        buffer_.resize(64);
        /* Fill with pattern */
        for (size_t i = 0; i < buffer_.size(); i++) {
            buffer_[i] = static_cast<uint8_t>(i & 0xFF);
        }
    }
    
    void TearDown() override {
        secure_zero(buffer_.data(), buffer_.size());
    }
};

/**
 * @brief Parameterized test fixture for different message sizes
 */
class MessageSizeTest : public TweetNaClTest,
                        public ::testing::WithParamInterface<size_t> {
};

#endif /* TEST_FIXTURE_H */
