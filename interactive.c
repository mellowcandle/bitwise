/* Copyright 2019
 * Ramon Fried <ramon.fried@gmail.com>
 */

#include <stdlib.h>
#include <form.h>
#include <ctype.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "bitwise.h"

#define MAX_DEC_DIGITS 19
#define MAX_HEX_DIGITS 16
#define MAX_OCTA_DIGITS 32

#define FIELDS_VIEW 0
#define BINARY_VIEW 1

#define CLEAR_BIT 0
#define SET_BIT 1
#define TOGGLE_BIT 2

#ifdef TRACE
FILE *fd;
#endif

static void update_fields(int index);
void position_binary_curser(int previous_pos, int next_pos);
static void update_binary();

char title[] = "Bitwise manipulator version 0.1 By Ramon Fried";

static FIELD *field[5];
WINDOW *fields_win;
WINDOW *binary_win;
static FORM  *form;

static uint64_t val;

static int bit_pos;
static int view = FIELDS_VIEW;

char binary_field[180];
int base[4] = {
	10,
	16,
	8,
	2,
};

static void update_bit(int pos, int op)
{
	LOG("update bit: %u %u\n", pos, op);
	if (op == SET_BIT)
		val |= BIT(63-pos);
	else if (op == CLEAR_BIT)
		val &= ~(BIT(63 - pos));
	else
		val ^= BIT(63-pos);

	update_binary();
}

static void update_binary()
{
	int i;
	int pos = 0;

	for (i = 64; i > 0; i--) {
		if ((i % 8 == 0) && (i != 64)) {
			binary_field[pos] = '|';
			binary_field[pos+1] = ' ';
			pos += 2;
		}
		if (val & BIT(i-1))
			binary_field[pos] = '1';
		else
			binary_field[pos] = '0';
		binary_field[pos+1] = ' ';
		pos+=2;
	}

	binary_field[pos] = '\0';
	mvwprintw(binary_win, 1, 2, "%s", binary_field);

	pos = 6;
	if (has_color)
		wattron(binary_win, COLOR_PAIR(3));
	for (i = 0; i < 8; i++, pos += 18)
		mvwprintw(binary_win, 2, pos, "%2d - %2d", 63 - (i * 8));
	if (has_color)
		wattroff(binary_win, COLOR_PAIR(3));
	wrefresh(binary_win);
}

static void update_fields(int index)
{
	FIELD *tmp_field = current_field(form);
	int *cur_base = field_userptr(tmp_field);
	char *buffer;
	char number[64];
	int *base;

	if (index != -1) {
		buffer = field_buffer(tmp_field, 0);
		assert(buffer);
		val = strtoll(buffer, NULL, *cur_base);
	}
	LOG("Val = %lu\n", val);
	for (int i=0; i < 3; i++) {
		if (i == index)
			continue;
		base = field_userptr(field[i]);
		lltostr(val, number, *base);
		LOG("updating field %d\n", i);
		set_field_buffer(field[i], 0, number);
	}
	form_driver(form, REQ_VALIDATION);
	wrefresh(fields_win);
	refresh();
}

void set_active_field(bool none)
{
	if (!has_color)
		return;

	if (!none) {
		set_field_fore(current_field(form), COLOR_PAIR(1));/* Put the field with blue background */
		set_field_back(current_field(form), COLOR_PAIR(2));/* and white foreground (characters */
	}

	for (int i=0; i < 3; i++) {
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

	pos = 2 + (2 * next_pos) + (2 * (next_pos / 8));
	mvwchgat(binary_win, 1, pos, 1, A_UNDERLINE, COLOR_PAIR(0), NULL);

	mvprintw(LINES - 2, 0, "bit %u  \n", 63 - next_pos);
	wrefresh(binary_win);
	refresh();
}

void process_binary(int ch)
{
	switch(ch) {
	case KEY_RIGHT:
	case 'l':
		LOG("Key right\n");
		if (bit_pos == 63) {
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
		LOG("Key up\n");
		view = FIELDS_VIEW;
		set_active_field(false);
		form_driver(form, REQ_END_LINE);
		form_driver(form, REQ_VALIDATION);
		wrefresh(fields_win);
		break;
	case KEY_BACKSPACE:
	case 127:
		LOG("Backspace\n");
		if (bit_pos != 0) {
			bit_pos--;
			update_bit(bit_pos, CLEAR_BIT);
			update_fields(-1);
		}
		else
			beep();
		break;
	default:
		if (ch == '1') {
			update_bit(bit_pos, SET_BIT);
			position_binary_curser(bit_pos, (bit_pos + 1) == 63 ?
					       (bit_pos) : (bit_pos + 1));
			update_fields(-1);
			if (bit_pos != 63)
				bit_pos++;
			else
				beep();
			break;
		} else if (ch == '0') {
			update_bit(bit_pos, CLEAR_BIT);
			position_binary_curser(bit_pos, (bit_pos + 1) == 63 ?
					       (bit_pos) : (bit_pos + 1));
			update_fields(-1);
			if (bit_pos != 63)
				bit_pos++;
			else
				beep();
			break;
		} else if (ch == ' ') {
			update_bit(bit_pos, TOGGLE_BIT);
			update_fields(-1);
		}
		break;
	}
}

void process_fields(int ch)
{
	FIELD *tmp_field;
	int *cur_base;

	switch(ch) {
	case KEY_RIGHT:
	case 'l':
		LOG("Key right\n");
		/* Go to next field */
		form_driver(form, REQ_NEXT_FIELD);
		/* Go to the end of the present buffer */
		/* Leaves nicely at the last character */
		form_driver(form, REQ_END_LINE);
		set_active_field(false);
		wrefresh(fields_win);
		break;
	case KEY_LEFT:
	case 'h':
		LOG("Key left\n");
		/* Go to previous field */
		form_driver(form, REQ_PREV_FIELD);
		form_driver(form, REQ_END_LINE);
		set_active_field(false);
		wrefresh(fields_win);
		break;
	case KEY_DOWN:
	case 'j':
		LOG("Key down\n");
		view = BINARY_VIEW;
		set_active_field(true);
		form_driver(form, REQ_VALIDATION);
		wrefresh(fields_win);
		position_binary_curser(0, bit_pos);
		break;
	case KEY_BACKSPACE:
	case 127:
		LOG("Backspace\n");
		LOG("Userptr field %d\n", *((int *)current_field(form)->usrptr));
		form_driver(form, REQ_DEL_PREV);
		form_driver(form, REQ_VALIDATION);
		update_fields(field_index(current_field(form)));
		update_binary();
		break;
	default:
		LOG("default char\n");

		tmp_field = current_field(form);
		cur_base = field_userptr(tmp_field);

		if (validate_input(ch, *cur_base))
			break;

		form_driver(form, ch);
		form_driver(form, REQ_VALIDATION);
		update_fields(field_index(tmp_field));
		update_binary();
		break;
	}
}

int start_interactive(void)
{
	int ch, rows, cols;
	int rc;

#ifdef TRACE
	fd = fopen("log.txt", "w");
#endif
	init_terminal();
	refresh();

	/* Initialize the fields */
	field[0] = new_field(1, MAX_DEC_DIGITS, 1,
			     10, 0, 0);

	field[1] = new_field(1, MAX_HEX_DIGITS, 1,
			     40, 0, 0);
	field[2] = new_field(1, MAX_OCTA_DIGITS, 1,
			     70, 0, 0);
	field[3] = NULL;

	for (int i=0; i < 3; i++) {
		set_field_back(field[i], A_UNDERLINE);
		field_opts_off(field[i], O_AUTOSKIP );
		set_field_buffer(field[i], 0, "0");
		set_field_userptr(field[i], &base[i]);
	}

	if (has_color) {
		init_pair(1, COLOR_BLUE, COLOR_BLACK);
		init_pair(2, COLOR_BLUE, COLOR_BLACK);
		init_pair(3, COLOR_GREEN, COLOR_BLACK);
	}

	form = new_form(field);
	if (!form)
		die("new form failed\n");

	scale_form(form, &rows, &cols);

	fields_win = newwin(rows + 3, cols + 3, 2, (COLS - cols) / 2);
	//	fields_win = subwin(stdscr, rows + 3, cols + 3, 0, 4);
	keypad(fields_win, TRUE);

	binary_win = newwin(4, 145, 8, (COLS - 145) / 2);
	//	binary_win = subwin(stdscr, 4, 145, 8,1);
	box(binary_win, 0, 0);

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
	mvwprintw(fields_win, 1, 10, "Decimal:");
	mvwprintw(fields_win, 1, 40, "Hexdecimal:");
	mvwprintw(fields_win, 1, 70, "Octal:");

	wrefresh(fields_win);
	update_binary();
	refresh();

	rc = post_form(form);
	if (rc != E_OK)
		die("post_form failed: %d\n", rc);

	while((ch = wgetch(fields_win)) != 'q') {
		LOG("ch= %d\n", ch);

		if (view == BINARY_VIEW)
			process_binary(ch);
		else
			process_fields(ch);

		refresh();
	}

	unpost_form(form);
	free_form(form);
	for (int i=0; i < 4; i++)
		free_field(field[i]);
	delwin(fields_win);
	delwin(binary_win);

#ifdef TRACE
	fclose(fd);
#endif
	deinit_terminal();

	return 0;
}
