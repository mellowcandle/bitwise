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

#define BIT(nr) (1ULL << (nr))
#define MASK(s) (~0ULL >> (64 - s))

int set_width(char width);
void set_width_by_val(uint64_t val);

int validate_input(int ch, int base);
void lltostr(uint64_t val, char *buf, int base);
void init_terminal(void);
void deinit_terminal(void);
uint64_t base_scanf(const char *buf, int base);
int sprintf_size(uint64_t val, char *buf);
void readline_redisplay(void);
void die(const char *fmt, ...);
int start_interactive(uint64_t start);

#define TRACE
#ifdef TRACE
#define LOG(...) fprintf(fd, __VA_ARGS__);fflush(fd)
extern FILE *fd;
#else
#define LOG(...)
#endif

extern WINDOW *fields_win;
extern WINDOW *cmd_win;
extern int g_has_color;
extern int g_width;
extern bool g_input_avail;
extern int g_input;
extern bool g_leave_req;
#endif /* end of include guard: BITWISE_H */
