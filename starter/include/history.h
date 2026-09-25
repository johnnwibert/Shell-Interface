#ifndef HISTORY_H
#define HISTORY_H

/* Number of valid commands remembered for the `exit` built-in. */
#define HISTORY_SIZE 3

/* Record a command line that was valid (parsed, resolved, and launched). */
void history_add(const char *cmdline);

/*
 * Print what `exit` must show:
 *   - 3 or more valid commands so far -> the last three
 *   - 1 or 2 valid commands so far    -> just the last valid one
 *   - none                            -> a message saying so
 */
void history_print_last(void);

void history_cleanup(void);

#endif
