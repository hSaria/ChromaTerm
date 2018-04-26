// This program is protected under the GNU GPL (See COPYING)

#include "defs.h"

#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif
#include <errno.h>
#include <termios.h>

void init_terminal() {
  struct termios io;

  if (tcgetattr(0, &gtd->old_terminal)) {
    perror("tcgetattr");

    exit(errno);
  }

  io = gtd->old_terminal;

  //  Canonical mode off
  DEL_BIT(io.c_lflag, ICANON);

  io.c_cc[VMIN] = 1;
  io.c_cc[VTIME] = 0;
  io.c_cc[VSTART] = 255;
  io.c_cc[VSTOP] = 255;

  DEL_BIT(io.c_lflag, ECHO | ECHONL | IEXTEN | ISIG);

  SET_BIT(io.c_cflag, CS8);

  if (tcsetattr(0, TCSANOW, &io)) {
    perror("tcsetattr");

    exit(errno);
  }
}

void restore_terminal(void) { tcsetattr(0, TCSANOW, &gtd->old_terminal); }

void init_screen_size(struct session *ses) {
  struct winsize screen;

  if (ses == gts) {
    if (ioctl(0, TIOCGWINSZ, &screen) == -1) {
      ses->rows = SCREEN_HEIGHT;
      ses->cols = SCREEN_WIDTH;
    } else {
      ses->rows = screen.ws_row;
      ses->cols = screen.ws_col;
    }
  } else {
    ses->rows = gts->rows;
    ses->cols = gts->cols;
  }
}

int get_scroll_size(struct session *ses) { return ses->rows; }
