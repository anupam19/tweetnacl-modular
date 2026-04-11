/**
 * Google Test Tests for Secure Memory Functions (Additional C++ tests)
 */

#include <gtest/gtest.h>
#include <cstdint>
#include <cstring>
#include "../../includes/secure_mem.h"

TEST(SecureMemTest, SecureMemsetBasic) {
    uint8_t buffer[64];
    
    secure_memset(buffer, 0xAB, sizeof(buffer));
    
    for (size_t i = 0; i < 64; i++) {
        EXPECT_EQ(buffer[i], 0xAB);
    }
}

TEST(SecureMemTest, SecureMemcpyBasic) {
    uint8_t src[32], dst[32];
    
    for (size_t i = 0; i < 32; i++) {
        src[i] = static_cast<uint8_t>(i);
    }
    
    secure_memcpy(dst, src, 32);
    
    for (size_t i = 0; i < 32; i++) {
        EXPECT_EQ(dst[i], src[i]);
    }
}
