#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "rickshell.h"
#include "rickshell/error.h"
#include "rickshell/parse.h"

#define ARGS_CAPACITY_INCREMENT 10
#define IFS " \t\n"

void remove_quotes(char *str) {
  char *p1 = str;
  char *p2 = str;
  while (*p1) {
    if (*p1 != '\"' && *p1 != '\'') {
      *p2++ = *p1;
    }
    p1++;
  }
  *p2 = '\0';
}

void add_arg(char ***args, int *num_args, const char *arg, int *args_capacity) {
  if (*num_args >= *args_capacity) {
    *args_capacity += ARGS_CAPACITY_INCREMENT;
    char **new_args = realloc(*args, (*args_capacity + 1) * sizeof(char *));
    if (!new_args) {
      fprintf(stderr, "Memory allocation failed\n");
      exit(1);
    }
    *args = new_args;
  }
  (*args)[*num_args] = strdup(arg);
  if (!(*args)[*num_args]) {
    fprintf(stderr, "Memory allocation failed\n");
    exit(1);
  }
  (*num_args)++;
}

char **parse_input(char *input, int *num_args, int *redirect, int *pipeline, int* background) {
  char *symbols[] = { ">", "<", ">>", "<<", "2>", "2>>", "&>", "&>>", "|", "&", NULL };
  int args_capacity = ARGS_CAPACITY_INCREMENT;
  char **args = malloc((args_capacity + 1) * sizeof(char *));
  if (!args) {
    fprintf(stderr, "Memory allocation failed\n");
    return NULL;
  }
  *num_args = 0;
  *redirect = 0;
  *pipeline = 0;
  *background = 0;
  char *cursor = input;
  while (*cursor) {
    if (isspace((unsigned char)*cursor)) {
      cursor++;
      continue;
    }
    int found_symbol = 0;
    for (int i = 0; symbols[i] != NULL; i++) {
      int symbol_len = strlen(symbols[i]);
      if (strncmp(cursor, symbols[i], symbol_len) == 0) {
        found_symbol = 1;
        if (i < 8) *redirect = 1;
        else if (i == 8) *pipeline = 1;
        else if (i == 9) *background = 1;
        if (i!=9) add_arg(&args, num_args, symbols[i], &args_capacity);
        cursor += symbol_len;
        break;
      }
    }
    if (found_symbol) continue;
    char *start_of_arg = cursor;
    while (*cursor && !isspace((unsigned char)*cursor) && strchr("<>&|", *cursor) == NULL) cursor++;
    if (cursor > start_of_arg) {
      char *arg = strndup(start_of_arg, cursor - start_of_arg);
      add_arg(&args, num_args, arg, &args_capacity);
      free(arg);
    }
  }
  args[*num_args] = NULL;
  return args;
}