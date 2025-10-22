#ifndef FORT_TEST_H
#define FORT_TEST_H

#include <inttypes.h>  // for PRId32, PRId64
#include <stdbool.h>   // for true, false
#include <stdio.h>     // for fprintf, stderr, NULL
#include <string.h>    // for strlen, strncmp

#include "common.h"    // for FORT_UNUSED

typedef enum {
    TEST_RESULT_OK = 0,
    TEST_RESULT_FAIL = 1,
    TEST_RESULT_ERR = 2,
} test_result_t;

#define TEST_RESULT_ test_result_t

#define TEST_SUITENAME_ test_suite_name
#define TEST_FINAL_RESULT_ test_final_result
#define TEST_FILTER_TEST_PREFIX_ test_filter_test_prefix

#define TEST_INIT_(suite_name, filter_test_prefix)                                                 \
    const char* TEST_SUITENAME_ = (suite_name);                                                    \
    TEST_RESULT_ TEST_FINAL_RESULT_ = TEST_RESULT_OK;                                              \
    const char* TEST_FILTER_TEST_PREFIX_ = (filter_test_prefix)

#define TEST_FINAL_RESULT_UPDATE_(test_result)                                                     \
    do {                                                                                           \
        if ((test_result) != TEST_RESULT_OK) {                                                     \
            TEST_FINAL_RESULT_ = (test_result);                                                    \
        }                                                                                          \
    } while (0)

#define TEST_EXIT_(final_result)                                                                   \
    do {                                                                                           \
        switch (final_result) {                                                                    \
        case TEST_RESULT_OK:                                                                       \
            return 0;                                                                              \
        case TEST_RESULT_FAIL:                                                                     \
            return 1;                                                                              \
        case TEST_RESULT_ERR:                                                                      \
            return 2;                                                                              \
        }                                                                                          \
    } while (0)

#define TEST_INIT(suite_name, argc, argv) TEST_INIT_(suite_name, (argc) > 1 ? (argv)[1] : NULL)

#define TEST_EXIT() TEST_EXIT_(TEST_FINAL_RESULT_)

#define TEST_RUN(test_function)                                                                    \
    do {                                                                                           \
        if (TEST_FILTER_TEST_PREFIX_ != NULL &&                                                    \
            (strncmp(TEST_FILTER_TEST_PREFIX_,                                                     \
                     #test_function,                                                               \
                     strlen(TEST_FILTER_TEST_PREFIX_)) != 0)) {                                    \
            break;                                                                                 \
        }                                                                                          \
        FORT_UNUSED(fprintf(stderr, "RUN: %s.%s\n", TEST_SUITENAME_, #test_function));             \
        const TEST_RESULT_ test_result = (test_function)();                                        \
        TEST_FINAL_RESULT_UPDATE_(test_result);                                                    \
        const char* result_str = NULL;                                                             \
        switch (test_result) {                                                                     \
        case TEST_RESULT_OK:                                                                       \
            result_str = "OK";                                                                     \
            break;                                                                                 \
        case TEST_RESULT_FAIL:                                                                     \
            result_str = "FAIL";                                                                   \
            break;                                                                                 \
        case TEST_RESULT_ERR:                                                                      \
            result_str = "ERROR";                                                                  \
            break;                                                                                 \
        default:                                                                                   \
            result_str = "UNKNOWN";                                                                \
        }                                                                                          \
        FORT_UNUSED(                                                                               \
            fprintf(stderr, "END: %s.%s %s\n\n", TEST_SUITENAME_, #test_function, result_str));    \
    } while (0)

#define TEST(test_name, test_body)                                                                 \
    static TEST_RESULT_ test_name(void) {                                                          \
        test_body;                                                                                 \
        TEST_OK();                                                                                 \
    }

#define TEST_LOG_(fmt, ...)                                                                        \
    do {                                                                                           \
        const int res =                                                                            \
            fprintf(stderr, "%s:%d\n\t" fmt "\n", __FILE__, __LINE__ __VA_OPT__(, ) __VA_ARGS__);  \
        FORT_UNUSED(res);                                                                          \
    } while (0)

#define TEST_ASSERT_EQ_BOOL_(val, exp)                                                             \
    do {                                                                                           \
        if ((val) != (exp)) {                                                                      \
            TEST_LOG_("assertion failed: %s == %s\n"                                               \
                      "\tactual:   %s\n"                                                           \
                      "\texpected: %s",                                                            \
                      #val,                                                                        \
                      #exp,                                                                        \
                      (val) ? "true" : "false",                                                    \
                      (exp) ? "true" : "false");                                                   \
            return TEST_RESULT_FAIL;                                                               \
        }                                                                                          \
    } while (0)

#define TEST_ASSERT_TRUE(expr) TEST_ASSERT_EQ_BOOL_(expr, true)
#define TEST_ASSERT_FALSE(expr) TEST_ASSERT_EQ_BOOL_(expr, false)

#define TEST_ASSERT_NONNULL(expr)                                                                  \
    do {                                                                                           \
        if ((expr) == NULL) {                                                                      \
            TEST_LOG_("assertion failed: %s != NULL", #expr);                                      \
            return TEST_RESULT_FAIL;                                                               \
        }                                                                                          \
    } while (0)

#define TEST_ASSERT_OP_(val, exp, cond, op, fmt)                                                   \
    do {                                                                                           \
        if (!(cond)) {                                                                             \
            TEST_LOG_("assertion failed: %s %s %s\n"                                               \
                      "\tactual:   " fmt "\n"                                                      \
                      "\texpected: " fmt,                                                          \
                      #val,                                                                        \
                      op,                                                                          \
                      #exp,                                                                        \
                      val,                                                                         \
                      exp);                                                                        \
            return TEST_RESULT_FAIL;                                                               \
        }                                                                                          \
    } while (0)

#define TEST_ASSERT_EQ_(val, exp, fmt) TEST_ASSERT_OP_(val, exp, (val) == (exp), "==", fmt)
#define TEST_ASSERT_EQ_CHAR(val, exp) TEST_ASSERT_EQ_(val, exp, "%c")
#define TEST_ASSERT_EQ_INT32(val, exp) TEST_ASSERT_EQ_(val, exp, "%" PRId32)
#define TEST_ASSERT_EQ_INT64(val, exp) TEST_ASSERT_EQ_(val, exp, "%" PRId64)
#define TEST_ASSERT_EQ_SIZE(val, exp) TEST_ASSERT_EQ_(val, exp, "%zu")

#define TEST_ASSERT_NE_(val, exp, fmt) TEST_ASSERT_OP_(val, exp, (val) != (exp), "!=", fmt)
#define TEST_ASSERT_NE_CHAR(val, exp) TEST_ASSERT_NE_(val, exp, "%c")

#define TEST_ASSERT_GE_(val, exp, fmt) TEST_ASSERT_OP_(val, exp, (val) >= (exp), ">=", fmt)
#define TEST_ASSERT_GE_INT32(val, exp) TEST_ASSERT_GE_(val, exp, "%" PRId32)
#define TEST_ASSERT_GE_INT64(val, exp) TEST_ASSERT_GE_(val, exp, "%" PRId64)
#define TEST_ASSERT_GE_SIZE(val, exp) TEST_ASSERT_GE_(val, exp, "%zu")

#define TEST_ASSERT_LE_(val, exp, fmt) TEST_ASSERT_OP_(val, exp, (val) <= (exp), "<=", fmt)
#define TEST_ASSERT_LE_INT32(val, exp) TEST_ASSERT_LE_(val, exp, "%" PRId32)
#define TEST_ASSERT_LE_INT64(val, exp) TEST_ASSERT_LE_(val, exp, "%" PRId64)
#define TEST_ASSERT_LE_SIZE(val, exp) TEST_ASSERT_LE_(val, exp, "%zu")

#define TEST_OK() return TEST_RESULT_OK

#define TEST_FAIL() return TEST_RESULT_FAIL

#define TEST_ERROR_NONZERO(expr)                                                                   \
    do {                                                                                           \
        if ((expr) != 0) {                                                                         \
            TEST_LOG_("test error: %s == 0\n\tactual: %" PRId32 "\n", #expr, expr);                \
            return TEST_RESULT_ERR;                                                                \
        }                                                                                          \
    } while (0)

#endif // FORT_TEST_H
