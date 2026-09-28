#ifndef HISTORY_H
#define HISTORY_H


//how many commands exit rememebrs
#define HISTORY_SIZE 3

//records a command that ran successfully
void history_add(const char *cmdline);

//prints last three valid commands
void history_print_last(void);

void history_cleanup(void);

#endif
