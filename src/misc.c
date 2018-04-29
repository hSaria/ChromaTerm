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

DO_COMMAND(do_commands) {
  char buf[BUFFER_SIZE] = {0}, add[BUFFER_SIZE];
  int cmd;

  display_header(" COMMANDS ");

  for (cmd = 0; *command_table[cmd].name != 0; cmd++) {
    if (*arg && !is_abbrev(arg, command_table[cmd].name)) {
      continue;
    }
    if ((int)strlen(buf) + 20 > gts->cols) {
      display_puts(FALSE, TRUE, buf);
      buf[0] = 0;
    }
    sprintf(add, "%20s", command_table[cmd].name);
    strcat(buf, add);
  }
  if (buf[0]) {
    display_puts(FALSE, TRUE, buf);
  }
  display_header("");
}

DO_COMMAND(do_exit) {
  if (*arg) {
    quitmsg(arg);
  } else {
    quitmsg(NULL);
  }
}

DO_COMMAND(do_run) {
  char command[BUFFER_SIZE], temp[BUFFER_SIZE];
  int desc, pid;
  struct winsize size;

  char *argv[4] = {"sh", "-c", "", NULL};

  sub_arg_in_braces(arg, command, GET_ALL, SUB_NONE);

  // Limit to single process
  if (process_already_running) {
    display_printf(FALSE, "#RUN: A PROCESS IS ALREADY RUNNING");
    return;
  } else {
    process_already_running = TRUE;
  }

  // If no process is provided, use the SHELL environment variable
  if (*command == 0) {
    strcpy(command, getenv("SHELL") ? getenv("SHELL") : "");
  }

  size.ws_row = get_scroll_size();
  size.ws_col = gts->cols;

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
    new_session(pid, desc);
    break;
  }
}

DO_COMMAND(do_showme) {
  char left[BUFFER_SIZE], right[BUFFER_SIZE], temp[STRING_SIZE];
  int lnf;

  get_arg_in_braces(arg, left, TRUE);

  lnf = (strlen(left) >= strlen("\\") &&
         !strcasecmp(left + strlen(left) - strlen("\\"), "\\"))
            ? FALSE
            : TRUE;

  substitute(left, temp, SUB_NONE);
  substitute(temp, left, SUB_COL | SUB_ESC);

  do_one_line(left);

  if (strip_vt102_strlen(gts->more_output) != 0) {
    sprintf(right, "\n\033[0m%s\033[0m", left);
  } else {
    sprintf(right, "\033[0m%s\033[0m", left);
  }

  printline(right, lnf);
}
