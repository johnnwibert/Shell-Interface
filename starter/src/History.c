#include <stdio.h>
#include <stdlib.h>
#include "history.h"
#include "util.h"

//keeps track of the last few commands that ran successfully
//entries[HISTORY_SIZE - 1] is the newest one, entries[0] is the oldest
static char *entries[HISTORY_SIZE];
static int stored = 0;   //how many of the slots above are actually filled

void history_add(const char *cmdline)
{
    //drop the oldest command and shift everything down by one spot
    free(entries[0]);
    for (int i = 0; i < HISTORY_SIZE - 1; i++)
        entries[i] = entries[i + 1];

    //put the new command in the last slot
    entries[HISTORY_SIZE - 1] = xstrdup(cmdline);

    if (stored < HISTORY_SIZE)
        stored++;
}

void history_print_last(void)
{
    //exit needs to print different things depending on how many commands we actually have stored
    if (stored == 0) {
        printf("No valid commands.\n");
    } else if (stored < HISTORY_SIZE) {
        //less than 3 so far, just print most recent one
        printf("Last valid command:\n%s\n", entries[HISTORY_SIZE - 1]);
    } else {
        //we have 3 or more,print all of them oldest to newest
        printf("Last three valid commands:\n");
        for (int i = 0; i < HISTORY_SIZE; i++)
            printf("%s\n", entries[i]);
    }
}

void history_cleanup(void)
{
    //free everything before the program ends so we no leak memory
    for (int i = 0; i < HISTORY_SIZE; i++) {
        free(entries[i]);
        entries[i] = NULL;
    }
    stored = 0;
}
