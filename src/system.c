// This program is protected under the GNU GPL (See COPYING)

#include "defs.h"

#ifdef HAVE_PTY_H
#include <pty.h>
#else
#ifdef HAVE_UTIL_H
#include <util.h>
#endif
#endif

int process_already_running = FALSE;

DO_COMMAND(do_run) {
  char command[BUFFER_SIZE], temp[BUFFER_SIZE];
  int desc, pid;
  struct winsize size;

  char *argv[4] = {"sh", "-c", "", NULL};

  sub_arg_in_braces(ses, arg, command, GET_ALL, SUB_VAR | SUB_FUN);

  // Limit to single process
  if (process_already_running) {
    display_printf2(ses, "#RUN: A PROCESS IS ALREADY RUNNING");
    return ses;
  } else {
    process_already_running = TRUE;
  }

  // If no process is provided, use the SHELL environment variable
  if (*command == 0) {
    strcpy(command, getenv("SHELL") ? getenv("SHELL") : "");
  }

  size.ws_row = get_scroll_size(ses);
  size.ws_col = ses->cols;

  pid = forkpty(&desc, NULL, &gtd->active_terminal, &size);

  switch (pid) {
  case -1:
    perror("forkpty");
    break;
  case 0:
    sprintf(temp, "exec %s", command);
    argv[2] = temp;
    execv("/bin/sh", argv);
    break;
  default:
    new_session(ses, command, pid, desc);
    break;
  }

  return gtd->ses;
}
