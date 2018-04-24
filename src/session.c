// This program is protected under the GNU GPL (See COPYING)

#include "defs.h"

#include <errno.h>
#include <signal.h>

struct session *new_session(struct session *ses, char *name, char *command,
                            int pid, int socket) {
  int cnt = 0;
  struct session *newsession;

  for (newsession = gts; newsession; newsession = newsession->next) {
    if (!strcmp(newsession->name, name)) {
      display_puts(ses, "THERE'S A SESSION WITH THAT NAME ALREADY.");

      if (close(socket) == -1) {
        syserr("close in new_session");
      }

      kill(pid, SIGKILL);

      return ses;
    }
  }

  newsession = (struct session *)calloc(1, sizeof(struct session));

  newsession->name = strdup(name);
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

  SET_BIT(newsession->flags, SES_FLAG_CONNECTED | SES_FLAG_RUN);

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

    if (HAS_BIT(ses->flags, SES_FLAG_RUN)) {
      kill(ses->pid, SIGKILL);
    }

    DEL_BIT(ses->flags, SES_FLAG_CONNECTED);
  }

  if (exit_after_session) {
    quitmsg(NULL);
  }

  if (ses == gtd->ses) {
    if (gts->next) {
      gtd->ses = gts->next;
    } else {
      gtd->ses = gts;
    }
  }

  display_printf(ses, "\n#EXIT to terminate.", ses->name);

  LINK(ses, gtd->dispose_next, gtd->dispose_prev);

  return;
}
