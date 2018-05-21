/* This program is protected under the GNU GPL (See COPYING) */

#include "defs.h"

pthread_t input_thread;
pthread_t output_thread;

void *poll_input(void *arg) {
  fd_set readfds;

  FD_ZERO(&readfds); /* Initialise the file descriptor */
  FD_SET(STDIN_FILENO, &readfds);

  if (arg) { /* Making a warning shut up */
  }

  while (TRUE) {
    /* Blocking operation until FD is ready */
    if (select(STDIN_FILENO + 1, &readfds, NULL, NULL, NULL) <= 0) { /* error */
      return 0;
    }

    read_key();

    fflush(stdout);
  }
}

void *poll_output(void *arg) {
  fd_set readfds;

  FD_ZERO(&readfds); /* Initialise the file descriptor */

  if (arg) { /* Making a warning shut up */
  }

  while (TRUE) {
    /* Mandatoy wait before assuming no more output on the current line */
    struct timeval wait = {0, WAIT_FOR_NEW_LINE};

    FD_SET(gd.socket, &readfds);

    /* If there's no current output on the mud, block until there's something to
     * read. However, if there's already some output in the buffer, wait a bit
     * to see if there's more out on that line. */
    int rv = select(gd.socket + 1, &readfds, NULL, NULL,
                    gd.mud_output_len == 0 ? NULL : &wait);

    if (rv == 0) { /* timed-out while waiting for FD (no more output) */
      read_output_buffer(FALSE); /* Process all that's left */
      continue;
    } else if (rv < 0) { /* error */
      return 0;
    }

    /* Read what's is in the buffer */
    gd.mud_output_len += read(gd.socket, &gd.mud_output_buf[gd.mud_output_len],
                              MUD_OUTPUT_MAX - gd.mud_output_len - 1);

    if (gd.mud_output_len <= 0) { /* error */
      return 0;
    }

    /* Failsafe: if the buffer is full, process all of pending output.
     * Otherwise, process until the line that doesn't end with \n. */
    read_output_buffer(MUD_OUTPUT_MAX - gd.mud_output_len <= 1 ? FALSE : TRUE);
  }
}

void script_driver(char *str) {
  /* Skip any unnecessary command chars or spaces before the actual command */
  while (*str == gd.command_char || isspace((int)*str)) {
    str++;
  }

  if (*str != 0) {
    char *args, line[BUFFER_SIZE];
    int i;

    /* Command stored in line, the rest in args */
    args = get_arg(str, line);

    for (i = 0; *command_table[i].name != 0; i++) {
      if (is_abbrev(line, command_table[i].name)) {
        (*command_table[i].command)(args);
        *args = 0;
        return;
      }
    }

    display_printf("%cERROR: Unknown command '%s'", gd.command_char, line);
  }
}
