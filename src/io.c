/* This program is protected under the GNU GPL (See COPYING) */

#include "defs.h"

int beginning_of_line = TRUE;

void convert_meta(char *input, char *output) {
  char *pti, *pto;

  pti = input;
  pto = output;

  while (*pti) {
    switch (*pti) {
    case ESCAPE:
      *pto++ = '\\';
      *pto++ = 'e';
      pti++;
      break;
    case 127:
      *pto++ = '\\';
      *pto++ = 'b';
      pti++;
      break;
    case '\a':
      *pto++ = '\\';
      *pto++ = 'a';
      pti++;
      break;
    case '\b':
      *pto++ = '\\';
      *pto++ = 'b';
      pti++;
      break;
    case '\t':
      *pto++ = '\\';
      *pto++ = 't';
      pti++;
      break;
    case '\r':
      *pto++ = '\\';
      *pto++ = 'r';
      pti++;
      break;
    case '\n':
      *pto++ = *pti++;
      break;
    default:
      if (*pti > 0 && *pti < 32) {
        *pto++ = '\\';
        *pto++ = 'c';
        if (*pti <= 26) {
          *pto++ = 'a' + *pti - 1;
        } else {
          *pto++ = 'A' + *pti - 1;
        }
        pti++;
        break;
      } else {
        *pto++ = *pti++;
      }
      break;
    }
  }
  *pto = 0;
}

/* To remove ^C from the output of read */
void sigint_handler_during_read(int sig) {
  if (sig) { /* Just to make a compiler warning shut up */
  }

  /* Repair line before adding the command char */
  printf("\n%s%c:", gtd.mud_current_line, gtd.command_char);
  fflush(stdout);
}

void read_key(void) {
  char c = 0;
  while ((int)(c = getc(stdin)) != EOF) {
    if (beginning_of_line && c == gtd.command_char) {
      int len = 0;
      char command_buffer[BUFFER_SIZE];
      struct termios temp_attributes;
      struct sigaction new, old;

      new.sa_handler = sigint_handler_during_read;

      printf("%c:", gtd.command_char);
      fflush(stdout);

      tcgetattr(STDIN_FILENO, &temp_attributes);

      /* Recover original terminal state */
      tcsetattr(STDIN_FILENO, TCSANOW, &gtd.saved_terminal);

      /* Ignore inturrupt signals */
      sigaction(SIGINT, &new, &old);

      len = (int)read(STDIN_FILENO, command_buffer, sizeof(command_buffer));

      /* Restore original action */
      sigaction(SIGINT, &old, NULL);

      /* Restore CT terminal state (noncanonical mode) */
      tcsetattr(STDIN_FILENO, TCSANOW, &temp_attributes);

      /* Remove the trailing \n */
      command_buffer[strlen(command_buffer) - 1] = 0;
      script_driver(command_buffer);
      memset(&command_buffer, 0, len);

      printline(gtd.mud_current_line, TRUE);
    } else {
      beginning_of_line = c == '\n' ? TRUE : FALSE;
      c = c == '\n' ? '\r' : c;

      if (write(gtd.socket, &c, 1) < 0) {
        quitmsg("failed on socket write", 1);
      }
    }
  }
}

/* Will process only the lines that end of \n */
void readmud(int wait_for_new_line) {
  char *line, *next_line;

  gtd.mud_output_buf[gtd.mud_output_len] = 0;

  /* separate into lines and print away */
  for (line = gtd.mud_output_buf; line && *line; line = next_line) {
    char linebuf[BUFFER_SIZE];

    next_line = strchr(line, '\n');

    if (next_line) {
      *next_line = 0;
      next_line++;

      /* Reset the repair buffer */
      memset(gtd.mud_current_line, 0, strlen(gtd.mud_current_line));
    } else if (wait_for_new_line || *line == 0) {
      break;
    }

    strcpy(linebuf, line);

    if (HAS_BIT(gtd.flags, SES_FLAG_HIGHLIGHT)) {
      check_all_highlights(linebuf);
    }

    /* Used to repair the output after a CT command */
    strcat(gtd.mud_current_line, linebuf);

    printline(linebuf, next_line == NULL);
  }

  if (wait_for_new_line) {
    char temp[BUFFER_SIZE];

    strcpy(temp, line);
    strcpy(gtd.mud_output_buf, temp);

    gtd.mud_output_len = (int)strlen(line);
    return;
  }

  gtd.mud_output_len = 0;
}

void readmud_buffer(void) {
  gtd.mud_output_len +=
      read(gtd.socket, &gtd.mud_output_buf[gtd.mud_output_len],
           MUD_OUTPUT_MAX - gtd.mud_output_len - 1);

  if (gtd.mud_output_len <= 0) {
    quitmsg(NULL, 0);
  }
}
