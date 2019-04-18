/* Copyright 2019
 * Ramon Fried <ramon.fried@gmail.com>
 */

#include <stdlib.h>
#include <stdint.h>
#include <getopt.h>
#include "bitwise.h"


static uint64_t parse_input(char *input)
{
	int base;

	if (input[0] == '0')
		if (input[1] == 'x' || input[1] == 'X')
			base = 16;
		else
			base = 8;
	else
		base = 10;

	return base_scanf(input, base);
}

int print_conversions(uint64_t val)
{
	char buf_size[16];
	char binary[180];
	int pos = 0;
	int i, j;

	buf_size[0] = '\0';
	sprintf_size(val, buf_size);

	printf("Decimal: %lu\n", val);
	printf("Hexdecimal: 0x%lX\n", val);
	printf("Octal: 0%lo\n", val);
	if (buf_size[0])
		printf("Size: %s\n", buf_size);
	printf("Binary:\n");
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
	printf("%s\n    ", binary);
	for (i = 0; i < 8; i++) {
		printf("%2d - %2d", 63 - (i * 8), 56 - (i * 8));
		for (j = 0; j < 11; j++)
			putchar(' ');
	}
	printf("\n");
	return 0;
}

static void print_version(void)
{
	printf("Bitwise v0.1\n");
}

static void print_help(FILE *out)
{
	fprintf(out, "Usage: bitwise [OPTION...] [input]\n\n");
	fprintf(out, "[input] can be decimal, hexdecimal or octal number, depending on the prefix (0x | 0)\n\n");
	fprintf(out, "  -i, --interactive\t Load interactive mode (default if no input)\n");
	fprintf(out, "  -h, --help\t\t Display this help and exit\n");
	fprintf(out, "  -v, --version\t\t Output version information and exit\n");
	fprintf(out, "      --no-color\t Start without color support\n\n");
}

int main(int argc, char *argv[])
{
	int c;
	int interactive = 0;
	uint64_t val = 0;

	while (1) {
		static struct option long_options[] = {
	          {"no-color", no_argument, &has_color, 0},
	          {"version", no_argument, 0, 'v'},
	          {"help", no_argument, 0, 'h'},
	          {"interactive", no_argument, 0, 'i'},
	          {0, 0, 0, 0}
		};

		int option_index = 0;

		c = getopt_long (argc, argv, "vhi", long_options, &option_index);
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
		case 'i':
			interactive = 1;
			break;

		case '?':
		default:
			print_help(stderr);
			exit(1);
		}
	}

	if (optind < argc) {
		val = parse_input(argv[optind]);
		if (!interactive)
			return print_conversions(val);
	}

	return start_interactive(val);
}
