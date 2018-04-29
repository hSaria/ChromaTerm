// This program is protected under the GNU GPL (See COPYING)

#include "defs.h"

// return: TRUE if s1 is an abbrevation of s2
int is_abbrev(char *s1, char *s2) {
  if (*s1 == 0) {
    return FALSE;
  }
  return !strncasecmp(s2, s1, strlen(s1));
}

int hex_number(char *str) {
  int value = 0;

  if (str) {
    if (isdigit((int)*str)) {
      value += 16 * (*str - '0');
    } else {
      value += 16 * (toupper((int)*str) - 'A' + 10);
    }
    str++;
  }

  if (str) {
    if (isdigit((int)*str)) {
      value += *str - '0';
    } else {
      value += toupper((int)*str) - 'A' + 10;
    }
    str++;
  }

  return value;
}

int oct_number(char *str) {
  int value = 0;

  if (str) {
    if (isdigit((int)*str)) {
      value += 8 * (*str - '0');
    }
    str++;
  }

  if (str) {
    if (isdigit((int)*str)) {
      value += *str - '0';
    }
    str++;
  }

  return value;
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
  char buf[STRING_SIZE];
  int size;

  va_list args;

  va_start(args, fmt);
  size = vsprintf(buf, fmt, args);
  va_end(args);

  strcat(dest, buf);

  return size;
}

void ins_sprintf(char *dest, char *fmt, ...) {
  char buf[STRING_SIZE], tmp[STRING_SIZE];

  va_list args;

  va_start(args, fmt);
  vsprintf(buf, fmt, args);
  va_end(args);

  strcpy(tmp, dest);
  strcpy(dest, buf);
  strcat(dest, tmp);
}

void show_message(char *format, ...) {
  char buf[STRING_SIZE];
  va_list args;

  va_start(args, format);
  vsprintf(buf, format, args);
  va_end(args);

  if (gts->input_level == 0) {
    display_puts(FALSE, TRUE, buf);
  }
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

  display_puts(FALSE, FALSE, buf);
}

void socket_printf(size_t length, char *format, ...) {
  char buf[STRING_SIZE];
  va_list args;

  va_start(args, format);
  vsprintf(buf, format, args);
  va_end(args);

  if (HAS_BIT(gts->flags, SES_FLAG_CONNECTED)) {
    write(gts->socket, buf, length);
  }
}

void display_printf(int came_from_commend, char *format, ...) {
  char buf[STRING_SIZE];
  va_list args;

  va_start(args, format);
  vsprintf(buf, format, args);
  va_end(args);

  if (came_from_commend) {
    display_puts(TRUE, TRUE, buf);
  } else {
    display_puts(FALSE, TRUE, buf);
  }
}

void display_puts(int came_from_mud, int with_color, char *string) {
  char output[STRING_SIZE];

  if (came_from_mud) {
    do_one_line(string);
  }

  if (gtd->quiet) {
    return;
  }

  if (with_color) {
    if (strip_vt102_strlen(gts->more_output) != 0) {
      sprintf(output, "\n\033[0m%s\033[0m", string);
    } else {
      sprintf(output, "\033[0m%s\033[0m", string);
    }
  } else {
    if (strip_vt102_strlen(gts->more_output) != 0) {
      sprintf(output, "\n%s", string);
    } else {
      sprintf(output, "%s", string);
    }
  }

  printline(output, FALSE);
}

void printline(char *str, int prompt) {
  char wrapped_str[STRING_SIZE];

  if (HAS_BIT(gts->flags, SES_FLAG_CONVERTMETA)) {
    convert_meta(str, wrapped_str);
  } else {
    strcpy(wrapped_str, str);
  }

  if (prompt) {
    printf("%s", wrapped_str);
  } else {
    printf("%s\n", wrapped_str);
  }

  return;
}

// print system call error message and terminate
void syserr(char *msg) {
  char s[128], *syserrmsg;

  syserrmsg = strerror(errno);

  if (syserrmsg) {
    sprintf(s, "ERROR: %s (%d: %s)", msg, errno, syserrmsg);
  } else {
    sprintf(s, "ERROR: %s (%d)", msg, errno);
  }
  quitmsg(s, 1);
}
