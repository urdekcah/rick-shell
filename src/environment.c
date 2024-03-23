#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rickshell.h"
#include "rickshell/error.h"
#include "rickshell/environment.h"

char **env_vars = NULL;
int env_var_count = 0;
int env_var_capacity = 0;

void add_env_var(char *variable) {
  if (env_var_count >= env_var_capacity) {
    env_var_capacity += 10;
    env_vars = realloc(env_vars, sizeof(char *) * env_var_capacity);
    if (env_vars == NULL) RICK_EFAIL("realloc");
  }
  env_vars[env_var_count] = strdup(variable);
}

char* get_env_value(char *name) {
  for (int i = 0; i < env_var_count; i++) {
    char *env_var = env_vars[i];
    if (strncmp(env_var, name, strlen(name)) == 0) {
      char *value = strchr(env_var, '=') + 1;
      return value;
    }
  }
  return NULL;
}