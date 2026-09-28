#include "path.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

char *find_command(const char *command)
{
    // For commands that do not include a slash and are not built-in functions
    // strchr() looks for a character in a string
    if (strchr(command, '/') == NULL)
    {
        char *path = getenv("PATH");

        // make a copy as to not modify env variable's storage
        char *path_copy = malloc(strlen(path) + 1);
        strcpy(path_copy, path);

        // store directory as a string
        // strtok splits into tokens based on a delimiter
        char *dir = strtok(path_copy, ":");

        while (dir != NULL)
        {
            char *candidate = malloc(strlen(dir) + strlen(command) + 2);
            // %s/%s is two string placeholders separated by a slash
            sprintf(candidate, "%s/%s", dir, command);

            if (access(candidate, X_OK) == 0)
            {
                free(path_copy);
                return candidate;
            }
            else 
            {
                free(candidate);
                dir = strtok(NULL, ":");
            }
        }
		return NULL;
    }
    return NULL;
}