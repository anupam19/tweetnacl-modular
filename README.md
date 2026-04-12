# TweetNaCl-Modular 🔐

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)]()
[![Security Audit](https://img.shields.io/badge/security-audited-blue)](doc/SECURITY.md)
[![CERT C](https://img.shields.io/badge/CERT_C-compliant-green.svg)]()
[![Platform](https://img.shields.io/badge/platform-x86%20%7C%20ARM%20%7C%20RISC--V-lightgrey.svg)]()

**Production-grade, modular TweetNaCl with Post-Quantum Cryptography**

A security-critical C library that refactors the original [TweetNaCl](https://tweetnacl.cr.yp.to) into a modular, production-ready system with multi-architecture support, hardware DRNG, post-quantum cryptography integration, and CERT C secure coding compliance.

---

## 🚀 Quick Start

```bash
git clone https://github.com/anupam19/tweetnacl-modular.git && \
cd tweetnacl-modular && cmake -B build && cmake --build build --parallel && \
cd build && ctest --output-on-failure
```

### Key Commands
| Command | Description |
|---------|-------------|
| `cmake -B build && cmake --build build` | Build library, tests, and app |
| `cd build && ctest --output-on-failure` | Run all tests |
| `./run_full_test_suite.sh` | Full test suite with report |
| `sudo cmake --install build` | Install system-wide |

### CMake Options
| Flag | Default | Description |
|------|---------|-------------|
| `-DENABLE_DRNG` | ON | Hardware RNG (RDRAND/RDSEED/ARM RNDR) |
| `-DENABLE_PQC` | ON | Post-Quantum Cryptography |
| `-DBUILD_TESTS` | ON | Build test suite |
| `-DENABLE_SANITIZERS` | OFF | ASan + UBSan |
| `-DENABLE_COVERAGE` | OFF | gcov/lcov coverage |

Full build guide → [doc/BUILD.md](doc/BUILD.md)

---

## ✨ Security Highlights

- ✅ **Constant-time** implementations — no timing side-channels
- ✅ **Zero dynamic allocation** — embedded-friendly, no heap
- ✅ **Secure memory wiping** — compiler-resistant zeroization
- ✅ **Post-Quantum Hybrid modes** — Kyber, Dilithium, Falcon
- ✅ **Hardware DRNG** — RDRAND/RDSEED/ARM RNDR with fallback
- ✅ **CERT C Compliant** — secure coding standards
- ✅ **Known Answer Tests** — NIST CAVP vectors for all primitives
- ✅ **NIST 800-193** self-tests and integrity checks

Security details → [doc/SECURITY.md](doc/SECURITY.md) | [doc/CONSTANT_TIME.md](doc/CONSTANT_TIME.md)

---

## 🏗️ Architecture

```
┌───────────────────────────────────────────────┐
│  Application Layer (box, sign, secretbox)     │
├───────────────────────────────────────────────┤
│  PQC Hybrid Layer (Kyber, Dilithium, Falcon)  │
├───────────────────────────────────────────────┤
│  Primitives (Curve25519, Ed25519, Salsa20)    │
├───────────────────────────────────────────────┤
│  Secure Memory / Architecture / DRNG Layer    │
└───────────────────────────────────────────────┘
```

**Architectures:** x86_64, x86_32, ARM64, ARM32, RISC-V 64/32

**Hardware acceleration:** SSE2, AVX2, NEON (software fallback always available)

Architecture details → [doc/ARCHITECTURE.md](doc/ARCHITECTURE.md)

---

## 📊 Performance

| Operation | x86_64 @ 3GHz | ARM64 @ 2GHz |
|-----------|---------------|--------------|
| SHA-512 (1KB) | ~15 μs | ~22 μs |
| Ed25519 Sign | ~833 μs | ~1200 μs |
| Kyber768 Keygen | ~500 μs | ~750 μs |

Detailed benchmarks → [doc/BUILD.md](doc/BUILD.md#performance)

---

## 📚 Documentation

| Document | Description |
|----------|-------------|
| [BUILD.md](doc/BUILD.md) | Build instructions, cross-compilation, performance |
| [API_REFERENCE.md](doc/API_REFERENCE.md) | Complete API documentation with examples |
| [PQC.md](doc/PQC.md) | Post-Quantum Cryptography guide |
| [SECURITY.md](doc/SECURITY.md) | Security policies, threat model, NIST 800-193 |
| [CONSTANT_TIME.md](doc/CONSTANT_TIME.md) | Side-channel resistance analysis |
| [ARCHITECTURE.md](doc/ARCHITECTURE.md) | Code organization and design |
| [TESTING.md](doc/TESTING.md) | Testing framework, fuzzing, coverage |
| [PORTING_GUIDE.md](doc/PORTING_GUIDE.md) | Migration from other libraries |
| [FAQ.md](doc/FAQ.md) | Frequently asked questions |
| [CHANGELOG.md](doc/CHANGELOG.md) | Version history |
| [CONTACT.md](doc/CONTACT.md) | Contact information |

Full documentation index → [doc/INDEX.md](doc/INDEX.md)

---

## 🔒 Security Contact

**Anupam Datta** — [adbd04@gmail.com](mailto:adbd04@gmail.com)

For vulnerabilities, email with `[SECURITY]` prefix. See [doc/SECURITY.md](doc/SECURITY.md) for our disclosure policy.

---

## 📄 License

Original TweetNaCl: **Public Domain**
This fork: **MIT**

See [LICENSE](LICENSE) for details.
