#include <unity.h>
#include <Arduino.h>

void setUp(void) {
    // setup code
}

void tearDown(void) {
    // teardown code
}

void test_function_calculator_addition(void) {
    TEST_ASSERT_EQUAL(32, 25 + 7);
}

void test_function_calculator_multiplication(void) {
    TEST_ASSERT_EQUAL(50, 25 * 2);
}

void test_function_calculator_division(void) {
    TEST_ASSERT_EQUAL(32, 96 / 3);
}

void RUN_UNITY_TESTS() {
    UNITY_BEGIN();
    RUN_TEST(test_function_calculator_addition);
    RUN_TEST(test_function_calculator_multiplication);
    RUN_TEST(test_function_calculator_division);
    UNITY_END();
}

int main(int argc, char **argv) {
    RUN_UNITY_TESTS();
    return 0;
} 