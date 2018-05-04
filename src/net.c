/* This program is protected under the GNU GPL (See COPYING) */

#include "defs.h"

int read_buffer_mud() {
  gtd->mud_output_len +=
      read(gts->socket, &gtd->mud_output_buf[gtd->mud_output_len],
           gtd->mud_output_max - gtd->mud_output_len - 1);

  if (gtd->mud_output_len <= 0) {
    return FALSE;
  }

  gtd->mud_output_buf[gtd->mud_output_len] = 0;

  return TRUE;
}

void readmud() {
  char *line, *next_line;

  gtd->mud_output_len = 0;

  /* separate into lines and print away */
  for (line = gtd->mud_output_buf; line && *line; line = next_line) {
    char line_buf[BUFFER_SIZE];
    next_line = strchr(line, '\n');

    if (next_line) {
      /* Used to repair the mud line when a command's output clears it */
      gtd->mud_output_current_line_start = gtd->mud_output_buf;
      *next_line = 0;
      next_line++;
    } else if (*line == 0) {
      break;
    }

    strcpy(linebuf, line);

    if (HAS_BIT(gts->flags, SES_FLAG_HIGHLIGHT)) {
      check_all_highlights(linebuf);
    }

    printline(linebuf, next_line == NULL);
  }
}
