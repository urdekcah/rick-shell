#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#include "rickshell.h"
#include "rickshell/error.h"
#include "rickshell/file.h"

char* homepath(const char* file_path) {
  const char* home_dir = getenv("HOME");
  if (home_dir == NULL) RICK_EFAIL("getenv");
  char *full_path = malloc(strlen(home_dir) + strlen(file_path) + 1);
  if (full_path == NULL) RICK_EFAIL("malloc");
  strcpy(full_path, home_dir);
  strcat(full_path, file_path);
  return full_path;
}

void ensure_directory_exists(const char* dir_path) {
  struct stat st = {0};
  if (stat(dir_path, &st) == -1) {
    if (mkdir(dir_path, S_IRWXU) != 0) RICK_EFAIL("mkdir");
  }
}

void ensure_file_exists(const char* file_path) {
  int fd = open(file_path, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
  if (fd == -1) RICK_EFAIL("open");
  close(fd);
}

char* get_current_directory() {
  static char cwd[MAX_PATH_LEN];
  if (!getcwd(cwd, sizeof(cwd))) RICK_EFAIL("getcwd");
  return cwd;
}