"""Custom exceptions for TweetNaCl Python bindings."""


class CryptoError(Exception):
    """Base exception for cryptographic operation failures."""
    pass


class VerificationError(CryptoError):
    """Raised when cryptographic verification fails (bad MAC, signature, etc.)."""
    pass


class KeyGenerationError(CryptoError):
    """Raised when key generation fails."""
    pass
