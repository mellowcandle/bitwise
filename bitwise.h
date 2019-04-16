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

#define BIT(nr) (1ULL << (nr))

int validate_input(int ch, int base);
void lltostr(uint64_t val, char *buf, int base);
void init_terminal(void);
void deinit_terminal(void);
uint64_t base_scanf(const char *buf, int base);

void die(const char *fmt, ...);
int start_interactive(void);
#ifdef TRACE
#define LOG(...) fprintf(fd, __VA_ARGS__);fflush(fd)
extern FILE *fd;
#else
#define LOG(...)
#endif

extern WINDOW *fields_win;
extern bool has_color;
#endif /* end of include guard: BITWISE_H */
