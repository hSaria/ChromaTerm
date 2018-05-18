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

  char buf[BUFFER_SIZE * 2];
  va_list args;

  va_start(args, format);
  vsprintf(buf, format, args);
  va_end(args);

  printline(buf, FALSE);
}

/* The outer-most braces (if any) are stripped; all else left as is */
char *get_arg(char *string, char *result) {
  char *pti, *pto;
  int nest = 1;

  pti = space_out(string);
  pto = result;

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

  /* Advance past the DEFAULT_OPEN (nest is 1 right now) */
  pti++;

  while (*pti) {
    if (*pti == DEFAULT_OPEN) {
      nest++;
    } else if (*pti == DEFAULT_CLOSE) {
      nest--;

      if (nest == 0) {
        break;
      }
    }
    *pto++ = *pti++;
  }

  if (*pti == 0) {
    display_printf("%cERROR: Unmatched brackets", gd.command_char);
  } else {
    pti++;
  }
  *pto = '\0';

  return pti;
}

/* return: TRUE if s1 is an abbrevation of s2 */
int is_abbrev(char *s1, char *s2) {
  if (*s1 == 0) {
    return FALSE;
  }
  return !strncasecmp(s2, s1, strlen(s1));
}

void printline(char *str, int isaprompt) {
  if (HAS_BIT(gd.flags, SES_FLAG_CONVERTMETA)) {
    char wrapped_str[BUFFER_SIZE * 2];

    convert_meta(str, wrapped_str);
    printf("%s", wrapped_str);
  } else {
    printf("%s", str);
  }

  if (!isaprompt) {
    printf("\n");
  }

  fflush(stdout);
}

/* advance ptr to next none-space */
char *space_out(char *string) {
  while (isspace((int)*string)) {
    string++;
  }
  return string;
}
