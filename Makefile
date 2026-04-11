# File Name : Makefile
# Location : [@top root directory] tweetnacl-modular/
# Author : Anupam Datta
# Mail : adbd04@gmail.com
# Description : compiles tweetnacl library and produces shared library libtweetnacl.so inside [.libs] directory (tweetnacl-modular/.libs/)
# [-cont.] compile your project with '-ltweetnacl'
# 
# Refactored with:
# - Multi-architecture support (x86, ARM, RISC-V)
# - Post-Quantum Cryptography (PQC) support
# - Comprehensive unit tests (CUnit + GTest)

CC:=$(CC)
ARCH ?= native

# Architecture-specific flags
ifeq ($(ARCH),arm64)
    ARCH_CFLAGS := -march=armv8-a
    ARCH_NAME := ARM64
else ifeq ($(ARCH),arm32)
    ARCH_CFLAGS := -march=armv7-a
    ARCH_NAME := ARM32
else ifeq ($(ARCH),riscv64)
    ARCH_CFLAGS := -march=rv64gc
    ARCH_NAME := RISCV64
else ifeq ($(ARCH),riscv32)
    ARCH_CFLAGS := -march=rv32gc
    ARCH_NAME := RISCV32
else ifeq ($(ARCH),x86_64)
    ARCH_CFLAGS := -march=x86-64
    ARCH_NAME := x86_64
else ifeq ($(ARCH),x86_32)
    ARCH_CFLAGS := -march=i686
    ARCH_NAME := x86_32
else
    ARCH_CFLAGS :=
    ARCH_NAME := native
endif

# Base compiler flags
BASE_CFLAGS:=-Wextra -Wformat-security -Winline -Wmissing-declarations -Wmissing-prototypes \
             -Wnested-externs -Wpointer-arith -Wshadow -Wstrict-prototypes -Wswitch-default \
             -Wswitch-enum -Wunused -Wunused-result -Wlogical-op -g -O2 -DWITH_POSIX -fPIC

# Combined flags
CFLAGS:=$(BASE_CFLAGS) $(ARCH_CFLAGS)

LDFLAGS:=-shared

.PHONY: all .libs app tests clean install uninstall test test-all test-cunit test-gtest test-valgrind test-coverage help

PREFIX:=/usr/local
SRC:=src
INCLUDE:=includes
APP:=app
TESTS:=tests
ARCH_DIR:=arch
PQC_DIR:=pqc
OBJ:=tweetnacl.o randombytes.o secure_mem.o secure_utils.o
PQC_OBJ:=pqc/pqc.o
SHARED:=libtweetnacl.so
LIB:=.libs

all: $(LIB) $(OBJ) $(SHARED) $(APP) $(TESTS)

$(LIB):
	mkdir -p $(LIB)

randombytes.o:	$(SRC)/randombytes.c
	$(CC) -c $(CFLAGS) -I$(INCLUDE) -I$(ARCH_DIR) $<

tweetnacl.o:	$(SRC)/tweetnacl.c
	$(CC) -c $(CFLAGS) -I$(INCLUDE) -I$(ARCH_DIR) $<

secure_mem.o:	$(SRC)/secure_mem.c $(INCLUDE)/secure_mem.h
	$(CC) -c $(CFLAGS) -I$(INCLUDE) -I$(ARCH_DIR) $< -o $@

secure_utils.o:	$(SRC)/secure_utils.c $(INCLUDE)/secure_utils.h
	$(CC) -c $(CFLAGS) -I$(INCLUDE) -I$(ARCH_DIR) $< -o $@

pqc/pqc.o: $(PQC_DIR)/pqc.c $(PQC_DIR)/pqc.h
	$(CC) -c $(CFLAGS) -I$(INCLUDE) -I$(ARCH_DIR) -I$(PQC_DIR) $< -o $@

$(SHARED): $(OBJ)
	mkdir -p $(LIB)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(LIB)/$(SHARED) $(OBJ)
	@rm -f $(PROGRAMS) $(OBJ) *~

$(APP):
	$(MAKE) -C $(APP)

# Build tests
$(TESTS): $(LIB) $(OBJ) $(PQC_OBJ)
	$(MAKE) -C $(TESTS) CFLAGS="$(CFLAGS)" LDFLAGS="-L./.libs -ltweetnacl"

# Run tests
test: $(TESTS)
	./$(TESTS)/test_all

# Run CUnit tests
test-cunit:
	@echo "Building and running CUnit tests..."
	$(MAKE) -C tests/cunit
	./tests/cunit/test_runner

# Run GTest tests  
test-gtest:
	@echo "Building and running GTest tests..."
	$(MAKE) -C tests/gtest
	./tests/gtest/test_tweetnacl

# Run all tests (original + CUnit + GTest)
test-all: test test-cunit test-gtest

# Run tests with Valgrind memory checking
test-valgrind: $(TESTS)
	@echo "Running Valgrind memory check on original tests..."
	valgrind --tool=memcheck --leak-check=full --error-exitcode=1 ./$(TESTS)/test_all
	@echo "Running Valgrind on CUnit tests..."
	$(MAKE) -C tests/cunit
	valgrind --tool=memcheck --leak-check=full --error-exitcode=1 ./tests/cunit/test_runner

# Generate code coverage report
test-coverage: clean
	@echo "Building with coverage instrumentation..."
	CFLAGS="$(CFLAGS) --coverage -fprofile-arcs -ftest-coverage" $(MAKE) all
	$(MAKE) test
	@echo "Generating coverage report..."
	gcov src/*.c
	lcov --capture --directory . --output-file coverage.info 2>/dev/null || true
	genhtml coverage.info --output-directory coverage_html 2>/dev/null || echo "Install lcov for HTML coverage reports"
	@echo "Coverage report generated. Open coverage_html/index.html in a browser."

# Help target
help:
	@echo "TweetNaCl Modular Build System"
	@echo "=============================="
	@echo ""
	@echo "Targets:"
	@echo "  all        - Build library, app, and tests (default)"
	@echo "  test       - Build and run original unit tests"
	@echo "  test-cunit - Build and run CUnit test suite"
	@echo "  test-gtest - Build and run GTest test suite"
	@echo "  test-all   - Run all tests (original + CUnit + GTest)"
	@echo "  test-valgrind - Run tests with Valgrind memory checking"
	@echo "  test-coverage - Generate code coverage report"
	@echo "  clean      - Remove build artifacts"
	@echo "  install    - Install to system (PREFIX=$(PREFIX))"
	@echo "  uninstall  - Remove from system"
	@echo "  help       - Show this help message"
	@echo ""
	@echo "Architecture Options (ARCH=):"
	@echo "  native   - Native architecture (default)"
	@echo "  x86_64   - x86 64-bit"
	@echo "  x86_32   - x86 32-bit"
	@echo "  arm64    - ARM 64-bit (AArch64)"
	@echo "  arm32    - ARM 32-bit"
	@echo "  riscv64  - RISC-V 64-bit"
	@echo "  riscv32  - RISC-V 32-bit"
	@echo ""
	@echo "Examples:"
	@echo "  make                    # Build with native settings"
	@echo "  make ARCH=arm64         # Build for ARM64"
	@echo "  make ARCH=riscv64 test  # Build for RISC-V64 and run tests"
	@echo "  make test-cunit         # Run CUnit test suite"
	@echo "  make test-coverage      # Generate coverage report"
	@echo ""

clean:
	@rm -rf $(LIB)
	@rm -f $(PROGRAMS) $(OBJ) $(PQC_OBJ) *~ *.so
	$(MAKE) -C $(APP) clean
	$(MAKE) -C $(TESTS) clean

install:
	cp $(LIB)/$(SHARED) $(DESTDIR)$(PREFIX)/lib/$(SHARED)
	$(MKDIR) $(DESTDIR)$(PREFIX)/include/tweetnacl
	cp -a includes/* $(DESTDIR)$(PREFIX)/include/tweetnacl/
	cp -a $(ARCH_DIR)/*.h $(DESTDIR)$(PREFIX)/include/tweetnacl/
	cp -a $(PQC_DIR)/*.h $(DESTDIR)$(PREFIX)/include/tweetnacl/

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/lib/$(SHARED)
	rm -rf $(DESTDIR)$(PREFIX)/include/tweetnacl/
