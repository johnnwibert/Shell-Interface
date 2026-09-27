#ifndef EXTERNAL_H
#define EXTERNAL_H


#include "lexer.h"

//void execute_external(tokenlist *tokens);

//background-run in background w/out waiting
//cmdline- the orig input line, for jobs/history
int execute_external(tokenlist *tokens, int background, const char *cmdline);

#endif
