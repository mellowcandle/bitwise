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
#include <readline/history.h>
#include <readline/readline.h>

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
uint64_t base_scanf(const char *buf, int base);
int validate_input(int ch, int base);

void init_terminal(void);
void deinit_terminal(void);

/* Interactive */
int sprintf_size(uint64_t val, char *buf);
int start_interactive(uint64_t start);
void set_active_field(bool none);
void position_binary_curser(int previous_pos, int next_pos);
void paint_screen(void);
void unpaint_screen(void);

/* Command */
void process_cmd(int ch);
void init_readline(void);
void deinit_readline(void);
void readline_redisplay(void);

#define TRACE
#ifdef TRACE
#define LOG(...) fprintf(fd, __VA_ARGS__);fflush(fd)
extern FILE *fd;
#else
#define LOG(...)
#endif

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
