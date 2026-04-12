# RISC-V 32-bit cross-compilation toolchain
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR riscv32)

set(CMAKE_C_COMPILER riscv32-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER riscv32-linux-gnu-g++)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Architecture-specific optimizations
set(CMAKE_C_FLAGS_INIT "-march=rv32gc -mabi=ilp32d -mtune=sifive-7-series")
