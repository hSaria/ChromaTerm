/* This program is protected under the GNU GPL (See COPYING) */

#include "defs.h"

void *poll_input(void *arg) {
  fd_set readfds;

  if (arg) { /* Making a warning shut up */
  }

  while (TRUE) {
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);

    /* Blocking operation */
    if (select(FD_SETSIZE, &readfds, NULL, NULL, NULL) <= 0) {
      quitmsg(NULL, 0);
    }

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
    FD_SET(gts.socket, &readfds);

    /* Blocking operation */
    if (select(FD_SETSIZE, &readfds, NULL, NULL, NULL) <= 0) {
      quitmsg(NULL, 0);
    }

    if (read_buffer_mud() == FALSE) {
      quitmsg(NULL, 0);
    }

    if (gtd.mud_output_len) {
      readmud();
    }

    fflush(stdout);
  }
}

void script_driver(char *str) {
  if (*str != 0) {
    char *args, line[BUFFER_SIZE];
    int cmd = -1, i;

    /* Skip redundant command chars before the actual command */
    while (*str == gtd.command_char || isspace((int)*str)) {
      str++;
    }

    /* Command stored in line, rest of string in args */
    args = get_arg(str, line);

    for (i = 0; *command_table[i].name != 0; i++) {
      if (is_abbrev(line, command_table[i].name)) {
        cmd = i;
        break;
      }
    }

    if (cmd == -1) {
      display_printf("%cERROR: Unknown command '%s'", gtd.command_char, line);
    } else {
      (*command_table[cmd].command)(args);
      *args = 0;
    }
  }
}
