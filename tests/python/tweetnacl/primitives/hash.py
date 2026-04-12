"""Hash functions (crypto_hash: SHA-512)."""

from tweetnacl.bindings import _lib, CryptoTypes


class Hash:
    """SHA-512 hashing."""

    BYTES = 64

    @classmethod
    def sha512(cls, message: bytes) -> bytes:
        out = (CryptoTypes.u8 * cls.BYTES)()
        m_arr = CryptoTypes.u8_array(len(message))(*message)
        result = _lib.crypto_hash(out, m_arr, len(message))
        if result != 0:
            raise RuntimeError("Hashing failed")
        return bytes(out)

    @classmethod
    def verify(cls, message: bytes, expected_hash: bytes) -> bool:
        computed = cls.sha512(message)
        return computed == expected_hash
