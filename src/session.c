// This program is protected under the GNU GPL (See COPYING)

#include <errno.h>
#include <signal.h>

#include "defs.h"

struct session *new_session(struct session *ses, int pid, int socket) {
  gts->pid = pid;
  gts->socket = socket;

  SET_BIT(gts->flags, SES_FLAG_CONNECTED);

  return gtd->ses;
}

void cleanup_session(struct session *ses) {
  if (kill(ses->pid, 0) && ses->socket) {
    if (close(ses->socket) == -1) {
      syserr("close in cleanup");
    }
    if (ses->pid) {
      kill(ses->pid, SIGKILL);
    }

    DEL_BIT(ses->flags, SES_FLAG_CONNECTED);
  }
}
