#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>

#define SHELL_NAME "shell"

//allocation helpers that abort the shell on an out of mem
void *xmalloc(size_t n);
void *xrealloc(void *ptr, size_t n);
char *xstrdup(const char *s);
char *xstrndup(const char *s, size_t n);
void die(const char *msg);

#endif
