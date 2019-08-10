/* Copyright 2019
 * Ramon Fried <ramon.fried@gmail.com>
 */

#include "bitwise.h"

static const char *help_header = PACKAGE " " VERSION;
static const char *modes = "Interactive mode: type numbers in different bases and directly manipulate bits\n"\
			    "Command mode: type expressions or commands\n";
static const char *navigation = "Interactive mode: To move around use the arrow keys, or use vi key bindings ('h|j|k|l')\n";

void show_help(void)
{
	WINDOW *help_win;

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
		wattron(help_win, A_UNDERLINE | COLOR_PAIR(1));
	wprintw(help_win, "Modes:\n");
	if (g_has_color)
		wattroff(help_win, A_UNDERLINE | COLOR_PAIR(1));
	wprintw(help_win, "%s\n", modes);

	if (g_has_color)
		wattron(help_win, A_UNDERLINE | COLOR_PAIR(1));
	wprintw(help_win, "Navigation:\n");
	if (g_has_color)
		wattroff(help_win, A_UNDERLINE | COLOR_PAIR(1));
	wprintw(help_win, "%s", navigation);

	wrefresh(help_win);
	refresh();
	wgetch(help_win);

	werase(help_win);
	wrefresh(help_win);
	delwin(help_win);
}
