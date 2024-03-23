#include "rickshell.h"
#include "rickshell/error.h"

int need_exit(int set, int get) {
  static int exit_flag = 0;
  static int _set = 0;
  if (set==0 && get==0) return _set;
  if (get!=1) {
    exit_flag = get;
    _set = 1;
  }
  return exit_flag;
}