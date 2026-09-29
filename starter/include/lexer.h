#pragma once

#include <stdlib.h>
#include <stdbool.h>

typedef struct {
    char ** items;
    size_t size;
} tokenlist;

char * get_input(void);
tokenlist * get_tokens(char *input);
tokenlist * new_tokenlist(void);
void add_token(tokenlist *tokens, char *item);
void expand_env_variables(tokenlist *tokens);
void expand_tilde(tokenlist *tokens);
void free_tokens(tokenlist *tokens);
int count_pipes(tokenlist *tokens);

char ***split_commands(tokenlist *tokens, int pipe_count);

void free_commands(char ***commands, int command_count);
