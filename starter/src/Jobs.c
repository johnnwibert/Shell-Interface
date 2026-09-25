#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "jobs.h"
#include "util.h"

typedef struct job {
    int number;
    pid_t *pids;      /* one per process in the pipeline; -1 once reaped */
    int npids;
    pid_t last_pid;   /* PID reported to the user */
    char *cmdline;
    struct job *next;
} job_t;

static job_t *job_head = NULL;
static job_t *job_tail = NULL;
static int next_job_number = 1;   /* never decremented: numbers are not reused */

void jobs_init(void)
{
    job_head = NULL;
    job_tail = NULL;
    next_job_number = 1;
}

/* Try to reap one child. Returns 1 if it is gone, 0 if it is still running. */
static int reap_pid(pid_t pid, int block)
{
    int status;

    for (;;) {
        pid_t r = waitpid(pid, &status, block ? 0 : WNOHANG);
        if (r == pid)
            return 1;
        if (r == 0)
            return 0;
        if (errno == EINTR)
            continue;
        return 1;   /* ECHILD etc.: nothing left to wait for */
    }
}

/* Reap what we can of a job. Returns 1 when every process has finished. */
static int update_job(job_t *j, int block)
{
    int all_done = 1;

    for (int i = 0; i < j->npids; i++) {
        if (j->pids[i] < 0)
            continue;
        if (reap_pid(j->pids[i], block))
            j->pids[i] = -1;
        else
            all_done = 0;
    }
    return all_done;
}

static void free_job(job_t *j)
{
    free(j->pids);
    free(j->cmdline);
    free(j);
}

int jobs_add(const pid_t *pids, int npids, const char *cmdline)
{
    if (npids <= 0)
        return -1;

    job_t *j = xmalloc(sizeof *j);
    j->number = next_job_number++;
    j->pids = xmalloc((size_t)npids * sizeof(pid_t));
    memcpy(j->pids, pids, (size_t)npids * sizeof(pid_t));
    j->npids = npids;
    j->last_pid = pids[npids - 1];
    j->cmdline = xstrdup(cmdline);
    j->next = NULL;

    if (job_tail != NULL)
        job_tail->next = j;
    else
        job_head = j;
    job_tail = j;

    printf(JOB_START_FMT, j->number, (int)j->last_pid);
    return j->number;
}

void jobs_reap_finished(void)
{
    job_t *prev = NULL;
    job_t *j = job_head;

    while (j != NULL) {
        job_t *next = j->next;

        if (update_job(j, 0)) {
            printf(JOB_DONE_FMT, j->number, j->cmdline);
            if (prev != NULL)
                prev->next = next;
            else
                job_head = next;
            if (j == job_tail)
                job_tail = prev;
            free_job(j);
        } else {
            prev = j;
        }
        j = next;
    }
}

void jobs_wait_all(void)
{
    while (job_head != NULL) {
        job_t *j = job_head;

        update_job(j, 1);
        printf(JOB_DONE_FMT, j->number, j->cmdline);
        job_head = j->next;
        free_job(j);
    }
    job_tail = NULL;
}

void jobs_print_active(void)
{
    if (job_head == NULL) {
        printf("No active background processes.\n");
        return;
    }
    for (job_t *j = job_head; j != NULL; j = j->next)
        printf(JOB_LIST_FMT, j->number, (int)j->last_pid, j->cmdline);
}

int jobs_active_count(void)
{
    int n = 0;

    for (job_t *j = job_head; j != NULL; j = j->next)
        n++;
    return n;
}

void jobs_cleanup(void)
{
    while (job_head != NULL) {
        job_t *j = job_head;
        job_head = j->next;
        free_job(j);
    }
    job_tail = NULL;
}
