/* Copyright 2019
 * Ramon Fried <ramon.fried@gmail.com>
 */

#include "bitwise.h"

static const char *help_header = PACKAGE " " VERSION;
static const char *interactive_mode = "Type numbers in different bases and directly manipulate bits.\n" \
				       "To move around use the arrow keys, or use vi key bindings ('h|j|k|l').\n" \
				       "In the binary field, toggle a bit using 'SPACE'.\n" \
				       "You can jump a byte forward using 'w' and backwards one byte using 'b'.\n\n" \
				       "Additional key mapping:\n" \
				       "'F1' Show this help screen.\n" \
				       "'~'  Perform logical NOT.\n" \
				       "'<'  Perform left shift.\n" \
				       "'>'  Perform right shift.\n\n" \
				       "'!'  Set bit width to 8bit.\n" \
				       "'@'  Set bit width to 16bit.\n" \
				       "'$'  Set bit width to 32bit.\n" \
				       "'*'  Set bit width to 64bit.\n" \
				       "'r'  Reverse the endianness\n" \
				       "':'  Switch to command mode.\n" \
				       "'q'  Exit back to shell.\n";

static const char *command_mode = "Type expressions or commands, result will be appended to history window.\n" \
				   "Supported operators: (,),+, -, *, /, !, ~, <<_, >>_, BIT(_).\n" \
				   "Precedence of operators is defined as C language precedence.\n\n" \
				   "You can referr to the last result with '$' inside an expression.\n" \
				   "To return to interactive mode type 'ESC'.\n\n" \
				   "Supported commands:\n" \
				   "'help'      Show this help screen.\n" \
				   "'clear'     Clear the history window.\n" \
				   "'width []'  Set the bit width to [8 | 16 | 32 | 64]\n" \
				   "'output []' Set the default output for results [decimal | hex | octal | binary | all]\n" \
				   "'q'         Exit back to shell.\n";

void show_help(void)
{
	WINDOW *help_win;
	int ch;

	/* Create full screen window */
	help_win = newwin(0, 0, 0, 0);

	if (g_has_color)
		wattron(help_win, COLOR_PAIR(2));
	mvwprintw(help_win, 0, COLS / 2 - strlen(help_header), "%s",
		  help_header);
	if (g_has_color)
		wattroff(help_win, COLOR_PAIR(2));

	wmove(help_win, 1, 0);
	if (g_has_color)
		wattron(help_win, A_UNDERLINE | COLOR_PAIR(3));
	wprintw(help_win, "Interactive Mode:\n\n");
	if (g_has_color)
		wattroff(help_win, A_UNDERLINE | COLOR_PAIR(3));

	wprintw(help_win, "%s\n", interactive_mode);
	if (g_has_color)
		wattroff(help_win, A_UNDERLINE | COLOR_PAIR(1));

	if (g_has_color)
		wattron(help_win, A_UNDERLINE | COLOR_PAIR(3));
	wprintw(help_win, "Command Mode:\n\n");
	if (g_has_color)
		wattroff(help_win, A_UNDERLINE | COLOR_PAIR(3));
	wprintw(help_win, "%s", command_mode);

	wrefresh(help_win);
	refresh();
	ch = wgetch(help_win);

	werase(help_win);
	wrefresh(help_win);
	delwin(help_win);

	if (ch == 'q' || ch == 'Q')
		g_leave_req = true;

	if (ch == KEY_RESIZE) {
		rl_resize_terminal();
		show_help();
	}
}
