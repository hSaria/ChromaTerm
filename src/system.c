// This program is protected under the GNU GPL (See COPYING)

#include "defs.h"

#ifdef HAVE_PTY_H
#include <pty.h>
#else
#ifdef HAVE_UTIL_H
#include <util.h>
#endif
#endif

DO_COMMAND(do_run) {
  char left[BUFFER_SIZE], right[BUFFER_SIZE], temp[BUFFER_SIZE];
  int desc, pid;
  struct winsize size;

  char *argv[4] = {"sh", "-c", "", NULL};

  arg = sub_arg_in_braces(ses, arg, left, GET_ONE, SUB_VAR | SUB_FUN);
  arg = sub_arg_in_braces(ses, arg, right, GET_ALL, SUB_VAR | SUB_FUN);

  if (*left == 0) {
    display_printf2(ses, "#RUN: PROVIDE A SESSION NAME.");

    return ses;
  }

  if (*right == 0) {
    strcpy(right, getenv("SHELL") ? getenv("SHELL") : "");
  }

  size.ws_row = get_scroll_size(ses);
  size.ws_col = ses->cols;

  pid = forkpty(&desc, NULL, &gtd->old_terminal, &size);

  switch (pid) {
  case -1:
    perror("forkpty");
    break;

  case 0:
    sprintf(temp, "exec %s", right);
    argv[2] = temp;
    execv("/bin/sh", argv);
    break;

  default:
    ses = new_session(ses, left, right, pid, desc);
    break;
  }
  return gtd->ses;
}
