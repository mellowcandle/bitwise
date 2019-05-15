/* Copyright 2019
 * Ramon Fried <ramon.fried@gmail.com>
 */

#include <stdlib.h>
#include <stdint.h>
#include <getopt.h>
#include "bitwise.h"
#include "config.h"

static uint64_t parse_input(char *input)
{
	int base;

	if (tolower(input[0]) == 'b')
		base = 2;
	else if (input[0] == '0')
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
	for (i = g_width; i > 0; i--) {
		if ((i % 8 == 0) && (i != g_width)) {
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
	for (i = 0; i < g_width / 8; i++) {
		printf("%2d - %2d", g_width - 1 - (i * 8), (g_width - 8) - (i * 8));
		for (j = 0; j < 11; j++)
			putchar(' ');
	}
	printf("\n");
	return 0;
}

static void print_version(void)
{
	printf(PACKAGE " " VERSION "\n");
}

static void print_help(FILE *out)
{
	fprintf(out, "Usage: bitwise [OPTION...] [input]\n\n");
	fprintf(out, "[input] can be decimal, hexdecimal, octal or binary number, depending on the prefix (0x | 0 | b)\n\n");
	fprintf(out, "  -i, --interactive\t Load interactive mode (default if no input)\n");
	fprintf(out, "  -w, --width[b|w|l|d]\t Set bit width (default: l)\n");
	fprintf(out, "  -h, --help\t\t Display this help and exit\n");
	fprintf(out, "  -v, --version\t\t Output version information and exit\n");
	fprintf(out, "      --no-color\t Start without color support\n\n");
}

int main(int argc, char *argv[])
{
	int c;
	char width;
	int interactive = 0;
	uint64_t val = 0;

	while (1) {
		static struct option long_options[] = {
	          {"no-color", no_argument, &g_has_color, 0},
	          {"version", no_argument, 0, 'v'},
	          {"help", no_argument, 0, 'h'},
	          {"interactive", no_argument, 0, 'i'},
		  {"width", required_argument, 0, 'w'},
	          {0, 0, 0, 0}
		};

		int option_index = 0;

		c = getopt_long(argc, argv, "vhiw:", long_options, &option_index);
		if (c == -1)
			break;
		switch (c) {
		case 0:
			break;
		case 'v':
			print_version();
			exit(EXIT_SUCCESS);
		case 'h':
			print_help(stdout);
			exit(EXIT_SUCCESS);
		case 'i':
			interactive = 1;
			break;
		case 'w':
			width = *optarg;
			if (!set_width(width))
				break;
			fprintf(stderr, "Unsupported width size: accepted values are: [b|w|l|d]\n");
		case '?':
		default:
			print_help(stderr);
			exit(EXIT_FAILURE);
		}
	}
	if (optind < argc) {
		val = parse_input(argv[optind]);
		if (!g_width) {
			set_width_by_val(val);
		}
		val &= MASK(g_width);
		if (!interactive)
			return print_conversions(val);
	}

	if (!g_width)
		g_width = 32;
	return start_interactive(val);
}
