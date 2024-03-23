#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>

#include "rickshell.h"
#include "rickshell/error.h"
#include "rickshell/pipeline.h"

char **split_args(char *args[], int start, int end) {
  int length = end - start;
  char **new_args = malloc((length + 1) * sizeof(char *));
  memcpy(new_args, &args[start], length * sizeof(char *));
  new_args[length] = NULL;
  return new_args;
}

void execute_pipeline(char *args1[]) {
  int fd[2];
  int in = 0;
  int start = 0;
  int i;

  for (i = 0; args1[i] != NULL; i++) {
    if (strcmp(args1[i], "|") == 0) {
      args1[i] = NULL;
      char **cmd = split_args(args1, start, i);
      if (pipe(fd) == -1) RICK_EFAIL("pipe");
      pid_t pid = fork();
      if (pid == -1) RICK_EFAIL("fork");
      if (pid == 0) {
        close(fd[0]);
        dup2(in, STDIN_FILENO);
        dup2(fd[1], STDOUT_FILENO);
        close(fd[1]);
        if (execvp(cmd[0], cmd) == -1) RICK_EFAIL("execvp");
        exit(EXIT_FAILURE);
      }
      close(fd[1]);
      in = fd[0];
      start = i + 1;
    }
  }

  char **cmd = split_args(args1, start, i);
  pid_t pid = fork();
  if (pid == -1) RICK_EFAIL("fork");
  if (pid == 0) {
    dup2(in, STDIN_FILENO);
    if (in != 0) close(in);
    if (execvp(cmd[0], cmd) == -1) RICK_EFAIL("execvp");
    exit(EXIT_FAILURE);
  }

  if (in != 0) close(in);

  while ((pid = waitpid(-1, NULL, 0))) {
    if (errno == ECHILD) break;
  }
}