#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "rickshell.h"
#include "rickshell/error.h"
#include "rickshell/parse.h"
#include "rickshell/exec.h"

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

void add_arg(char ***args, int *num_args, char *arg, int *args_capacity) {
  if (*num_args >= *args_capacity) {
    *args_capacity += ARGS_CAPACITY_INCREMENT;
    char **new_args = realloc(*args, (*args_capacity + 1) * sizeof(char *));
    if (!new_args) {
      fprintf(stderr, "Memory allocation failed\n");
      for (int i = 0; i < *num_args; i++) free((*args)[i]);
      free(*args);
      *args = NULL;
      return;
    }
  }
  (*args)[*num_args] = strdup(arg);
  if (!(*args)[*num_args]) {
    fprintf(stderr, "Memory allocation failed\n");
    for (int i = 0; i < *num_args; i++) free((*args)[i]);
    free(*args);
    *args = NULL;
    return;
  }
  (*num_args)++;
}

char **parse_input(char *input, int *num_args, exec_option_t* opt) {
  char *symbols[] = { ">", "<", ">>", "<<", "2>", "2>>", "&>", "&>>", "|", "&&", "&", NULL };
  int args_capacity = ARGS_CAPACITY_INCREMENT;
  char **args = malloc((args_capacity + 1) * sizeof(char *));
  if (!args) {
    fprintf(stderr, "Memory allocation failed\n");
    return NULL;
  }
  *num_args = 0;
  memset(opt, 0, sizeof(exec_option_t));
  char *cursor = input;
  while (*cursor) {
    while (isspace((unsigned char)*cursor)) cursor++;
    if (*cursor == '"') {
      char *end_quote = cursor;
      do {
        end_quote = strchr(end_quote + 1, '"');
      } while (end_quote && *(end_quote - 1) == '\\');
      if (!end_quote) {
        fprintf(stderr, "Unclosed quote\n");
        for(int i = 0; i < *num_args; i++)
          free(args[i]);
        free(args);
        return NULL;
      }
      int arg_len = end_quote - cursor - 1;
      char *arg = (char *)malloc(arg_len + 1);
      if (!arg) {
        fprintf(stderr, "Memory allocation failed\n");
        for(int i = 0; i < *num_args; i++)
          free(args[i]);
        free(args);
        return NULL;
      }
      strncpy(arg, cursor + 1, arg_len);
      arg[arg_len] = '\0';
      add_arg(&args, num_args, arg, &args_capacity);
      free(arg);
      cursor = end_quote + 1;
      continue;
    }
    for (int i = 0; symbols[i] != NULL; i++) {
      int symbol_len = strlen(symbols[i]);
      if (strncmp(cursor, symbols[i], symbol_len) == 0) {
        if (i < 8) opt->redirect = 1;
        else if (i == 8) opt->pipeline = 1;
        else if (i == 9) opt->logical_and = 1;
        else if (i == 10) opt->background = 1;
        if (i != 10)
          add_arg(&args, num_args, symbols[i], &args_capacity);
        cursor += symbol_len;
        goto found_symbol;
      }
    }
    {
      char *start_of_arg = cursor;
      while (*cursor && !isspace((unsigned char)*cursor) && !strchr("<>&|\"", *cursor)) cursor++;
      if (cursor != start_of_arg) {
        char *arg = strndup(start_of_arg, cursor - start_of_arg);
        add_arg(&args, num_args, arg, &args_capacity);
        free(arg);
      }
    }
    found_symbol: ;
  }
  args[*num_args] = NULL;
  return args;
}

void parse_input_args(char **args, int *num_args, exec_option_t* opt) {
  *num_args = 0;
  memset(opt, 0, sizeof(exec_option_t));
  char *symbols[] = { ">", "<", ">>", "<<", "2>", "2>>", "&>", "&>>", "|", "&&", "&", NULL };
  for (int i = 0; args[i] != NULL; i++) {
    for (int j = 0; symbols[j] != NULL; j++) {
      if (strcmp(args[i], symbols[j]) == 0) {
        if (j < 8) opt->redirect = 1;
        else if (j == 8) opt->pipeline = 1;
        else if (j == 9) opt->logical_and = 1;
        else if (j == 10) opt->background = 1;
        break;
      }
    }
    (*num_args)++;
  }
}