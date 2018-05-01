// This program is protected under the GNU GPL (See COPYING)

#include "defs.h"

void write_line_socket(char *line, int size) {
  static int retry;

  if (!HAS_BIT(gts->flags, SES_FLAG_CONNECTED)) {
    display_printf("%cERROR: No child process. Use %cRUN {PROCESS}",
                   gtd->command_char, gtd->command_char);
    return;
  }

  if (write(gts->socket, line, size) == -1) {
    if (retry++ < 10) {
      usleep(100000);

      write_line_socket(line, size);

      return;
    }
    perror("write in write_line_socket");
    quitmsg(NULL, 74);

    return;
  }

  retry = 0;
}

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
    next_line = strchr(line, '\n');

    if (next_line) {
      *next_line = 0;
      next_line++;
    } else if (*line == 0) {
      break;
    }

    process_mud_output(line, next_line == NULL);
  }
}

void process_mud_output(char *linebuf, int prompt) {
  char line[BUFFER_SIZE * 2];

  strip_vt102_codes(linebuf, line);

  if (HAS_BIT(gts->flags, SES_FLAG_HIGHLIGHT)) {
    do_one_line(linebuf); /* changes linebuf */
  }

  printline(linebuf, prompt);
}
