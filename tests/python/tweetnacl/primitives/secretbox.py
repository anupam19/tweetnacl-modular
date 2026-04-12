"""Secret-key authenticated encryption (crypto_secretbox: XSalsa20-Poly1305)."""

from tweetnacl.bindings import _lib, CryptoTypes
from tweetnacl.exceptions import CryptoError, VerificationError


class SecretBox:
    """Secret-key authenticated encryption."""

    KEY_BYTES = 32
    NONCE_BYTES = 24
    ZERO_BYTES = 32

    def __init__(self, key: bytes):
        if len(key) != self.KEY_BYTES:
            raise ValueError(f"Key must be {self.KEY_BYTES} bytes")
        self.key = key

    def encrypt(self, message: bytes, nonce: bytes) -> bytes:
        if len(nonce) != self.NONCE_BYTES:
            raise ValueError(f"Nonce must be {self.NONCE_BYTES} bytes")

        padded = b'\x00' * self.ZERO_BYTES + message
        buf_size = len(padded)
        c = (CryptoTypes.u8 * buf_size)()
        m_arr = CryptoTypes.u8_array(buf_size)(*padded)
        n_arr = CryptoTypes.u8_array(self.NONCE_BYTES)(*nonce)
        k_arr = CryptoTypes.u8_array(self.KEY_BYTES)(*self.key)

        result = _lib.crypto_secretbox(c, m_arr, buf_size, n_arr, k_arr)
        if result != 0:
            raise CryptoError("Encryption failed")

        return bytes(c)[self.ZERO_BYTES:]

    def decrypt(self, ciphertext: bytes, nonce: bytes) -> bytes:
        if len(nonce) != self.NONCE_BYTES:
            raise ValueError(f"Nonce must be {self.NONCE_BYTES} bytes")

        padded = b'\x00' * self.ZERO_BYTES + ciphertext
        buf_size = len(padded)
        m = (CryptoTypes.u8 * buf_size)()
        c_arr = CryptoTypes.u8_array(buf_size)(*padded)
        n_arr = CryptoTypes.u8_array(self.NONCE_BYTES)(*nonce)
        k_arr = CryptoTypes.u8_array(self.KEY_BYTES)(*self.key)

        result = _lib.crypto_secretbox_open(m, c_arr, buf_size, n_arr, k_arr)
        if result != 0:
            raise VerificationError("Decryption failed — invalid ciphertext or key")

        return bytes(m)[self.ZERO_BYTES:]
