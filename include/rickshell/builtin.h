#ifndef __RICKSHELL_BUILTIN_H__
#define __RICKSHELL_BUILTIN_H__
int builtin_help(char **args);
int builtin_cd(char **args);
int builtin_exit(char **args);
int builtin_history(char **args);
int builtin_echo(char **args);
#endif