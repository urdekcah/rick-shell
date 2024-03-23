#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <wchar.h>
#include <ctype.h>
#include <readline/history.h>

#include "rickshell.h"
#include "rickshell/file.h"
#include "rickshell/error.h"
#include "rickshell/builtin.h"

extern char* help_strings[RICK_BCMD_LEN];
extern char* help_detail_strings[RICK_BCMD_LEN];
char **builtin_str;
int (*builtin_func[RICK_BCMD_LEN])(char **);
int bufsize_builtin_str=RICK_BCMD_LEN;

char* resolve_path(const char *path, int resolve_symlinks) {
  if (!path || strlen(path) == 0) return NULL;
  char resolved_path[MAX_PATH_LEN];
  if (resolve_symlinks) {
    if (realpath(path, resolved_path) == NULL) {
      perror("realpath");
      return NULL;
    }
  } else {
    if (getcwd(resolved_path, MAX_PATH_LEN) == NULL) {
      perror("getcwd");
      return NULL;
    }
    strncat(resolved_path, "/", 2);
    strncat(resolved_path, path, MAX_PATH_LEN - strlen(resolved_path) - 1);
  }
  return strdup(resolved_path);
}

char* canonicalize_path(char *path) {
  char *canonical_path = realpath(path, NULL);
  if (canonical_path == NULL) {
    perror("realpath");
    return NULL;
  }
  return canonical_path;
}

char* convert_to_relative_path(const char *path, const char *cwd) {
  char *rel_path = realpath(cwd, NULL);
  if (rel_path == NULL) {
    perror("realpath");
    return NULL;
  }
  int cwd_len = strlen(rel_path);
  int path_len = strlen(path);
  if (path_len < cwd_len || strncmp(path, rel_path, cwd_len) != 0) {
    fprintf(stderr, "cd: Unable to convert path to relative: Path is not within current directory\n");
    free(rel_path);
    return NULL;
  }
  char *relative_path = strdup(path + cwd_len);
  free(rel_path);
  return relative_path;
}

int builtin_cd(char **args) {
  int opt;
  int logical = 0, args_len = 0;
  for (int i = 0; args[i] != NULL; i++) args_len++;
  while ((opt = getopt(args_len, args, "LP")) != -1) {
    switch (opt) {
      case 'L': logical = 1; break;
      case 'P': logical = 0; break;
      default: fprintf(stderr, "Usage: cd [-L|-P] [directory]\n"); return -1;
    }
  }
  char* current_dir = get_current_directory();
  char *directory = (args_len > optind) ? args[optind] : NULL;
  if (directory && directory[0] == '~') {
    char *home_dir = getenv("HOME");
    if (home_dir == NULL || strlen(home_dir) == 0) {
      fprintf(stderr, "cd: HOME environment variable not set\n");
      return -1;
    }
    char *new_directory = malloc(strlen(home_dir) + strlen(directory));
    if (new_directory == NULL) {
      perror("malloc");
      return -1;
    }
    strcpy(new_directory, home_dir);
    strcat(new_directory, directory + 1);
    directory = new_directory;
  } else if (directory == NULL || strcmp(directory, "~") == 0) {
    directory = getenv("HOME");
    if (directory == NULL || strlen(directory) == 0) {
      fprintf(stderr, "cd: HOME environment variable not set\n");
      return -1;
    }
  } else if (strcmp(directory, "-") == 0) {
    directory = getenv("OLDPWD");
    if (directory == NULL || strlen(directory) == 0) {
      fprintf(stderr, "cd: OLDPWD environment variable not set\n");
      return -1;
    }
  }
  char *curpath = NULL;
  if (logical) curpath = resolve_path(directory, 0);
  else curpath = resolve_path(directory, 1);
  if (curpath == NULL) return -1;
  char *canonical_path = canonicalize_path(curpath);
  if (canonical_path == NULL) return -1;
  if (chdir(canonical_path) == -1) {
    perror("chdir");
    free(canonical_path);
    return -1;
  }
  if (setenv("OLDPWD", current_dir, 1) == -1) {
    perror("setenv");
    
    free(canonical_path);
    return -1;
  }
  free(curpath);
  if (setenv("PWD", canonical_path, 1) == -1) {
    perror("setenv");
    free(canonical_path);
    return -1;
  }
  if (logical) printf("%s\n", canonical_path);
  free(canonical_path);
  return 0;
}

size_t convert_to_wchar(const char* mbstr, wchar_t* wcstr, size_t max_len) {
  mbstate_t state;
  memset(&state, 0, sizeof(mbstate_t));
  size_t len = mbsrtowcs(wcstr, &mbstr, max_len, &state);
  return len;
}

void print_escaped_char(char **p, int interpret_escapes) {
  if (!interpret_escapes || **p != '\\') {
    putchar(**p);
    return;
  }
  (*p)++;
  switch(**p) {
    case 'a': printf("\a"); break;
    case 'b': printf("\b"); break;
    case 'c': exit(0); break;
    case 'e': // Fall through
    case 'E': printf("\x1B"); break; 
    case 'f': printf("\f"); break;
    case 'n': printf("\n"); break;
    case 'r': printf("\r"); break;
    case 't': printf("\t"); break;
    case 'v': printf("\v"); break;
    case '\\': printf("\\"); break;
    case '0': {
      unsigned int ch = 0;
      int count = 0;
      while (**p >= '0' && **p <= '7' && count < 3) {
        ch = ch * 8 + (**p - '0');
        (*p)++;
        count++;
      }
      (*p)--;
      printf("%c", ch);
      break;
    }
    case 'x': {
      unsigned int ch = 0;
      (*p)++;
      sscanf(*p, "%2x", &ch);
      printf("%c", ch);
      if (isxdigit(*((*p)+1))) (*p)++;
      break;
    }
    case 'u': {
      wchar_t wcstr[5] = L"";
      wchar_t wc;
      (*p)++;
      convert_to_wchar(*p, wcstr, 5);
      swscanf(wcstr, L"%4hx", &wc);
      wprintf(L"%lc", wc);
      (*p)+=3;
      break;
    }
    case 'U': {
      wchar_t wcstr[9] = L"";
      wchar_t wc;
      (*p)++;
      convert_to_wchar(*p, wcstr, 9);
      swscanf(wcstr, L"%8lx", &wc);
      wprintf(L"%lc", wc);
      (*p)+=7;
      break;
    }
    default: {
      putchar('\\');
      putchar(**p);
      break;
    }
  }
}

int builtin_echo(char **args) {
  int newline = 1;
  int interpret_escapes = 0;
  for (int i = 1; args[i] != NULL; i++) {
    if (strcmp(args[i], "-n") == 0) {
      newline = 0;
      continue;
    } else if (strcmp(args[i], "-e") == 0) {
      interpret_escapes = 1;
      continue;
    } else if (strcmp(args[i], "-E") == 0) {
      interpret_escapes = 0;
      continue;
    }
    for (char *p = args[i]; *p; p++) print_escaped_char(&p, interpret_escapes);
    if (args[i + 1] != NULL) printf(" ");
  }
  if (newline) printf("\n");
  return 0;
}

int last_command_exit_status = 0;
int builtin_exit(char** args) {
  int exit_status = last_command_exit_status;
  if (args[1] != NULL) exit_status = atoi(args[1]);
  _set_exit(exit_status);
  return RE_NEED_EXIT;
}

int builtin_history(char **args) {
  // const char *filename = NULL;
  for (int i = 1; args[i] != NULL; i++) {
    if (strcmp(args[i], "-c") == 0) {
      clear_history();
      return 0;
    } else if (strcmp(args[i], "-d") == 0) {
      if (args[++i] == NULL) {
        fprintf(stderr, "Option -d requires an argument.\n");
        return 1;
      }
      int offset = atoi(args[i]) - 1;
      HIST_ENTRY *entry = remove_history(offset);
      if (entry != NULL) free_history_entry(entry);
      continue;
    } else if (strcmp(args[i], "-p") == 0) {
      continue;
    } else if (strcmp(args[i], "-s") == 0) {
      char *concatenated = NULL;
      int len = 0;
      for (int j = i + 1; args[j] != NULL; j++) {
        len += strlen(args[j]) + 1;
        concatenated = realloc(concatenated, len);
        if (concatenated == NULL) {
          fprintf(stderr, "Memory allocation failed.\n");
          return 1;
        }
        strcat(concatenated, args[j]);
        strcat(concatenated, " ");
      }
      if (concatenated) {
        add_history(concatenated);
        free(concatenated);
      }
      return 0;
    } else {
      // Handling additional arguments such as numbers or filenames
      // TODO: Rickshell currently does not implement more complex logic for these cases
    }
  }
  HIST_ENTRY **the_history = history_list();
  if (the_history) {
    for (int i = 0; the_history[i]; i++)
      printf("%d: %s\n", i + history_base, the_history[i]->line);
  }
  return 0;
}

int builtin_help(char** args) {
  if (args[1] == NULL) {
    printf("%s\n", help_detail_strings[0]);
    for (int i = 1; i<bufsize_builtin_str; i++) printf(" %s\n", help_strings[i]);
  } else {
    for (int i = 0; i < bufsize_builtin_str; i++) {
      if (strcmp(args[1], builtin_str[i]) == 0) {
        printf("%s\n%s\n", help_strings[i], help_detail_strings[i]);
        return 0;
      }
    }
    fprintf(stderr, "rickshell: help: no help topics match. Try `man -k %s` or `info %s`\n", args[1], args[1]);
  }
  return 0;
}