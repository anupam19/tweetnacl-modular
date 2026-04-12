# =============================================================================
# TweetNaCl-Modular — Example Application CMake Configuration
# =============================================================================

if(BUILD_SHARED_LIBS)
    set(TWEETNACL_LIB tweetnacl)
elseif(BUILD_STATIC_LIBS)
    set(TWEETNACL_LIB tweetnacl_static)
else()
    message(FATAL_ERROR "At least one of BUILD_SHARED_LIBS or BUILD_STATIC_LIBS must be ON")
endif()

add_executable(tweetnacl_app main.c)
target_include_directories(tweetnacl_app PRIVATE
    ${CMAKE_SOURCE_DIR}/includes
)
target_link_libraries(tweetnacl_app PRIVATE ${TWEETNACL_LIB})
target_compile_options(tweetnacl_app PRIVATE
    -Wextra -Wformat-security -Wmissing-declarations -Wmissing-prototypes
    -Wnested-externs -Wpointer-arith -Wshadow -Wstrict-prototypes
    -g -O0
)
