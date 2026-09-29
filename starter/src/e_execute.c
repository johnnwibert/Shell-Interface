#include "e_execute.h"
#include "path.h"
#include "redirect.h"
#include "jobs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

//runs cmd that isnt one of our built-ins like cd, exit or jobs 
//background: 1 if this should run in the background instead of waiting on it
//cmdline - the orig line typed in, used for jobs/history
int execute_external(tokenlist *tokens, int background, const char *cmdline)
{
    char **argv;
    int argc;
    redirect_t r;

    //pulls out any < or > redirects first, leaving clean argv behind
    if (redirect_split(tokens->items, (int)tokens->size, &argv, &argc, &r) != 0)
        return -1;

    //make sure the input file actually exists before we fork 
    if (redirect_check_input(&r) != 0)
    {
        free(argv);
        return -1;
    }

    char *command;
    int free_command = 0; 

    if (strchr(argv[0], '/') != NULL)
    {
        //command already has a path in it
        command = argv[0];
    }
    else
    {
        //otherwise search $PATH for it
        command = find_command(argv[0]);

        if (command == NULL)
        {
            printf("%s: command not found\n", argv[0]);
            free(argv);
            return -1;
        }
        free_command = 1;
    }

    pid_t pid = fork();   //returns zero in child, the childs pid in the parent
    int ok = 0;

    if (pid < 0)
    {
        //fork flailed, no child made
        perror("fork");
        ok = -1;
    }
    else if (pid == 0)
    {
        //we are the chil, so set up redirects, then become the cmd
        if (redirect_apply(&r, NULL) != 0)
            exit(EXIT_FAILURE);

        execv(command, argv);

        //execv only returns if it failed
        perror("execv");
        exit(EXIT_FAILURE);
    }
    else if (background)
    {
        //don't wait for it, just track it 
	jobs_add(&pid, 1, cmdline);
    }
    else
    {
        //norm case - wait for the child to finish before continuing
        waitpid(pid, NULL, 0);
    }

    if (free_command)
        free(command);
    free(argv);
    return ok;
}
/*void execute_external(tokenlist *tokens)
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
}*/
