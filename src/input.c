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
void print_backspace(int sig) {
  if (sig) { /* Just to make a compiler warning shut up */
  }
  /* Two backspaces for each char, then overwrite the output with spaces, then
   * remove said spaces */
  printf("\b\b  \b\b\n%c:", gtd->command_char);
  fflush(stdout);
}

void read_key(void) {
  char c;
  while (command_prompt || (c = getc(stdin)) != EOF) {
    if (command_prompt ||
        (beginning_of_line && (c == gtd->command_char ||
                               !HAS_BIT(gts->flags, SES_FLAG_CONNECTED)))) {
      int len = 0;
      char command_buffer[BUFFER_SIZE];
      struct termios temp_attributes;

      printf("%c:", gtd->command_char);
      fflush(stdout);

      tcgetattr(STDIN_FILENO, &temp_attributes);

      /* Recover original terminal state */
      tcsetattr(STDIN_FILENO, TCSANOW, &gtd->saved_terminal);

      /* Ignore inturrupt signals */
      signal(SIGINT, print_backspace);

      len = read(STDIN_FILENO, command_buffer, sizeof(command_buffer));
      signal(SIGINT, abort_and_trap_handler);

      /* Restore CT terminal state (noncanonical mode) */
      tcsetattr(STDIN_FILENO, TCSANOW, &temp_attributes);

      /* Remove the trailing \n */
      command_buffer[strlen(command_buffer) - 1] = 0;
      script_driver(command_buffer);
      memset(&command_buffer, 0, len);
    } else {
      if (c == '\n') {
        beginning_of_line = TRUE;
        socket_printf(1, "%c", '\r');
      } else {
        beginning_of_line = FALSE;
        socket_printf(1, "%c", c);
      }
    }
  }
}
