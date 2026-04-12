"""Unit tests for crypto_hash (SHA-512)."""

import hashlib
import pytest
from tweetnacl.primitives.hash import Hash


class TestHash:
    """Tests for SHA-512 hashing."""

    def test_sha512_length(self, random_message):
        message = random_message()
        result = Hash.sha512(message)
        assert len(result) == Hash.BYTES

    def test_sha512_deterministic(self, random_message):
        message = random_message()
        h1 = Hash.sha512(message)
        h2 = Hash.sha512(message)
        assert h1 == h2

    def test_sha512_against_python_hashlib(self, random_message):
        message = random_message()
        our_hash = Hash.sha512(message)
        py_hash = hashlib.sha512(message).digest()
        assert our_hash == py_hash

    def test_sha512_different_messages(self, random_message):
        msg1 = random_message()
        msg2 = random_message()
        h1 = Hash.sha512(msg1)
        h2 = Hash.sha512(msg2)
        assert h1 != h2

    def test_sha512_known_vector_abc(self):
        expected = bytes.fromhex(
            "ddaf35a193617abacc417349ae20413112e6fa4e89a97ea2"
            "0a9eeee64b55d39a2192992a274fc1a836ba3c23a3feebbd"
            "454d4423643ce80e2a9ac94fa54ca49f"
        )
        assert Hash.sha512(b"abc") == expected

    def test_sha512_known_vector_empty(self):
        expected = bytes.fromhex(
            "cf83e1357eefb8bdf1542850d66d8007d620e4050b5715dc"
            "83f4a921d36ce9ce47d0d13c5d85f2b0ff8318d2877eec2f"
            "63b931bd47417a81a538327af927da3e"
        )
        assert Hash.sha512(b"") == expected
