/* Copyright 2019
 * Ramon Fried <ramon.fried@gmail.com
 */

#ifndef BITWISE_H
#define BITWISE_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <form.h>
#include <string.h>
#include <readline/history.h>
#include <readline/readline.h>

#define MAX_HISTORY_LEN 100

#define FIELDS_WIN  0
#define BINARY_WIN  1
#define COMMAND_WIN 2

#define BIT(nr) (1ULL << (nr))
#define MASK(s) (~0ULL >> (64 - s))
#define ARRAY_SIZE(x)	(sizeof(x) / sizeof((x)[0]))

/* Misc */
void lltostr(uint64_t val, char *buf, int base);
int set_width(char width);
void set_width_by_val(uint64_t val);
void die(const char *fmt, ...);
int parse_input(const char *input, uint64_t *val);
int validate_input(int ch, int base);

void init_terminal(void);
void deinit_terminal(void);

/* Interactive */
extern FORM *form;

int sprintf_size(uint64_t val, char *buf, bool si);
int start_interactive(uint64_t start);
void set_active_field(bool none);
void set_fields_width(int width);
void position_binary_curser(int previous_pos, int next_pos);
void paint_screen(void);
void unpaint_screen(void);
void update_binary();
int update_fields(int index);

/* Command */
void process_cmd(int ch);
void init_readline(void);
void deinit_readline(void);
void readline_redisplay(void);

/* calc */
int calc(int argc, char *argv[]);

#ifdef TRACE
#define LOG(...) fprintf(fd, __VA_ARGS__);fflush(fd)
extern FILE *fd;
#else
#define LOG(...)
#endif

/* Colors */
void init_colors(void);

extern char *color_green;
extern char *color_red;
extern char *color_blue;
extern char *color_magenta;
extern char *color_cyan;
extern char *color_white;
extern char *color_reset;

extern WINDOW *fields_win;
extern WINDOW *binary_win;
extern WINDOW *cmd_win;
extern int active_win;
extern int last_win;

extern int bit_pos;
extern int g_has_color;
extern int g_width;
extern bool g_input_avail;
extern int g_input;
extern bool g_leave_req;
extern uint64_t g_val;

/* History */
typedef enum history_type {
	TYPE_INPUT_COMMAND = 0,
	TYPE_INPUT_EXPRESSION,
	TYPE_OUTPUT_RESULT,
	TYPE_OUTPUT_ERROR,
} history_type;

struct history_entry {
	history_type type;
	char *line;
};

extern struct history_entry history[];
extern unsigned int history_pos;

void flush_history(void);
void update_history_win(void);

static inline void append_to_history(const char *str, history_type type) {
	char *new_line;

	new_line = strdup(str);
	if (!new_line) {
		LOG("No memory to allocate string\n");
		return;
	}

	history[history_pos % MAX_HISTORY_LEN].line = new_line;
	history[history_pos % MAX_HISTORY_LEN].type = type;
	history_pos++;

	update_history_win();
}

static inline WINDOW *get_win(int win)
{
	if (win == FIELDS_WIN)
		return fields_win;
	else if (win == BINARY_WIN)
		return binary_win;
	else if (win == COMMAND_WIN)
		return cmd_win;
	else
		return NULL;
}

#endif /* end of include guard: BITWISE_H */
