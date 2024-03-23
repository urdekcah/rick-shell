#ifndef __RICKSHELL_PARSE_H__
#define __RICKSHELL_PARSE_H__
char **parse_input(char *input, int *num_args, int *redirect, int *pipeline, int* background);
#endif