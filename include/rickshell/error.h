#ifndef __RICKSHELL_ERROR_H__
#define __RICKSHELL_ERROR_H__
#include <stdio.h>
#include <stdlib.h>

#define RICK_EFAIL(msg) { perror(msg); exit(EXIT_FAILURE); }
#define RICK_ERETN(msg) { perror(msg); return; }
#define RICK_ERETN_(msg, code) { perror(msg); return code; }
int need_exit(int set, int get);
#define _set_exit(code) need_exit(1, code)
#define _get_exit() need_exit(0, 1)
#define _exit_code_set() need_exit(0, 0)
#endif