/* This program is protected under the GNU GPL (See COPYING) */

#include "defs.h"

int get_scroll_size() { return gts->rows; }

void init_screen_size() {
  struct winsize screen;

  if (ioctl(0, TIOCGWINSZ, &screen) == -1) {
    gts->rows = SCREEN_HEIGHT;
    gts->cols = SCREEN_WIDTH;
  } else {
    gts->rows = screen.ws_row;
    gts->cols = screen.ws_col;
  }
}

void init_terminal() {
  struct termios io;

  if (tcgetattr(0, &gtd->active_terminal)) {
    perror("tcgetattr");
    exit(errno);
  }

  io = gtd->active_terminal;
  /*  Canonical mode off */
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

void restore_terminal(void) { tcsetattr(0, TCSANOW, &gtd->active_terminal); }
