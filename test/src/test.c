#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include "test.h"

int assert(void *actual, void *expected) {
    if (actual != expected) {
        printf("==== ASSERTION FAILED: ====\n");
        printf("%p (actual) != %p (expected)\n", actual, expected);
        return 1;
    }
    return 0;
}

int assert_int(int actual, int expected) {
    if (actual != expected) {
        printf("==== ASSERTION FAILED: ====\n");
        printf("%d (actual) != %d (expected)\n", actual, expected);
        return 1;
    }
    return 0;
}

void gather_tests(Test_Function *tests, ...) {
    va_list test_list;
    va_start(test_list, tests);
    Test_Function *test = tests;
    int failed = 0;
    int succeeded = 0;
    while (test != NULL) {
        int result = (*test)();
        if (result) {
            printf("Test failed! index: %d\n", failed);
            failed += 1;
        } else {
            succeeded += 1;
        }
        test = va_arg(test_list, Test_Function *);
    }
    printf("==== TEST SUMMARY: ====\n");
    printf("Tests succeeded: %d\n", succeeded);
    printf("Tests failed: %d\n", failed);
}
