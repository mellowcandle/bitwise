/* Copyright 2019
 * Ramon Fried <ramon.fried@gmail.com
 */

#include <stdlib.h>
#include <form.h>
#include <ctype.h>
#include <stdint.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "bitwise.h"


#define MAX_DEC_DIGITS 19
#define MAX_HEX_DIGITS 16
#define MAX_OCTA_DIGITS 32

#ifdef TRACE
FILE *fd;
#endif

#if 1
static FIELD *field[5];
static FORM  *form;
static uint64_t val;

WINDOW *fields_win;

int base[4] = {
	10,
	16,
	8,
	2,
};



static void update_fields(int index)
{
	FIELD *tmp_field = current_field(form);
	int *cur_base = field_userptr(tmp_field);
	char *buffer;
	char number[64];
	int *base;

	buffer = field_buffer(tmp_field, 0);
	assert(buffer);
	val = strtoll(buffer, NULL, *cur_base);
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
}

void set_active_field()
{
	set_field_fore(current_field(form), COLOR_PAIR(1));/* Put the field with blue background */
	set_field_back(current_field(form), COLOR_PAIR(2));/* and white foreground (characters */

	for (int i=0; i < 3; i++) {
		if (field[i] == current_field(form))
			continue;
		set_field_fore(field[i], COLOR_PAIR(0));
		set_field_back(field[i], COLOR_PAIR(0));
	}
}
						/* are printed in white 	*/


int main(int argc, char *argv[])
{

	int ch;
	int rc;

#ifdef TRACE
	fd = fopen("log.txt", "w");
#endif
	init_terminal();
	refresh();
	fields_win = newwin(4, COLS, 0, 0);
	box(fields_win, 0, 0);
	wrefresh(fields_win);


/* Initialize the fields */
	field[0] = new_field(1, MAX_DEC_DIGITS, 2,
				10, 0, 0);

	field[1] = new_field(1, MAX_HEX_DIGITS, 2,
				40, 0, 0);
	field[2] = new_field(1, MAX_OCTA_DIGITS, 2,
				70, 0, 0);
	field[3] = NULL;
/*
	field[3] = new_field(1, 15, 10, 20, 0, 0);
	field[4] = NULL;
*/
	//for (int i=0; i < 4; i++) {
	for (int i=0; i < 3; i++) {
		set_field_back(field[i], A_UNDERLINE);
		field_opts_off(field[i], O_AUTOSKIP );
		set_field_buffer(field[i], 0, "0");
		set_field_userptr(field[i], &base[i]);
	}

	init_pair(1, COLOR_BLUE, COLOR_BLACK);
	init_pair(2, COLOR_BLUE, COLOR_BLACK);

	form = new_form(field);
	if (!form)
		die("new form failed\n");
	rc = set_form_win(form, fields_win);
	if (rc != E_OK)
		die("set_form_win failed\n");

	rc = set_form_sub(form, fields_win);
	if (rc != E_OK)
		die("set_form_sub failed\n");

	set_current_field(form, field[0]);
	set_active_field();
	rc = post_form(form);
	if (rc != E_OK)
		die("post_form failed: %d\n", rc);

	//refresh();
	mvwprintw(fields_win, 1, 10, "Decimal:");
	mvwprintw(fields_win, 1, 40, "Hexdecimal:");
	mvwprintw(fields_win, 1, 70, "Octal:");
//	mvprintw(10, 10, "Binary:");
	wmove(fields_win, 2, 10);
	wrefresh(fields_win);
	refresh();

	int times = 0;

	while((ch = getch()) != KEY_F(1)) {
		LOG("times: %u ch= %d\n", times++, ch);

		switch(ch) {
		case KEY_RIGHT:
			LOG("Key right\n");
			/* Go to next field */
			form_driver(form, REQ_NEXT_FIELD);
			/* Go to the end of the present buffer */
			/* Leaves nicely at the last character */
			form_driver(form, REQ_END_LINE);
			set_active_field();
			wrefresh(fields_win);
			break;
		case KEY_LEFT:
			LOG("Key left\n");
			/* Go to previous field */
			form_driver(form, REQ_PREV_FIELD);
			form_driver(form, REQ_END_LINE);
			set_active_field();
			wrefresh(fields_win);
			break;
		case KEY_BACKSPACE:
		case 127:
			LOG("Backspace\n");
			form_driver(form, REQ_DEL_PREV);
			form_driver(form, REQ_VALIDATION);
			update_fields(field_index(current_field(form)));
			break;
		default:
			{
				FIELD *tmp_field = current_field(form);
				int *cur_base = field_userptr(tmp_field);

				LOG("default char\n");

				if (validate_input(ch, *cur_base))
					break;

				form_driver(form, ch);
				form_driver(form, REQ_VALIDATION);
				update_fields(field_index(tmp_field));
				break;
			}
		}
		wrefresh(fields_win);
		refresh();
	}
	getch();
	unpost_form(form);
	free_form(form);
	for (int i=0; i < 4; i++)
		free_field(field[i]);

	fclose(fd);
	deinit_terminal();

	return 0;
}

#else



WINDOW *create_newwin(int height, int width, int starty, int startx);
void destroy_win(WINDOW *local_win);

int main(int argc, char *argv[])
{	WINDOW *my_win;
	int startx, starty, width, height;
	int ch;

	initscr();			/* Start curses mode 		*/
	cbreak();			/* Line buffering disabled, Pass on
					 * everty thing to me 		*/
	keypad(stdscr, TRUE);		/* I need that nifty F1 	*/

	height = 3;
	width = 10;
	starty = (LINES - height) / 2;	/* Calculating for a center placement */
	startx = (COLS - width) / 2;	/* of the window		*/
	printw("Press F1 to exit");
	refresh();
	my_win = create_newwin(height, width, starty, startx);

	while((ch = getch()) != KEY_F(1))
	{	switch(ch)
		{	case KEY_LEFT:
				destroy_win(my_win);
				my_win = create_newwin(height, width, starty,--startx);
				break;
			case KEY_RIGHT:
				destroy_win(my_win);
				my_win = create_newwin(height, width, starty,++startx);
				break;
			case KEY_UP:
				destroy_win(my_win);
				my_win = create_newwin(height, width, --starty,startx);
				break;
			case KEY_DOWN:
				destroy_win(my_win);
				my_win = create_newwin(height, width, ++starty,startx);
				break;	
		}
	}
		
	endwin();			/* End curses mode		  */
	return 0;
}

WINDOW *create_newwin(int height, int width, int starty, int startx)
{	WINDOW *local_win;

	local_win = newwin(height, width, starty, startx);
	box(local_win, 0 , 0);		/* 0, 0 gives default characters 
					 * for the vertical and horizontal
					 * lines			*/
	wrefresh(local_win);		/* Show that box 		*/

	return local_win;
}

void destroy_win(WINDOW *local_win)
{	
	/* box(local_win, ' ', ' '); : This won't produce the desired
	 * result of erasing the window. It will leave it's four corners 
	 * and so an ugly remnant of window. 
	 */
	wborder(local_win, ' ', ' ', ' ',' ',' ',' ',' ',' ');
	/* The parameters taken are 
	 * 1. win: the window on which to operate
	 * 2. ls: character to be used for the left side of the window 
	 * 3. rs: character to be used for the right side of the window 
	 * 4. ts: character to be used for the top side of the window 
	 * 5. bs: character to be used for the bottom side of the window 
	 * 6. tl: character to be used for the top left corner of the window 
	 * 7. tr: character to be used for the top right corner of the window 
	 * 8. bl: character to be used for the bottom left corner of the window 
	 * 9. br: character to be used for the bottom right corner of the window
	 */
	wrefresh(local_win);
	delwin(local_win);
}
#endif
