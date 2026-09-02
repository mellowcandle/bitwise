/*
 * libFuzzer harness for the expression evaluator.
 *
 * Build and run:
 *
 *     ./bootstrap.sh
 *     ./configure CC=clang --enable-fuzzer --enable-asan --enable-ubsan
 *     make tests/fuzz-shunting-yard
 *     mkdir -p corpus && tests/fuzz-shunting-yard -max_len=256 corpus
 *
 * shunting_yard() is the whole attack surface reachable from a command
 * line argument or the interactive prompt: tokenizing, number parsing,
 * the operator and operand stacks, and every arithmetic edge case.
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "shunting-yard.h"

/*
 * Normally defined in interactive.c, which drags in curses. The parser
 * only reads it, to evaluate the "$" constant; give it a non-zero value
 * so expressions using "$" exercise something.
 */
uint64_t g_val = 0x512;

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
	uint64_t result = 0;
	char *expression;

	/* The input is not NUL-terminated; shunting_yard() needs a string. */
	expression = malloc(size + 1);
	if (!expression)
		return 0;
	memcpy(expression, data, size);
	expression[size] = '\0';

	shunting_yard(expression, &result);

	free(expression);
	return 0;
}
