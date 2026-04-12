# x86_64 (AMD64) native and cross-compilation toolchain
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# Use native compiler by default, override for cross-compilation
if(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "x86_64")
    set(CMAKE_C_COMPILER cc)
    set(CMAKE_CXX_COMPILER c++)
else()
    set(CMAKE_C_COMPILER x86_64-linux-gnu-gcc)
    set(CMAKE_CXX_COMPILER x86_64-linux-gnu-g++)
endif()

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Architecture-specific optimizations
set(CMAKE_C_FLAGS_INIT "-march=x86-64 -mtune=generic")
