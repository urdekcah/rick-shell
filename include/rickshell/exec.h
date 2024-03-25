#ifndef __RICKSHELL_EXEC_H__
#define __RICKSHELL_EXEC_H__
typedef struct {
  int background;
  int redirect;
  int pipeline;
  int logical_and;
} exec_option_t;

int execute_command(char *args[], exec_option_t* opt);
int execute_with_pid(pid_t pid, char *args[], exec_option_t* opt);
#endif