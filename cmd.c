/* Copyright 2019
 * Ramon Fried <ramon.fried@gmail.com>
 */
#include <string.h>
#include "bitwise.h"

#define MAX_TOKENS 4

static int cmd_set_width(char **argv, int argc);

struct cmd {
	const char *name;
	int min_args;
	int max_args;
	int (*func)(char **argv, int argc);
};

static struct cmd cmds[] = {
	{"width", 1, 1, cmd_set_width},
};

static int get_cmd(const char *cmd_name)
{
	int i = 0;

	for (i = 0; i < ARRAY_SIZE(cmds); i++) {
		if (!strcmp(cmds[i].name, cmd_name)) {
			return i;
		}
	}

	return -1;
}

static int parse_cmd(char *cmdline)
{
	static char *tokens[MAX_TOKENS];
	int cmd_entry;
	int i = 0;
	int rc;
	uint64_t a;

	LOG("got command: %s\n", cmdline);

	tokens[i] = strtok(cmdline, " ");
	LOG("%s\n", tokens[i]);

	do {
		i++;
		tokens[i] = strtok(NULL, " ");
		LOG("%s\n", tokens[i]);
	} while(tokens[i] != NULL && (i) < MAX_TOKENS);

	LOG("Finished tokenizing %d tokens\n", i);

	calc(i, tokens);
	cmd_entry = get_cmd(tokens[0]);
	if (cmd_entry >= 0) {
		if ((i - 1 >= cmds[cmd_entry].min_args) &&
		    (i - 1 <= cmds[cmd_entry].max_args))
			rc = cmds[cmd_entry].func(&tokens[1], i-1);
	} else if (!parse_input(tokens[0], &a))
		LOG("It's a number\n");

	if (rc) {
		LOG("Unsupported parameter\n");
		werase(cmd_win);
		mvwprintw(cmd_win, 0, 0, "%s: Unsupported parameter", tokens[0]);
		wrefresh(cmd_win);
		refresh();
		return -1;
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

	active_win = last_win;
	if (active_win == FIELDS_WIN) {
		set_active_field(false);
		wrefresh(fields_win);

	} else if (active_win == BINARY_WIN) {
		position_binary_curser(0, bit_pos);
	}

	if (!rc) {
		keypad(stdscr, FALSE);
		curs_set(0);
		werase(cmd_win);
		wrefresh(cmd_win);
	}
	return;
}

void readline_redisplay(void)
{
	if (active_win != COMMAND_WIN)
		return;

	size_t prompt_width = strlen(rl_display_prompt);
	size_t cursor_col = prompt_width + strlen(rl_line_buffer);
	LOG("Placing curser at %d\n", cursor_col);
	werase(cmd_win);
	mvwprintw(cmd_win, 0, 0, "%s%s", rl_display_prompt, rl_line_buffer);
	wmove(cmd_win, 0, cursor_col);
	curs_set(2);
	wrefresh(cmd_win);
	refresh();
}


void init_readline(void)
{
	rl_catch_signals = 0;
	rl_catch_sigwinch = 0;
	rl_change_environment = 0;
	rl_getc_function = readline_getc;
	rl_input_available_hook = readline_input_avail;
	rl_redisplay_function = readline_redisplay;
	rl_callback_handler_install(":", got_command);

	rl_bind_key('\t', rl_insert);
}

void deinit_readline(void)
{
	rl_callback_handler_remove();
}

void process_cmd(int ch)
{
	LOG("Process cmd %u\n", ch);

	/*
	 * TODO:
	 * I would love also to catch the ESC key (27), but these come
	 * also when keypad is used, there can be a way to handle this by
	 * using ESCDELAY stuff, but I will get to that later */

	/* If TAB || BACKSPACE (to -1) */
	if (ch == '\t' || (ch == 127 && !rl_point)) {
		LOG("Detected exit cmd\n");
		active_win = last_win;
		if (active_win == FIELDS_WIN) {
			set_active_field(false);
			wrefresh(fields_win);

		} else if (active_win == BINARY_WIN) {
			position_binary_curser(0, bit_pos);
		}

		keypad(stdscr, FALSE);
		curs_set(0);
		werase(cmd_win);
		wrefresh(cmd_win);
		return;
	}

	g_input = ch;
	g_input_avail = true;
	rl_callback_read_char();
}

static int cmd_set_width(char **argv, int argc)
{
	LOG("cmd_set_width: argc %d\n", argc);
	if (!strcmp("64", argv[0]))
		set_fields_width(64);
	else if (!strcmp("32", argv[0]))
		set_fields_width(32);
	else if (!strcmp("16", argv[0]))
		set_fields_width(16);
	else if (!strcmp("8", argv[0]))
		set_fields_width(8);
	else return -1;

	unpaint_screen();
	paint_screen();

	return 0;
}

