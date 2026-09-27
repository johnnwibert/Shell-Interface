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

static int exit_requested = 0;

void builtins_exit_shell(void)
{
    jobs_wait_all();
    history_print_last();
    exit_requested = 1;
}

int builtins_exit_requested(void)
{
    return exit_requested;
}

//give each built-in a number so builtin_run knows which one to do
#define BUILTIN_EXIT 0
#define BUILTIN_CD   1
#define BUILTIN_JOBS 2

int builtin_lookup(const char *name)
{
    if (strcmp(name, "exit") == 0)
        return BUILTIN_EXIT;
    if (strcmp(name, "cd") == 0)
        return BUILTIN_CD;
    if (strcmp(name, "jobs") == 0)
        return BUILTIN_JOBS;

    return -1;
}

int builtin_run(int id, int argc, char **argv, int in_child)
{
    //exit-wat for jobs, print history, then quit
    if (id == BUILTIN_EXIT) {
        if (in_child)
            return 0;
        builtins_exit_shell();
        return 0;
    }

    //cd [PATH] - no arg $HOME, eroros on too many arg, not a directory, or a missing target
    if (id == BUILTIN_CD) {
        char *target;
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

        //keep $pwd in sync since the prompt is built from it
        char *cwd = getcwd(NULL, 0);
        if (cwd != NULL) {
            setenv("PWD", cwd, 1);
            free(cwd);
        }
        return 0;
    }

    //jobs: list active background processes
    if (id == BUILTIN_JOBS) {
        (void)argc;
        (void)argv;
        jobs_reap_finished();
        jobs_print_active();
        return 0;
    }

    return 1;
}
