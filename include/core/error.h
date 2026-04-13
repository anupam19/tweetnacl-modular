/**
 * @file error.h
 * @brief Error codes for TweetNaCl-Modular cryptographic library
 * @details Standardized error codes for consistent error handling across all modules.
 *          Follows FIPS 140-3 error handling requirements.
 * 
 * @copyright MIT License
 * @author Anupam Datta
 * @version 1.0.0
 * @date 2024
 * 
 * @defgroup error_handling Error Handling
 * @brief Error code definitions and handling utilities
 * @{
 */

#ifndef NACL_ERROR_H
#define NACL_ERROR_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Standard error codes for NaCl operations
 * 
 * All functions return these standardized error codes to ensure
 * consistent error handling across the library.
 */
typedef enum {
    /** Success - operation completed successfully */
    NACL_SUCCESS = 0,
    
    /** Invalid parameter - NULL pointer or out-of-range value */
    NACL_ERROR_INVALID_PARAM = -1,
    
    /** Buffer too small - output buffer cannot hold result */
    NACL_ERROR_BUFFER_TOO_SMALL = -2,
    
    /** Verification failed - cryptographic verification did not pass */
    NACL_ERROR_VERIFICATION_FAILED = -3,
    
    /** Self-test failed - FIPS power-up or conditional self-test failed */
    NACL_ERROR_SELF_TEST_FAILED = -4,
    
    /** RNG failure - random number generator failed */
    NACL_ERROR_RNG_FAILURE = -5,
    
    /** Integrity failure - data integrity check failed */
    NACL_ERROR_INTEGRITY_FAILURE = -6,
    
    /** Algorithm not supported - requested algorithm is not available */
    NACL_ERROR_ALGORITHM_NOT_SUPPORTED = -7,
    
    /** Key generation failed - unable to generate valid key pair */
    NACL_ERROR_KEYGEN_FAILED = -8,
    
    /** Memory allocation failed - insufficient memory */
    NACL_ERROR_MEMORY_FAILURE = -9,
    
    /** Internal error - unexpected internal state */
    NACL_ERROR_INTERNAL = -10,
    
    /** Feature not available - CPU feature required but not present */
    NACL_ERROR_FEATURE_NOT_AVAILABLE = -11,
    
    /** Overflow detected - integer overflow would occur */
    NACL_ERROR_OVERFLOW = -12,
    
    /** Catastrophic failure - RNG catastrophic failure detected */
    NACL_ERROR_CATASTROPHIC_FAILURE = -13
} nacl_error_t;

/**
 * @brief Check if an error code indicates success
 * @param err Error code to check
 * @return 1 if success, 0 if error
 */
static inline int nacl_is_success(nacl_error_t err) {
    return (err == NACL_SUCCESS) ? 1 : 0;
}

/**
 * @brief Check if an error code indicates failure
 * @param err Error code to check
 * @return 1 if error, 0 if success
 */
static inline int nacl_is_error(nacl_error_t err) {
    return (err != NACL_SUCCESS) ? 1 : 0;
}

/**
 * @brief Convert error code to human-readable string
 * @param err Error code to convert
 * @return Pointer to static string describing the error
 * 
 * @note Returns "Unknown error" for unrecognized error codes
 */
const char* nacl_error_string(nacl_error_t err);

#ifdef __cplusplus
}
#endif

#endif /* NACL_ERROR_H */

/** @} */
