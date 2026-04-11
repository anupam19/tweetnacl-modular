# TweetNaCl-Modular Documentation Index

Welcome to the comprehensive documentation for TweetNaCl-Modular, a production-grade, modular implementation of TweetNaCl with Post-Quantum Cryptography support.

---

## 🚀 Getting Started

| Document | Description |
|----------|-------------|
| [README.md](../README.md) | Project overview, quick start, and essential information |
| [BUILD.md](BUILD.md) | Comprehensive build instructions for all platforms |
| [PORTING_GUIDE.md](PORTING_GUIDE.md) | Migration guide from other cryptographic libraries |

---

## 📖 API & Usage

| Document | Description |
|----------|-------------|
| [API_REFERENCE.md](API_REFERENCE.md) | Complete API documentation with function signatures and examples |
| [PQC.md](PQC.md) | Post-Quantum Cryptography integration guide (Kyber, Dilithium, Falcon) |
| [TESTING.md](TESTING.md) | **NEW** Testing framework documentation (CUnit + GTest) |

---

## 🔒 Security & Architecture

| Document | Description |
|----------|-------------|
| [SECURITY.md](SECURITY.md) | Security policies, audit information, and vulnerability disclosure |
| [CONSTANT_TIME.md](CONSTANT_TIME.md) | Side-channel resistance analysis and constant-time guarantees |
| [ARCHITECTURE.md](ARCHITECTURE.md) | Code organization, module design, and architectural decisions |
| [CONTACT.md](CONTACT.md) | **NEW** Contact information and communication guidelines |

---

## 🛠️ Maintenance & Support

| Document | Description |
|----------|-------------|
| [CHANGELOG.md](CHANGELOG.md) | Version history, release notes, and breaking changes |
| [FAQ.md](FAQ.md) | Frequently asked questions and troubleshooting |

---

## 📋 Quick Reference

### Build Commands

```bash
# Standard build
make

# Build for specific architecture
make ARCH=arm64

# Run all tests
make test-all

# Generate coverage report
make test-coverage

# Install library
sudo make install
```

### Key Directories

```
tweetnacl-modular/
├── src/                    # Implementation files
├── includes/               # Public headers
│   └── internal/           # Internal/private headers
├── tests/                  # Test suites
│   ├── cunit/              # CUnit tests
│   ├── gtest/              # Google Test (C++)
│   └── vectors/            # Test vectors
├── doc/                    # Documentation (you are here)
├── pqc/                    # Post-Quantum Cryptography
├── arch/                   # Architecture-specific code
└── app/                    # Example applications
```

### Supported Architectures

- **x86_64**: 64-bit Intel/AMD processors
- **x86_32**: 32-bit x86 processors
- **ARM64**: ARM AArch64 (Cortex-A series)
- **ARM32**: ARM 32-bit (Cortex-M, Cortex-A)
- **RISC-V 64**: RISC-V 64-bit processors
- **RISC-V 32**: RISC-V 32-bit processors

---

## 🔗 External Resources

- **Original TweetNaCl**: https://tweetnacl.cr.yp.to
- **NaCl Reference**: https://nacl.cr.yp.to
- **SUPERCOP**: https://bench.cr.yp.to
- **NIST PQC**: https://csrc.nist.gov/projects/post-quantum-cryptography

---

## 📧 Support & Contact

**Maintainer**: Anupam Datta  
**Email**: [adbd04@gmail.com](mailto:adbd04@gmail.com)

For security issues, see [CONTACT.md](CONTACT.md) and [SECURITY.md](SECURITY.md).

---

## 📄 License

- **Original TweetNaCl**: Public Domain
- **This Fork**: MIT License (see [LICENSE](../LICENSE))

---

**Documentation Last Updated**: 2024  
**Version**: 2.0.0
