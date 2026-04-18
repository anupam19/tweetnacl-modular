# Stack Memory Usage Requirements

## Overview

This document specifies the worst-case stack memory requirements for TweetNaCl-Modular functions on typical architectures. These figures are important for embedded systems with limited stack space (e.g., 2–8 KB).

## Methodology

Stack usage is measured as the maximum automatic (non-static) local variable allocation within a single function call, including:
- All local arrays and structs
- Compiler padding and alignment
- Does not include call frame overhead or return address

## Per-Function Worst-Case Stack Usage

### High-Stack Functions (>256 bytes)

| Function | Stack (x86_64) | Stack (ARM64) | Notes |
|----------|----------------|---------------|-------|
| `crypto_scalarmult` | ~1440 B | ~1440 B | Curve25519 scalar multiplication: `gf a,b,c,d,e,f` (6×16×8 B) + `i64 x[80]` (80×8 B) + `u8 z[32]` |
| `core` (Salsa20/HSalsa20) | ~208 B | ~208 B | `u32 w[16], x[16], y[16], t[4]` |
| `crypto_onetimeauth` (Poly1305) | ~340 B | ~340 B | `u32 x[17], r[17], h[17], c[17], g[17]` + scalars |
| `crypto_hashblocks` (SHA-512) | ~320 B | ~320 B | `u64 z[8], b[8], a[8], w[16]` + `u8 x[256]` (block buffer) |
| `crypto_hash` (SHA-512) | ~384 B | ~384 B | `u8 h[64], x[256]` plus locals |

### Moderate-Stack Functions (64–256 bytes)

| Function | Stack | Notes |
|----------|-------|-------|
| `crypto_stream_salsa20_xor` | ~80 B | `u8 z[16], x[64]` |
| `crypto_box`, `crypto_secretbox` | ~100–150 B | Layers of primitives; worst-case includes nested stack frames |
| `crypto_sign` (Ed25519) | ~50–100 B | (Exact measurement pending) |

## Platform-Specific Notes

- **x86_64 SSE2/AVX2**: Stack usage is similar to generic; SIMD registers reduce heap/static usage but do not significantly affect stack.
- **ARM64 NEON**: Equivalent to generic.
- **64-bit general-purpose registers**: `i64`/`u64` use 8 bytes; on 32-bit targets (if compiled), pointers and 64-bit integers are 4 bytes, reducing stack proportionally.

## Recommendations for Embedded Targets

1. **Minimum safe stack size**: 2048 bytes (2 KB) recommended for worst-case call chains.
2. **Isolation**: If running in a threaded or RTOS environment, ensure each thread's stack ≥ 2 KB.
3. **Public API wrappers**: Consider moving internally large arrays to a memory pool for extreme constraint environments (trade-off: constant-time guarantees and performance).
4. **Static analysis**: Use compiler flags like `-fstack-usage` (GCC/Clang) to generate per-function stack reports and verify against target limits.

## References

- CERT C Rule MEM35-C: Allocate sufficient memory for an object
- NIST SP 800-193: Platform firmware semantics (stack sizing considerations)
