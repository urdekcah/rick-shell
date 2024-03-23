#include <stdio.h>
#include <string.h>

#include "rickshell.h"
#include "rickshell/error.h"
#include "rickshell/init.h"
#include "rickshell/builtin.h"

extern char **builtin_str;
extern int (*builtin_func[RICK_BCMD_LEN])(char **);
extern int bufsize_builtin_str;

void rsh_init_builtins() {
  char* builtins[] = {"help", "cd", "history", "exit", "echo"};
  int (*builtin_funcs[])(char **) = {&builtin_help, &builtin_cd, &builtin_history, &builtin_exit, &builtin_echo};
  builtin_str = calloc(bufsize_builtin_str, sizeof(char *));
  if (!builtin_str) RICK_EFAIL("calloc");
  for (int i = 0; i < bufsize_builtin_str; i++) {
    builtin_str[i] = strdup(builtins[i]);
    if (!builtin_str[i]) RICK_EFAIL("strdup");
  }
  for (int i = 0; i < bufsize_builtin_str; i++)
    builtin_func[i] = builtin_funcs[i];
}