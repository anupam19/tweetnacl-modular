/**
 * @file error.c
 * @brief Error code utility implementations
 * @details Provides human-readable error strings and error handling utilities.
 * 
 * @copyright MIT License
 * @author Anupam Datta
 * @version 1.0.0
 * @date 2024
 */

#include "core/error.h"

/**
 * @brief Convert error code to human-readable string
 */
const char* nacl_error_string(nacl_error_t err) {
    switch (err) {
        case NACL_SUCCESS:
            return "Success";
        case NACL_ERROR_INVALID_PARAM:
            return "Invalid parameter";
        case NACL_ERROR_BUFFER_TOO_SMALL:
            return "Buffer too small";
        case NACL_ERROR_VERIFICATION_FAILED:
            return "Verification failed";
        case NACL_ERROR_SELF_TEST_FAILED:
            return "Self-test failed";
        case NACL_ERROR_RNG_FAILURE:
            return "RNG failure";
        case NACL_ERROR_INTEGRITY_FAILURE:
            return "Integrity failure";
        case NACL_ERROR_ALGORITHM_NOT_SUPPORTED:
            return "Algorithm not supported";
        case NACL_ERROR_KEYGEN_FAILED:
            return "Key generation failed";
        case NACL_ERROR_MEMORY_FAILURE:
            return "Memory allocation failed";
        case NACL_ERROR_INTERNAL:
            return "Internal error";
        case NACL_ERROR_FEATURE_NOT_AVAILABLE:
            return "Feature not available";
        case NACL_ERROR_OVERFLOW:
            return "Integer overflow detected";
        case NACL_ERROR_CATASTROPHIC_FAILURE:
            return "Catastrophic failure";
        default:
            return "Unknown error";
    }
}
