"""Digital signatures (crypto_sign: Ed25519)."""

from dataclasses import dataclass

from tweetnacl.bindings import _lib, CryptoTypes
from tweetnacl.exceptions import CryptoError, VerificationError


@dataclass(frozen=True)
class SignKeyPair:
    public_key: bytes
    secret_key: bytes

    def __post_init__(self) -> None:
        if len(self.public_key) != 32:
            raise ValueError("Public key must be 32 bytes")
        if len(self.secret_key) != 64:
            raise ValueError("Secret key must be 64 bytes")


class Sign:
    """Ed25519 digital signatures."""

    PUBLIC_KEY_BYTES = 32
    SECRET_KEY_BYTES = 64
    SIGNATURE_BYTES = 64

    def __init__(self, public_key: bytes, secret_key: bytes = None):
        if len(public_key) != self.PUBLIC_KEY_BYTES:
            raise ValueError(f"Public key must be {self.PUBLIC_KEY_BYTES} bytes")
        self.public_key = public_key
        self.secret_key = secret_key

    @classmethod
    def generate_keypair(cls) -> SignKeyPair:
        pk = (CryptoTypes.u8 * cls.PUBLIC_KEY_BYTES)()
        sk = (CryptoTypes.u8 * cls.SECRET_KEY_BYTES)()
        result = _lib.crypto_sign_keypair(pk, sk)
        if result != 0:
            raise CryptoError("Key generation failed")
        return SignKeyPair(public_key=bytes(pk), secret_key=bytes(sk))

    def sign(self, message: bytes) -> bytes:
        if self.secret_key is None:
            raise CryptoError("Secret key required for signing")

        msg_len = len(message)
        sm_len = CryptoTypes.u64(0)
        sm = (CryptoTypes.u8 * (msg_len + self.SIGNATURE_BYTES))()
        m_arr = CryptoTypes.u8_array(msg_len)(*message)
        sk_arr = CryptoTypes.u8_array(self.SECRET_KEY_BYTES)(*self.secret_key)

        result = _lib.crypto_sign(sm, sm_len, m_arr, msg_len, sk_arr)
        if result != 0:
            raise CryptoError("Signing failed")

        return bytes(sm[:sm_len.value])

    def verify(self, signed_message: bytes) -> bytes:
        sm_len = len(signed_message)
        m_len = CryptoTypes.u64(0)
        m = (CryptoTypes.u8 * sm_len)()
        sm_arr = CryptoTypes.u8_array(sm_len)(*signed_message)
        pk_arr = CryptoTypes.u8_array(self.PUBLIC_KEY_BYTES)(*self.public_key)

        result = _lib.crypto_sign_open(m, m_len, sm_arr, sm_len, pk_arr)
        if result != 0:
            raise VerificationError("Signature verification failed")

        return bytes(m[:m_len.value])
