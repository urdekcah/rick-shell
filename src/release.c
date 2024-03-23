#include <stdio.h>

#include "rickshell.h"
#include "rickshell/error.h"
#include "rickshell/history.h"
#include "rickshell/release.h"

void rsh_release() {
  save_history();
}