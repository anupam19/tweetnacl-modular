# =============================================================================
# TweetNaCl-Modular — Test Suite CMake Configuration
# =============================================================================

# Determine which library target to link against
if(BUILD_STATIC_LIBS)
    set(TWEETNACL_LIB tweetnacl_static)
elseif(BUILD_SHARED_LIBS)
    set(TWEETNACL_LIB tweetnacl)
else()
    message(FATAL_ERROR "At least one of BUILD_SHARED_LIBS or BUILD_STATIC_LIBS must be ON")
endif()

# ─── Original Test Suite (backward compatibility) ───────────────────────────
# Note: Do NOT include pqc/pqc.c here — it's already part of tweetnacl_static
add_executable(test_original
    test_all.c
    test_secure_mem.c
)
target_include_directories(test_original PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)
target_link_libraries(test_original PRIVATE ${TWEETNACL_LIB})
# Only apply GCC/Clang specific compiler flags to non-MSVC compilers
if(CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID STREQUAL "Clang")
    target_compile_options(test_original PRIVATE
        -Wextra -Wformat-security -Wmissing-declarations -Wmissing-prototypes
        -Wnested-externs -Wpointer-arith -Wshadow -Wstrict-prototypes
    )
elseif(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(test_original PRIVATE /W3)
endif()

add_test(NAME OriginalTests COMMAND test_original)
