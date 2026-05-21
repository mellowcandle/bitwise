#ifndef COMPAT_H
#define COMPAT_H
// Functions needed to substitute some POSIX extensions on Windows/MinGW

#ifndef HAVE_STRNDUP
char *strndup(const char *s, size_t n);
#endif

#ifndef HAVE_L64A
char *l64a(long value);
#endif

#endif

