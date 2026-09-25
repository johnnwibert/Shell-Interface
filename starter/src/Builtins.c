#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "builtins.h"
#include "history.h"
#include "jobs.h"
#include "util.h"

typedef int (*builtin_fn)(int argc, char **argv, int in_child);

static int exit_requested = 0;

void builtins_exit_shell(void)
{
    jobs_wait_all();        /* must not leave background work behind */
    history_print_last();
    exit_requested = 1;
}

int builtins_exit_requested(void)
{
    return exit_requested;
}

/* exit: wait for background jobs, show the last valid commands, quit. */
static int builtin_exit(int argc, char **argv, int in_child)
{
    (void)argc;
    (void)argv;

    if (in_child)
        return 0;
    builtins_exit_shell();
    return 0;
}

/*
 * cd [PATH]
 *   no argument     -> $HOME
 *   more than one   -> error
 *   not a directory -> error
 *   nonexistent     -> error
 */
static int builtin_cd(int argc, char **argv, int in_child)
{
    const char *target;
    struct stat st;

    (void)in_child;

    if (argc > 2) {
        fprintf(stderr, "%s: cd: too many arguments\n", SHELL_NAME);
        return 1;
    }

    if (argc == 2) {
        target = argv[1];
    } else {
        target = getenv("HOME");
        if (target == NULL || target[0] == '\0') {
            fprintf(stderr, "%s: cd: HOME not set\n", SHELL_NAME);
            return 1;
        }
    }

    if (stat(target, &st) != 0) {
        fprintf(stderr, "%s: cd: %s: %s\n", SHELL_NAME, target, strerror(errno));
        return 1;
    }
    if (!S_ISDIR(st.st_mode)) {
        fprintf(stderr, "%s: cd: %s: Not a directory\n", SHELL_NAME, target);
        return 1;
    }
    if (chdir(target) != 0) {
        fprintf(stderr, "%s: cd: %s: %s\n", SHELL_NAME, target, strerror(errno));
        return 1;
    }

    /* The prompt is built from $PWD, so keep it in sync. */
    char *cwd = getcwd(NULL, 0);
    if (cwd != NULL) {
        setenv("PWD", cwd, 1);
        free(cwd);
    }
    return 0;
}

/* jobs: list active background processes. */
static int builtin_jobs(int argc, char **argv, int in_child)
{
    (void)argc;
    (void)argv;
    (void)in_child;

    /* Report anything that finished since the last prompt first. */
    jobs_reap_finished();
    jobs_print_active();
    return 0;
}

static const struct {
    const char *name;
    builtin_fn fn;
} builtin_table[] = {
    { "exit", builtin_exit },
    { "cd",   builtin_cd },
    { "jobs", builtin_jobs },
};

#define NUM_BUILTINS (sizeof(builtin_table) / sizeof(builtin_table[0]))

int builtin_lookup(const char *name)
{
    for (size_t i = 0; i < NUM_BUILTINS; i++) {
        if (strcmp(name, builtin_table[i].name) == 0)
            return (int)i;
    }
    return -1;
}

int builtin_run(int id, int argc, char **argv, int in_child)
{
    if (id < 0 || (size_t)id >= NUM_BUILTINS)
        return 1;
    return builtin_table[id].fn(argc, argv, in_child);
}
