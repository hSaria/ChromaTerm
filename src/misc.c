/* This program is protected under the GNU GPL (See COPYING) */

#include "defs.h"

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
      display_printf(buf);
      buf[0] = 0;
    }
    sprintf(add, "%20s", command_table[cmd].name);
    strcat(buf, add);
  }
  if (buf[0]) {
    display_printf(buf);
  }
  display_header("");
}

DO_COMMAND(do_exit) {
  if (*arg) {
    quitmsg(arg, 0);
  } else {
    quitmsg(NULL, 0);
  }
}

DO_COMMAND(do_run) {
  char temp[BUFFER_SIZE];
  int desc, pid;
  struct winsize size;

  command_prompt = FALSE;

  char *argv[4] = {"sh", "-c", "", NULL};

  /* Limit to single process */
  if (process_already_running) {
    display_printf("%cRUN: A process is already running", gtd->command_char);
    return;
  } else {
    process_already_running = TRUE;
  }

  /* If no process is provided, use the SHELL environment variable */
  if (*arg == 0) {
    strcpy(arg, getenv("SHELL") ? getenv("SHELL") : "");
  }

  size.ws_row = get_scroll_size();
  size.ws_col = gts->cols;

  pid = forkpty(&desc, NULL, &gtd->active_terminal, &size);

  switch (pid) {
  case -1:
    perror("forkpty");
    break;
  case 0:
    sprintf(temp, "exec %s", arg);
    argv[2] = temp;
    execv("/bin/sh", argv);
    break;
  default:
    new_session(pid, desc);
    break;
  }

  memset(arg, 0, strlen(arg));
}
