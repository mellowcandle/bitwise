#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([bitwise], [v0.42], [rfried.dev@gmail.com])
AC_CONFIG_SRCDIR([src/misc.c])
AC_CONFIG_HEADERS([inc/config.h])

dnl  Document where we keep our .m4 file.
AC_CONFIG_MACRO_DIR([m4])
dnl  Make sure aclocal actually found it!
m4_pattern_forbid([^AX_])

AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])

# Checks for programs.
AC_PROG_CC
AC_PROG_CC_STDC
AC_PROG_INSTALL
AC_PROG_CC_C99

AX_LIB_READLINE
if test "$ac_cv_have_readline" = no; then
    AC_MSG_ERROR([requires readline library])
fi

# Checks for libraries.
AC_CHECK_LIB([form], [form_driver])
AC_CHECK_LIB([ncurses], [newwin])
AC_SEARCH_LIBS([sqrt], [m])

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h inttypes.h stdint.h stdlib.h string.h curses.h form.h])

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

AC_ARG_ENABLE([trace],
	[AS_HELP_STRING([--enable-trace],
		[Build with trace support])],
	[],
	[enable_trace=no])
AM_CONDITIONAL([COND_TRACE],[test '!' "$enable_trace" = no])

AC_ARG_ENABLE([debug],
	[AS_HELP_STRING([--enable-debug],
		[Build with debug symbols])],
	[],
	[enable_debug=no])
AM_CONDITIONAL([COND_DEBUG],[test '!' "$enable_debug" = no])

AC_OUTPUT
