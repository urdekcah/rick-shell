#ifndef __RICKSHELL_PARSE_H__
#define __RICKSHELL_PARSE_H__
#include "rickshell/exec.h"
char **parse_input(char *input, int *num_args, exec_option_t* opt);
void parse_input_args(char **args, int *num_args, exec_option_t* opt);
#endif