OBJS := main.o
CFLAGS := -Wall -lncurses -lform
LDFLAGS := -lncurses -lform

all: $(OBJS)
	$(CC) $(LDFLAGS) -o bitwise $(OBJS)

clean:
	rm -rf *.o bitwise
