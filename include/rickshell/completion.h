#ifndef __RICKSHELL_COMPLETION_H__
#define __RICKSHELL_COMPLETION_H__
void add_suggestion(char ***suggestions, int *num_suggestions, int *capacity, const char *suggestion);
void explore_directory(const char *dir_path, const char *cmd, char ***suggestions, int *num_suggestions, int *capacity);
void suggest_command(const char *cmd);
#endif