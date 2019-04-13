/* Copyright 2019
 * Ramon Fried <ramon.fried@gmail.com>
 */

#include <stdlib.h>
#include <stdint.h>
#include <getopt.h>

#include "bitwise.h"

int print_conversions(char *input)
{
	char binary[180];
	uint64_t val;
	int base;
	int i;
	int pos = 0;

	if (input[0] == '0')
		if (input[1] == 'x' || input[1] == 'X')
			base = 16;
		else
			base = 8;
	else
		base = 10;

	val = base_scanf(input, base);

	printf("Decimal: %lu\tHexdecimal: 0x%lX\tOctal:0%lo\n", val, val, val);

	for (i = 64; i > 0; i--) {
		if ((i % 8 == 0) && (i != 64)) {
			binary[pos] = '|';
			binary[pos+1] = ' ';
			pos += 2;
		}
		if (val & BIT(i-1))
			binary[pos] = '1';
		else
			binary[pos] = '0';
		binary[pos+1] = ' ';
		pos+=2;
	}

	binary[pos] = '\0';
	printf("%s\n", binary);

	return 0;
}

int main(int argc, char *argv[])
{

	if (argc > 1)
		return print_conversions(argv[1]);

	return start_interactive();
}
