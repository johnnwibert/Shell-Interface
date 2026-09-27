#ifndef JOBS_H
#define JOBS_H

#include <sys/types.h>

//Background hob tracking (part 8)


#define JOB_START_FMT "[%d] %d\n"		//job number, PID
#define JOB_DONE_FMT  "[%d]+ done %s\n"		//job number, command line
#define JOB_LIST_FMT  "[%d]+ %d %s\n"		//job number, PID, command line

void jobs_init(void);


//Registers a background job and prints the start line thne returns new job number :)
int jobs_add(const pid_t *pids, int npids, const char *cmdline);

//checks for finished jobs and prints done line for each
void jobs_reap_finished(void);

//waits for every remaing job, prints done lien for each
void jobs_wait_all(void);

//Printts active jobs or a messsage if there happens to be none
void jobs_print_active(void);

int jobs_active_count(void);

void jobs_cleanup(void);

#endif
