/* This program is protected under the GNU GPL (See COPYING) */

#include "defs.h"

void *poll_input(void *arg) {
  fd_set readfds;

  FD_ZERO(&readfds); /* Initialise the file descriptor */
  FD_SET(STDIN_FILENO, &readfds);

  if (arg) { /* Making a warning shut up */
  }

  while (TRUE) {
    /* Blocking operation until FD is ready */
    if (select(STDIN_FILENO + 1, &readfds, NULL, NULL, NULL) <= 0) {
      quitmsg(NULL, 0);
    }

    read_key();

    fflush(stdout);
  }
}

/* BUG: A SIGINT can overwrite the output buffer */
void *poll_session(void *arg) {
  fd_set readfds;

  FD_ZERO(&readfds); /* Initialise the file descriptor */

  if (arg) { /* Making a warning shut up */
  }

  while (TRUE) {
    /* Mandatoy wait before assuming no more output on the current line */
    struct timeval wait = {0, WAIT_FOR_NEW_LINE};

    FD_SET(gts.socket, &readfds);

    /* If there's no current output on the mud, block until FD is ready */
    int rv = select(gts.socket + 1, &readfds, NULL, NULL,
                    gtd.mud_output_len == 0 ? NULL : &wait);

    if (rv == 0) { /* timed-out while waiting for FD to be ready. */
      /* Process all that's left */
      readmud(FALSE);
      continue;
    } else if (rv < 0) { /* error */
      quitmsg(NULL, 0);
    }

    /* Read from buffer and process as much as you can until the line without a
     * \n. This way, no performance hit on long output */
    readmud_buffer();

    /* Failsafe: if the buffer is full, process all of pending output */
    readmud(MUD_OUTPUT_MAX - gtd.mud_output_len <= 1 ? FALSE : TRUE);
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
