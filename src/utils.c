/* This program is protected under the GNU GPL (See COPYING) */

#include "defs.h"

/* return: TRUE if s1 is an abbrevation of s2 */
int is_abbrev(char *s1, char *s2) {
  if (*s1 == 0) {
    return FALSE;
  }
  return !strncasecmp(s2, s1, strlen(s1));
}

char *capitalize(char *str) {
  static char outbuf[BUFFER_SIZE];
  int cnt;

  for (cnt = 0; str[cnt] != 0; cnt++) {
    outbuf[cnt] = toupper((int)str[cnt]);
  }
  outbuf[cnt] = 0;

  return outbuf;
}

int cat_sprintf(char *dest, char *fmt, ...) {
  char buf[BUFFER_SIZE * 2];
  int size;

  va_list args;

  va_start(args, fmt);
  size = vsprintf(buf, fmt, args);
  va_end(args);

  strcat(dest, buf);

  return size;
}

void ins_sprintf(char *dest, char *fmt, ...) {
  char buf[BUFFER_SIZE * 2], tmp[BUFFER_SIZE * 2];

  va_list args;

  va_start(args, fmt);
  vsprintf(buf, fmt, args);
  va_end(args);

  strcpy(tmp, dest);
  strcpy(dest, buf);
  strcat(dest, tmp);
}

void display_header(char *format, ...) {
  char arg[BUFFER_SIZE], buf[BUFFER_SIZE];
  va_list args;

  va_start(args, format);
  vsprintf(arg, format, args);
  va_end(args);

  if ((int)strlen(arg) > gts->cols - 2) {
    arg[gts->cols - 2] = 0;
  }

  memset(buf, '#', gts->cols);
  memcpy(&buf[(gts->cols - strlen(arg)) / 2], arg, strlen(arg));

  buf[gts->cols] = 0;

  display_printf(buf);
}

void socket_printf(unsigned int length, char *format, ...) {
  char buf[BUFFER_SIZE * 2];
  va_list args;

  va_start(args, format);
  vsprintf(buf, format, args);
  va_end(args);

  if (HAS_BIT(gts->flags, SES_FLAG_CONNECTED)) {
    write(gts->socket, buf, length);
  }
}

void display_printf(char *format, ...) {
  if (gtd->quiet) {
    return;
  }

  char buf[BUFFER_SIZE * 2];
  va_list args;

  va_start(args, format);
  vsprintf(buf, format, args);
  va_end(args);

  printline(buf, FALSE);
}

void printline(char *str, int prompt) {
  char wrapped_str[BUFFER_SIZE * 2];

  if (HAS_BIT(gts->flags, SES_FLAG_CONVERTMETA)) {
    convert_meta(str, wrapped_str);
  } else {
    strcpy(wrapped_str, str);
  }
  printf("%s", wrapped_str);
  if (!prompt) {
    printf("\n");
  }
}
