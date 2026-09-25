#ifndef JOBS_H
#define JOBS_H

#include <sys/types.h>

/*
 * Background job bookkeeping (Part 8).
 *
 * Output formats live here so they are easy to change in one place.
 *   start : [Job number] [PID of the last process in the pipeline]
 *   done  : [Job number]+ done [command line]
 *   list  : [Job number]+ [PID] [command line]
 */
#define JOB_START_FMT "[%d] %d\n"
#define JOB_DONE_FMT  "[%d]+ done %s\n"
#define JOB_LIST_FMT  "[%d]+ %d %s\n"

void jobs_init(void);

/*
 * Register a background job made of npids processes (one per command in the
 * pipeline, in pipeline order). Assigns the next job number (numbers are never
 * reused), prints the "start" line, and returns the job number.
 */
int jobs_add(const pid_t *pids, int npids, const char *cmdline);

//Non-blocking: reap finished jobs and print a "done" line for each. */
void jobs_reap_finished(void);

//Blocking: wait for every remaining job, printing a "done" line for each. */
void jobs_wait_all(void);

//Print active jobs, or a message if there are none (the `jobs` built-in).
void jobs_print_active(void);

int jobs_active_count(void);

void jobs_cleanup(void);

#endif
