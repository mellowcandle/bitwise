bin_PROGRAMS=bitwise
bitwise_SOURCES=main.c misc.c interactive.c bitwise.h

if COND_GCOV
MAYBE_COVERAGE=--coverage --no-inline
endif

AM_CFLAGS = $(MAYBE_COVERAGE)

clean-local:
		rm -f *.gcno
