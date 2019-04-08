OBJS := main.o
CFLAGS := -Wall -lncurses -lform
LDFLAGS := -lncurses -lform

all: $(OBJS)
	$(CC) -o bitwise $(OBJS) $(LDFLAGS)

clean:
	rm -rf *.o bitwise
