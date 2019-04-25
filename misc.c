/* Copyright 2019
 * Ramon Fried <ramon.fried@gmail.com>
 */

#include <ctype.h>
#include "bitwise.h"

#define KB (1ULL << 10)
#define MB (1ULL << 20)
#define GB (1ULL << 30)
#define TB (1ULL << 40)
#define PB (1ULL << 50)

int g_has_color = 1;
int g_width = 0;
void init_terminal(void)
{
	initscr();
	if(has_colors() == FALSE)
		g_has_color = 0;
	else
		start_color();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);
	curs_set(0);
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
	vfprintf(stderr, fmt, args);
	va_end(args);
	exit(1);
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

uint64_t base_scanf(const char *buf, int base)
{
	uint64_t value = 0;
	int ret = 0;

	switch (base) {
	case 10:
		ret = sscanf(buf, "%lu", &value);
		break;
	case 16:
		ret = sscanf(buf, "%lx", &value);
		break;
	case 8:
		ret = sscanf(buf, "%lo", &value);
		break;
	default:
		fprintf(stderr, "Unknown base\n");
		break;
	}

	if (ret == EOF || !ret) {
		fprintf(stderr, "Couldn't parse parameter\n");
		exit(1);
	}

	return value;
}

void lltostr(uint64_t val, char *buf, int base)
{
	switch (base) {
	case 10:
		sprintf(buf, "%lu", val);
		return;
	case 16:
		sprintf(buf, "%lx", val);
		return;
	case 8:
		sprintf(buf, "%lo", val);
		return;
	case 2:
		sprintf(buf, "Not implemeted");
	}
}

int sprintf_size(uint64_t val, char *buf)
{
	int ret;
	double f_val = val;

	if (val >= PB)
		ret = sprintf(buf, "%.2lfPB", f_val / PB);
	else if (val >= TB)
		ret = sprintf(buf, "%.2lfTB", f_val / TB);
	else if (val >= GB)
		ret = sprintf(buf, "%.2lfGB", f_val / GB);
	else if (val >= MB)
		ret = sprintf(buf, "%.2lfMB", f_val / MB);
	else if (val >= KB)
		ret = sprintf(buf, "%.2lfKB", f_val / KB);
	else
		ret = sprintf(buf, "%luB", val);

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
