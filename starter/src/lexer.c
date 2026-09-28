#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "builtins.h"
#include "jobs.h"
#include "history.h"



// TESTING PART 4 IMPLEMENTATION
#include "path.h"

// TESTING PART 5 IMPLEMENTATION
#include "e_execute.h"

static void print_prompt(void)
{
	char *user = getenv("USER");
	char *machine = getenv("MACHINE");
	char *pwd = getenv("PWD");

	if (user == NULL)
		user = "?";
	
	if (machine == NULL)
		machine = "?";
	
	if (pwd == NULL)
		pwd = "?";

	printf("%s@%s:%s> ", user, machine, pwd);
	fflush(stdout);

}

int main()
{
	jobs_init();	//added by Robert
	while (!builtins_exit_requested()) {
		jobs_reap_finished();
		print_prompt();

		/* input contains the whole command
		 * tokens contains substrings from input split by spaces
		 */

		char *input = get_input();
		printf("whole input: %s\n", input);

		tokenlist *tokens = get_tokens(input);
		expand_env_variables(tokens);
		expand_tilde(tokens);

		// TESTING PART 7 IMPLEMENTATION
		// counting pipes
		int pipe_count = count_pipes(tokens);
		if (pipe_count > 2)
		{
			printf("Error: maximum of two pipes allowed\n");

			free(input);
			free_tokens(tokens);
			continue;
		}
		// testing split_commands
		if (pipe_count > 0)
		{
			char ***commands = split_commands(tokens, pipe_count);

			for (int i = 0; i < pipe_count + 1; i++)
			{
				printf("Command %d:\n", i);

				for (int j = 0; commands[i][j] != NULL; j++)
				{
					printf("    [%s]\n", commands[i][j]);
				}
			}

			free_commands(commands, pipe_count + 1);
		}

		for (int i = 0; i < tokens->size; i++) {
			printf("token %d: (%s)\n", i, tokens->items[i]);
		}

		// TESTING PART 4 IMPLEMENTATION
		/*
		char *path = find_command(tokens->items[0]);
		if (path != NULL)
		{
			printf("Found command: %s\n", path);
			free(path);
		}
		else
		{
			printf("%s: command not found\n", tokens->items[0]);
		}
		*/


		//Adding Roberts part below this line
		if(tokens->size == 0) {
                        free(input);
                        free_tokens(tokens);
                        continue;
                }

                /* PART 8: a trailing '&' means run in the background */
                int background = 0;
                if (strcmp(tokens->items[tokens->size - 1], "&") == 0) {
                        background = 1;
                        free(tokens->items[tokens->size - 1]);
                        tokens->size -= 1;
                        tokens->items[tokens->size] = NULL;
                }

                if (tokens->size == 0) {
                        free(input);
                        free_tokens(tokens);
                        continue;
                }

                /* PART 9: built-ins run here, never through execute_external */
                int ok;
                int builtin_id = builtin_lookup(tokens->items[0]);
                if (builtin_id >= 0)
                        ok = (builtin_run(builtin_id, tokens->size, tokens->items, 0) == 0) ? 0 : -1;
                else
                        // TESTING PART 5 IMPLEMENTATION
                        ok = execute_external(tokens, background, input);

                if (ok == 0 && !builtins_exit_requested())
                        history_add(input);

		// TESTING PART 5 IMPLEMENTATION
		//execute_external(tokens);

		free(input);
		free_tokens(tokens);
	}

	jobs_cleanup();
	history_cleanup();
	return 0;
}

char *get_input(void) {
	char *buffer = NULL;
	int bufsize = 0;
	char line[5];
	while (fgets(line, 5, stdin) != NULL)
	{
		int addby = 0;
		char *newln = strchr(line, '\n');
		if (newln != NULL)
			addby = newln - line;
		else
			addby = 5 - 1;
		buffer = (char *)realloc(buffer, bufsize + addby);
		memcpy(&buffer[bufsize], line, addby);
		bufsize += addby;
		if (newln != NULL)
			break;
	}
	buffer = (char *)realloc(buffer, bufsize + 1);
	buffer[bufsize] = 0;
	return buffer;
}

tokenlist *new_tokenlist(void) {
	tokenlist *tokens = (tokenlist *)malloc(sizeof(tokenlist));
	tokens->size = 0;
	tokens->items = (char **)malloc(sizeof(char *));
	tokens->items[0] = NULL; /* make NULL terminated */
	return tokens;
}

void add_token(tokenlist *tokens, char *item) {
	int i = tokens->size;

	tokens->items = (char **)realloc(tokens->items, (i + 2) * sizeof(char *));
	tokens->items[i] = (char *)malloc(strlen(item) + 1);
	tokens->items[i + 1] = NULL;
	strcpy(tokens->items[i], item);

	tokens->size += 1;
}

tokenlist *get_tokens(char *input) {
	char *buf = (char *)malloc(strlen(input) + 1);
	strcpy(buf, input);
	tokenlist *tokens = new_tokenlist();
	char *tok = strtok(buf, " ");
	while (tok != NULL)
	{
		add_token(tokens, tok);
		tok = strtok(NULL, " ");
	}
	free(buf);
	return tokens;
}

void expand_env_variables(tokenlist *tokens)
{
	for (size_t i = 0; i < tokens->size; i++)
	{
		char *token = tokens->items[i];

		// Expand tokens starting with '$'
		if (token[0] == '$')
		{
			char *variable_name = token + 1;
			char *variable_value = getenv(variable_name);
			// If the env variable is undefined, empty string
			if (variable_value == NULL)
				variable_value = "";
			
			char *expanded = malloc(strlen(variable_value) + 1);

			if (expanded == NULL)
			{
				perror("malloc");
				exit(EXIT_FAILURE);
			}

			strcpy(expanded, variable_value);

			free(tokens->items[i]);
			tokens->items[i] = expanded;
		}
	}
}

void free_tokens(tokenlist *tokens) {
	for (int i = 0; i < tokens->size; i++)
		free(tokens->items[i]);
	free(tokens->items);
	free(tokens);
}

// ~ expands to env variable $HOME
void expand_tilde(tokenlist *tokens)
{
	for (size_t i = 0; i < tokens->size; i++)
	{
		char *token = tokens->items[i];

		// skips to next iteration if token does not start with ~
		if(token[0] != '~')
			continue;

		// if the char after ~ isn't either the end of the string of /, don't expand it
		if (token[1] != '\0' && token[1] != '/')
			continue;

		// get user's home directory
		char *home = getenv("HOME");

		if (home == NULL)
			continue;

		// allocate space for new expanded string
		// no need for +1 because we're replacing one char, ~, with the home path
		char *expanded = malloc(strlen(home) + strlen(token));

		if (expanded == NULL)
		{
			perror("malloc");
			exit(EXIT_FAILURE);
		}

		// copy home directory and append everything after ~
		// token + 1 means start at the character one position after the beginning of token
		strcpy(expanded, home);
		strcat(expanded, token + 1);

		// free (release memory) old token, replace with expanded token
		free(tokens->items[i]);
		tokens->items[i] = expanded;
	}
}

// PART 7

// identify and count the number of pipes
int count_pipes(tokenlist *tokens)
{
    // identify pipe tokens "|"
    int pipe_count = 0;
    for (int i = 0; i < tokens->size; i++)
    {
        if (strcmp(tokens->items[i], "|") == 0)
        {
            pipe_count++;
        }
    }
	return pipe_count;
}
char ***split_commands(tokenlist *tokens, int pipe_count)
{
	// pipe count + 1 because 0 pipes = 1 command, 1 pipe = two commands, etc.
    char ***commands = malloc((pipe_count + 1) * sizeof(char **));

	if (commands == NULL)
	{
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	int command_start = 0;
	int command_index = 0;

	// split the token list into commands
    for (int i = 0; i <= tokens->size; i++)
    {
		// either we found a pipe or reached the end
        if (i == tokens->size || strcmp(tokens->items[i], "|") == 0)
        {
            int argument_count = i - command_start;

			// allocate individual command's argv array, + 1 is for terminating NULL
			commands[command_index] = malloc((argument_count + 1) * sizeof(char *));

			if (commands[command_index] == NULL)
			{
				perror("malloc");
				exit(EXIT_FAILURE);
			}

			// copy pointers to original token strings
			for (int j = 0; j < argument_count; j++)
			{
				commands[command_index][j] = tokens->items[command_start + j];
			}

			// execv() requires NULL termination; set last token of individual command to NULL
			commands[command_index][argument_count] = NULL;

			command_index++;
			command_start = i + 1;
        }
    }
	return commands;
}
void free_commands(char ***commands, int command_count)
{
    for (int i = 0; i < command_count; i++)
        free(commands[i]);

    free(commands);
}
