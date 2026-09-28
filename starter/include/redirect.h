#ifndef REDIRECT_H
#define REDIRECT_H

//I/O  redirection Part 6: cmd < filein / cmd > fileout


//file names for one command. Null if not given
typedef struct {
    const char *in_file;    
    const char *out_file;   
} redirect_t;

//Saved fds so redirect_restore can undo redirect apply
typedef struct {
    int saved_in;
    int saved_out;
} redirect_saved_t;

//pulls redirects out of a commands tokens, leaving a clean argv for execvI. retuns zero one success  // -1 on syntax error
int redirect_split(char *const *tokens, int ntokens,
                   char ***argv_out, int *argc_out, redirect_t *r);

//checks the input file exists and is a reg file
int redirect_check_input(const redirect_t *r);

//redirects stdin stdout to the files. pass saved == NULL in a forked child. pass struct inside the   // shell. returns either zero or -1 afterng print error
int redirect_apply(const redirect_t *r, redirect_saved_t *saved);

void redirect_restore(redirect_saved_t *saved);

#endif
