#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#include "rickshell.h"
#include "rickshell/file.h"
#include "rickshell/error.h"
#include "rickshell/completion.h"
#include "rickshell/redirect.h"
#include "rickshell/pipeline.h"
#include "rickshell/exec.h"

void execute_command(char *args[], int background, int redirect, int pipeline) {
  pid_t pid = fork();
  if (pid == -1) RICK_EFAIL("fork");
  if (pid == 0) {
    if (redirect) handle_redirection(args);
    if (pipeline) {
      execute_pipeline(args);
      exit(0); 
    } else if (execvp(args[0], args) == -1) {
      perror(args[0]);
      exit(EXIT_FAILURE);
    }
  } else {
    if (!background) {
      int status;
      waitpid(pid, &status, 0);
      if (WIFEXITED(status)) {
        if (WEXITSTATUS(status) != 0)
          fprintf(stderr, "Command failed with exit code %d\n", WEXITSTATUS(status));
      } else if (WIFSIGNALED(status)) fprintf(stderr, "Command terminated by signal %d\n", WTERMSIG(status));
    }
  }
}