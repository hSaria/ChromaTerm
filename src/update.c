/* This program is protected under the GNU GPL (See COPYING) */

#include "defs.h"

void *poll_input(void *arg) {
  fd_set readfds;

  if (command_prompt) {
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
