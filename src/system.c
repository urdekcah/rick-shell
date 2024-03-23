#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pwd.h>
#include <unistd.h>

#include "rickshell.h"
#include "rickshell/error.h"
#include "rickshell/file.h"
#include "rickshell/system.h"

char* get_username() {
  struct passwd *pw = getpwuid(getuid());
  if (!pw) RICK_EFAIL("getpwuid");
  return pw->pw_name;
}

char* get_hostname() {
  static char host[MAX_PATH_LEN];
  if (gethostname(host, sizeof(host)) == -1) RICK_EFAIL("gethostname");
  return host;
}