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

  (void)arg; /* Making a warning shut up */

  while (
      (gd.output_length += read(gd.socket, &gd.output_buffer[gd.output_length],
                                OUTPUT_MAX - gd.output_length - 1)) > 0) {
    /* Mandatoy wait before assuming no more output on the current line */
    struct timeval wait = {0, WAIT_FOR_NEW_LINE};

    FD_SET(gd.socket, &readfds);

    /* Block for a small amount to see if there's more to read. If something
     * came up, stop waiting and move on. */
    int rv = select(gd.socket + 1, &readfds, NULL, NULL, &wait);

    if (rv > 0) { /* More data came up while waiting */
      /* Failsafe: if the buffer is full, process all of pending output.
       * Otherwise, process until the line that doesn't end with \n. */
      read_output_buffer(OUTPUT_MAX - gd.output_length <= 1 ? FALSE : TRUE);
    } else if (rv == 0) { /* timed-out while waiting for FD (no more output) */
      read_output_buffer(FALSE); /* Process all that's left */
    } else if (rv < 0) {         /* error */
      break;
    }
  }
  return 0;
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
