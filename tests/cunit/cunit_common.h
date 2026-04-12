/* Stub: init functions declared in test_runner.c but defined in separate files */
/* Each test file defines its own init_<name>_tests() function. */
/* This file provides stubs for any suites not yet fully implemented. */

#include <CUnit/CUnit.h>

/* Provided by test_tweetnacl_box.c */
extern int init_box_tests(void);

/* Provided by test_tweetnacl_sign.c */
extern int init_sign_tests(void);

/* Provided by test_tweetnacl_secretbox.c */
extern int init_secretbox_tests(void);

/* Provided by test_tweetnacl_pqc.c */
extern int init_pqc_tests(void);
