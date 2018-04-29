// This program is protected under the GNU GPL (See COPYING)

#include <errno.h>
#include <signal.h>

#include "defs.h"

struct session *new_session(int pid, int socket) {
  gts->pid = pid;
  gts->socket = socket;

  SET_BIT(gts->flags, SES_FLAG_CONNECTED);

  return gtd->ses;
}

void cleanup_session() {
  if (kill(gts->pid, 0) && gts->socket) {
    if (close(gts->socket) == -1) {
      syserr("close in cleanup");
    }
    if (gts->pid) {
      kill(gts->pid, SIGKILL);
    }

    DEL_BIT(gts->flags, SES_FLAG_CONNECTED);
  }
}
