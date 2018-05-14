/* This program is protected under the GNU GPL (See COPYING) */

#include "defs.h"

void cleanup_session() {
  if (kill(gts->pid, 0) && gts->socket) {
    close(gts->socket);
    if (gts->pid) {
      kill(gts->pid, SIGKILL);
    }

    DEL_BIT(gts->flags, SES_FLAG_CONNECTED);
  }
}

int find_command(char *command) {
  if (isalpha((int)*command)) {
    int cmd;
    for (cmd = 0; *command_table[cmd].name != 0; cmd++) {
      if (is_abbrev(command, command_table[cmd].name)) {
        return cmd;
      }
    }
  }
  return -1;
}

struct session *new_session(int pid, int socket) {
  gts->pid = pid;
  gts->socket = socket;

  SET_BIT(gts->flags, SES_FLAG_CONNECTED);

  if (pthread_create(&output_thread, NULL, poll_session, NULL) != 0) {
    quitmsg("failed to create input thread", 1);
  }

  return gts;
}

void *poll_input(void *arg) {
  fd_set readfds;

  if (gtd->command_prompt) {
    read_key();
  }

  if (arg) { /* Making a warning shut up */
  }

  while (TRUE) {
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);

    /* Blocking operation */
    select(FD_SETSIZE, &readfds, NULL, NULL, NULL);

    read_key();

    fflush(stdout);
  }
}

void *poll_session(void *arg) {
  fd_set readfds;
  FD_ZERO(&readfds); /* Initialise the file descriptor */

  if (arg) { /* Making a warning shut up */
  }

  while (TRUE) {
    if (HAS_BIT(gts->flags, SES_FLAG_CONNECTED)) {
      FD_SET(gts->socket, &readfds);

      /* Blocking operation */
      if (select(FD_SETSIZE, &readfds, NULL, NULL, NULL) <= 0) {
        quitmsg(NULL, 0);
      }

      if (read_buffer_mud() == FALSE) {
        quitmsg(NULL, 0);
      }

      if (gtd->mud_output_len) {
        readmud();
      }

      fflush(stdout);
    }
  }
}

void script_driver(char *str) {
  str = space_out(str);

  if (*str != 0) {
    char *args, line[BUFFER_SIZE];
    int cmd;

    while (*str == gtd->command_char) {
      str++;
    }

    /* Command stored in line, rest of string in args */
    args = get_arg_stop_spaces(str, line);

    cmd = find_command(line);

    if (cmd == -1) {
      display_printf("%cERROR: Unknown command '%s'", gtd->command_char, str);
    } else {
      (*command_table[cmd].command)(args);
      *args = 0;
    }
  }
}
