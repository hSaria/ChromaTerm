/* This program is protected under the GNU GPL (See COPYING) */

#include "defs.h"

void mainloop(void) {
  static struct timeval curr_time, wait_time, last_time;
  int usec_loop, usec_wait;

  wait_time.tv_sec = 0;

  while (TRUE) {
    gettimeofday(&last_time, NULL);

    poll_input();
    poll_sessions();
    fflush(stdout);

    gettimeofday(&curr_time, NULL);

    if (curr_time.tv_sec == last_time.tv_sec) {
      usec_loop = curr_time.tv_usec - last_time.tv_usec;
    } else {
      usec_loop = 1000000 - last_time.tv_usec + curr_time.tv_usec;
    }

    usec_wait = 1000000 / PULSE_PER_SECOND - usec_loop;

    wait_time.tv_usec = usec_wait;

    if (usec_wait > 0) {
      select(0, NULL, NULL, NULL, &wait_time);
    }
  }
}

void poll_input(void) {
  fd_set readfds;
  static struct timeval to;

  while (TRUE) {
    FD_ZERO(&readfds);
    FD_SET(0, &readfds);

    if (select(FD_SETSIZE, &readfds, NULL, NULL, &to) <= 0) {
      return;
    }

    if (FD_ISSET(0, &readfds)) {
      process_input();
    } else {
      return;
    }
  }
}

void poll_sessions(void) {
  fd_set readfds;
  static struct timeval to;

  if (gts) {
    FD_ZERO(&readfds);

    if (HAS_BIT(gts->flags, SES_FLAG_CONNECTED)) {
      while (TRUE) {
        FD_SET(gts->socket, &readfds);

        if (select(FD_SETSIZE, &readfds, NULL, NULL, &to) <= 0) {
          break;
        }

        if (read_buffer_mud() == FALSE) {
          quitmsg(NULL, 0);
        }

        if (gtd->mud_output_len) {
          readmud();
        }
      }
    }
  }
}
