#include <stdio.h>
#include "rickshell.h"
#include "rickshell/color.h"

char* help_strings[RICK_BCMD_LEN] = {
  "help [-dms] [pattern ...]",
  "cd [-L|[-P [-e]] [-@]] [dir]",
  "history [-c] [-d offset] [n] or history -anrw [filename] or history -ps arg [arg...]",
  "exit [n]",
  "echo [-neE] [arg ...]"
};

char* help_detail_strings[RICK_BCMD_LEN] = {
  "rickshell, version 0.1.0\nThese shell commands are defined internally.  Type `help' to see this list.\nType `help name' to find out more about the function `name'.\n\nA star (*) next to a name means that the command is disabled.\n" ANSI_COLOR_BRIGHT_BOLD_YELLOW "Слава Украине! "ANSI_COLOR_BRIGHT_BOLD_BLUE"Героям слава!" ANSI_COLOR_RESET "\n",
  "    Change the shell working directory.\n"
  "    Change the current directory to DIR. The default DIR is the value of the\n"
  "    HOME shell variable.\n\n"
  "    Options:\n"
  "      -L        force symbolic links to be followed: resolve symbolic\n"
  "                links in DIR after processing instances of `..'\n"
  "      -P        use the physical directory structure without following\n"
  "                symbolic links: resolve symbolic links in DIR before\n"
  "                processing instances of `..'",
  "    Display or manipulate the history list.\n\n"
  "    Display the history list with line numbers, prefixing each modified\n"
  "    entry with a `*'. An argument of N lists only the last N entries.\n\n"
  "    Options:\n"
  "      -c        clear the history list by deleting all of the entries\n"
  "      -d offset delete the history entry at position OFFSET",
  "    Exit the shell.\n\n"
  "    Exits the shell with a status of N. If N is omitted, the exit status\n"
  "    is that of the last command executed.",
  "    Write arguments to the standard output.\n\n"
  "    Display the ARGs on the standard output followed by a newline. The\n"
  "    return status is always 0. If -n is specified, the trailing newline is\n"
  "    suppressed. If the -e option is given, interpretation of the following\n"
  "    backslash-escaped characters is enabled. The -E option disables the\n"
  "    interpretation of these escape characters, even on systems where they\n"
  "    are interpreted by default."
};