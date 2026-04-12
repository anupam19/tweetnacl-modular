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
add_executable(test_original
    test_all.c
    test_secure_mem.c
    ${CMAKE_SOURCE_DIR}/pqc/pqc.c
)
target_include_directories(test_original PRIVATE
    ${CMAKE_SOURCE_DIR}/includes
    ${CMAKE_SOURCE_DIR}/arch
    ${CMAKE_SOURCE_DIR}/pqc
)
target_link_libraries(test_original PRIVATE ${TWEETNACL_LIB})
target_compile_options(test_original PRIVATE
    -Wextra -Wformat-security -Wmissing-declarations -Wmissing-prototypes
    -Wnested-externs -Wpointer-arith -Wshadow -Wstrict-prototypes
)

add_test(NAME OriginalTests COMMAND test_original)
