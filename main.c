#include <stdlib.h>
#include <form.h>
#include <ctype.h>
#include <stdint.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/stat.h>

#define TRACE

#define MAX_DEC_DIGITS 19
#define MAX_HEX_DIGITS 16
#define MAX_OCTA_DIGITS 32

#ifdef TRACE
#define LOG(...) fprintf(fd, __VA_ARGS__);fflush(fd)
#else
#define LOG(...)
#endif

#if 1
static FIELD *field[5];
static FORM  *form;
static uint64_t val;
FILE *fd;

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
	2,
};

static void deinit_terminal(void)
{
	endwin();
}


static void lltostr(uint64_t val, char *buf, int base)
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
	for (int i=0; i < 4; i++) {
		if (i == index)
			continue;
		base = field_userptr(field[i]);
		lltostr(val, number, *base);
		LOG("updating field %d\n", i);
		set_field_buffer(field[i], 0, number);
	}
	form_driver(form, REQ_VALIDATION);
	refresh();
}

int validate_input(int ch, int base)
{
	switch (base) {
	case 2:
		 LOG("validating 2\n");
		if (ch == '0' || ch == '1')
			return 0;
		break;
	case 8:
		 LOG("validating 8\n");
		if (ch >= '0' && ch <= '7')
			return 0;
		break;
	case 16:
		 LOG("validating 16\n");
		if ((ch >= '0' && ch <= '9') ||
		     (ch >= 'A' && ch <= 'F') ||
		     (ch >= 'a' && ch <= 'f'))
			 return 0;
		 break;
	case 10:
		 LOG("validating 10\n");
		 if (isdigit(ch))
			 return 0;
		 break;
	default:
		 LOG("What the fuck\n");
		 break;
	}


	LOG("validating input failed\n");
	return 1;
}

int main(int argc, char *argv[])
{

	int ch;
	char *buffer;

#ifdef TRACE
	fd = fopen("log.txt", "w");
#endif
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
		LOG("times: %u ch= %d\n", times++, ch);

		switch(ch) {
		case KEY_DOWN:
			LOG("Key down\n");
			/* Go to next field */
			form_driver(form, REQ_NEXT_FIELD);
			/* Go to the end of the present buffer */
			/* Leaves nicely at the last character */
			form_driver(form, REQ_END_LINE);
			break;
		case KEY_UP:
			LOG("Key up\n");
			/* Go to previous field */
			form_driver(form, REQ_PREV_FIELD);
			form_driver(form, REQ_END_LINE);
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

		refresh();

	}

	unpost_form(form);
	free_form(form);
	for (int i=0; i < 4; i++)
		free_field(field[i]);

	fclose(fd);
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
