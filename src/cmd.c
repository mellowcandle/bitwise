/* Copyright 2019
 * Ramon Fried <ramon.fried@gmail.com>
 */
#include <string.h>
#include <inttypes.h>
#include <ctype.h>
#include "bitwise.h"
#include "shunting-yard.h"

#define MAX_TOKENS 4

static int cmd_clear(char **argv, int argc);
static int cmd_help(char **argv, int argc);
static int cmd_set_width(char **argv, int argc);
static int cmd_set_output(char **argv, int argc);

struct cmd {
	const char *name;
	int min_args;
	int max_args;
	int (*func)(char **argv, int argc);
};

static struct cmd cmds[] = {
	{"clear", 0, 0, cmd_clear},
	{"help", 0, 0, cmd_help},
	{"width", 1, 1, cmd_set_width},
	{"output", 1, 1, cmd_set_output},
};

static int get_cmd(const char *cmd_name)
{
	size_t i = 0;

	for (i = 0; i < ARRAY_SIZE(cmds); i++) {
		LOG("comparing %s to command %s\n", cmd_name, cmds[i].name);
		if (!strncmp(cmds[i].name, cmd_name, strlen(cmds[i].name)))
			return i;
	}

	return -1;
}

void show_error(Status status)
{
	char *message = NULL;

	switch (status) {
	case ERROR_SYNTAX:
		message = "Syntax error";
		break;
	case ERROR_OPEN_PARENTHESIS:
		message = "Missing parenthesis";
		break;
	case ERROR_CLOSE_PARENTHESIS:
		message = "Extra parenthesis";
		break;
	case ERROR_UNRECOGNIZED:
		message = "Unknown character";
		break;
	case ERROR_NO_INPUT:
		message = "Empty expression";
		break;
	case ERROR_UNDEFINED_FUNCTION:
		message = "Unknown function";
		break;
	case ERROR_FUNCTION_ARGUMENTS:
		message = "Missing function arguments";
		break;
	case ERROR_UNDEFINED_CONSTANT:
		message = "Unknown constant";
		break;
	case ERROR_WRONG_ARGUMENTS:
		message = "Wrong arguments";
		break;
	default:
		message = "Unknown error";
	}

	werase(cmd_win);
	mvwprintw(cmd_win, 0, 0, "%s", message);
	append_to_history(message, TYPE_OUTPUT_ERROR);
	wrefresh(cmd_win);
}

static int is_whitespace(const char *string)
{
	size_t i;

	for (i = 0; i < strlen(string); i++)
		if (!isspace(string[i]))
			return false;

	return true;
}

static int parse_cmd(char *cmdline)
{
	static char *tokens[MAX_TOKENS];
	int cmd_entry;
	int i = 0;
	int rc = 0;
	uint64_t result;

	if (is_whitespace(cmdline))
		return 0;

	LOG("got command: %s\n", cmdline);

	/* First let's see if this is a command */
	cmd_entry = get_cmd(cmdline);
	if (cmd_entry >= 0) {
		/* It looks like a command, let's tokenize this */
		append_to_history(cmdline, TYPE_INPUT_COMMAND);

		tokens[i] = strtok(cmdline, " ");
		LOG("%s\n", tokens[i]);
		do {
			i++;
			tokens[i] = strtok(NULL, " ");
			LOG("%s\n", tokens[i]);
		} while (tokens[i] != NULL && (i) < MAX_TOKENS);

		LOG("Finished tokenizing %d tokens\n", i);

		if ((i - 1 >= cmds[cmd_entry].min_args) &&
		    (i - 1 <= cmds[cmd_entry].max_args))
			rc = cmds[cmd_entry].func(&tokens[1], i - 1);
		else {
			werase(cmd_win);
			mvwprintw(cmd_win, 0, 0, "%s: Invalid parameter(s)", tokens[0]);
			wrefresh(cmd_win);
			LOG("Invalid parameters\n");
			return -1;
		}
		if (rc) {
			show_error(rc);
			return -1;
		}

	} else {
		append_to_history(cmdline, TYPE_INPUT_EXPRESSION);
		Status status = shunting_yard(cmdline, &result);
		if (status != OK) {
			show_error(status);
			return -1;
		} else {
			char result_string[256];
			g_val = result;

			if (g_output == CMD_OUTPUT_ALL) {
				sprintf_type(result, result_string,
					     CMD_OUTPUT_DECIMAL);
				append_to_history(result_string,
						  TYPE_OUTPUT_RESULT);
				sprintf_type(result, result_string,
					     CMD_OUTPUT_HEXADECIMAL);
				append_to_history(result_string,
						  TYPE_OUTPUT_RESULT);
				sprintf_type(result, result_string,
					     CMD_OUTPUT_OCTAL);
				append_to_history(result_string,
						  TYPE_OUTPUT_RESULT);
				sprintf_type(result, result_string,
					     CMD_OUTPUT_BINARY);
				append_to_history(result_string,
						  TYPE_OUTPUT_RESULT);
				update_binary();
				update_fields(-1);
			} else {
				sprintf_type(result, result_string, g_output);
				update_binary();
				update_fields(-1);
				append_to_history(result_string,
						  TYPE_OUTPUT_RESULT);
			}
		}
	}

	return 0;
}

static int readline_input_avail(void)
{
	return g_input_avail;
}

static int readline_getc(FILE *dummy)
{
	g_input_avail = false;
	return g_input;
}

static void got_command(char *line)
{
	int rc;
	/* Handle exit commands immediately */
	if (!line) {
		/* Handle CTL-D */
		g_leave_req = true;
		return;
	}

	if (strcmp("q", line) == 0)
		g_leave_req = true;

	if (*line)
		add_history(line);

	rc = parse_cmd(line);
	free(line);

	if (!rc) {
		keypad(stdscr, FALSE);
		werase(cmd_win);
		wrefresh(cmd_win);
	}

	return;
}

void readline_redisplay(void)
{
	if (active_win != COMMAND_WIN)
		return;

	werase(cmd_win);
	mvwprintw(cmd_win, 0, 0, "%s%s", rl_display_prompt, rl_line_buffer);
	wmove(cmd_win, 0, rl_point + 1);
	wrefresh(cmd_win);
}


void init_readline(void)
{
	rl_catch_signals = 0;
	rl_catch_sigwinch = 0;
	rl_deprep_term_function = NULL;
	rl_prep_term_function = NULL;
	rl_change_environment = 0;
	rl_getc_function = readline_getc;
	rl_input_available_hook = readline_input_avail;
	rl_redisplay_function = readline_redisplay;
	//rl_bind_key('\t', rl_insert);
	rl_callback_handler_install(":", got_command);

}

void deinit_readline(void)
{
	rl_callback_handler_remove();
}

void process_cmd(int ch)
{
	int new_char;

	LOG("Process cmd %u\n", ch);

	if (ch == 27) {
		nodelay(get_win(active_win), true);
		new_char = wgetch(get_win(active_win));
		nodelay(get_win(active_win), false);

		if (new_char == ERR) {
			LOG("IT's a real escape\n");
			active_win = last_win;
			if (active_win == FIELDS_WIN) {
				set_active_field(false);
				form_driver(form, REQ_END_LINE);
				form_driver(form, REQ_VALIDATION);
				curs_set(1);
				wrefresh(fields_win);
			} else if (active_win == BINARY_WIN) {
				curs_set(0);
				position_binary_curser(0, bit_pos);
				wrefresh(binary_win);
			}
			keypad(stdscr, FALSE);
			werase(cmd_win);
			wrefresh(cmd_win);
			return;
		} else
			ungetch(new_char);
	}

	/* If TAB || BACKSPACE (to -1) */
	if (ch == 127 && !rl_point) {
		LOG("Detected exit cmd\n");
		active_win = last_win;
		if (active_win == FIELDS_WIN) {
			set_active_field(false);
			form_driver(form, REQ_END_LINE);
			form_driver(form, REQ_VALIDATION);
			curs_set(1);
			wrefresh(fields_win);
		} else if (active_win == BINARY_WIN) {
			curs_set(0);
			position_binary_curser(0, bit_pos);
			wrefresh(binary_win);
		}
		keypad(stdscr, FALSE);
		werase(cmd_win);
		wrefresh(cmd_win);
		return;
	}

	g_input = ch;
	g_input_avail = true;
	rl_callback_read_char();
}

static int cmd_clear(char **argv, int argc)
{
	LOG("%s: argc %d\n", __func__, argc);
	flush_history();
	update_history_win();
	return 0;
}

static int cmd_set_width(char **argv, int argc)
{
	LOG("%s: argc %d\n", __func__, argc);
	if (!strcmp("64", argv[0]))
		set_fields_width(64);
	else if (!strcmp("32", argv[0]))
		set_fields_width(32);
	else if (!strcmp("16", argv[0]))
		set_fields_width(16);
	else if (!strcmp("8", argv[0]))
		set_fields_width(8);
	else
		return ERROR_WRONG_ARGUMENTS;

	unpaint_screen();
	paint_screen();

	return 0;
}

static int cmd_set_output(char **argv, int argc)
{
	LOG("%s: argc %d\n", __func__, argc);
	if (!strcmp("hex", argv[0]))
		g_output = CMD_OUTPUT_HEXADECIMAL;
	else if (!strcmp("decimal", argv[0]))
		g_output = CMD_OUTPUT_DECIMAL;
	else if (!strcmp("octal", argv[0]))
		g_output = CMD_OUTPUT_OCTAL;
	else if (!strcmp("binary", argv[0]))
		g_output = CMD_OUTPUT_BINARY;
	else if (!strcmp("all", argv[0]))
		g_output = CMD_OUTPUT_ALL;
	else
		return ERROR_WRONG_ARGUMENTS;

	return 0;
}

static int cmd_help(char **argv, int argc)
{
	unpaint_screen();
	curs_set(0);
	show_help();
	curs_set(2);
	paint_screen();

	return 0;
}

