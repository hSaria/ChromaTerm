/* This program is protected under the GNU GPL (See COPYING) */

#include "defs.h"

char *capitalize(char *str) {
  static char outbuf[BUFFER_SIZE];
  int cnt;

  for (cnt = 0; str[cnt] != 0; cnt++) {
    outbuf[cnt] = toupper((int)str[cnt]);
  }
  outbuf[cnt] = 0;

  return outbuf;
}

void cat_sprintf(char *dest, char *fmt, ...) {
  char buf[BUFFER_SIZE * 2];

  va_list args;

  va_start(args, fmt);
  vsprintf(buf, fmt, args);
  va_end(args);

  strcat(dest, buf);
}

void display_header(char *str) {
  char buf[BUFFER_SIZE];

  if ((int)strlen(str) > gts.cols - 2) {
    str[gts.cols - 2] = 0;
  }

  memset(buf, '#', gts.cols);
  memcpy(&buf[(gts.cols - strlen(str)) / 2], str, strlen(str));

  buf[gts.cols] = 0;

  display_printf(buf);
}

void display_printf(char *format, ...) {
  if (gtd.quiet) {
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
    display_printf("%cERROR: Unmatched brackets", gtd.command_char);
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
  char wrapped_str[BUFFER_SIZE * 2];

  if (HAS_BIT(gts.flags, SES_FLAG_CONVERTMETA)) {
    convert_meta(str, wrapped_str);
    printf("%s", wrapped_str);
  } else {
    printf("%s", str);
  }

  if (!isaprompt) {
    printf("\n");
  }
}

/* advance ptr to next none-space */
char *space_out(char *string) {
  while (isspace((int)*string)) {
    string++;
  }
  return string;
}
