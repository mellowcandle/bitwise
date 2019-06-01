bin_PROGRAMS=bitwise
bitwise_SOURCES= src/main.c src/misc.c src/interactive.c src/cmd.c inc/bitwise.h

dist_man_MANS=bitwise.1

if COND_GCOV
MAYBE_COVERAGE=--coverage --no-inline
endif

AM_CFLAGS = $(MAYBE_COVERAGE)

clean-local:
		rm -f *.gcno
