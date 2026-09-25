#ifndef BUILTINS_H
#define BUILTINS_H

/* Returns the built-in's id for `name`, or -1 if it is not a built-in. */
int builtin_lookup(const char *name);

/*
 * Run a built-in. `in_child` is non-zero when running inside a forked child
 * (pipeline stage or background job); `exit` is then a no-op, exactly like
 * in bash where it would only end the subshell.
 * Returns 0 on success, non-zero on error.
 */
int builtin_run(int id, int argc, char **argv, int in_child);

/*
 * `exit` logic: waits for all background jobs, prints the last valid
 * commands, and asks the main loop to stop. Also used when stdin hits EOF.
 */
void builtins_exit_shell(void);

/* Non-zero once the shell has been asked to terminate. */
int builtins_exit_requested(void);

#endif
