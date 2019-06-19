#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([bitwise], [v0.21], [rfried.dev@gmail.com])
AC_CONFIG_SRCDIR([src/misc.c])
AC_CONFIG_HEADERS([inc/config.h])

AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])

# Checks for programs.
AC_PROG_CC
AC_PROG_CC_STDC
AC_PROG_INSTALL
AC_PROG_CC_C99

# Checks for libraries.
AC_CHECK_LIB([form], [form_driver])
AC_CHECK_LIB([ncurses], [newwin])
AC_CHECK_LIB([readline], [rl_bind_key])

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h stdint.h stdlib.h string.h curses.h form.h readline/history.h readline/readline.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_UINT64_T

# Checks for library functions.
AC_FUNC_STRCOLL
AC_CHECK_FUNCS([memchr memmove memset stpcpy strchr strcspn strdup strerror strpbrk strrchr strspn strstr])

AC_CONFIG_FILES([Makefile])

AC_ARG_ENABLE([gcov],
	[AS_HELP_STRING([--enable-gcov],
		[Build with coverage support])],
	[],
	[enable_gcov=no])
AM_CONDITIONAL([COND_GCOV],[test '!' "$enable_gcov" = no])

AC_OUTPUT
