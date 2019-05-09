/* Copyright 2019
 * Ramon Fried <ramon.fried@gmail.com>
 */
#include <string.h>
#include "bitwise.h"

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
	if (!line) {
		/* Handle CTL-D */
		g_leave_req = true;
		return;
	}

	if (strcmp("q", line) == 0)
	    g_leave_req = true;

        if (*line)
            add_history(line);

	LOG("got command: %s\n", line);
	free(line);
}

void readline_redisplay(void)
{
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
	/* If TAB || ESC || BACKSPACE (to -1) */
	if (ch == '\t' || ch == 27 ||
	    (ch == 127 && !rl_point)) {
		active_win = last_win;
		if (active_win == fields_win) {
			set_active_field(false);
			wrefresh(fields_win);

		} else if (active_win == binary_win) {
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

