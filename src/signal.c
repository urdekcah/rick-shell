#include <stdio.h>
#include <signal.h>

#include "rickshell.h"
#include "rickshell/signal.h"

void handle_signal(int signo) {
  if (signo == SIGINT) fflush(stdout);
}