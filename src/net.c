// This program is protected under the GNU GPL (See COPYING)

#include "defs.h"

void write_line_socket(struct session *ses, char *line, int size) {
  static int retry;

  if (!HAS_BIT(ses->flags, SES_FLAG_CONNECTED)) {
    display_printf2(ses,
                    "#SESSION NOT CONNECTED. USE: %cRUN {command} TO CONNECT",
                    gtd->command_char);
    return;
  }

  if (write(ses->socket, line, size) == -1) {
    if (retry++ < 10) {
      usleep(100000);

      write_line_socket(ses, line, size);

      return;
    }
    perror("write in write_line_socket");
    cleanup_session(ses);

    return;
  }

  retry = 0;
  return;
}

int read_buffer_mud(struct session *ses) {
  gtd->mud_output_len +=
      read(ses->socket, &gtd->mud_output_buf[gtd->mud_output_len],
           gtd->mud_output_max - gtd->mud_output_len - 1);

  if (gtd->mud_output_len <= 0) {
    return FALSE;
  }

  gtd->mud_output_buf[gtd->mud_output_len] = 0;

  return TRUE;
}

void readmud(struct session *ses) {
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

    process_mud_output(ses, line, next_line == NULL);
  }

  return;
}

void process_mud_output(struct session *ses, char *linebuf, int prompt) {
  char line[STRING_SIZE];

  strip_vt102_codes(linebuf, line);

  do_one_line(linebuf, ses); /* changes linebuf */

  if (ses == gtd->ses) {
    printline(ses, linebuf, prompt);
  }
}
