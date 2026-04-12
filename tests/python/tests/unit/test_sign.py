"""Unit tests for crypto_sign (Ed25519)."""

import pytest
from tweetnacl.primitives.sign import Sign
from tweetnacl.exceptions import VerificationError


class TestSign:
    """Tests for Ed25519 digital signatures."""

    def test_keypair_generation(self, sign_keypair):
        assert len(sign_keypair.public_key) == Sign.PUBLIC_KEY_BYTES
        assert len(sign_keypair.secret_key) == Sign.SECRET_KEY_BYTES

    def test_sign_verify(self, sign_keypair, random_message):
        message = random_message()
        signer = Sign(sign_keypair.public_key, sign_keypair.secret_key)
        signed = signer.sign(message)
        verifier = Sign(sign_keypair.public_key)
        verified = verifier.verify(signed)
        assert verified == message

    def test_wrong_key_fails(self, sign_keypair, random_message):
        message = random_message()
        signer = Sign(sign_keypair.public_key, sign_keypair.secret_key)
        signed = signer.sign(message)

        wrong_keys = Sign.generate_keypair()
        wrong_verifier = Sign(wrong_keys.public_key)
        with pytest.raises(VerificationError):
            wrong_verifier.verify(signed)

    def test_tampered_message(self, sign_keypair, random_message):
        message = random_message()
        signer = Sign(sign_keypair.public_key, sign_keypair.secret_key)
        signed = bytearray(signer.sign(message))
        # Tamper with message portion (after 64-byte signature)
        if len(signed) > 64:
            signed[64] ^= 0xFF

        verifier = Sign(sign_keypair.public_key)
        with pytest.raises(VerificationError):
            verifier.verify(bytes(signed))

    def test_deterministic_signatures(self, sign_keypair, random_message):
        message = random_message()
        signer = Sign(sign_keypair.public_key, sign_keypair.secret_key)
        sig1 = signer.sign(message)
        sig2 = signer.sign(message)
        assert sig1 == sig2
