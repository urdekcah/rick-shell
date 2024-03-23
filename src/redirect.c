#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#include "rickshell.h"
#include "rickshell/error.h"
#include "rickshell/redirect.h"

int open_file_descriptor(char *path, int flags, mode_t mode) {
  int fd = open(path, flags, mode);
  if (fd < 0) RICK_EFAIL("open");
  return fd;
}

void redirect_output(int *fd, int newfd) {
  dup2(*fd, newfd);
  close(*fd);
}

void handle_redirection(char *args[]) {
  for (int i = 0; args[i] != NULL; i++) {
    if (strcmp(args[i], ">") == 0 || strcmp(args[i], ">>") == 0 || 
        strcmp(args[i], "2>") == 0 || strcmp(args[i], "2>>") == 0 || 
        strcmp(args[i], "&>") == 0 || strcmp(args[i], "&>>") == 0) {
      if (args[i + 1] == NULL) {
        fprintf(stderr, "Error: No file specified for redirection\n");
        exit(EXIT_FAILURE);
      }
      int flags = O_WRONLY | O_CREAT | (strcmp(args[i], ">>") == 0 || strcmp(args[i], "2>>") == 0 || strcmp(args[i], "&>>") == 0 ? O_APPEND : O_TRUNC);
      int fd = open_file_descriptor(args[i + 1], flags, 0644);
      if (strcmp(args[i], "2>") == 0 || strcmp(args[i], "2>>") == 0) {
        redirect_output(&fd, STDERR_FILENO);
      } else {
        redirect_output(&fd, STDOUT_FILENO);
        if (strcmp(args[i], "&>") == 0 || strcmp(args[i], "&>>") == 0)
          redirect_output(&fd, STDERR_FILENO);
      }
      args[i] = NULL;
      break;
    }
  }
}