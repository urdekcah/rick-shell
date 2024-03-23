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

char **parse_input(char *input, int *num_args) {
  int args_capacity = ARGS_CAPACITY_INCREMENT;
  char **args = malloc((args_capacity + 1) * sizeof(char *));
  if (!args) {
    fprintf(stderr, "Memory allocation failed\n");
    return NULL;
  }

  char *cursor = input;
  int local_num_args = 0;
  while (*cursor) {
    while (isspace((unsigned char)*cursor)) cursor++;
    if (*cursor == '\0') break;
    char *start_of_arg = cursor;
    char *end_of_arg;
    int in_quote = 0;
    char quote_char = '\0';
    if (*cursor == '\"' || *cursor == '\'') {
      in_quote = 1;
      quote_char = *cursor++;
      start_of_arg = cursor;
      while (*cursor && !(*cursor == quote_char && in_quote)) {
        if (*cursor == quote_char) in_quote = 0;
        else cursor++;
      }
      end_of_arg = cursor;
      if (*cursor) cursor++;
    } else {
      while (*cursor && !isspace((unsigned char)*cursor)) cursor++;
      end_of_arg = cursor;
    }
    int arg_length = end_of_arg - start_of_arg;
    if (local_num_args >= args_capacity) {
      args_capacity += ARGS_CAPACITY_INCREMENT;
      char **new_args = realloc(args, (args_capacity + 1) * sizeof(char *));
      if (!new_args) {
        fprintf(stderr, "Memory allocation failed\n");
        for (int i = 0; i < local_num_args; i++) free(args[i]);
        free(args);
        return NULL;
      }
      args = new_args;
    }
    args[local_num_args] = malloc(arg_length + 1);
    if (!args[local_num_args]) {
      fprintf(stderr, "Memory allocation failed\n");
      for (int i = 0; i < local_num_args; i++) free(args[i]);
      free(args);
      return NULL;
    }
    strncpy(args[local_num_args], start_of_arg, arg_length);
    args[local_num_args][arg_length] = '\0';
    local_num_args++;
  }
  args[local_num_args] = NULL;
  if (num_args) *num_args = local_num_args;
  return args;
}


void split_command(char *command, char *symbol, char ***args1, int *flag) {
  char *pos = strstr(command, symbol);
  if (pos) {
    *flag = 1;
    *pos = '\0';
    pos += strlen(symbol);
    while (*pos == ' ') pos++;
    *args1 = parse_input(command, NULL);
    int num_args1 = 0;
    while ((*args1)[num_args1] != NULL) num_args1++;
    *args1 = realloc(*args1, (num_args1 + 2) * sizeof(char *));
    (*args1)[num_args1] = strdup(symbol);
    char *temp_pos = pos;
    int in_quote = 0;
    while (*temp_pos) {
      if (*temp_pos == '\"') {
        in_quote = !in_quote;
      } else if (*temp_pos == ' ' && !in_quote) {
        break;
      }
      temp_pos++;
    }
    if (*temp_pos) {
      (*args1)[num_args1 + 1] = strdup(pos);
      (*args1)[num_args1 + 2] = NULL;
    } else {
      char **args2 = parse_input(pos, NULL);
      int num_args2 = 0;
      while (args2[num_args2] != NULL) num_args2++;
      *args1 = realloc(*args1, (num_args1 + num_args2 + 2) * sizeof(char *));
      for (int i = 0; i < num_args2; i++) {
        (*args1)[num_args1 + i + 1] = args2[i];
      }
      (*args1)[num_args1 + num_args2 + 1] = NULL;
      free(args2);
    }
  }
}


void parse_command(char *command, char ***args1, int *redirect, int *pipeline) {
  char *redirect_symbols[] = { ">", "<", ">>", "<<", "2>", "2>>", "&>", "&>>" };
  char *pipeline_symbol = "|";
  *redirect = 0;
  *pipeline = 0;
  for (size_t i = 0; i < sizeof(redirect_symbols) / sizeof(char *); i++) {
    split_command(command, redirect_symbols[i], args1, redirect);
    if (*redirect) return;
  }
  split_command(command, pipeline_symbol, args1, pipeline);
  if (*pipeline) return;
  *args1 = parse_input(command, NULL);
}