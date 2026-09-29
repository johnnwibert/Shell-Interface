#ifndef BUILTINS_H
#define BUILTINS_H

//looks up a builtin by name, returns id, or -1 if not found
int builtin_lookup(const char *name);


//ruins built-ins. returns zero on success, non zero on error
int builtin_run(int id, int argc, char **argv, int in_child);

//runs exit logic. watit for jobs, prints history, stops loop
void builtins_exit_shell(void);

// True once xit has been called
int builtins_exit_requested(void);

#endif
