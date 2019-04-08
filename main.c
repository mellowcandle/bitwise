#include <stdlib.h>
#include <form.h>
#include <ctype.h>
#include <stdint.h>
#include <assert.h>
#if 1
static FIELD *field[5];
static FORM  *form;
static uint64_t val;

static void init_terminal(void)
{
	initscr();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);
}

int base[4] = {
	10,
	16,
	8,
	10,
};

static void deinit_terminal(void)
{
	endwin();
}


static void lltostr(uint64_t val, char *buf, int base)
{
	switch (base) {
	case 10:
		sprintf(buf, "%llu", val);
		return;
	case 16:
		sprintf(buf, "%llx", val);
		return;
	case 8:
		sprintf(buf, "%llo", val);
		return;
	}
}

static void update_fields(int index)
{
	char *number[32];
	int *base;

	for (int i=0; i < 4; i++) {
		if (i == index)
			break;
		base = field_userptr(field[i]);
		lltostr(val, number, *base);
		set_field_buffer(field[i], 0, number);
	}
	form_driver(form, REQ_VALIDATION);
	refresh();
}

int main(int argc, char *argv[])
{

	int ch;
	char *buffer;

	init_terminal();
/* Initialize the fields */
	field[0] = new_field(1, 15, 4, 20, 0, 0);
	field[1] = new_field(1, 15, 6, 24, 0, 0);
	field[2] = new_field(1, 15, 8, 18, 0, 0);
	field[3] = new_field(1, 15, 10, 20, 0, 0);
	field[4] = NULL;

	for (int i=0; i < 4; i++) {
		set_field_back(field[i], A_UNDERLINE);
		field_opts_off(field[i], O_AUTOSKIP );
//		set_field_buffer(field[i], 0, "0");
		set_field_userptr(field[i], &base[i]);
	}

	form = new_form(field);
	post_form(form);
	refresh();

	mvprintw(4, 10, "Decimal:");
	mvprintw(6, 10, "Hexdecimal:");
	mvprintw(8, 10, "Octal:");
	mvprintw(10, 10, "Binary:");
	set_current_field(form, field[0]);
	refresh();

	int times = 0;

	while((ch = getch()) != KEY_F(1)) {
//			mvprintw(LINES -1, 0, "place: %u ch= %d", times++, ch);
		switch(ch) {
		case KEY_DOWN:
			/* Go to next field */
			form_driver(form, REQ_NEXT_FIELD);
			/* Go to the end of the present buffer */
			/* Leaves nicely at the last character */
			form_driver(form, REQ_END_LINE);
			break;
		case KEY_UP:
			/* Go to previous field */
			form_driver(form, REQ_PREV_FIELD);
			form_driver(form, REQ_END_LINE);
			break;
		default:
			{
				int *cur_base;
				FIELD *tmp_field;

//				if (!isdigit(ch))
//					break;

				if (ch == KEY_BACKSPACE)
					form_driver(form, REQ_DEL_PREV);
				else
					form_driver(form, ch);

				form_driver(form, REQ_VALIDATION);
				tmp_field = current_field(form);
				buffer = field_buffer(tmp_field, 0);
				assert(buffer);
				cur_base = field_userptr(tmp_field);
				val = strtoll(buffer, NULL, *cur_base);
				update_fields(field_index(tmp_field));
				mvprintw(LINES - 1, 0, "base = %u buffer = %s val = %llu",*cur_base, buffer, val);
				break;
			}
		}

		refresh();

	}
	unpost_form(form);
	free_form(form);
	for (int i=0; i < 4; i++)
		free_field(field[i]);

	deinit_terminal();

	return 0;
}

#else



#include <form.h>

int main()
{	FIELD *field[3];
	FORM  *my_form;
	int ch;
	
	/* Initialize curses */
	initscr();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);

	/* Initialize the fields */
	field[0] = new_field(1, 10, 4, 18, 0, 0);
	field[1] = new_field(1, 10, 6, 18, 0, 0);
	field[2] = NULL;

	/* Set field options */
	set_field_back(field[0], A_UNDERLINE);	/* Print a line for the option	*/
	field_opts_off(field[0], O_AUTOSKIP);	/* Don't go to next field when this */
						/* Field is filled up		*/
	set_field_back(field[1], A_UNDERLINE); 
	field_opts_off(field[1], O_AUTOSKIP);

	/* Create the form and post it */
	my_form = new_form(field);
	post_form(my_form);
	refresh();
	
	mvprintw(4, 10, "Value 1:");
	mvprintw(6, 10, "Value 2:");
	refresh();

	/* Loop through to get user requests */
	while((ch = getch()) != KEY_F(1))
	{	switch(ch)
		{	case KEY_DOWN:
				/* Go to next field */
				form_driver(my_form, REQ_NEXT_FIELD);
				/* Go to the end of the present buffer */
				/* Leaves nicely at the last character */
				form_driver(my_form, REQ_END_LINE);
				break;
			case KEY_UP:
				/* Go to previous field */
				form_driver(my_form, REQ_PREV_FIELD);
				form_driver(my_form, REQ_END_LINE);
				break;
			default:
				/* If this is a normal character, it gets */
				/* Printed				  */	
				form_driver(my_form, ch);
				break;
		}
	}

	/* Un post form and free the memory */
	unpost_form(my_form);
	free_form(my_form);
	free_field(field[0]);
	free_field(field[1]); 

	endwin();
	return 0;
}

#endif
