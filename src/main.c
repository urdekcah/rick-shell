#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <locale.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "rickshell.h"
#include "rickshell/file.h"
#include "rickshell/error.h"
#include "rickshell/pipeline.h"
#include "rickshell/init.h"
#include "rickshell/builtin.h"
#include "rickshell/signal.h"
#include "rickshell/history.h"
#include "rickshell/system.h"
#include "rickshell/environment.h"
#include "rickshell/exec.h"
#include "rickshell/color.h"
#include "rickshell/parse.h"
#include "rickshell/release.h"

extern char **builtin_str;
extern int (*builtin_func[RICK_BCMD_LEN])(char **);
extern int bufsize_builtin_str;

#define PATH_ENV "PATH=/bin:/usr/bin"

void execute(char **args/*, int num_args*/) {
  int builtined = 0;
  for (int i = 0; i < bufsize_builtin_str; i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      builtined = 1;
      int status = (*builtin_func[i])(args);
      if (status != 0 && status != RE_NEED_EXIT) 
        fprintf(stderr, "Command failed with exit code %d\n", status);
      break;
    }
  }
  if (!builtined) {
    int redirect = 0, pipeline = 0;
    execute_command(args, 0, redirect, pipeline);
  }
}

int main() {
  setlocale(LC_ALL, "");
  signal(SIGINT, handle_signal);
  rsh_init_builtins();
  load_history();
  char *path = strdup("PATH=/bin:/usr/bin");
  if (!path) {
    fprintf(stderr, "Memory allocation failed\n");
    return 1;
  }
  add_env_var(path);
  free(path);
  while (1) {
    if (_exit_code_set()) {
      printf("Goodbye!\n");
      break;
    }
    char *cwd = get_current_directory();
    char *username = get_username();
    char *host = get_hostname();
    char prompt[MAX_PATH_LEN + 50];
    snprintf(prompt, sizeof(prompt), ANSI_COLOR_BOLD_BLUE "%s@%s" ANSI_COLOR_BOLD_BLACK ":" ANSI_COLOR_RESET ANSI_COLOR_BOLD_YELLOW "%s" ANSI_COLOR_BOLD_RED "$" ANSI_COLOR_RESET " ", username, host, cwd);
    char *input = readline(prompt);
    if (!input) {
      printf("\n");
      break;
    }
    if (strlen(input) > 0) {
      int num_args = 0;
      char **args = parse_input(input, &num_args);
      if (args == NULL) {
        free(args);
        free(input);
        continue;
      }
      execute(args/*, num_args*/);
      add_history(input);
      for (int i = 0; i < num_args; i++) free(args[i]);
      free(args);
    }
    free(input);
  }
  rsh_release();
  exit(_get_exit());
  return 0;
}