#include <stdio.h>
#include <stdlib.h>
#include "history.h"
#include "util.h"

/* entries[HISTORY_SIZE - 1] is the most recent; entries[0] the oldest kept. */
static char *entries[HISTORY_SIZE];
static int stored = 0;   /* how many slots are filled (max HISTORY_SIZE) */

void history_add(const char *cmdline)
{
    free(entries[0]);
    for (int i = 0; i < HISTORY_SIZE - 1; i++)
        entries[i] = entries[i + 1];
    entries[HISTORY_SIZE - 1] = xstrdup(cmdline);

    if (stored < HISTORY_SIZE)
        stored++;
}

void history_print_last(void)
{
    if (stored == 0) {
        printf("No valid commands.\n");
    } else if (stored < HISTORY_SIZE) {
        printf("Last valid command:\n%s\n", entries[HISTORY_SIZE - 1]);
    } else {
        printf("Last three valid commands:\n");
        for (int i = 0; i < HISTORY_SIZE; i++)
            printf("%s\n", entries[i]);
    }
}

void history_cleanup(void)
{
    for (int i = 0; i < HISTORY_SIZE; i++) {
        free(entries[i]);
        entries[i] = NULL;
    }
    stored = 0;
}
