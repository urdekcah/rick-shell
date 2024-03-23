#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>

#include "rickshell.h"
#include "rickshell/error.h"
#include "rickshell/job.h"

job* jobs_head = NULL;
int job_counter = 0;
int available_id = 1;

int find_available_job_id() {
  int current_id = available_id;
  job* j = jobs_head;
  while (j != NULL) {
    if (j->id >= current_id) current_id = j->id + 1;
    j = j->next;
  }
  available_id = current_id;
  return current_id;
}

void add_job(pid_t pid, const char* command) {
  check_background_jobs();
  job* new_job = (job*)malloc(sizeof(job));
  if (!new_job) RICK_EFAIL("malloc");
  new_job->id = find_available_job_id();
  new_job->pid = pid;
  new_job->command = strdup(command);
  gettimeofday(&new_job->start_time, NULL);
  new_job->status = RUNNING;
  new_job->next = jobs_head;
  jobs_head = new_job;
  printf("[%d] %d\n", new_job->id, new_job->pid);
}

int remove_job(int id) {
  int job_id;
  job **indirect = &jobs_head, *temp;
  while (*indirect && (*indirect)->id != id)
    indirect = &(*indirect)->next;
  if (*indirect == NULL) return -1;
  job_id = (*indirect)->id;
  temp = *indirect;
  *indirect = temp->next;
  free_job(temp);
  return job_id;
}

int remove_job_by_pid(pid_t pid) {
  job **indirect = &jobs_head, *temp;
  while (*indirect && (*indirect)->pid != pid)
    indirect = &(*indirect)->next;
  if (*indirect == NULL) return -1;
  int job_id = (*indirect)->id;
  temp = *indirect;
  *indirect = temp->next;
  free_job(temp);
  return job_id;
}

void update_job_status(pid_t pid, int status) {
  job* j = find_job(pid);
  if (j) {
    j->status = status;
    gettimeofday(&j->end_time, NULL);
  }
}

void print_jobs() {
  for (job* j = jobs_head; j != NULL; j = j->next) 
    printf("[%d] %s - %s\n", j->id, j->command, status_to_string(j->status));
}

job* find_job(pid_t pid) {
  for (job* j = jobs_head; j != NULL; j = j->next)
    if (j->pid == pid) return j;
  return NULL;
}

void free_job(job* j) {
  if (j) {
    free(j->command);
    free(j);
  }
}

char* status_to_string(job_status status) {
  switch (status) {
    case RUNNING: return "Running";
    case COMPLETED: return "Completed";
    case STOPPED: return "Stopped";
    default: return "Unknown";
  }
}

void check_background_jobs() {
  int status;
  pid_t pid;
  while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
    update_job_status(pid, WIFEXITED(status) ? COMPLETED : STOPPED);
    char* command = strdup(find_job(pid)->command);
    int removed_job_id = remove_job_by_pid(pid);
    if (removed_job_id != -1) printf("[%d]+ Done                    %s\n", removed_job_id, command);
    free(command);
  }
  if (jobs_head == NULL) available_id = 1;
}