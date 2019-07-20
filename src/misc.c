/* Copyright 2019
 * Ramon Fried <ramon.fried@gmail.com>
 */

#include <ctype.h>
#include <inttypes.h>
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
		if (isdigit(ch))
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

	/* Skip the leading b */
	buf++;

	while (*buf) {
		switch (*buf) {

		case '0':
			value <<= 1;
			break;
		case '1':
			value <<= 1;
			value++;
			break;
		default:
			return 0;
		}
		buf++;
	}

	*val = value;

	return 1;
}

int base_scanf(const char *buf, int base, uint64_t *value)
{
	int ret = 0;

	switch (base) {
	case 10:
		ret = sscanf(buf, "%" PRIu64, value);
		break;
	case 16:
		ret = sscanf(buf, "%" PRIX64, value);
		break;
	case 8:
		ret = sscanf(buf, "%" PRIo64, value);
		break;
	case 2:
		ret = binary_scanf(buf, value);
		break;
	default:
		fprintf(stderr, "Unknown base\n");
		break;
	}

	if (ret == EOF || !ret) {
		LOG("Couldn't parse number: %s\n", buf);
		return 1;
	}

	return 0;
}

int parse_input(const char *input, uint64_t *val)
{
	int base;
	uint64_t tmp_val;

	if (tolower(input[0]) == 'b')
		base = 2;
	else if (input[0] == '0')
		if (input[1] == 'x' || input[1] == 'X')
			base = 16;
		else
			base = 8;
	else
		base = 10;

	return base_scanf(input, base, val);
}

void lltostr(uint64_t val, char *buf, int base)
{
	switch (base) {
	case 10:
		sprintf(buf, "%" PRIu64, val);
		return;
	case 16:
		sprintf(buf, "%" PRIx64, val);
		return;
	case 8:
		sprintf(buf, "%" PRIo64, val);
		return;
	case 2:
	default:
		sprintf(buf, "Not implemeted");
	}
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
	int size;

	if (tolower(width) == 'b')
		g_width = 8;
	else if (tolower(width) == 'w')
		g_width = 16;
	else if (tolower(width) == 'l')
		g_width = 32;
	else if (tolower(width) == 'd')
		g_width = 64;
	else
		return 1;

	return 0;
}
