"""Unit tests for crypto_box (Curve25519-XSalsa20-Poly1305)."""

import pytest
from tweetnacl.primitives.box import Box
from tweetnacl.exceptions import VerificationError


class TestBox:
    """Tests for public-key authenticated encryption."""

    def test_keypair_generation(self, box_keypair):
        assert len(box_keypair.public_key) == Box.PUBLIC_KEY_BYTES
        assert len(box_keypair.secret_key) == Box.SECRET_KEY_BYTES
        assert box_keypair.public_key != box_keypair.secret_key

    def test_keypair_uniqueness(self):
        k1 = Box.generate_keypair()
        k2 = Box.generate_keypair()
        assert k1.public_key != k2.public_key
        assert k1.secret_key != k2.secret_key

    def test_encrypt_decrypt(self, box_keypair, random_message, box_nonce):
        message = random_message()
        box = Box(box_keypair.public_key, box_keypair.secret_key)
        ciphertext = box.encrypt(message, box_nonce)
        decrypted = box.decrypt(ciphertext, box_nonce)
        assert decrypted == message

    def test_wrong_key_fails(self, box_keypair, random_message, box_nonce):
        message = random_message()
        box = Box(box_keypair.public_key, box_keypair.secret_key)
        ciphertext = box.encrypt(message, box_nonce)

        wrong_keys = Box.generate_keypair()
        wrong_box = Box(wrong_keys.public_key, wrong_keys.secret_key)

        with pytest.raises(VerificationError):
            wrong_box.decrypt(ciphertext, box_nonce)

    def test_tampered_ciphertext(self, box_keypair, random_message, box_nonce):
        message = random_message()
        box = Box(box_keypair.public_key, box_keypair.secret_key)
        ciphertext = bytearray(box.encrypt(message, box_nonce))
        ciphertext[0] ^= 0xFF

        with pytest.raises(VerificationError):
            box.decrypt(bytes(ciphertext), box_nonce)

    def test_empty_message(self, box_keypair, box_nonce):
        box = Box(box_keypair.public_key, box_keypair.secret_key)
        ciphertext = box.encrypt(b'', box_nonce)
        assert box.decrypt(ciphertext, box_nonce) == b''

    def test_nonce_reuse_different_ciphertexts(self, box_keypair, box_nonce):
        box = Box(box_keypair.public_key, box_keypair.secret_key)
        ct1 = box.encrypt(b'msg1', box_nonce)
        ct2 = box.encrypt(b'msg2', box_nonce)
        assert ct1 != ct2
