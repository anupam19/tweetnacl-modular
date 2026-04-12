"""pytest fixtures and configuration."""

import secrets
import pytest
from tweetnacl.primitives.box import Box, BoxKeyPair
from tweetnacl.primitives.sign import Sign, SignKeyPair
from tweetnacl.primitives.secretbox import SecretBox
from tweetnacl.primitives.hash import Hash


@pytest.fixture
def random_message():
    """Generate a random message of given or random size."""
    def _make(size: int = None) -> bytes:
        if size is None:
            size = secrets.randbelow(1024) + 1
        return secrets.token_bytes(size)
    return _make


@pytest.fixture
def box_keypair() -> BoxKeyPair:
    """Generate a box key pair."""
    return Box.generate_keypair()


@pytest.fixture
def sign_keypair() -> SignKeyPair:
    """Generate a signing key pair."""
    return Sign.generate_keypair()


@pytest.fixture
def secretbox_key() -> bytes:
    """Generate a secretbox key."""
    return secrets.token_bytes(32)


@pytest.fixture
def box_nonce() -> bytes:
    """Generate a 24-byte nonce."""
    return secrets.token_bytes(24)


@pytest.fixture
def secretbox_nonce() -> bytes:
    """Generate a 24-byte nonce."""
    return secrets.token_bytes(24)
