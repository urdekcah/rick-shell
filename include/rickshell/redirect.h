#ifndef __RICKSHELL_REDIRECT_H__
#define __RICKSHELL_REDIRECT_H__
#include <fcntl.h>

int open_file_descriptor(char *path, int flags, mode_t mode);
void redirect_output(int *fd, int newfd);
void handle_redirection(char *args[]);
#endif