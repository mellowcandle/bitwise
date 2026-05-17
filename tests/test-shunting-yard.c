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

static void test_bitwise_and()
{
	ASSERT_RESULT("0xFF & 0x0F", 0x0F);
	ASSERT_RESULT("7 & 3", 3);
	ASSERT_RESULT("0xABCD & 0xFF00", 0xAB00);
}

static void test_bitwise_or()
{
	ASSERT_RESULT("0xF0 | 0x0F", 0xFF);
	ASSERT_RESULT("4 | 2", 6);
	ASSERT_RESULT("0 | 0", 0);
}

static void test_bitwise_xor()
{
	ASSERT_RESULT("0xFF ^ 0x0F", 0xF0);
	ASSERT_RESULT("5 ^ 5", 0);
	ASSERT_RESULT("0xA ^ 0x5", 0xF);
}

static void test_bitwise_not()
{
	extern int g_width;

	/* With 8-bit width, ~0 should be 0xFF */
	g_width = 8;
	ASSERT_RESULT("~0", 0xFF);
	ASSERT_RESULT("~0xFF", 0);
	ASSERT_RESULT("~0x0F", 0xF0);

	/* With 16-bit width */
	g_width = 16;
	ASSERT_RESULT("~0", 0xFFFF);
	ASSERT_RESULT("~0xFF", 0xFF00);

	/* With 32-bit width */
	g_width = 32;
	ASSERT_RESULT("~0", 0xFFFFFFFF);
	ASSERT_RESULT("~1", 0xFFFFFFFE);

	/* Reset */
	g_width = 0;
}

static void test_logical_not()
{
	ASSERT_RESULT("!0", 1);
	ASSERT_RESULT("!1", 0);
	ASSERT_RESULT("!42", 0);
}

static void test_combined_operations()
{
	ASSERT_RESULT("(1 << 4) | (1 << 2)", 20);
	ASSERT_RESULT("0xFF & (0x0F << 4)", 0xF0);
	ASSERT_RESULT("(3 + 2) * (8 - 3)", 25);
	ASSERT_RESULT("BIT(0) | BIT(1) | BIT(2)", 7);
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
	ASSERT_STATUS("10 / 0", ERROR_DIVIDE_BY_ZERO);
	ASSERT_STATUS("10 % 0", ERROR_DIVIDE_BY_ZERO);
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
	    !CU_add_test(suite, "bitwise AND", test_bitwise_and) ||
	    !CU_add_test(suite, "bitwise OR", test_bitwise_or) ||
	    !CU_add_test(suite, "bitwise XOR", test_bitwise_xor) ||
	    !CU_add_test(suite, "bitwise NOT", test_bitwise_not) ||
	    !CU_add_test(suite, "logical NOT", test_logical_not) ||
	    !CU_add_test(suite, "combined operations", test_combined_operations) ||
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
