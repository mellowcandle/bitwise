/* Copyright 2019
 * Ramon Fried <ramon.fried@gmail.com>
 */

#include <stdlib.h>
#include <form.h>
#include <ctype.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/stat.h>
#include <math.h>
#include "bitwise.h"

#define MAX_DEC_DIGITS_64 20
#define MAX_HEX_DIGITS_64 16
#define MAX_OCT_DIGITS_64 32

#define MAX_DEC_DIGITS_32 10
#define MAX_HEX_DIGITS_32 8
#define MAX_OCT_DIGITS_32 16

#define MAX_DEC_DIGITS_16 5
#define MAX_HEX_DIGITS_16 4
#define MAX_OCT_DIGITS_16 8

#define MAX_DEC_DIGITS_8 3
#define MAX_HEX_DIGITS_8 2
#define MAX_OCT_DIGITS_8 4

#define CLEAR_BIT 0
#define SET_BIT 1
#define TOGGLE_BIT 2

#ifdef TRACE
FILE *fd;
#endif

int max_dec_digits, max_hex_digits, max_oct_digits;
int min_frame_size;
static char title[] = "Bitwise";
static char *width_str;
WINDOW *fields_win;
WINDOW *binary_win;
WINDOW *cmd_win;
WINDOW *history_win;

int active_win, last_win;
int g_output = CMD_OUTPUT_HEXADECIMAL;

static FIELD *field[5];
FORM  *form;
uint64_t g_val;
int bit_pos;
static int binary_field_size;

static int base[3] = {
	10,
	16,
	8,
};

struct history_entry history[MAX_HISTORY_LEN] = { { 0, NULL } };
unsigned int history_pos = 0;
int max_display_history;

#define BINARY_WIN_LEN 17
#define BYTE_BINARY_WIN_LEN (BINARY_WIN_LEN + 2)
#define WORD_BINARY_WIN_LEN (BINARY_WIN_LEN * 2) + 3
#define LONG_BINARY_WIN_LEN (BINARY_WIN_LEN * 4) + 5
#define DBL_BINARY_WIN_LEN  (BINARY_WIN_LEN * 8) + 9

void flush_history(void)
{
	int i;

	history_pos = 0;

	for (i = 0; i < MAX_HISTORY_LEN; i++)
		if (history[i].line) {
			free(history[i].line);
			history[i].line = NULL;
		}
}

void update_history_win(void)
{
	unsigned int max_display = LINES - 17;
	int i, j;

	if (history_pos < max_display)
		max_display = history_pos;

	werase(history_win);
	box(history_win, 0, 0);

	for (i = max_display, j = 1; i > 0; i--, j++) {
		if (g_has_color)
			switch (history[history_pos -i].type) {
			case  TYPE_INPUT_COMMAND:
				wattron(history_win, COLOR_PAIR(2));
				break;
			case  TYPE_INPUT_EXPRESSION:
				wattron(history_win, COLOR_PAIR(1));
				break;
			case TYPE_OUTPUT_RESULT:
				wattron(history_win, COLOR_PAIR(3));
				break;
			case TYPE_OUTPUT_ERROR:
				wattron(history_win, COLOR_PAIR(5));
				break;
			}

		mvwprintw(history_win, j, 2, "%s", history[history_pos - i].line);
	}

	if (g_has_color)
		wattroff(history_win, COLOR_PAIR(1));
	wrefresh(history_win);
}

char binary_field[DBL_BINARY_WIN_LEN];
int dec_pos, hex_pos, oct_pos;
bool g_leave_req;
void set_fields_width(int width)
{
	int min_field_distance;
	g_width = width;
	switch (width) {
	case 64:
		binary_field_size = DBL_BINARY_WIN_LEN;
		max_dec_digits = MAX_DEC_DIGITS_64;
		max_hex_digits = MAX_HEX_DIGITS_64;
		max_oct_digits = MAX_OCT_DIGITS_64;
		min_field_distance = 4;
		width_str = " 64Bit ";
		break;
	case 32:
		g_val &= 0xFFFFFFFF;
		binary_field_size = LONG_BINARY_WIN_LEN;
		max_dec_digits = MAX_DEC_DIGITS_32;
		max_hex_digits = MAX_HEX_DIGITS_32;
		max_oct_digits = MAX_OCT_DIGITS_32;
		min_field_distance = 6;
		width_str = " 32Bit ";
		break;
	case 16:
		g_val &= 0xFFFF;
		binary_field_size = WORD_BINARY_WIN_LEN;
		max_dec_digits = MAX_DEC_DIGITS_16;
		max_hex_digits = MAX_HEX_DIGITS_16;
		max_oct_digits = MAX_OCT_DIGITS_16;
		min_field_distance = 8;
		width_str = " 16Bit ";
		break;
	case 8:
		g_val &= 0xFF;
		binary_field_size = BYTE_BINARY_WIN_LEN;
		max_dec_digits = MAX_DEC_DIGITS_8;
		max_hex_digits = MAX_HEX_DIGITS_8;
		max_oct_digits = MAX_OCT_DIGITS_8;
		min_field_distance = 10;
		width_str = " 8Bit ";
		break;
	}

	/* Reserve space for the cursor at the end */
	max_dec_digits += 1;
	max_hex_digits += 1;
	max_oct_digits += 1;

	dec_pos = 0;
	hex_pos = dec_pos + max_dec_digits + min_field_distance - 2;
	oct_pos = hex_pos + max_hex_digits + min_field_distance + 1;
}

static void update_bit(int pos, int op)
{
	if (op == SET_BIT)
		g_val |= BIT(g_width - 1 - pos);
	else if (op == CLEAR_BIT)
		g_val &= ~(BIT(g_width - 1 - pos));
	else
		g_val ^= BIT(g_width - 1 - pos);
	LOG("g_width: %d\ng_val: %llu\n", g_width, g_val);


	update_binary();
}

void update_binary()
{
	int i;
	int pos = 0;

	for (i = g_width; i > 0; i--) {
		if ((i % 8 == 0) && (i != g_width)) {
			binary_field[pos] = '|';
			binary_field[pos + 1] = ' ';
			pos += 2;
		}
		if (g_val & BIT(i - 1))
			binary_field[pos] = '1';
		else
			binary_field[pos] = '0';
		binary_field[pos + 1] = ' ';
		pos += 2;
	}

	binary_field[pos] = '\0';
	mvwprintw(binary_win, 1, 2, "%s", binary_field);

	pos = 6;
	if (g_has_color)
		wattron(binary_win, COLOR_PAIR(3));
	for (i = 0; i < g_width / 8; i++, pos += 18)
		mvwprintw(binary_win, 2, pos, "%2d - %2d",
		          (g_width - 1) - (i * 8),
		          (g_width - 8) - (i * 8));
	if (g_has_color)
		wattroff(binary_win, COLOR_PAIR(3));
	wrefresh(binary_win);
}


void mouse_to_bit(int xPos){
	int binaryDigitWinSize = 18;
	//Indicates that user is clicking a separating bar
	if(xPos % binaryDigitWinSize == 0){
		return;
	}
	int field = floor(xPos / binaryDigitWinSize);
	// How many bits from the previous separating line was the click
	int bitFromLine = floor((xPos%binaryDigitWinSize)/2) - 1;
	// If they are on the first bit
	if (bitFromLine < 0) {
		bitFromLine = 0;
	}
	int relativeBit = (8 * (field)) + bitFromLine;
	update_bit(relativeBit, TOGGLE_BIT);
	update_fields(-1);
}

WINDOW * process_mouse(MEVENT *event)
{
	WINDOW *mouse_window = NULL;
	if (wmouse_trafo(binary_win, &event->y, &event->x, FALSE) == TRUE) {
		mouse_window = binary_win;
		mouse_to_bit(event->x);
	} else if (wmouse_trafo(fields_win, &event->y, &event->x, FALSE) == TRUE) {
		mouse_window = fields_win;
	}

	return mouse_window;
}


int update_fields(int index)
{
	FIELD *tmp_field = current_field(form);
	int *cur_base = field_userptr(tmp_field);
	char *buffer;
	char number[64];
	int *base;
	uint64_t tmp_val;
	if (index != -1) {
		buffer = field_buffer(tmp_field, 0);
		assert(buffer);

		tmp_val = strtoull(buffer, NULL, *cur_base);
		if (tmp_val == LLONG_MAX) {
			/*
			 * Make sure we don't get a number which is
			 * too big to represent in 64bit */
			beep();
			return 1;
		}
		if (tmp_val > MASK(g_width)) {
			FIELD *tmp_field = current_field(form);
			unpaint_screen();
			set_fields_width(g_width * 2);
			paint_screen();
			set_current_field(form, tmp_field);
			set_active_field(false);
			LOG("Overflow: tmp_val = %" PRIu64 "\n", tmp_val);
			index = -1;
		}
		g_val = tmp_val;
	}
	LOG("Val = %" PRIu64 "\n", g_val);
	for (int i = 0; i < 3; i++) {
		base = field_userptr(field[i]);
		if (g_val)
			lltostr(g_val, number, *base);
		else
			number[0] = '\0';
		LOG("updating field %d\n", i);
		set_field_buffer(field[i], 0, number);
	}
	form_driver(form, REQ_VALIDATION);
	form_driver(form, REQ_END_FIELD);
	wrefresh(fields_win);

	return 0;
}

void set_active_field(bool none)
{
	if (!g_has_color)
		return;

	if (!none) {
		set_field_fore(current_field(form), COLOR_PAIR(1));
		set_field_back(current_field(form),
		               A_UNDERLINE | COLOR_PAIR(1));
	}

	for (int i = 0; i < 3; i++) {
		if ((field[i] == current_field(form)) && (!none))
			continue;
		set_field_fore(field[i], COLOR_PAIR(0));
		set_field_back(field[i], COLOR_PAIR(0));
	}
}

void position_binary_curser(int previous_pos, int next_pos)
{

	int pos;

	pos = 2 + (2 * previous_pos) + (2 * (previous_pos / 8));
	mvwchgat(binary_win, 1, pos, 1, A_NORMAL, COLOR_PAIR(0), NULL);

	if (next_pos != -1) {
		pos = 2 + (2 * next_pos) + (2 * (next_pos / 8));
		mvwchgat(binary_win, 1, pos, 1, A_UNDERLINE,
		         COLOR_PAIR(0), NULL);
		mvwprintw(binary_win, 3, 1, "bit %.2u\t\t",
			  g_width - 1 - next_pos);
	}
	wrefresh(binary_win);
}

void process_binary(int ch)
{
	int tmp;

	switch (ch) {
	case 'w':
	case 'W':
		LOG("word right\n");
		tmp = (bit_pos + 8) & ~(8 - 1);
		if (tmp >= g_width - 1) {
			beep();
			break;
		}
		position_binary_curser(bit_pos, tmp);
		bit_pos = tmp;
		break;

	case 'b':
	case 'B':
		LOG("word left\n");
		tmp = (bit_pos - 8) & ~(8 - 1);
		if (tmp <  0) {
			beep();
			break;
		}
		position_binary_curser(bit_pos, tmp);
		bit_pos = tmp;
		break;
	case KEY_RIGHT:
	case 'l':
		LOG("Key right\n");
		if (bit_pos == g_width - 1) {
			beep();
			break;
		}
		position_binary_curser(bit_pos, bit_pos + 1);
		bit_pos++;
		break;
	case KEY_LEFT:
	case 'h':
		LOG("Key left\n");
		if (bit_pos == 0) {
			beep();
			break;
		}
		position_binary_curser(bit_pos, bit_pos - 1);
		bit_pos--;
		break;
	case KEY_UP:
	case 'k':
	case '\t':
		LOG("Key up\n");
		active_win = FIELDS_WIN;
		set_active_field(false);
		position_binary_curser(bit_pos, -1);
		form_driver(form, REQ_END_LINE);
		form_driver(form, REQ_VALIDATION);
		curs_set(1);
		wrefresh(fields_win);
		break;
	case KEY_BACKSPACE:
	case 127:
		LOG("Backspace\n");
		if (bit_pos != 0) {
			bit_pos--;
			update_bit(bit_pos, CLEAR_BIT);
			update_fields(-1);
		} else
			beep();
		break;
	default:
		if (ch == '1') {
			update_bit(bit_pos, SET_BIT);
			position_binary_curser(bit_pos,
			                       (bit_pos + 1) == (g_width) ?
			                       (bit_pos) : (bit_pos + 1));
			update_fields(-1);
			if (bit_pos != g_width - 1)
				bit_pos++;
			break;
		} else if (ch == '0') {
			update_bit(bit_pos, CLEAR_BIT);
			position_binary_curser(bit_pos,
			                       (bit_pos + 1) == (g_width) ?
			                       (bit_pos) : (bit_pos + 1));
			update_fields(-1);
			if (bit_pos != g_width - 1)
				bit_pos++;
			break;
		} else if (ch == ' ') {
			update_bit(bit_pos, TOGGLE_BIT);
			position_binary_curser(bit_pos, bit_pos);
			update_fields(-1);
		}
		break;
	}
}

void process_fields(int ch)
{
	FIELD *tmp_field;
	int *cur_base;

	switch (ch) {
	case KEY_RIGHT:
	case 'l':
		LOG("Key right\n");
		/* Go to next field */
		form_driver(form, REQ_NEXT_FIELD);
		/* Go to the end of the present buffer */
		/* Leaves nicely at the last character */
		form_driver(form, REQ_END_FIELD);
		set_active_field(false);
		wrefresh(fields_win);
		break;
	case KEY_LEFT:
	case 'h':
		LOG("Key left\n");
		/* Go to previous field */
		form_driver(form, REQ_PREV_FIELD);
		form_driver(form, REQ_END_FIELD);
		set_active_field(false);
		wrefresh(fields_win);
		break;
	case KEY_DOWN:
	case 'j':
	case '\t':
		LOG("Key down\n");
		active_win = BINARY_WIN;
		curs_set(0);
		set_active_field(true);
		form_driver(form, REQ_VALIDATION);
		position_binary_curser(0, bit_pos);
		wrefresh(fields_win);
		break;
	case KEY_BACKSPACE:
	case 127:
		LOG("Backspace\n");
		form_driver(form, REQ_DEL_CHAR);
		form_driver(form, REQ_DEL_PREV);
		form_driver(form, REQ_VALIDATION);
		update_fields(field_index(current_field(form)));
		set_active_field(false);
		update_binary();
		wrefresh(fields_win);
		break;
	default:
		LOG("default char\n");

		tmp_field = current_field(form);
		cur_base = field_userptr(tmp_field);

		if (validate_input(ch, *cur_base))
			break;
		form_driver(form, ch);
		form_driver(form, REQ_VALIDATION);
		if (update_fields(field_index(tmp_field))) {
			form_driver(form, REQ_DEL_CHAR);
			form_driver(form, REQ_PREV_CHAR);
			form_driver(form, REQ_VALIDATION);
		}
		update_binary();
		wrefresh(fields_win);
		break;
	}
}

void paint_screen(void)
{
	int rows, cols;
	int rc;

	/* Initialize the fields */
	field[0] = new_field(1, max_dec_digits, 1,
	                     dec_pos, 0, 0);

	field[1] = new_field(1, max_hex_digits, 1,
	                     hex_pos, 0, 0);
	field[2] = new_field(1, max_oct_digits, 1,
	                     oct_pos, 0, 0);
	field[3] = NULL;

	for (int i = 0; i < 3; i++) {
		set_field_back(field[i], A_UNDERLINE);
		field_opts_off(field[i], O_AUTOSKIP);
		set_field_userptr(field[i], &base[i]);
	}

	if (g_has_color) {
		init_pair(1, COLOR_BLUE, COLOR_BLACK);
		init_pair(2, COLOR_GREEN, COLOR_BLACK);
		init_pair(3, COLOR_CYAN, COLOR_BLACK);
		init_pair(4, COLOR_MAGENTA, COLOR_BLACK);
		init_pair(5, COLOR_RED, COLOR_BLACK);
	}

	form = new_form(field);
	if (!form)
		die("new form failed\n");

	scale_form(form, &rows, &cols);

	fields_win = newwin(rows + 3, cols + 6, 2, 0);
	keypad(fields_win, TRUE);

	binary_win = newwin(5, binary_field_size, 8, 0);
	keypad(binary_win, TRUE);
	box(binary_win, 0, 0);

	history_win = newwin(LINES - 15, COLS, 13, 0);
	box(history_win, 0, 0);

	rc = set_form_win(form, fields_win);
	if (rc != E_OK)
		die("set_form_win failed\n");

	rc = set_form_sub(form, derwin(fields_win, rows, cols, 2, 2));
	if (rc != E_OK)
		die("set_form_sub failed\n");

	set_current_field(form, field[0]);
	set_active_field(false);

	mvprintw(0, (COLS - strlen(title)) / 2, "%s", title);
	box(fields_win, 0, 0);
	mvwprintw(fields_win, 0, (cols + 6 - strlen(width_str)) / 2, "%s",
	          width_str);
	if (g_has_color)
		wattron(fields_win, COLOR_PAIR(2));
	mvwprintw(fields_win, 1, dec_pos + 2, "Decimal:");
	mvwprintw(fields_win, 1, hex_pos + 2, "Hexdecimal:");
	mvwprintw(fields_win, 1, oct_pos + 2, "Octal:");
	if (g_has_color)
		wattroff(binary_win, COLOR_PAIR(2));

	cmd_win = newwin(1, COLS, LINES - 1, 0);

	wrefresh(fields_win);
	update_binary();
	update_history_win();

	wrefresh(cmd_win);
	refresh();

	rc = post_form(form);
	if (rc != E_OK)
		die("post_form failed: %d\n", rc);

	update_fields(-1);
}

void unpaint_screen(void)
{
	int i;

	unpost_form(form);
	free_form(form);
	for (i = 0; i < 3; i++)
		free_field(field[i]);
	delwin(fields_win);
	delwin(binary_win);
	delwin(history_win);
	delwin(cmd_win);
	clear();
	refresh();
}

int start_interactive(uint64_t start)
{
	int ch;
	uint64_t tmp_val;
	MEVENT event;

	g_val = start;

	init_terminal();
	init_readline();
	mousemask(BUTTON1_CLICKED, NULL);

	refresh();

	set_fields_width(g_width);

	paint_screen();
	last_win = active_win = FIELDS_WIN;

	while (true) {
		if (g_leave_req)
			break;

		ch = wgetch(get_win(active_win));
		LOG("%d window ch= %d\n", active_win, ch);

		if (active_win == COMMAND_WIN) {
			process_cmd(ch);
			continue;
		}

		switch (ch) {
		case KEY_MOUSE:
			if(getmouse(&event) == OK){
				process_mouse(&event);
			}
			break;

				
		case KEY_F(1):
			unpaint_screen();
			show_help();
			paint_screen();
			break;
		case 'q':
		case 'Q':
			g_leave_req = true;
			continue;
		case KEY_RESIZE:
			LOG("Terminal resize\n");
			unpaint_screen();
			paint_screen();
			rl_resize_terminal();
			break;
		case '!':
			unpaint_screen();
			set_fields_width(8);
			if (bit_pos > 7)
				bit_pos = 7;
			paint_screen();
			break;
		case '@':
			unpaint_screen();
			set_fields_width(16);
			if (bit_pos > 15)
				bit_pos = 15;
			paint_screen();
			break;
		case '$':
			unpaint_screen();
			set_fields_width(32);
			if (bit_pos > 31)
				bit_pos = 31;
			paint_screen();
			break;
		case '*':
			unpaint_screen();
			set_fields_width(64);
			paint_screen();
			break;
		case '~':
			unpaint_screen();
			g_val = ~g_val & MASK(g_width);
			paint_screen();
			break;
		case 'r':
			unpaint_screen();
			switch (g_width) {
				case 16:
					g_val = __builtin_bswap16(g_val);
					break;
				case 32:
					g_val = __builtin_bswap32(g_val);
					break;
				case 64:
					g_val = __builtin_bswap64(g_val);
					break;
			}
			paint_screen();
			break;
		case '>':
			unpaint_screen();
			g_val = g_val >> 1;
			paint_screen();
			break;
		case '<':
			tmp_val = (g_val << 1);

			if (tmp_val < MASK(g_width)) {
				unpaint_screen();
				g_val = tmp_val;
				paint_screen();
			}
			break;
		case ':':
			last_win = active_win;
			active_win = COMMAND_WIN;
			if (last_win == FIELDS_WIN) {
				set_active_field(true);
				form_driver(form, REQ_VALIDATION);
				wrefresh(fields_win);
			} else if (last_win == BINARY_WIN)
				position_binary_curser(bit_pos, -1);

			curs_set(1);
			keypad(stdscr, FALSE);
			intrflush(NULL, FALSE);
			readline_redisplay();
			break;
		default:
			if (active_win == BINARY_WIN)
				process_binary(ch);
			else if (active_win == FIELDS_WIN)
				process_fields(ch);
		}

		if (active_win == BINARY_WIN) {
			LOG("Bit pos = %u\n", bit_pos);
			position_binary_curser(0, bit_pos);
		}

	}

	unpaint_screen();
	/* OK. this is really weird, it seems that by calling
	 * deinit_readline() before exiting is breaking bash
	 * afterwards and must be fixed by typing reset in the
	 * terminal.
	 * I couldn't figure this one out, let's just leave it like
	 * this, hopefully it won't affect other env */

	/* deinit_readline(); */

	flush_history();
	deinit_terminal();

	return 0;
}
