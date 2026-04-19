/*
 * TweetNaCl-Modular GTest Suite
 * C++ tests with RAII wrappers, parameterized tests, and fixtures
 */

#include <gtest/gtest.h>
#include <cstdint>
#include <cstring>
#include <vector>
#include <string>
#include "../includes/tweetnacl.h"
#include "../includes/randombytes.h"
#include "../includes/secure_mem.h"
#include "vectors/test_vectors.h"

/* ─── RAII Wrappers ───────────────────────────────────────────────────── */

class NaClKeyPair {
public:
    NaClKeyPair() {
        EXPECT_EQ(crypto_box_keypair(pk_.data(), sk_.data()), 0);
    }
    const std::array<uint8_t, 32>& pk() const { return pk_; }
    const std::array<uint8_t, 32>& sk() const { return sk_; }
private:
    std::array<uint8_t, 32> pk_{};
    std::array<uint8_t, 32> sk_{};
};

class SecureBuffer {
public:
    explicit SecureBuffer(size_t size) : size_(size), data_(size, 0) {}
    ~SecureBuffer() { secure_zero(data_.data(), data_.size()); }
    uint8_t* data() { return data_.data(); }
    const uint8_t* data() const { return data_.data(); }
    size_t size() const { return size_; }
private:
    size_t size_;
    std::vector<uint8_t> data_;
};

/* ─── Fixtures ────────────────────────────────────────────────────────── */

class TweetNaClTest : public ::testing::Test {
protected:
    void SetUp() override {
        ASSERT_EQ(NACL_SUCCESS, randombytes_safe(nonce_.data(), 24));
    }
    std::array<uint8_t, 24> nonce_{};
};

class SecureMemTest : public ::testing::Test {
protected:
    void SetUp() override {
        memset(buf_, 0x55, sizeof(buf_));
    }
    uint8_t buf_[256]{};
};

/* ─── SHA-512 Tests ───────────────────────────────────────────────────── */

TEST_F(TweetNaClTest, SHA512KnownVectors) {
    std::array<uint8_t, 64> hash{};
    crypto_hash(hash.data(), reinterpret_cast<const uint8_t*>("abc"), 3);
    EXPECT_EQ(memcmp(hash.data(), tv_sha512_abc, 64), 0);

    crypto_hash(hash.data(), reinterpret_cast<const uint8_t*>(""), 0);
    EXPECT_EQ(memcmp(hash.data(), tv_sha512_empty, 64), 0);
}

TEST_F(TweetNaClTest, SHA512Deterministic) {
    std::array<uint8_t, 64> h1{}, h2{};
    const char msg[] = "test";
    crypto_hash(h1.data(), reinterpret_cast<const uint8_t*>(msg), 4);
    crypto_hash(h2.data(), reinterpret_cast<const uint8_t*>(msg), 4);
    EXPECT_EQ(h1, h2);
}

/* ─── Box Tests ───────────────────────────────────────────────────────── */

TEST_F(TweetNaClTest, BoxEncryptDecrypt) {
    NaClKeyPair alice, bob;
    SecureBuffer msg(100), ct(100), pt(100);

    memset(msg.data(), 0, 32);
    memcpy(msg.data() + 32, "Hello GTest!", 13);

    EXPECT_EQ(crypto_box(ct.data(), msg.data(), msg.size(), nonce_.data(),
                         bob.pk().data(), alice.sk().data()), 0);
    EXPECT_EQ(crypto_box_open(pt.data(), ct.data(), ct.size(), nonce_.data(),
                              alice.pk().data(), bob.sk().data()), 0);
    EXPECT_EQ(memcmp(msg.data() + 32, pt.data() + 32, 13), 0);
}

TEST_F(TweetNaClTest, BoxWrongKeyFails) {
    NaClKeyPair alice, bob, eve;
    SecureBuffer msg(100), ct(100), pt(100);
    memset(msg.data(), 0, 32);

    EXPECT_EQ(crypto_box(ct.data(), msg.data(), msg.size(), nonce_.data(),
                         bob.pk().data(), alice.sk().data()), 0);
    EXPECT_NE(crypto_box_open(pt.data(), ct.data(), ct.size(), nonce_.data(),
                              eve.pk().data(), bob.sk().data()), 0);
}

/* ─── Sign Tests ──────────────────────────────────────────────────────── */

TEST_F(TweetNaClTest, SignRoundTrip) {
    uint8_t pk[32], sk[64];
    ASSERT_EQ(crypto_sign_keypair(pk, sk), 0);

    std::vector<uint8_t> msg(50);
    ASSERT_EQ(NACL_SUCCESS, randombytes_safe(msg.data(), msg.size()));

    std::vector<uint8_t> sm(msg.size() + 64);
    uint64_t smlen = 0;
    ASSERT_EQ(crypto_sign(sm.data(), &smlen, msg.data(), msg.size(), sk), 0);
    EXPECT_EQ(smlen, msg.size() + 64);

    std::vector<uint8_t> vm(msg.size() + 64);
    uint64_t vmlen = 0;
    ASSERT_EQ(crypto_sign_open(vm.data(), &vmlen, sm.data(), smlen, pk), 0);
    EXPECT_EQ(vmlen, msg.size());
    EXPECT_EQ(memcmp(msg.data(), vm.data(), msg.size()), 0);
}

/* ─── Secure Memory Tests ─────────────────────────────────────────────── */

TEST_F(SecureMemTest, SecureZeroClears) {
    memset(buf_, 0xAA, sizeof(buf_));
    secure_zero(buf_, sizeof(buf_));
    for (auto b : buf_) EXPECT_EQ(b, 0);
}

TEST_F(SecureMemTest, MemcmpEqual) {
    uint8_t other[256];
    memcpy(other, buf_, sizeof(buf_));
    EXPECT_EQ(secure_memcmp(buf_, other, sizeof(buf_)), 0);
}

TEST_F(SecureMemTest, MemcmpNotEqual) {
    uint8_t other[256];
    memcpy(other, buf_, sizeof(buf_));
    other[128] ^= 0xFF;
    EXPECT_NE(secure_memcmp(buf_, other, sizeof(buf_)), 0);
}

/* ─── Parameterized Tests ─────────────────────────────────────────────── */

class MessageSizeTest : public TweetNaClTest,
                        public ::testing::WithParamInterface<size_t> {};

TEST_P(MessageSizeTest, SecretBoxRoundTrip) {
    size_t msg_size = GetParam();
    size_t buf_size = msg_size + 32;  /* Include padding */

    uint8_t key[32];
    ASSERT_EQ(NACL_SUCCESS, randombytes_safe(key, 32));

    std::vector<uint8_t> msg(buf_size, 0);
    std::vector<uint8_t> ct(buf_size), pt(buf_size);

    /* Fill actual message after 32-byte zero padding */
    if (msg_size > 0) {
        ASSERT_EQ(NACL_SUCCESS, randombytes_safe(msg.data() + 32, msg_size));
    }

    EXPECT_EQ(crypto_secretbox(ct.data(), msg.data(), buf_size, nonce_.data(), key), 0);
    EXPECT_EQ(crypto_secretbox_open(pt.data(), ct.data(), buf_size, nonce_.data(), key), 0);
    EXPECT_EQ(memcmp(msg.data() + 32, pt.data() + 32, msg_size), 0);
}

INSTANTIATE_TEST_SUITE_P(VariousSizes, MessageSizeTest,
    ::testing::Values(0, 1, 32, 128, 1024));

/* ─── Death Tests (Debug builds) ──────────────────────────────────────── */

#ifndef NDEBUG
TEST(SecureMemDeathTest, NullDest) {
    /* safe_memcpy with NULL dest should return error, not crash */
    uint8_t buf[32];
    EXPECT_NE(safe_memcpy(nullptr, sizeof(buf), buf, 16), 0);
}
#endif
