#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "rickshell.h"
#include "rickshell/math.h"
#include "rickshell/error.h"
#include "rickshell/completion.h"

int contains(char **arr, int len, const char *str) {
  for (int i = 0; i < len; i++)
    if (strcmp(arr[i], str) == 0) return 1;
  return 0;
}

void add_suggestion(char ***suggestions, int *num_suggestions, int *capacity, const char *suggestion) {
  if (*num_suggestions == *capacity) {
    *capacity *= 2;
    char **temp = realloc(*suggestions, (*capacity) * sizeof(char *));
    if (!temp) RICK_ERETN("Failed to realloc suggestions");
    *suggestions = temp;
  }
  char *dup = strdup(suggestion);
  if (!dup) RICK_ERETN("Failed to strdup suggestion");
  (*suggestions)[*num_suggestions] = dup;
  (*num_suggestions)++;
}

void explore_directory(const char *dir_path, const char *cmd, char ***suggestions, int *num_suggestions, int *capacity) {
  DIR *dir = opendir(dir_path);
  if (!dir) return;
  struct dirent *entry;
  while ((entry = readdir(dir)) != NULL) {
    if (entry->d_type == 8) {
      if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
      int sim = similar(cmd, entry->d_name);
      if (sim < 3 && !contains(*suggestions, *num_suggestions, entry->d_name)) 
        add_suggestion(suggestions, num_suggestions, capacity, entry->d_name);
    }
  }
  closedir(dir);
}

void suggest_command(const char *cmd) {
  char *path = getenv("PATH");
  if (path == NULL) RICK_ERETN("Failed to get PATH")
  char **suggestions = malloc(sizeof(char *));
  if (!suggestions) RICK_ERETN("Failed to malloc suggestions")
  int num_suggestions = 0, capacity = 1;
  char *path_copy = strdup(path);
  if (!path_copy) {
    perror("Failed to strdup PATH");
    free(suggestions);
    return;
  }
  char *token;
  char *saveptr;
  for (token = strtok_r(path_copy, ":", &saveptr); token != NULL; token = strtok_r(NULL, ":", &saveptr))
    explore_directory(token, cmd, &suggestions, &num_suggestions, &capacity);
  if (num_suggestions == 0) {fprintf(stderr, "%s: command not found\n", cmd);}
  else {
    fprintf(stderr, "Command '%s' not found, did you mean:\n", cmd);
    for (int i = 0; i < num_suggestions; i++)
      fprintf(stderr, "  command '%s'\n", suggestions[i]);
    fprintf(stderr, "Try: sudo apt install <package-name>\n");
  }
  for (int i = 0; i < num_suggestions; i++)
    free(suggestions[i]);
  free(suggestions);
  free(path_copy);
}