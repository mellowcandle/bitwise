/* Copyright 2019
 * Ramon Fried <ramon.fried@gmail.com
 */

#include <stdlib.h>
#include <stdint.h>
#include "bitwise.h"

#define BASE_DECIMAL 0

int print_conversions(char *input)
{
	uint64_t val;
	int base;

	if (input[0] == '0')
		if (input[1] == 'x' || input[1] == 'X')
			base = 16;
		else
			base = 8;
	else
		base = 10;

	val = base_scanf(input, base);

	printf("Decimal: %lu\tHexdecimal: 0x%lX\tOctal:0%lo\n", val, val, val);

	return 0;
}



int main(int argc, char *argv[])
{
	if (argc > 1)
		return print_conversions(argv[1]);

	return start_interactive();
}

