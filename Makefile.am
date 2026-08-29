bin_PROGRAMS=bitwise
bitwise_SOURCES= src/main.c src/misc.c src/compat.c\
				 src/interactive.c src/cmd.c \
				 src/stack.c src/shunting-yard.c \
				 src/help.c inc/bitwise.h \
				 inc/stack.h inc/shunting-yard.h \
				 inc/compat.h

dist_man_MANS=bitwise.1

if COND_GCOV
MAYBE_COVERAGE=--coverage --no-inline
endif

if COND_TRACE
MAYBE_TRACE=-DTRACE
endif

if COND_DEBUG
MAYBE_DEBUG=-g -O0
endif

# The sanitizers have to reach the link too, hence AM_LDFLAGS below.
if COND_ASAN
MAYBE_ASAN=-fsanitize=address -fno-omit-frame-pointer
endif

if COND_UBSAN
MAYBE_UBSAN=-fsanitize=undefined -fno-sanitize-recover=all
endif

AM_CFLAGS = -Wall -Wextra $(MAYBE_COVERAGE) $(MAYBE_DEBUG) $(MAYBE_TRACE) \
	    $(MAYBE_ASAN) $(MAYBE_UBSAN)
AM_LDFLAGS = $(MAYBE_ASAN) $(MAYBE_UBSAN)

check_PROGRAMS = tests/test-shunting-yard
tests_test_shunting_yard_SOURCES = src/shunting-yard.c inc/shunting-yard.h \
                                   src/stack.c inc/stack.h inc/bitwise.h \
				   src/misc.c src/compat.c \
                                   tests/test-shunting-yard.c
tests_test_shunting_yard_LDADD = -lcunit

# The fuzzer is built by "make check" but deliberately kept out of TESTS:
# libFuzzer runs until it finds a crash, which is not a unit test.
TESTS = tests/test-shunting-yard

if COND_FUZZER
check_PROGRAMS += tests/fuzz-shunting-yard
tests_fuzz_shunting_yard_SOURCES = src/shunting-yard.c inc/shunting-yard.h \
                                   src/stack.c inc/stack.h inc/bitwise.h \
				   src/misc.c src/compat.c \
                                   tests/fuzz-shunting-yard.c
tests_fuzz_shunting_yard_CFLAGS = $(AM_CFLAGS) -fsanitize=fuzzer
tests_fuzz_shunting_yard_LDFLAGS = $(AM_LDFLAGS) -fsanitize=fuzzer
endif

clean-local:
		rm -f *.gcno
