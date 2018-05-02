/* This program is protected under the GNU GPL (See COPYING) */

#include "defs.h"

void *poll_input() {
  fd_set readfds;

  while (TRUE) {
    FD_ZERO(&readfds);
    FD_SET(0, &readfds);

    /* Blocking operation */
    select(FD_SETSIZE, &readfds, NULL, NULL, NULL);

    process_input();

    fflush(stdout);
  }
}

void *poll_session() {
  fd_set readfds;

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
