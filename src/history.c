#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <readline/history.h>

#include "rickshell.h"
#include "rickshell/error.h"
#include "rickshell/file.h"
#include "rickshell/history.h"

void load_history() {
  char* history_file = homepath(RICK_HISTORY_FILE);
  ensure_file_exists(history_file);
  FILE *file = fopen(history_file, "r");
  if (!file) return;
  char *line = NULL;
  size_t len = 0;
  ssize_t read;
  while ((read = getline(&line, &len, file)) != -1) {
    if (read > 0 && line[read - 1] == '\n') line[read - 1] = '\0';
    add_history(line);
  }
  free(line);
  fclose(file);
  free(history_file);
}

void save_history() {
  char* history_file = homepath(RICK_HISTORY_FILE);
  ensure_file_exists(history_file);
  FILE *file = fopen(history_file, "w");
  if (!file) RICK_ERETN("fopen");
  HISTORY_STATE *hist_state = history_get_history_state();
  for (int i = 0; i < hist_state->length; i++)
    fprintf(file, "%s\n", hist_state->entries[i]->line);
  fclose(file);
  free(history_file);
  free(hist_state);
}