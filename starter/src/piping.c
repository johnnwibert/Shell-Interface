#include "piping.h"
#include "path.h"
#include "redirect.h"
#include "jobs.h"
#include "builtins.h"

#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_PIPELINE_COMMANDS 3
#define MAX_PIPE_COUNT 2

typedef struct {
    char **argv;
    int argc;
    redirect_t redirect;
    char *path;
    int owns_path;
    int builtin_id;
} pipeline_stage_t;

static void close_all_pipes(int pipes[][2], int pipe_count)
{
    for (int i = 0; i < pipe_count; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }
}

static int command_argument_count(char **command)
{
    int count = 0;

    while (command[count] != NULL)
        count++;

    return count;
}

static void free_stages(
    pipeline_stage_t stages[],
    int stage_count
)
{
    for (int i = 0; i < stage_count; i++) {
        free(stages[i].argv);

        if (stages[i].owns_path)
            free(stages[i].path);
    }
}

static int wait_for_child(pid_t pid)
{
    while (waitpid(pid, NULL, 0) == -1) {
        if (errno != EINTR) {
            perror("waitpid");
            return -1;
        }
    }

    return 0;
}

int execute_pipeline(
    char ***commands,
    int command_count,
    int background,
    const char *cmdline
)
{
    int pipe_count = command_count - 1;
    int pipes[MAX_PIPE_COUNT][2];
    pid_t children[MAX_PIPELINE_COMMANDS];
    pipeline_stage_t stages[MAX_PIPELINE_COMMANDS] = {0};

    if (command_count < 2 ||
        command_count > MAX_PIPELINE_COMMANDS) {
        fprintf(stderr, "shell: invalid pipeline size\n");
        return -1;
    }

    for (int i = 0; i < command_count; i++) {
        int token_count =
            command_argument_count(commands[i]);

        if (redirect_split(
                commands[i],
                token_count,
                &stages[i].argv,
                &stages[i].argc,
                &stages[i].redirect
            ) != 0) {
            free_stages(stages, i + 1);
            return -1;
        }

        if (redirect_check_input(
                &stages[i].redirect
            ) != 0) {
            free_stages(stages, i + 1);
            return -1;
        }

        stages[i].builtin_id =
            builtin_lookup(stages[i].argv[0]);

        if (stages[i].builtin_id >= 0)
            continue;

        if (strchr(stages[i].argv[0], '/') != NULL) {
            stages[i].path = stages[i].argv[0];
            stages[i].owns_path = 0;
        }
        else {
            stages[i].path =
                find_command(stages[i].argv[0]);

            if (stages[i].path == NULL) {
                fprintf(
                    stderr,
                    "%s: command not found\n",
                    stages[i].argv[0]
                );

                free_stages(stages, i + 1);
                return -1;
            }

            stages[i].owns_path = 1;
        }
    }

    for (int i = 0; i < pipe_count; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            close_all_pipes(pipes, i);
            free_stages(stages, command_count);
            return -1;
        }
    }

    // One child per command
    for (int i = 0; i < command_count; i++) {
        children[i] = fork();

        if (children[i] < 0) {
            perror("fork");
            close_all_pipes(pipes, pipe_count);

            for (int j = 0; j < i; j++)
                wait_for_child(children[j]);

            free_stages(stages, command_count);
            return -1;
        }

        if (children[i] == 0) {
            if (i > 0) {
                if (dup2(
                        pipes[i - 1][0],
                        STDIN_FILENO
                    ) == -1) {
                    perror("dup2");
                    _exit(EXIT_FAILURE);
                }
            }

            if (i < command_count - 1) {
                if (dup2(
                        pipes[i][1],
                        STDOUT_FILENO
                    ) == -1) {
                    perror("dup2");
                    _exit(EXIT_FAILURE);
                }
            }

            close_all_pipes(pipes, pipe_count);

            if (redirect_apply(
                    &stages[i].redirect,
                    NULL
                ) != 0) {
                _exit(EXIT_FAILURE);
            }

            if (stages[i].builtin_id >= 0) {
                int result = builtin_run(
                    stages[i].builtin_id,
                    stages[i].argc,
                    stages[i].argv,
                    1
                );

                fflush(stdout);
                fflush(stderr);

                _exit(
                    result == 0
                        ? EXIT_SUCCESS
                        : EXIT_FAILURE
                );
            }

            execv(
                stages[i].path,
                stages[i].argv
            );

            perror(stages[i].argv[0]);
            _exit(127);
        }
    }

    close_all_pipes(pipes, pipe_count);

    int result = 0;

    if (background) {
        if (jobs_add(
                children,
                command_count,
                cmdline
            ) < 0) {
            fprintf(
                stderr,
                "shell: too many active background jobs\n"
            );

            for (int i = 0; i < command_count; i++)
                wait_for_child(children[i]);

            result = -1;
        }
    }
    else {
        /*
         * Foreground pipeline: wait for every command.
         */
        for (int i = 0; i < command_count; i++) {
            if (wait_for_child(children[i]) != 0)
                result = -1;
        }
    }

    free_stages(stages, command_count);
    return result;
}