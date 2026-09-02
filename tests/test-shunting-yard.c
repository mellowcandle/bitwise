// Copyright 2012 - 2014 Brian Marshall. All rights reserved.
//
// Use of this source code is governed by the BSD 2-Clause License that can be
// found in the LICENSE file.
//
// Based on CUnit example code: <http://cunit.sourceforge.net/example.html>.

#include "shunting-yard.h"

#include <CUnit/Basic.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

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

/*
 * Unary plus and minus. The operator table carried only the binary forms,
 * so get_operator() found no match for the unary use and every one of
 * these was rejected as a syntax error.
 *
 * Results are the unsigned wrap of the signed value: negation happens in
 * uint64_t and, unlike "~", is not masked to g_width here. Callers mask
 * the final result themselves.
 */
static void test_unary_sign()
{
	ASSERT_RESULT("-5", UINT64_C(0xFFFFFFFFFFFFFFFB));
	ASSERT_RESULT("-0", 0);
	ASSERT_RESULT("+5", 5);
	ASSERT_RESULT("+ 5", 5);

	/* After another operator. */
	ASSERT_RESULT("10 - -2", 12);
	ASSERT_RESULT("10 - +2", 8);
	ASSERT_RESULT("10 + -2", 8);
	ASSERT_RESULT("5 * -2", UINT64_C(0xFFFFFFFFFFFFFFF6));
	ASSERT_RESULT("2 * +3", 6);

	/* Stacked, and against a parenthesis. */
	ASSERT_RESULT("- -5", 5);
	ASSERT_RESULT("-(3)", UINT64_C(0xFFFFFFFFFFFFFFFD));
	ASSERT_RESULT("(-3)", UINT64_C(0xFFFFFFFFFFFFFFFD));
	ASSERT_RESULT("-(2 + 3)", UINT64_C(0xFFFFFFFFFFFFFFFB));

	/* Unary binds tighter than the binary operators. */
	ASSERT_RESULT("-5 + 8", 3);
	ASSERT_RESULT("-2 * 3", UINT64_C(0xFFFFFFFFFFFFFFFA));

	/* Still an error with nothing to apply it to. */
	ASSERT_STATUS("-", ERROR_SYNTAX);
	ASSERT_STATUS("+", ERROR_SYNTAX);
	ASSERT_STATUS("5 -", ERROR_SYNTAX);

	/* A missing unary form is still a syntax error for other symbols. */
	ASSERT_STATUS("*1", ERROR_SYNTAX);
	ASSERT_STATUS("2+*2", ERROR_SYNTAX);
}

/*
 * Number parsing. base_scanf() used to accept anything sscanf() managed a
 * partial conversion on, so a bare prefix, trailing garbage and an
 * out-of-range literal all came back as valid numbers.
 */
static void test_number_parsing()
{
	char expr[80];

	/* Well-formed literals in every base still parse. */
	ASSERT_RESULT("0777", 511);
	ASSERT_RESULT("b101", 5);
	ASSERT_RESULT("0b101", 5);
	ASSERT_RESULT("0x1f", 31);
	ASSERT_RESULT("0xFFFFFFFFFFFFFFFF", UINT64_MAX);
	ASSERT_RESULT("18446744073709551615", UINT64_MAX);

	/* A prefix with no digits behind it is not the number zero. */
	ASSERT_STATUS("0x", ERROR_SYNTAX);
	ASSERT_STATUS("b", ERROR_SYNTAX);
	ASSERT_STATUS("0b", ERROR_SYNTAX);

	/* Trailing garbage is not silently dropped. */
	ASSERT_STATUS("12a", ERROR_SYNTAX);

	/* Values past UINT64_MAX are refused, not clamped to it. */
	ASSERT_STATUS("18446744073709551616", ERROR_SYNTAX);
	ASSERT_STATUS("9999999999999999999999", ERROR_SYNTAX);
	ASSERT_STATUS("0xFFFFFFFFFFFFFFFFF", ERROR_SYNTAX);

	/* 64 binary digits fit; a 65th significant one does not. */
	expr[0] = '0';
	expr[1] = 'b';
	memset(&expr[2], '1', 64);
	expr[66] = '\0';
	ASSERT_RESULT(expr, UINT64_MAX);

	expr[2] = '1';
	memset(&expr[3], '0', 64);
	expr[67] = '\0';
	ASSERT_STATUS(expr, ERROR_SYNTAX);

	/* Leading zeros are free, so this is 1 rather than an overflow. */
	memset(&expr[2], '0', 64);
	expr[66] = '1';
	expr[67] = '\0';
	ASSERT_RESULT(expr, 1);
}

/*
 * Shifting a uint64_t by 64 or more is undefined; every bit is shifted
 * out, so the answer is 0. On x86 the count used to be masked to 6 bits,
 * making "1 << 64" evaluate to 1.
 */
static void test_shift_edges()
{
	ASSERT_RESULT("1 << 63", 0x8000000000000000ULL);
	ASSERT_RESULT("1 << 64", 0);
	ASSERT_RESULT("1 << 65", 0);
	ASSERT_RESULT("1 << 200", 0);

	ASSERT_RESULT("0x8000000000000000 >> 63", 1);
	ASSERT_RESULT("1 >> 64", 0);
	ASSERT_RESULT("0xFFFFFFFFFFFFFFFF >> 64", 0);
	ASSERT_RESULT("0xFFFFFFFFFFFFFFFF >> 200", 0);

	/* BIT() is a shift too, with the count coming straight from input. */
	ASSERT_RESULT("BIT(63)", 0x8000000000000000ULL);
	ASSERT_RESULT("BIT(64)", 0);
	ASSERT_RESULT("BIT(200)", 0);
}

/*
 * Tokens longer than the old fixed 64-byte scratch buffer. These used to
 * overflow the stack while tokenizing; the point of the test is that they
 * return a status at all rather than what the status is.
 */
static void test_long_tokens()
{
	char expr[512];

	memset(expr, 'a', 300);
	expr[300] = '\0';
	ASSERT_STATUS(expr, ERROR_SYNTAX);

	expr[0] = '0';
	expr[1] = 'x';
	memset(&expr[2], 'a', 200);
	expr[202] = '\0';
	ASSERT_STATUS(expr, ERROR_SYNTAX);

	memset(expr, '9', 400);
	expr[400] = '\0';
	ASSERT_STATUS(expr, ERROR_SYNTAX);

	/* Long but in range: 0x, 61 zeros, then "1f" -- still 31. */
	expr[0] = '0';
	expr[1] = 'x';
	memset(&expr[2], '0', 61);
	expr[63] = '1';
	expr[64] = 'f';
	expr[65] = '\0';
	ASSERT_RESULT(expr, 31);
}

static void test_compound_assignment()
{
	ASSERT_RESULT("$ |= BIT(0)", 0x513);
	ASSERT_RESULT("0x0F |= 0xF0", 0xFF);

	ASSERT_RESULT("$ &= ~BIT(4)", 0x502);
	ASSERT_RESULT("0xFF &= 0x0F", 0x0F);

	ASSERT_RESULT("$ ^= BIT(0)", 0x513);
	ASSERT_RESULT("$ ^= BIT(1)", 0x510);

	ASSERT_RESULT("$ & BIT(1)", 0x2);
	ASSERT_RESULT("$ & BIT(0)", 0);
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
	    !CU_add_test(suite, "unary sign", test_unary_sign) ||
	    !CU_add_test(suite, "compound assignment", test_compound_assignment) ||
	    !CU_add_test(suite, "combined operations", test_combined_operations) ||
	    !CU_add_test(suite, "functions", test_functions) ||
	    !CU_add_test(suite, "constants", test_constants) ||
	    !CU_add_test(suite, "operator precedence", test_precedence) ||
	    !CU_add_test(suite, "error handling", test_errors) ||
	    !CU_add_test(suite, "number parsing", test_number_parsing) ||
	    !CU_add_test(suite, "shift edges", test_shift_edges) ||
	    !CU_add_test(suite, "long tokens", test_long_tokens))
		goto exit;

	CU_basic_set_mode(CU_BRM_NORMAL);
	CU_basic_run_tests();
	tests_failed = CU_get_number_of_tests_failed();
exit:
	CU_cleanup_registry();
	return tests_failed ? EXIT_FAILURE : CU_get_error();
}
