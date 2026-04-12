"""Low-level ctypes bindings for TweetNaCl-Modular library."""

import ctypes
import os
import platform
from pathlib import Path


def _find_library() -> str:
    """Find the compiled tweetnacl shared library."""
    system = platform.system()
    if system == "Linux":
        lib_name = "libtweetnacl.so"
    elif system == "Darwin":
        lib_name = "libtweetnacl.dylib"
    elif system == "Windows":
        lib_name = "tweetnacl.dll"
    else:
        raise OSError(f"Unsupported platform: {system}")

    # Search paths: build dir first, then root, then system
    script_dir = Path(__file__).resolve().parent.parent.parent
    search_paths = [
        script_dir / "build",
        script_dir,
        Path("/usr/local/lib"),
        Path("/usr/lib"),
    ]

    for path in search_paths:
        lib_path = path / lib_name
        if lib_path.exists():
            return str(lib_path)

    raise FileNotFoundError(
        f"Could not find {lib_name}. Build the library first with cmake."
    )


_lib = ctypes.CDLL(_find_library())


class CryptoTypes:
    """C type definitions for TweetNaCl functions."""
    u8 = ctypes.c_ubyte
    u32 = ctypes.c_uint
    u64 = ctypes.c_ulonglong

    @staticmethod
    def u8_array(size: int):
        """Create a ctypes array of unsigned bytes."""
        return ctypes.c_ubyte * size


def _setup_signatures() -> None:
    """Setup ctypes function signatures for type safety."""

    # crypto_box_keypair(pk, sk) -> int
    _lib.crypto_box_keypair.argtypes = [
        ctypes.POINTER(CryptoTypes.u8),
        ctypes.POINTER(CryptoTypes.u8),
    ]
    _lib.crypto_box_keypair.restype = ctypes.c_int

    # crypto_box(c, m, d, n, pk, sk) -> int
    _lib.crypto_box.argtypes = [
        ctypes.POINTER(CryptoTypes.u8),
        ctypes.POINTER(CryptoTypes.u8),
        CryptoTypes.u64,
        ctypes.POINTER(CryptoTypes.u8),
        ctypes.POINTER(CryptoTypes.u8),
        ctypes.POINTER(CryptoTypes.u8),
    ]
    _lib.crypto_box.restype = ctypes.c_int

    # crypto_box_open(m, c, d, n, pk, sk) -> int
    _lib.crypto_box_open.argtypes = [
        ctypes.POINTER(CryptoTypes.u8),
        ctypes.POINTER(CryptoTypes.u8),
        CryptoTypes.u64,
        ctypes.POINTER(CryptoTypes.u8),
        ctypes.POINTER(CryptoTypes.u8),
        ctypes.POINTER(CryptoTypes.u8),
    ]
    _lib.crypto_box_open.restype = ctypes.c_int

    # crypto_sign_keypair(pk, sk) -> int
    _lib.crypto_sign_keypair.argtypes = [
        ctypes.POINTER(CryptoTypes.u8),
        ctypes.POINTER(CryptoTypes.u8),
    ]
    _lib.crypto_sign_keypair.restype = ctypes.c_int

    # crypto_sign(sm, smlen, m, n, sk) -> int
    _lib.crypto_sign.argtypes = [
        ctypes.POINTER(CryptoTypes.u8),
        ctypes.POINTER(CryptoTypes.u64),
        ctypes.POINTER(CryptoTypes.u8),
        CryptoTypes.u64,
        ctypes.POINTER(CryptoTypes.u8),
    ]
    _lib.crypto_sign.restype = ctypes.c_int

    # crypto_sign_open(m, mlen, sm, n, pk) -> int
    _lib.crypto_sign_open.argtypes = [
        ctypes.POINTER(CryptoTypes.u8),
        ctypes.POINTER(CryptoTypes.u64),
        ctypes.POINTER(CryptoTypes.u8),
        CryptoTypes.u64,
        ctypes.POINTER(CryptoTypes.u8),
    ]
    _lib.crypto_sign_open.restype = ctypes.c_int

    # crypto_hash(out, m, n) -> int
    _lib.crypto_hash.argtypes = [
        ctypes.POINTER(CryptoTypes.u8),
        ctypes.POINTER(CryptoTypes.u8),
        CryptoTypes.u64,
    ]
    _lib.crypto_hash.restype = ctypes.c_int

    # crypto_scalarmult_base(q, n) -> int
    _lib.crypto_scalarmult_base.argtypes = [
        ctypes.POINTER(CryptoTypes.u8),
        ctypes.POINTER(CryptoTypes.u8),
    ]
    _lib.crypto_scalarmult_base.restype = ctypes.c_int

    # crypto_secretbox(c, m, d, n, k) -> int
    _lib.crypto_secretbox.argtypes = [
        ctypes.POINTER(CryptoTypes.u8),
        ctypes.POINTER(CryptoTypes.u8),
        CryptoTypes.u64,
        ctypes.POINTER(CryptoTypes.u8),
        ctypes.POINTER(CryptoTypes.u8),
    ]
    _lib.crypto_secretbox.restype = ctypes.c_int

    # crypto_secretbox_open(m, c, d, n, k) -> int
    _lib.crypto_secretbox_open.argtypes = [
        ctypes.POINTER(CryptoTypes.u8),
        ctypes.POINTER(CryptoTypes.u8),
        CryptoTypes.u64,
        ctypes.POINTER(CryptoTypes.u8),
        ctypes.POINTER(CryptoTypes.u8),
    ]
    _lib.crypto_secretbox_open.restype = ctypes.c_int

    # randombytes(buf, len) -> void
    _lib.randombytes.argtypes = [
        ctypes.POINTER(CryptoTypes.u8),
        ctypes.c_size_t,
    ]
    _lib.randombytes.restype = None

    # secure_memset(dest, val, count) -> void
    _lib.secure_memset.argtypes = [
        ctypes.c_void_p,
        ctypes.c_int,
        ctypes.c_size_t,
    ]
    _lib.secure_memset.restype = None

    # secure_memcmp(x, y, n) -> int
    _lib.secure_memcmp.argtypes = [
        ctypes.c_void_p,
        ctypes.c_void_p,
        ctypes.c_size_t,
    ]
    _lib.secure_memcmp.restype = ctypes.c_int


_setup_signatures()
