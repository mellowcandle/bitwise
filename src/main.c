/* Copyright 2019
 * Ramon Fried <ramon.fried@gmail.com>
 */

#include <ctype.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdint.h>
#include <getopt.h>
#include <locale.h>
#include <arpa/inet.h>
#include <limits.h>

#ifdef HAVE_DECL_BSWAP_32
#include <byteswap.h>
#else
#define bswap_32(x) \
     ((((x) & 0xff000000) >> 24) | (((x) & 0x00ff0000) >>  8) | \
      (((x) & 0x0000ff00) <<  8) | (((x) & 0x000000ff) << 24))

#endif

#include "bitwise.h"
#include "shunting-yard.h"


int print_conversions(uint64_t val, bool si)
{
	char buf_size[16];
	char binary[512];
	int pos = 0;
	int i, j;
	struct in_addr ip_addr;

	buf_size[0] = '\0';
	sprintf_size(val, buf_size, si);

	printf("%sUnsigned decimal: %s%" PRIu64 "\n", color_green, color_blue, val);

	switch (g_width) {
	case 64:
		printf("%sSigned decimal: %s%" PRId64 "\n", color_green,
		       color_blue, (int64_t)val);
		break;
	case 32:
		printf("%sSigned decimal: %s%" PRId32 "\n", color_green,
		       color_blue, (int32_t)val);
		break;
	case 16:
		printf("%sSigned decimal: %s%" PRId16 "\n", color_green,
		       color_blue, (int16_t)val);
		break;
	case 8:
		printf("%sSigned decimal: %s%" PRId8 "\n", color_green,
		       color_blue, (int8_t)val);
		break;
	default:
		break;
	}

	printf("%sHexadecimal: %s0x%" PRIx64 "\n", color_green, color_blue, val);
	printf("%sOctal: %s0%" PRIo64 "\n", color_green, color_blue, val);
	if (buf_size[0])
		printf("%sHuman: %s%s\n", color_green, color_blue, buf_size);

	if (g_width < 64)
		printf("%sRadix64: %s%s\n", color_green, color_blue, l64a(val));

	if (val >= UINT_MAX) {
		printf("%sIPv4: %s%s\n", color_green, color_blue, "Value too big to be a valid IPv4 address");
	} else {
		ip_addr.s_addr = val;
		printf("%sIPv4 (Network byte order - Big): %s %s\n", color_green, color_blue, inet_ntoa(ip_addr));
		ip_addr.s_addr = bswap_32(val);
		printf("%sIPv4 (Reversed byte order - Little): %s %s\n", color_green, color_blue, inet_ntoa(ip_addr));
	}

	printf("%sASCII: %s", color_green, color_blue);
	for (i = sizeof(uint64_t) - 1; i >= 0; i--) {
		char c = ((char *)&val)[i];
		if (isgraph(c))
			printf("%s%c", color_blue, c);
		else
			printf("%s.", color_white);
	}

	printf("\n%sBinary:\n%s", color_green, color_reset);
	for (i = g_width; i > 0; i--) {
		if ((i % 8 == 0) && (i != g_width)) {
			pos += sprintf(&binary[pos], "%s", color_white);
			binary[pos] = '|';
			binary[pos + 1] = ' ';
			pos += 2;
		}
		if (val & BIT(i - 1)) {
			pos += sprintf(&binary[pos], "%s", color_blue);
			binary[pos] = '1';
		}
		else {
			pos += sprintf(&binary[pos], "%s", color_magenta);
			binary[pos] = '0';
		}
		binary[pos + 1] = ' ';
		pos += 2;
	}

	binary[pos-1] = '\0';
	printf("%s\n    ", binary);
	fputs(color_cyan, stdout);
	for (i = 0; i < g_width / 8; i++) {
		printf("%2d - %2d", g_width - 1 - (i * 8), (g_width - 8) - (i * 8));
		if (i != (g_width / 8) - 1)
			for (j = 0; j < 11; j++)
				putchar(' ');
	}
	printf("\n");
	puts(color_reset);
	return 0;
}

static void print_version(void)
{
	printf(PACKAGE " " VERSION "\n");
}

static void print_help(FILE *out)
{
	fprintf(out, "Usage: bitwise [OPTION...] [expression]\n\n");
	fprintf(out,
	        "[expression] mathematical expression\n\n");
	fprintf(out,
	        "  -i, --interactive\t Load interactive mode (default if no input)\n");
	fprintf(out, "  -w, --width[b|w|l|d]\t Set bit width (default: l)\n");
	fprintf(out, "  -h, --help\t\t Display this help and exit\n");
	fprintf(out, "  -v, --version\t\t Output version information and exit\n");
	fprintf(out, "  -s, --si\t\t Print size according to SI standard. (default: IEC standard)\n");
	fprintf(out, "      --no-color\t Start without color support\n\n");
}

int main(int argc, char *argv[])
{
	int c;
	char width;
	int interactive = 0;
	uint64_t val = 0;
	int rc;
	bool si = false;

#ifdef TRACE
	fd = fopen("log.txt", "w");
#endif

	setlocale(LC_ALL, "");

	while (1) {
		static struct option long_options[] = {
			{"no-color", no_argument, &g_has_color, 0},
			{"version", no_argument, 0, 'v'},
			{"help", no_argument, 0, 'h'},
			{"interactive", no_argument, 0, 'i'},
			{"si", no_argument, 0, 's'},
			{"width", required_argument, 0, 'w'},
			{0, 0, 0, 0}
		};

		int option_index = 0;

		c = getopt_long(argc, argv, "vhisw:", long_options, &option_index);
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
		case 's':
			si = true;
			break;
		case 'w':
			width = *optarg;
			if (!set_width(width))
				break;
			fprintf(stderr, "Unsupported width size: accepted values are: [b|w|l|d]\n");
			// fall through
		case '?':
		default:
			print_help(stderr);
			exit(EXIT_FAILURE);
		}
	}

	init_colors();

	if (optind < argc) { // non-interactive mode
		uint32_t expr_len = argc - optind; // account for ' ' between args
		for (int i = optind; i < argc; i++)
			expr_len += strlen(argv[i]);

		char *expression = malloc(sizeof(char) * expr_len);
		if (expression == NULL) {
			fprintf(stderr, "Error parsing arguments");
			exit(EXIT_FAILURE);
		}

		uint32_t expr_pos = 0;
		for (int i = optind; i < argc; i++) {
			strncpy(&expression[expr_pos], argv[i], expr_len - expr_pos);
			expr_pos += strlen(argv[i]);
			expression[expr_pos++] = ' ';
		}
		expression[expr_pos - 1] = '\0';

		rc = shunting_yard(expression, &val);
		if (rc) {
			fprintf(stderr, "Couldn't parse expression: %s\n", expression);
			print_help(stderr);
			exit(EXIT_FAILURE);
		}
		free(expression);
		if (!g_width)
			set_width_by_val(val);
		val &= MASK(g_width);
		if (!interactive)
			return print_conversions(val, si);
	}

	if (!g_width)
		g_width = 32;
	start_interactive(val);

#ifdef TRACE
	fclose(fd);
#endif
	return 0;
}
