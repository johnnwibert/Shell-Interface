#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
	while (1) {
		print_prompt();

		/* input contains the whole command
		 * tokens contains substrings from input split by spaces
		 */

		char *input = get_input();
		printf("whole input: %s\n", input);

		tokenlist *tokens = get_tokens(input);
		expand_env_variables(tokens);

		for (int i = 0; i < tokens->size; i++) {
			printf("token %d: (%s)\n", i, tokens->items[i]);
		}

		free(input);
		free_tokens(tokens);
	}

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
