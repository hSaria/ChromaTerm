/* This program is protected under the GNU GPL (See COPYING) */

#include "defs.h"

void cat_sprintf(char *dest, char *fmt, ...) {
  char buf[BUFFER_SIZE * 2];
  va_list args;

  va_start(args, fmt);
  vsprintf(buf, fmt, args);
  va_end(args);

  strcat(dest, buf);
}

void display_printf(char *format, ...) {
  char buf[BUFFER_SIZE * 4];
  va_list args;

  va_start(args, format);
  vsprintf(buf, format, args);
  va_end(args);

  write(STDERR_FILENO, buf, strlen(buf));
  write(STDERR_FILENO, "\n", 1);
}

/* The outer-most braces (if any) are stripped; all else left as is */
char *get_arg(char *string, char *result) {
  char *pti, *pto, output[BUFFER_SIZE];
  int nest = 1;

  /* advance to the next none-space character */
  pti = string;
  pto = output;

  while (isspace((int)*pti)) {
    pti++;
  }

  /* Use a space as the separator if not wrapped with braces */
  if (*pti != DEFAULT_OPEN) {
    while (*pti) {
      if (isspace((int)*pti)) {
        pti++;
        break;
      }
      *pto++ = *pti++;
    }
  } else {
    /* Advance past the DEFAULT_OPEN (nest is 1 for this reason) */
    pti++;

    while (*pti) {
      if (*pti == DEFAULT_OPEN) {
        nest++;
      } else if (*pti == DEFAULT_CLOSE) {
        nest--;

        /* Stop once we've met the closing backet for the openning we advanced
         * past before this loop */
        if (nest == 0) {
          break;
        }
      }
      *pto++ = *pti++;
    }

    if (*pti == 0) {
      display_printf("ERROR: Missing closing bracket");
    } else {
      pti++;
    }
  }

  *pto = '\0';

  strcpy(result, output);
  return pti;
}

/* TRUE if s1 is an abbrevation of s2 (case-insensitive) */
int is_abbrev(char *s1, char *s2) {
  if (*s1 == 0) {
    return FALSE;
  }
  return !strncasecmp(s2, s1, strlen(s1));
}

void script_driver(char *str) {
  char *pti = str;

  /* Skip any unnecessary command chars or spaces before the actual command */
  while (*pti == gd.command_char || isspace((int)*pti)) {
    pti++;
  }

  if (*pti != 0) {
    char args[BUFFER_SIZE], command[BUFFER_SIZE];
    int cmd;

    strcpy(args, get_arg(pti, command));

    for (cmd = 0; *command_table[cmd].name != 0; cmd++) {
      if (is_abbrev(command, command_table[cmd].name)) {
        (*command_table[cmd].command)(args);
        return;
      }
    }

    display_printf("ERROR: Unknown command '%s'", command);
  }
}
