// This program is protected under the GNU GPL (See COPYING)

#include "defs.h"

#include <errno.h>
#include <signal.h>

struct session *new_session(struct session *ses, char *command, int pid,
                            int socket) {
  int cnt = 0;
  struct session *newsession;

  newsession = (struct session *)calloc(1, sizeof(struct session));

  newsession->command = strdup(command);
  newsession->pid = pid;

  newsession->group = strdup(gts->group);
  newsession->flags = gts->flags;

  LINK(newsession, gts->next, gts->prev);

  for (cnt = 0; cnt < LIST_MAX; cnt++) {
    newsession->list[cnt] = copy_list(newsession, gts->list[cnt], cnt);
  }

  newsession->rows = gts->rows;
  newsession->cols = gts->cols;

  gtd->ses = newsession;

  SET_BIT(newsession->flags, SES_FLAG_CONNECTED);

  gtd->ses = newsession;

  gtd->ses->socket = socket;

  return gtd->ses;
}

void cleanup_session(struct session *ses) {
  if (ses == gtd->update) {
    gtd->update = ses->next;
  }

  UNLINK(ses, gts->next, gts->prev);

  if (ses->socket) {
    if (close(ses->socket) == -1) {
      syserr("close in cleanup");
    }

    kill(ses->pid, SIGKILL);

    DEL_BIT(ses->flags, SES_FLAG_CONNECTED);
  }

  quitmsg(NULL);

  return;
}
