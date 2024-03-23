#ifndef __RICKSHELL_PARSE_H__
#define __RICKSHELL_PARSE_H__
char **parse_input(char *input, int *num_args);
void parse_command(char *command, char ***args1, int *redirect, int *pipeline);
#endif