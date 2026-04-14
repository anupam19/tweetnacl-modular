/**
 * @file fips_selftest.h
 * @brief FIPS 140-3 Self-Test Interface
 * @details Provides power-up and conditional self-tests for FIPS 140-3 compliance
 *
 * @copyright MIT License
 * @author Anupam Datta
 * @version 1.0.0
 * @date 2024
 */

#ifndef CORE_FIPS_SELFTEST_H
#define CORE_FIPS_SELFTEST_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Run all FIPS 140-3 power-up self-tests
 * @return 0 on success, error code on failure
 */
int fips_power_up_self_tests(void);

/**
 * @brief Run conditional self-test for key generation
 * @return 0 on success, error code on failure
 */
int fips_conditional_selftest_keygen(void);

/**
 * @brief Get power-up test status
 * @return 1 if passed, 0 otherwise
 */
int fips_get_power_up_status(void);

/**
 * @brief Get last test result
 * @return Error code or success value
 */
int fips_get_last_test_result(void);

/**
 * @brief Get key generation counter
 * @return Number of key generations
 */
uint64_t fips_get_keygen_counter(void);

/**
 * @brief Enable/disable conditional tests
 * @param enable 1 to enable, 0 to disable
 */
void fips_enable_conditional_tests(int enable);

/**
 * @brief Check if self-tests have passed
 * @return 1 if passed, 0 otherwise
 */
int fips_are_self_tests_passed(void);

#ifdef __cplusplus
}
#endif

#endif /* CORE_FIPS_SELFTEST_H */
