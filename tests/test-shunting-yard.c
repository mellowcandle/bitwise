// Copyright 2012 - 2014 Brian Marshall. All rights reserved.
//
// Use of this source code is governed by the BSD 2-Clause License that can be
// found in the LICENSE file.
//
// Based on CUnit example code: <http://cunit.sourceforge.net/example.html>.

#include "../inc/shunting-yard.h"

#include <CUnit/Basic.h>
#include <stdlib.h>

#define ASSERT_RESULT(expression, expected) \
        ASSERT_STATUS(expression, STATUS_OK); \
        CU_ASSERT_EQUAL(result, expected)

#define ASSERT_STATUS(expression, expected) \
        CU_ASSERT(shunting_yard(expression, &result) == expected)

static uint64_t result = 0;
uint64_t g_val = 0x512;

static void test_addition()
{
	ASSERT_RESULT("2+2", 4);
	ASSERT_RESULT("0x2+0x2", 0x4);
	ASSERT_RESULT("0x2 + 0x2", 0x4);
	ASSERT_RESULT("0x2 + b101", 7);
	ASSERT_RESULT("0x2 + 0b101", 7);
	ASSERT_RESULT("0b101 + 0x2", 7);
	ASSERT_RESULT("2  +  2", 4);
	ASSERT_RESULT("3 + (5 + 1 + (2 + 2))", 13);
	ASSERT_RESULT("1+2+4+8+16 + 11", 42);

}

static void test_shifts()
{
	ASSERT_RESULT("1<<0", 1);
	ASSERT_RESULT("1 << 2", 4);
	ASSERT_RESULT("2 >> 1", 1);
	ASSERT_RESULT("0x2 >> 1", 1);
}

static void test_subtraction()
{
	ASSERT_RESULT("8-4", 4);
	ASSERT_RESULT("15-10", 5);
	ASSERT_RESULT("27 - (10 - 11)", 28);
}

static void test_multiplication()
{
	ASSERT_RESULT("13 * 2", 26);
	ASSERT_RESULT("2(3)", 6);
	ASSERT_RESULT("(2)(3)", 6);
	ASSERT_RESULT("0x2(0x3)", 6);
}

static void test_division()
{
	ASSERT_RESULT("2987898/34743", 86);
}

static void test_modulus()
{
	ASSERT_RESULT("10 % 6", 4);
	ASSERT_RESULT("2+3 % 3", 2);
	ASSERT_RESULT("6*5%21", 9);
	ASSERT_RESULT("10%11", 10);
	ASSERT_RESULT("5 %5", 0);
}


static void test_functions()
{
	ASSERT_RESULT("BIT(3)", 8);

}

static void test_constants()
{
	ASSERT_RESULT("$+0x30", 0x542);
}

static void test_precedence()
{
	ASSERT_RESULT("6/3*5", 10);
	ASSERT_RESULT("6+3*2", 12);
	ASSERT_RESULT("2+6/2*5+10/3-2/6", 20);
}

static void test_errors()
{
	ASSERT_STATUS("2+*2", ERROR_SYNTAX);
	ASSERT_STATUS("2**2", ERROR_SYNTAX);
	ASSERT_STATUS("*1", ERROR_SYNTAX);
	ASSERT_STATUS("2*.", ERROR_SYNTAX);
	ASSERT_STATUS("2*2 3", ERROR_SYNTAX);
	ASSERT_STATUS("(2+2", ERROR_OPEN_PARENTHESIS);
	ASSERT_STATUS("(2+2)+(2+2", ERROR_OPEN_PARENTHESIS);
	ASSERT_STATUS("(2+2))", ERROR_CLOSE_PARENTHESIS);
	ASSERT_STATUS("", ERROR_NO_INPUT);
	ASSERT_STATUS("       ", ERROR_NO_INPUT);
	ASSERT_STATUS("foo(2)", ERROR_UNDEFINED_FUNCTION);
//	ASSERT_STATUS("bit(foo)", ERROR_FUNCTION_ARGUMENTS);
//    ASSERT_STATUS("foo", ERROR_UNDEFINED_CONSTANT);

}

int main()
{
	if (CU_initialize_registry() != CUE_SUCCESS)
		return CU_get_error();

	unsigned int tests_failed = 0;
	CU_pSuite suite = CU_add_suite("Shunting Yard", NULL, NULL);
	if (!suite)
		goto exit;

	if (!CU_add_test(suite, "addition", test_addition) ||
	    !CU_add_test(suite, "shifts", test_shifts) ||
	    !CU_add_test(suite, "subtraction", test_subtraction) ||
	    !CU_add_test(suite, "multiplication", test_multiplication) ||
	    !CU_add_test(suite, "division", test_division) ||
	    !CU_add_test(suite, "modulus", test_modulus) ||
	    !CU_add_test(suite, "functions", test_functions) ||
	    !CU_add_test(suite, "constants", test_constants) ||
	    !CU_add_test(suite, "operator precedence", test_precedence) ||
	    !CU_add_test(suite, "error handling", test_errors))
		goto exit;

	CU_basic_set_mode(CU_BRM_NORMAL);
	CU_basic_run_tests();
	tests_failed = CU_get_number_of_tests_failed();
exit:
	CU_cleanup_registry();
	return tests_failed ? EXIT_FAILURE : CU_get_error();
}
