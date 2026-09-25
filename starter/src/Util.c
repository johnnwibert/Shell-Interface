#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"

void die(const char *msg)
{
    fprintf(stderr, "%s: fatal: %s\n", SHELL_NAME, msg);
    exit(1);
}

void *xmalloc(size_t n)
{
    void *p = malloc(n ? n : 1);
    if (p == NULL)
        die("out of memory");
    return p;
}

void *xrealloc(void *ptr, size_t n)
{
    void *p = realloc(ptr, n ? n : 1);
    if (p == NULL)
        die("out of memory");
    return p;
}

char *xstrdup(const char *s)
{
    return xstrndup(s, strlen(s));
}

char *xstrndup(const char *s, size_t n)
{
    char *copy = xmalloc(n + 1);
    memcpy(copy, s, n);
    copy[n] = '\0';
    return copy;
}
