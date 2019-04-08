OBJS := main.o
CFLAGS := -std=c99 -Wall -lncurses -lform
LDFLAGS := -lncurses -lform

all: $(OBJS)
	$(CC) -o bitwise $(OBJS) $(LDFLAGS)

clean:
	rm -rf *.o bitwise
