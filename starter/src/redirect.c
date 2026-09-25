#define _XOPEN_SOURCE 700   /* fchmod() etc. even under strict -std=c99/c11 */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "redirect.h"
#include "util.h"

#define OUT_FILE_MODE (S_IRUSR | S_IWUSR)   /* -rw------- */

static int is_operator(const char *tok)
{
    return strcmp(tok, "<") == 0 || strcmp(tok, ">") == 0 ||
           strcmp(tok, "|") == 0 || strcmp(tok, "&") == 0;
}

int redirect_split(char *const *tokens, int ntokens,
                   char ***argv_out, int *argc_out, redirect_t *r)
{
    char **argv = xmalloc((size_t)(ntokens + 1) * sizeof *argv);
    int argc = 0;

    r->in_file = NULL;
    r->out_file = NULL;

    for (int i = 0; i < ntokens; i++) {
        if (strcmp(tokens[i], "<") == 0 || strcmp(tokens[i], ">") == 0) {
            int is_in = (tokens[i][0] == '<');

            if (i + 1 >= ntokens || is_operator(tokens[i + 1])) {
                fprintf(stderr, "%s: syntax error: expected file name after '%s'\n",
                        SHELL_NAME, tokens[i]);
                free(argv);
                return -1;
            }
            i++;
            if (is_in)
                r->in_file = tokens[i];
            else
                r->out_file = tokens[i];
        } else {
            argv[argc++] = tokens[i];
        }
    }
    argv[argc] = NULL;

    if (argc == 0) {
        fprintf(stderr, "%s: syntax error: missing command\n", SHELL_NAME);
        free(argv);
        return -1;
    }
    *argv_out = argv;
    *argc_out = argc;
    return 0;
}

int redirect_check_input(const redirect_t *r)
{
    struct stat st;

    if (r->in_file == NULL)
        return 0;
    if (stat(r->in_file, &st) != 0) {
        fprintf(stderr, "%s: %s: %s\n", SHELL_NAME, r->in_file, strerror(errno));
        return -1;
    }
    if (!S_ISREG(st.st_mode)) {     /* also keeps us from blocking on a FIFO */
        fprintf(stderr, "%s: %s: Not a regular file\n", SHELL_NAME, r->in_file);
        return -1;
    }
    return 0;
}

void redirect_restore(redirect_saved_t *saved)
{
    if (saved == NULL)
        return;
    if (saved->saved_in >= 0) {
        dup2(saved->saved_in, STDIN_FILENO);
        close(saved->saved_in);
        saved->saved_in = -1;
    }
    if (saved->saved_out >= 0) {
        dup2(saved->saved_out, STDOUT_FILENO);
        close(saved->saved_out);
        saved->saved_out = -1;
    }
}

int redirect_apply(const redirect_t *r, redirect_saved_t *saved)
{
    redirect_saved_t local = { -1, -1 };
    redirect_saved_t *s = saved != NULL ? saved : &local;
    struct stat st;

    s->saved_in = -1;
    s->saved_out = -1;

    /* Input first: it is never modified, so it is opened read-only. */
    if (r->in_file != NULL) {
        int fd;

        if (redirect_check_input(r) != 0)
            return -1;
        fd = open(r->in_file, O_RDONLY);
        if (fd < 0) {
            fprintf(stderr, "%s: %s: %s\n", SHELL_NAME, r->in_file, strerror(errno));
            return -1;
        }
        if (saved != NULL)
            s->saved_in = dup(STDIN_FILENO);
        dup2(fd, STDIN_FILENO);
        close(fd);
    }

    if (r->out_file != NULL) {
        int fd = open(r->out_file, O_WRONLY | O_CREAT | O_TRUNC, OUT_FILE_MODE);

        if (fd < 0) {
            fprintf(stderr, "%s: %s: %s\n", SHELL_NAME, r->out_file, strerror(errno));
            redirect_restore(s);
            return -1;
        }
        /* An existing file keeps its old mode through open(); the assignment
         * wants overwritten files to end up -rw------- too. */
        if (fstat(fd, &st) == 0 && S_ISREG(st.st_mode))
            fchmod(fd, OUT_FILE_MODE);
        if (saved != NULL)
            s->saved_out = dup(STDOUT_FILENO);
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }
    return 0;
}
