"""Public-key authenticated encryption (crypto_box: Curve25519-XSalsa20-Poly1305)."""

from dataclasses import dataclass
from typing import Tuple

from tweetnacl.bindings import _lib, CryptoTypes
from tweetnacl.exceptions import CryptoError, VerificationError


@dataclass(frozen=True)
class BoxKeyPair:
    public_key: bytes
    secret_key: bytes

    def __post_init__(self) -> None:
        if len(self.public_key) != 32:
            raise ValueError("Public key must be 32 bytes")
        if len(self.secret_key) != 32:
            raise ValueError("Secret key must be 32 bytes")


class Box:
    """Public-key authenticated encryption box."""

    PUBLIC_KEY_BYTES = 32
    SECRET_KEY_BYTES = 32
    NONCE_BYTES = 24
    ZERO_BYTES = 32

    def __init__(self, their_public_key: bytes, my_secret_key: bytes):
        if len(their_public_key) != self.PUBLIC_KEY_BYTES:
            raise ValueError(f"Public key must be {self.PUBLIC_KEY_BYTES} bytes")
        if len(my_secret_key) != self.SECRET_KEY_BYTES:
            raise ValueError(f"Secret key must be {self.SECRET_KEY_BYTES} bytes")
        self.their_public_key = their_public_key
        self.my_secret_key = my_secret_key

    @classmethod
    def generate_keypair(cls) -> BoxKeyPair:
        pk = (CryptoTypes.u8 * cls.PUBLIC_KEY_BYTES)()
        sk = (CryptoTypes.u8 * cls.SECRET_KEY_BYTES)()
        result = _lib.crypto_box_keypair(pk, sk)
        if result != 0:
            raise CryptoError("Key generation failed")
        return BoxKeyPair(public_key=bytes(pk), secret_key=bytes(sk))

    def encrypt(self, message: bytes, nonce: bytes) -> bytes:
        if len(nonce) != self.NONCE_BYTES:
            raise ValueError(f"Nonce must be {self.NONCE_BYTES} bytes")

        padded = b'\x00' * self.ZERO_BYTES + message
        buf_size = len(padded)
        c = (CryptoTypes.u8 * buf_size)()
        m_arr = CryptoTypes.u8_array(buf_size)(*padded)
        n_arr = CryptoTypes.u8_array(self.NONCE_BYTES)(*nonce)
        pk_arr = CryptoTypes.u8_array(self.PUBLIC_KEY_BYTES)(*self.their_public_key)
        sk_arr = CryptoTypes.u8_array(self.SECRET_KEY_BYTES)(*self.my_secret_key)

        result = _lib.crypto_box(c, m_arr, buf_size, n_arr, pk_arr, sk_arr)
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
        pk_arr = CryptoTypes.u8_array(self.PUBLIC_KEY_BYTES)(*self.their_public_key)
        sk_arr = CryptoTypes.u8_array(self.SECRET_KEY_BYTES)(*self.my_secret_key)

        result = _lib.crypto_box_open(m, c_arr, buf_size, n_arr, pk_arr, sk_arr)
        if result != 0:
            raise VerificationError("Decryption failed — invalid ciphertext or key")

        return bytes(m)[self.ZERO_BYTES:]
