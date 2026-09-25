#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>

#define SHELL_NAME "shell"

/* Allocation helpers that abort the shell on out-of-memory. */
void *xmalloc(size_t n);
void *xrealloc(void *ptr, size_t n);
char *xstrdup(const char *s);
char *xstrndup(const char *s, size_t n);
void die(const char *msg);

#endif
