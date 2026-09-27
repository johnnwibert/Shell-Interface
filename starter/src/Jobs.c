#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "jobs.h"
#include "util.h"

//never have more than 10 background jobs running at once and never more than 2 pipes
// at once, and never more than 2 pipes (so at most 3 commands in a pipeline)
#define MAX_JOBS 10
#define MAX_PIDS_PER_JOB 3
#define MAX_CMDLINE 256

typedef struct {
    int active;             //zeor if this slot is empty / already reaped
    int number;             //job number shown to the user, like [1], [2]...
    pid_t pids[MAX_PIDS_PER_JOB];
    int npids;
    pid_t last_pid;         //the pid we actually print for this job
    char cmdline[MAX_CMDLINE];
} job_t;

static job_t jobs[MAX_JOBS];
static int job_count = 0;         //how many slots in the array are used
static int next_job_number = 1;   //keeps going up never goes back down

void jobs_init(void)
{
    for (int i = 0; i < MAX_JOBS; i++)
        jobs[i].active = 0;
    job_count = 0;
    next_job_number = 1;
}

//checks on one process without blocking
//returns one if done zero if still running
static int reap_pid(pid_t pid, int block)
{
    int status;

    while (1) {
        pid_t r = waitpid(pid, &status, block ? 0 : WNOHANG);
        if (r == pid)
            return 1;
        if (r == 0)
            return 0;
        if (errno == EINTR)
            continue;      //interrupted by a signal,try again
        return 1;          //something like ECHILD, treat it as done
    }
}

//check every process in one job. returns one once they've all finished
static int job_is_done(job_t *j, int block)
{
    int all_done = 1;

    for (int i = 0; i < j->npids; i++) {
        if (j->pids[i] < 0)
            continue;   //already reaped earlier
        if (reap_pid(j->pids[i], block))
            j->pids[i] = -1;
        else
            all_done = 0;
    }
    return all_done;
}

int jobs_add(const pid_t *pids, int npids, const char *cmdline)
{
    if (npids <= 0 || npids > MAX_PIDS_PER_JOB || job_count >= MAX_JOBS)
        return -1;

    job_t *j = &jobs[job_count];
    job_count++;

    j->active = 1;
    j->number = next_job_number;
    next_job_number++;

    j->npids = npids;
    for (int i = 0; i < npids; i++)
        j->pids[i] = pids[i];
    j->last_pid = pids[npids - 1];

    strncpy(j->cmdline, cmdline, MAX_CMDLINE - 1);
    j->cmdline[MAX_CMDLINE - 1] = '\0';

    printf(JOB_START_FMT, j->number, (int)j->last_pid);
    return j->number;
}

void jobs_reap_finished(void)
{
    for (int i = 0; i < job_count; i++) {
        if (!jobs[i].active)
            continue;

        if (job_is_done(&jobs[i], 0)) {
            printf(JOB_DONE_FMT, jobs[i].number, jobs[i].cmdline);
            jobs[i].active = 0;
        }
    }
}

void jobs_wait_all(void)
{
    for (int i = 0; i < job_count; i++) {
        if (!jobs[i].active)
            continue;

        job_is_done(&jobs[i], 1);   //block until this one finishes
        printf(JOB_DONE_FMT, jobs[i].number, jobs[i].cmdline);
        jobs[i].active = 0;
    }
}

void jobs_print_active(void)
{
    int printed = 0;

    for (int i = 0; i < job_count; i++) {
        if (!jobs[i].active)
            continue;
        printf(JOB_LIST_FMT, jobs[i].number, (int)jobs[i].last_pid, jobs[i].cmdline);
        printed++;
    }

    if (printed == 0)
        printf("No active background processes.\n");
}

int jobs_active_count(void)
{
    int n = 0;

    for (int i = 0; i < job_count; i++) {
        if (jobs[i].active)
            n++;
    }
    return n;
}

void jobs_cleanup(void)
{
    //nothing to free it's just a plain array, reset it anyway
    for (int i = 0; i < MAX_JOBS; i++)
        jobs[i].active = 0;
    job_count = 0;
}
