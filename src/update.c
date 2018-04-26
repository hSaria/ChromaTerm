// This program is protected under the GNU GPL (See COPYING)

#include "defs.h"

#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <termios.h>

void mainloop(void) {
  static struct timeval curr_time, wait_time, last_time;
  int usec_loop, usec_wait;
  short int pulse_poll_input = PULSE_POLL_INPUT;
  short int pulse_poll_sessions = PULSE_POLL_SESSIONS;
  short int pulse_update_packets = PULSE_UPDATE_PACKETS;
  short int pulse_update_terminal = PULSE_UPDATE_TERMINAL;
  short int pulse_update_memory = PULSE_UPDATE_MEMORY;

  wait_time.tv_sec = 0;

  while (TRUE) {
    gettimeofday(&last_time, NULL);

    if (--pulse_poll_input == 0) {
      pulse_poll_input = PULSE_POLL_INPUT;

      poll_input();
    }

    if (--pulse_poll_sessions == 0) {
      pulse_poll_sessions = PULSE_POLL_SESSIONS;

      poll_sessions();
    }

    if (--pulse_update_packets == 0) {
      pulse_update_packets = PULSE_UPDATE_PACKETS;

      packet_update();
    }

    if (--pulse_update_terminal == 0) {
      pulse_update_terminal = PULSE_UPDATE_TERMINAL;

      fflush(stdout);
    }

    if (--pulse_update_memory == 0) {
      pulse_update_memory = PULSE_UPDATE_MEMORY;

      memory_update();
    }

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
  fd_set readfds, excfds;
  static struct timeval to;
  struct session *ses;
  int rv;
  if (gts->next) {
    FD_ZERO(&readfds);
    FD_ZERO(&excfds);

    for (ses = gts->next; ses; ses = gtd->update) {
      gtd->update = ses->next;

      if (HAS_BIT(ses->flags, SES_FLAG_CONNECTED)) {
        while (TRUE) {
          FD_SET(ses->socket, &readfds);
          FD_SET(ses->socket, &excfds);

          rv = select(FD_SETSIZE, &readfds, NULL, &excfds, &to);

          if (rv <= 0) {
            break;
          }

          if (FD_ISSET(ses->socket, &readfds)) {
            if (read_buffer_mud(ses) == FALSE) {
              readmud(ses);

              cleanup_session(ses);

              gtd->mud_output_len = 0;

              break;
            }
          }

          if (FD_ISSET(ses->socket, &excfds)) {
            FD_CLR(ses->socket, &readfds);

            cleanup_session(ses);

            gtd->mud_output_len = 0;

            break;
          }
        }

        if (gtd->mud_output_len) {
          readmud(ses);
        }
      }
    }
  }
}

void packet_update(void) {
  char result[STRING_SIZE];
  struct session *ses;

  for (ses = gts->next; ses; ses = gtd->update) {
    gtd->update = ses->next;

    if (ses->check_output && gtd->time > ses->check_output) {
      SET_BIT(ses->flags, SES_FLAG_READMUD);

      strcpy(result, ses->more_output);

      ses->more_output[0] = 0;

      process_mud_output(ses, result, TRUE);

      DEL_BIT(ses->flags, SES_FLAG_READMUD);
    }
  }
}

void memory_update(void) {
  while (gtd->dispose_next) {
    int index;

    UNLINK(gtd->dispose_next, gtd->dispose_next, gtd->dispose_prev);

    for (index = 0; index < LIST_MAX; index++) {
      free_list(gtd->dispose_next->list[index]);
    }

    free(gtd->dispose_next->command);
    free(gtd->dispose_next->group);

    free(gtd->dispose_next);
  }
}
