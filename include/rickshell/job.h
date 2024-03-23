#ifndef __RICKSHELL_JOB_H__
#define __RICKSHELL_JOB_H__
#include <unistd.h>
#include <sys/time.h>

typedef enum job_status {
  RUNNING, COMPLETED, STOPPED
} job_status;

typedef struct job {
  int id;
  pid_t pid;
  char* command;
  struct timeval start_time;
  struct timeval end_time;
  job_status status;
  struct job* next;
} job;

void add_job(pid_t pid, const char* command);
int remove_job(int id);
int remove_job_by_pid(pid_t pid);
void update_job_status(pid_t pid, int status);
void print_jobs();
job* find_job(pid_t pid);
void free_job(job* job);
char* status_to_string(job_status status);
void check_background_jobs();
#endif