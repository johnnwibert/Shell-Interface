#ifndef REDIRECT_H
#define REDIRECT_H

/*
 * I/O redirection (Part 6):   cmd < file_in     cmd > file_out
 *
 * Works on a plain array of token strings (already expanded, i.e. after
 * $VAR / ~ expansion), so it does not depend on how the tokenizer stores
 * them. "<" and ">" must be tokens of their own.
 */

/* File names for one command. The pointers are BORROWED from the token
 * array (nothing is copied), so the tokens must outlive this struct. */
typedef struct {
    const char *in_file;    /* NULL when there is no `<` */
    const char *out_file;   /* NULL when there is no `>` */
} redirect_t;

/* Descriptors saved by redirect_apply() so redirect_restore() can undo it. */
typedef struct {
    int saved_in;
    int saved_out;
} redirect_saved_t;

/*
 * Split the tokens of ONE simple command (no '|' and no trailing '&' --
 * strip those first) into a plain argument list and its redirections.
 *
 * On success returns 0 and sets:
 *   *argv_out : NULL-terminated array, ready for execv(). It holds the same
 *               pointers as `tokens`, so free() the ARRAY ONLY, never its
 *               strings.
 *   *argc_out : number of arguments
 *   *r        : the file names (last one wins if a redirect is repeated)
 * On a syntax error (missing file name, no command) prints a message and
 * returns -1; nothing needs freeing.
 */
int redirect_split(char *const *tokens, int ntokens,
                   char ***argv_out, int *argc_out, redirect_t *r);

/* Input file must exist and be a regular file. Prints an error, returns -1. */
int redirect_check_input(const redirect_t *r);

/*
 * Point stdin/stdout at the files: input is opened read-only, output is
 * created/truncated with mode -rw-------. The input is handled first, so a
 * bad input file never leaves an empty output file behind.
 *
 * In a forked child pass saved == NULL. For a built-in that runs inside the
 * shell itself pass a struct and call redirect_restore() afterwards.
 * Returns 0, or -1 after printing an error (everything is undone).
 */
int redirect_apply(const redirect_t *r, redirect_saved_t *saved);

void redirect_restore(redirect_saved_t *saved);

#endif
