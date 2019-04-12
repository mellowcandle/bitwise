# Copyright Ramon Fried (2019)
OBJS := interactive.o main.o misc.o
CFLAGS := -g -std=c99 -Wall -Werror -lncurses -lform
LDFLAGS := -lncurses -lform

#DEPFILES := $(patsubst %.o,%.d,$(OBJS))
#include $(DEPFILES)   # include all dep files in the makefile


all: $(OBJS)
	$(CC) -o bitwise $(OBJS) $(LDFLAGS)

#%.d: %.c
#	$(CC) $(CFLAGS) $< -MM -MT $(@:.d=.o) >$@

clean:
	rm -rf *.o bitwise
