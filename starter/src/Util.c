#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"

//prints an error and quits the whole program
void die(const char *msg)
{
    fprintf(stderr, "%s: fatal: %s\n", SHELL_NAME, msg);
    exit(1);
}

// same as malloc, but kills the shell instead of returning Nll if fails so we do not have to check for NULL every single time we allocate
void *xmalloc(size_t n)
{
    void *p = malloc(n ? n : 1);
    if (p == NULL)
        die("out of memory");
    return p;
}

//same idea, but for realloc
void *xrealloc(void *ptr, size_t n)
{
    void *p = realloc(ptr, n ? n : 1);
    if (p == NULL)
        die("out of memory");
    return p;
}

//akes a copy of a whole string on the heap
char *xstrdup(const char *s)
{
    return xstrndup(s, strlen(s));
}

//makes a cpy of the first n characters of a string on the heap
char *xstrndup(const char *s, size_t n)
{
    char *copy = xmalloc(n + 1);
    memcpy(copy, s, n);
    copy[n] = '\0';
    return copy;
}
