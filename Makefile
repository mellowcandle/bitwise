# Copyright Ramon Fried (2019)
OBJS := interactive.o main.o misc.o
CFLAGS := -g -std=c99 -Wall -Werror -lncurses -lform
LDFLAGS := -lncurses -lform

bitwise: $(OBJS)
	$(CC) -o bitwise $(OBJS) $(LDFLAGS)

install: bitwise
		install -m 711 bitwise /usr/local/bin/bitwise
clean:
	rm -rf *.o bitwise

