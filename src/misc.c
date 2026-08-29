/* Copyright 2019
 * Ramon Fried <ramon.fried@gmail.com>
 */

#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <stdarg.h>
#include "bitwise.h"

/* IEC Standard */
#define KiB (1ULL << 10)
#define MiB (1ULL << 20)
#define GiB (1ULL << 30)
#define TiB (1ULL << 40)
#define PiB (1ULL << 50)

/* SI Standard */
#define kB (1000ULL)
#define MB (1000 * kB)
#define GB (1000 * MB)
#define TB (1000 * GB)
#define PB (1000 * TB)

int g_has_color = 1;
int g_width = 0;
bool g_input_avail;
int g_input;

#define RED   "\x1B[31m"
#define GREEN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLUE   "\x1B[34m"
#define MAGENTA   "\x1B[35m"
#define CYAN   "\x1B[36m"
#define WHITE   "\x1B[37m"
#define RESET "\x1B[0m"
#define NOTHING ""

char *color_green = NOTHING;
char *color_red = NOTHING;
char *color_blue = NOTHING;
char *color_magenta = NOTHING;
char *color_cyan = NOTHING;
char *color_white = NOTHING;
char *color_reset = NOTHING;

void init_colors(void)
{
	if (g_has_color) {
		color_green = GREEN;
		color_red = RED;
		color_blue = BLUE;
		color_magenta = MAGENTA;
		color_cyan = CYAN;
		color_white = WHITE;
		color_reset = RESET;
	} else {
		color_green = NOTHING;
		color_red = NOTHING;
		color_blue = NOTHING;
		color_magenta = NOTHING;
		color_cyan = NOTHING;
		color_white = NOTHING;
		color_reset = NOTHING;
	}
}

void init_terminal(void)
{
	initscr();
	if (has_colors() == FALSE)
		g_has_color = 0;
	else {
		start_color();
		init_colors();
	}
	cbreak();
	noecho();
	nonl();
	intrflush(NULL, FALSE);
	keypad(stdscr, TRUE);
	curs_set(2);
}

void deinit_terminal(void)
{
	endwin();
}

void die(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	deinit_terminal();
	/* See interactive.c for reasoning */
	/* deinit_readline(); */
	vfprintf(stderr, fmt, args);
	va_end(args);
	exit(EXIT_FAILURE);
}

int validate_input(int ch, int base)
{
	switch (base) {
	case 2:
		if (ch == '0' || ch == '1')
			return 0;
		break;
	case 8:
		if (ch >= '0' && ch <= '7')
			return 0;
		break;
	case 16:
		if ((ch >= '0' && ch <= '9') ||
		    (ch >= 'A' && ch <= 'F') ||
		    (ch >= 'a' && ch <= 'f'))
			return 0;
		break;
	case 10:
		/*
		 * A plain range check, like the other bases above. ch comes
		 * straight from getch(), so it can be a key code such as
		 * KEY_RESIZE (410); isdigit() is only defined for values
		 * representable as unsigned char or EOF, and glibc indexes a
		 * table with it, reading out of bounds for anything larger.
		 */
		if (ch >= '0' && ch <= '9')
			return 0;
		break;
	default:
		break;
	}

	return 1;
}

int binary_scanf(const char *buf, uint64_t *val)
{
	uint64_t value = 0;
	bool seen_digit = false;

	/* Skip the leading 0 */
	if (buf[0] == '0') {
		buf++;
	}

	/* Skip the leading b */
	buf++;

	while (*buf) {
		switch (*buf) {

		case '0':
		case '1':
			/*
			 * A set top bit would be shifted out of the value
			 * entirely, so the string does not fit in 64 bits.
			 * Leading zeros still cost nothing.
			 */
			if (value >> 63)
				return 0;
			value = (value << 1) | (uint64_t)(*buf - '0');
			seen_digit = true;
			break;
		default:
			return 0;
		}
		buf++;
	}

	/* A bare "b" or "0b" prefix is not the number zero. */
	if (!seen_digit)
		return 0;

	*val = value;

	return 1;
}

/*
 * Parse buf as a whole unsigned number in the given base. Anything the
 * string is not is rejected: no digits at all, trailing garbage after the
 * number, a sign, or a value that does not fit in uint64_t.
 * Returns 0 on success.
 */
int base_scanf(const char *buf, int base, uint64_t *value)
{
	char *end;
	uint64_t parsed;

	if (base == 2) {
		if (!binary_scanf(buf, value)) {
			LOG("Couldn't parse number: %s\n", buf);
			return 1;
		}
		return 0;
	}

	if (base != 8 && base != 10 && base != 16) {
		LOG("Unknown base: %d\n", base);
		return 1;
	}

	/* strtoull() wraps a negative value round rather than refusing it. */
	if (buf[0] == '-')
		return 1;

	errno = 0;
	parsed = strtoull(buf, &end, base);

	if (end == buf || *end != '\0' || errno == ERANGE) {
		LOG("Couldn't parse number: %s\n", buf);
		return 1;
	}

	*value = parsed;

	return 0;
}

int ip_scanf(const char *input, uint64_t *val)
{
	union {
		uint8_t ip[4];
		uint32_t ip32;
	} ip;
	if (sscanf(input, "%hhu.%hhu.%hhu.%hhu",
		   &ip.ip[0], &ip.ip[1], &ip.ip[2], &ip.ip[3]) != 4) {
		LOG("Failed parsing IPv4 address: %s\n", input);
		return 1;
	}

	*val = ip.ip32;
	return 0;
}

int parse_input(const char *input, uint64_t *val)
{
	int base;

	if (strchr(input, '.'))
		return ip_scanf(input, val);
	if (tolower((unsigned char)input[0]) == 'b')
		base = 2;
	else if (input[0] == '0')
		if (tolower((unsigned char)input[1]) == 'b')
			base = 2;
		else if (input[1] == 'x' || input[1] == 'X')
			base = 16;
		else
			base = 8;
	else
		base = 10;

	return base_scanf(input, base, val);
}

int lltostr(uint64_t val, char *buf, int base)
{
	int rc;

	switch (base) {
	case 10:
		rc = sprintf(buf, "%" PRIu64, val);
		break;
	case 16:
		rc = sprintf(buf, "%" PRIx64, val);
		break;
	case 8:
		rc = sprintf(buf, "%" PRIo64, val);
		break;
	case 2:
	default:
		sprintf(buf, "Not implemented");
		return -1;
	}

	if (rc < 0)
		LOG("sprintf failed with error: %d\n", rc);

	return rc;
}

int sprintf_type(uint64_t val, char *buf, output_type type)
{
	int i;
	int pos = 0;

	switch (type) {
	case CMD_OUTPUT_DECIMAL:
		sprintf(buf, "Decimal: %" PRIu64, val);
		break;
	case CMD_OUTPUT_HEXADECIMAL:
		sprintf(buf, "Hexadecimal: 0x%" PRIx64, val);
		break;
	case CMD_OUTPUT_OCTAL:
		sprintf(buf, "Octal: 0%" PRIo64, val);
		break;
	case CMD_OUTPUT_BINARY:
		pos = sprintf(buf, "Binary: ");
		for (i = g_width; i > 0; i--) {
			if ((i % 8 == 0) && (i != g_width)) {
				buf[pos] = '|';
				buf[pos + 1] = ' ';
				pos += 2;
			}
			if (val & BIT(i - 1)) {
				buf[pos] = '1';
			}
			else {
				buf[pos] = '0';
			}
			buf[pos + 1] = ' ';
			pos += 2;
		}
		buf[pos-1] = '\0';
		break;

	default:
		break;
	}

	return 0;
}

int sprintf_size(uint64_t val, char *buf, bool si)
{
	int ret;
	double f_val = val;

	if (si) {
		if (val >= PB)
			ret = sprintf(buf, "%.2lf PB", f_val / PB);
		else if (val >= TB)
			ret = sprintf(buf, "%.2lf TB", f_val / TB);
		else if (val >= GB)
			ret = sprintf(buf, "%.2lf GB", f_val / GB);
		else if (val >= MB)
			ret = sprintf(buf, "%.2lf MB", f_val / MB);
		else if (val >= kB)
			ret = sprintf(buf, "%.2lf Kb", f_val / kB);
		else
			ret = sprintf(buf, "%" PRIu64, val);
	} else {
		if (val >= PiB)
			ret = sprintf(buf, "%.2lf PiB", f_val / PiB);
		else if (val >= TiB)
			ret = sprintf(buf, "%.2lf TiB", f_val / TiB);
		else if (val >= GiB)
			ret = sprintf(buf, "%.2lf GiB", f_val / GiB);
		else if (val >= MiB)
			ret = sprintf(buf, "%.2lf MiB", f_val / MiB);
		else if (val >= KiB)
			ret = sprintf(buf, "%.2lf KiB", f_val / KiB);
		else
			ret = sprintf(buf, "%" PRIu64, val);
	}

	return ret;
}

void set_width_by_val(uint64_t val)
{
	if (val & 0xFFFFFFFF00000000)
		g_width = 64;
	else if (val & 0xFFFF0000)
		g_width = 32;
	else if (val & 0xFF00)
		g_width = 16;
	else if (val & 0xFF)
		g_width = 8;
	else
		g_width = 32;
}

int set_width(char width)
{
	if (tolower((unsigned char)width) == 'b')
		g_width = 8;
	else if (tolower((unsigned char)width) == 'w')
		g_width = 16;
	else if (tolower((unsigned char)width) == 'l')
		g_width = 32;
	else if (tolower((unsigned char)width) == 'd')
		g_width = 64;
	else
		return 1;

	return 0;
}


const char *
ipv4_to_str(uint32_t ip, char out[static 16])
{
    const unsigned char b0 = (ip >> 24) & 0xFF;
    const unsigned char b1 = (ip >> 16) & 0xFF;
    const unsigned char b2 = (ip >> 8)  & 0xFF;
    const unsigned char b3 = ip & 0xFF;

    snprintf(out, 16, "%u.%u.%u.%u", b0, b1, b2, b3);
    return out;
}

