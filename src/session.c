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

struct session *new_session(int pid, int socket) {
  gts->pid = pid;
  gts->socket = socket;

  SET_BIT(gts->flags, SES_FLAG_CONNECTED);

  if (pthread_create(&output_thread, NULL, poll_session, NULL) != 0) {
    quitmsg("failed to create input thread", 1);
  }

  return gts;
}
