#include "e_execute.h"
#include "path.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

void execute_external(tokenlist *tokens)
{
    char *command;

    if (strchr(tokens->items[0], '/') != NULL)
    {
        command = tokens->items[0];
    }
    else
    {
        command = find_command(tokens->items[0]);

        if (command == NULL)
        {
            printf("%s: command not found\n", tokens->items[0]);
            return;
        }
    }

    // fork process returns 0 for child process
    pid_t pid = fork();

    // creation of child process failed
    if (pid < 0)
    {
        perror("fork");
    }
    // if the current process is the child process, run the command using execv
    else if (pid == 0)
    {
        execv(command, tokens->items);

        // only reached if execv fails
        perror("execv");
        exit(EXIT_FAILURE);
    }
    else
    {
        waitpid(pid, NULL, 0);
    }
    
    free(command);
}