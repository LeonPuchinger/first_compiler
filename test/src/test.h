#ifndef TEST_H
#define TEST_H

#define FALSE 0
#define TRUE 1

//all test functions should have this type
//return 0 if the test succeeded, != 0 if failed
//
//example:
//
//int test_a() {
//  int err = assert(..., ...);
//  if (err) return err;
//  return 0;
//}
typedef int Test_Function();

//assert pointer equality
int assert(void *actual, void *expected);

//assert pointer inequality
int assert_not(void *actual, void *not_expected);

//assert int equality
int assert_int(int actual, int expected);

//assert int inequality
int assert_int_not(int actual, int not_expected);

//execute all given test functions and display a summary
//IMPORTANT: make sure to terminate tests varargs list with NULL at the end!
//
//example:
//
//gather_tests(test_a, test_b, NULL);
void gather_tests(Test_Function *tests, ...);

#endif
