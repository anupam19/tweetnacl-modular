# TweetNaCl-Modular 🔐

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)]()
[![Security Audit](https://img.shields.io/badge/security-audited-blue.svg)](doc/SECURITY.md)
[![CERT C](https://img.shields.io/badge/CERT_C-compliant-green.svg)]()
[![Platform](https://img.shields.io/badge/platform-x86%20%7C%20ARM%20%7C%20RISC--V-lightgrey.svg)]()

**Production-grade, modular TweetNaCl with Post-Quantum Cryptography support**

> "Auditable cryptography for the post-quantum era"

## 🚀 Quick Start

```bash
git clone https://github.com/anupam19/tweetnacl-modular.git 
cd tweetnacl-modular
make && make test
```

```c
#include "tweetnacl.h"
uint8_t pk[32], sk[32];
crypto_box_keypair(pk, sk);
```

## ✨ Security Highlights

- ✅ **Constant-time implementations** - No timing side-channels ([Analysis](doc/CONSTANT_TIME.md))
- ✅ **Zero dynamic allocation** - Embedded-friendly, no heap
- ✅ **Secure memory wiping** - Compiler-resistant zeroization
- ✅ **Post-Quantum Hybrid modes** - Kyber, Dilithium, Falcon ([Guide](doc/PQC.md))
- ✅ **CERT C Compliant** - Secure coding standards
- ✅ **Multi-architecture** - x86, ARM, RISC-V

📚 [Full Documentation Index](doc/INDEX.md) | 🔧 [Build Guide](doc/BUILD.md) | 📖 [API Reference](doc/API_REFERENCE.md) | 🧪 [Testing Guide](doc/TESTING.md)

## 📊 Performance

| Operation | x86_64 @ 3GHz | ARM64 @ 2GHz |
|-----------|---------------|--------------|
| SHA-512 (1KB) | 15 μs | 22 μs |
| Ed25519 Sign | 833 μs | 1200 μs |
| Kyber768 Keygen | 500 μs | 750 μs |

Detailed benchmarks → [doc/BUILD.md#performance](doc/BUILD.md)

## 🏗️ Architecture

```
┌─────────────────────────────────────────────┐
│  Application Layer (box, sign, secretbox)   │
├─────────────────────────────────────────────┤
│  PQC Hybrid Layer (optional)                │
├─────────────────────────────────────────────┤
│  Primitives (Curve25519, Ed25519, Salsa20)  │
├─────────────────────────────────────────────┤
│  Secure Memory / Architecture Abstraction   │
└─────────────────────────────────────────────┘
```

Architecture Details → [doc/ARCHITECTURE.md](doc/ARCHITECTURE.md)

## 🔒 Security

**Security Contact:** Anupam Datta <adbd04@gmail.com>

- 🛡️ [Security Policy](doc/SECURITY.md)
- ⏱️ [Constant-Time Analysis](doc/CONSTANT_TIME.md)
- 📧 Vulnerability Disclosure: Email with `[SECURITY]` prefix
- 📞 [Contact Information](doc/CONTACT.md)

## 📄 License

- Original TweetNaCl: Public Domain
- This fork: MIT + extensions

See [LICENSE](LICENSE) for details.

---

**Maintainer:** Anupam Datta <adbd04@gmail.com>  
**Documentation:** [doc/INDEX.md](doc/INDEX.md)
