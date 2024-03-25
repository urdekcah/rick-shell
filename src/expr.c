#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "rickshell.h"
#include "rickshell/error.h"
#include "rickshell/exec.h"
#include "rickshell/expr.h"
#include "rickshell/parse.h"

int handle_logical_and(char *args[]) {
  int i = 0, start = 0;
  char** cmd = NULL;
  int cmd_size = 0;
  while (args[i] != NULL) {
    if (strcmp(args[i], "&&") == 0 || args[i+1] == NULL) {
      int end = (args[i+1] == NULL) ? i + 1 : i;
      int num_cmds = end - start;
      cmd_size = num_cmds + 1;
      cmd = (char **)malloc(sizeof(char *) * cmd_size);
      if (cmd == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
      }
      for (int j = 0; j < num_cmds; j++)
        cmd[j] = args[start + j];
      cmd[num_cmds] = NULL;
      exec_option_t opt;
      parse_input_args(cmd, &num_cmds, &opt);
      if (execute_command(cmd, &opt) != 0) {
        free(cmd);
        break;
      }
      free(cmd);
      cmd = NULL;
      start = i + 1;
    }
    i++;
  }
  return 0;
}