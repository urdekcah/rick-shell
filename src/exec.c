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

void execute_command(char *args[], int background, int redirect, int pipeline) {
  pid_t pid, wpid;
  int status;
  pid = fork();
  if (pid == -1) RICK_EFAIL("fork");
  if (pid == 0) {
    if (redirect) handle_redirection(args);
    if (pipeline) {
      execute_pipeline(args);
      exit(0); 
    } else if (execvp(args[0], args) == -1) {
      suggest_command(args[0]);
      exit(EXIT_FAILURE);
    }
  } else {
    if (background) {
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
    } else {
      do {
        wpid = waitpid(pid, &status, WUNTRACED);
        if (wpid == -1) RICK_EFAIL("waitpid");
        if (WIFEXITED(status)) {
          // if (WEXITSTATUS(status) != 0)
          //   fprintf(stderr, "Command failed with exit code %d\n", WEXITSTATUS(status));
        }
        else if (WIFSIGNALED(status)) printf("Child killed by signal %d\n", WTERMSIG(status));
      } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
  }
  if (background) {
    do {
      wpid = waitpid(-1, &status, WNOHANG);
      if (wpid > 0) {
        int id = remove_job_by_pid(wpid);
        printf("[%d]+ Done                    %s\n", id, args[0]);
      }
    } while (wpid > 0);
  }
}
