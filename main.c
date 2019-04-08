#include <stdlib.h>
#include <ncurses.h>
#include <form.h>

static void init_terminal(void)
{
	initscr();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);
}

static void deinit_terminal(void)
{
	endwin();
}
int main(int argc, char *argv[])
{

	init_terminal();

	deinit_terminal();

	return 0;
}
