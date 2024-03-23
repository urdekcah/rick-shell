#ifndef __RICKSHELL_FILE_H__
#define __RICKSHELL_FILE_H__
#define MAX_PATH_LEN 4096
char* get_current_directory();
char* homepath(const char* file_path);
void ensure_directory_exists(const char* dir_path);
void ensure_file_exists(const char* file_path);
#endif