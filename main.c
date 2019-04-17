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

static void print_version(void)
{
	printf("Bitwise v0.1\n");
}

static void print_help(FILE *out)
{
	fprintf(out, "help menu\n");
}

int main(int argc, char *argv[])
{
	int c;

	while (1) {
		static struct option long_options[] = {
	          {"no-color", no_argument, &has_color, 0},
	          {"version", no_argument, 0, 'v'},
	          {"help", no_argument, 0, 'h'},
	          {0, 0, 0, 0}
		};

		int option_index = 0;

		c = getopt_long (argc, argv, "vh", long_options, &option_index);
		if (c == -1)
			break;
		switch (c) {
		case 0:
			break;
		case 'v':
			print_version();
			exit(0);
		case 'h':
			print_help(stdout);
			exit(0);
		case '?':
			break;
		default:
			print_help(stderr);
			exit(1);
		}
	}

	if (optind < argc)
		return print_conversions(argv[optind]);

	return start_interactive();
}
