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
  if (gd.quiet) {
    return;
  }

  char buf[BUFFER_SIZE * 4];
  va_list args;

  va_start(args, format);
  vsprintf(buf, format, args);
  va_end(args);

  write(gd.fd_ct, buf, strlen(buf));
  write(gd.fd_ct, "\n", 1);
}

/* The outer-most braces (if any) are stripped; all else left as is */
char *get_arg(char *string, char *result) {
  char *pti, *pto;
  int nest = 1;

  pti = space_out(string);
  pto = result;

  /*Use a space as the separator if not wrapped with braces */
  if (*pti != DEFAULT_OPEN) {
    while (*pti) {
      if (isspace((int)*pti)) {
        pti++;
        break;
      }
      *pto++ = *pti++;
    }
    *pto = '\0';
    return pti;
  }

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
    display_printf("%cERROR: No closing bracket for argument", gd.command_char);
  } else {
    pti++;
  }
  *pto = '\0';

  return pti;
}

/* TRUE if s1 is an abbrevation of s2 (case-insensitive) */
int is_abbrev(char *s1, char *s2) {
  if (*s1 == 0) {
    return FALSE;
  }
  return !strncasecmp(s2, s1, strlen(s1));
}

/* advance ptr to the next none-space character */
char *space_out(char *string) {
  while (isspace((int)*string)) {
    string++;
  }
  return string;
}
