"""Unit tests for crypto_secretbox (XSalsa20-Poly1305)."""

import pytest
from tweetnacl.primitives.secretbox import SecretBox
from tweetnacl.exceptions import VerificationError


class TestSecretBox:
    """Tests for secret-key authenticated encryption."""

    def test_encrypt_decrypt(self, secretbox_key, random_message, secretbox_nonce):
        message = random_message()
        box = SecretBox(secretbox_key)
        ciphertext = box.encrypt(message, secretbox_nonce)
        decrypted = box.decrypt(ciphertext, secretbox_nonce)
        assert decrypted == message

    def test_wrong_key_fails(self, secretbox_key, random_message, secretbox_nonce):
        message = random_message()
        box = SecretBox(secretbox_key)
        ciphertext = box.encrypt(message, secretbox_nonce)

        wrong_key = b'\xff' * 32
        wrong_box = SecretBox(wrong_key)
        with pytest.raises(VerificationError):
            wrong_box.decrypt(ciphertext, secretbox_nonce)

    def test_tampered_ciphertext(self, secretbox_key, random_message, secretbox_nonce):
        message = random_message()
        box = SecretBox(secretbox_key)
        ciphertext = bytearray(box.encrypt(message, secretbox_nonce))
        ciphertext[0] ^= 0xFF

        with pytest.raises(VerificationError):
            box.decrypt(bytes(ciphertext), secretbox_nonce)

    def test_empty_message(self, secretbox_key, secretbox_nonce):
        box = SecretBox(secretbox_key)
        ciphertext = box.encrypt(b'', secretbox_nonce)
        assert box.decrypt(ciphertext, secretbox_nonce) == b''
