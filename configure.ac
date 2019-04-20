#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([bitwise], [0.1], [rfried.dev@gmail.com])
AC_CONFIG_SRCDIR([misc.c])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lform':
AC_CHECK_LIB([form], [form_driver])
# FIXME: Replace `main' with a function in `-lncurses':
AC_CHECK_LIB([ncurses], [newwin])

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h stdint.h stdlib.h string.h curses.h form.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_UINT64_T

# Checks for library functions.
AC_FUNC_STRCOLL
AC_CHECK_FUNCS([memchr memmove memset stpcpy strchr strcspn strdup strerror strpbrk strrchr strspn strstr])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
