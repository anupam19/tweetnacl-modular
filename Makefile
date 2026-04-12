# =============================================================================
# TweetNaCl-Modular — Backward Compatibility Makefile (CMake Wrapper)
# Maintainer: Anupam Datta <adbd04@gmail.com>
#
# This thin wrapper calls CMake for backward compatibility.
# For full control, use CMake directly:
#   cmake -B build && cmake --build build
# =============================================================================

BUILD_DIR ?= build

.PHONY: all test clean install uninstall help

all:
	@echo "Using CMake build system..."
	@cmake -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Release
	@cmake --build $(BUILD_DIR) --parallel

test: all
	cd $(BUILD_DIR) && ctest --output-on-failure

clean:
	rm -rf $(BUILD_DIR)

install: all
	cmake --install $(BUILD_DIR)

uninstall:
	@cmake --build $(BUILD_DIR) --target uninstall 2>/dev/null || \
		echo "Uninstall target not available. Use: cmake --build $(BUILD_DIR) --target uninstall"

help:
	@echo "TweetNaCl-Modular — CMake Build System"
	@echo "======================================"
	@echo ""
	@echo "Quick start:"
	@echo "  cmake -B build && cmake --build build       # Build everything"
	@echo "  cd build && ctest --output-on-failure       # Run tests"
	@echo "  cmake --install build                        # Install"
	@echo ""
	@echo "Options:"
	@echo "  -DENABLE_DRNG=ON      Hardware RNG (RDRAND/RDSEED/ARM RNDR)"
	@echo "  -DENABLE_PQC=ON       Post-Quantum Cryptography"
	@echo "  -DBUILD_TESTS=ON      Build test suite"
	@echo "  -DBUILD_APP=ON        Build example app"
	@echo "  -DENABLE_COVERAGE=ON  Code coverage analysis"
	@echo "  -DENABLE_SANITIZERS=ON ASan + UBSan"
	@echo "  -DENABLE_LTO=ON       Link Time Optimization"
	@echo ""
	@echo "Cross-compilation:"
	@echo "  cmake -B build -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/arm64.cmake"
	@echo "  cmake -B build -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/riscv64.cmake"
