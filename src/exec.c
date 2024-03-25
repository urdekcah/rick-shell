#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#include "rickshell.h"
#include "rickshell/file.h"
#include "rickshell/error.h"
#include "rickshell/completion.h"
#include "rickshell/redirect.h"
#include "rickshell/pipeline.h"
#include "rickshell/exec.h"
#include "rickshell/job.h"
#include "rickshell/expr.h"
#include "rickshell/builtin.h"
#include "rickshell/parse.h"

extern char **builtin_str;
extern int (*builtin_func[RICK_BCMD_LEN])(char **);
extern int bufsize_builtin_str;

int execute_builtin(char *args[]);
int execute_child_process(char *args[]);
void handle_background_process(pid_t pid, char *args[]);
void handle_foreground_process(pid_t pid, int *status);
void cleanup_background_process(int *status, char *args[]);

int execute_builtin(char *args[]) {
  for (int i = 0; i < bufsize_builtin_str; i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }
  return BUILTIN_NOT_FOUND;
}

int execute_child_process(char *args[]) {
  if (execvp(args[0], args) == -1) {
    suggest_command(args[0]);
    exit(EXIT_FAILURE);
    return 1;
  }
  return 0;
}

void handle_background_process(pid_t pid, char *args[]) {
  char *command = (char *)malloc(1);
  command[0] = '\0';
  int length = 1;
  for (int i = 0; args[i] != NULL; i++) {
    length += strlen(args[i]) + 1;
    command = (char *)realloc(command, length);
    strcat(command, args[i]);
    strcat(command, " ");
  }
  command[length - 2] = '\0';
  add_job(pid, command);
  free(command);
}

void handle_foreground_process(pid_t pid, int *status) {
  pid_t wpid;
  do {
    wpid = waitpid(pid, status, WUNTRACED);
    if (wpid == -1) RICK_EFAIL("waitpid");
    if (WIFSIGNALED(*status)) printf("Child killed by signal %d\n", WTERMSIG(*status));
  } while (!WIFEXITED(*status) && !WIFSIGNALED(*status));
}

void cleanup_background_process(int *status, char *args[]) {
  pid_t wpid;
  do {
    wpid = waitpid(-1, status, WNOHANG);
    if (wpid > 0) {
      int id = remove_job_by_pid(wpid);
      printf("[%d]+ Done                    %s\n", id, args[0]);
    }
  } while (wpid > 0);
}

int __execvp(const char *__file, char *const *__argv, exec_option_t* opt) {
  if (opt->background) {
    exec_option_t _opt;
    int nargs = 0;
    parse_input_args(__argv, &nargs, &_opt);
    int result = execute_command(__argv, &_opt);
    exit(result);
    return result;
  }
  return execvp(__file, __argv);
}

int execute_command(char *args[], exec_option_t* opt) {
  pid_t pid;
  int status;
  if (opt->background) goto fork;
  if (opt->logical_and) return handle_logical_and(args);
  if (opt->pipeline) return execute_pipeline(args);
  int builtin_status = execute_builtin(args);
  if (builtin_status != BUILTIN_NOT_FOUND) return builtin_status;
fork:
  pid = fork();
  if (pid == -1) RICK_EFAIL("fork");
  if (pid == 0) {
    if (opt->redirect) handle_redirection(args);
    if (__execvp(args[0], args, opt) == -1) {
      suggest_command(args[0]);
      exit(EXIT_FAILURE);
    }
    return 1;
  } else {
    if (opt->background) handle_background_process(pid, args);
    else handle_foreground_process(pid, &status);
  }
  if (opt->background) cleanup_background_process(&status, args);
  return status;
}

int execute_with_pid(pid_t pid, char *args[], exec_option_t* opt) {
  int status = 0;
  if (opt->logical_and) return handle_logical_and(args);
  if (opt->pipeline) return execute_pipeline(args);
  int builtin_status = execute_builtin(args);
  if (builtin_status != -1) return builtin_status;
  if (opt->redirect) handle_redirection(args);
  execute_child_process(args);
  return status;
}