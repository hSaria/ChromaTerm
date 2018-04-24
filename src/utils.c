// This program is protected under the GNU GPL (See COPYING)

#include "defs.h"

#include <errno.h>
#include <sys/param.h>

// return: TRUE if s1 is an abbrevation of s2
int is_abbrev(char *s1, char *s2) {
  if (*s1 == 0) {
    return FALSE;
  }
  return !strncasecmp(s2, s1, strlen(s1));
}

// Keep synched with tintoi()
int is_number(char *str) {
  char *ptr = str;
  int i = 1, d = 0;

  if (*ptr == 0) {
    return FALSE;
  }

  ptr = str + strlen(str);

  while (TRUE) {
    ptr--;

    switch (*ptr) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      break;

    case '.':
      if (d) {
        return FALSE;
      }
      d = 1;
      break;

    case ':':
      if (i == 4) {
        return FALSE;
      }
      i++;
      break;

    case '!':
    case '~':
    case '+':
    case '-':
      if (ptr != str) {
        return FALSE;
      }
      break;

    default:
      return FALSE;
    }

    if (ptr == str) {
      break;
    }
  }
  return TRUE;
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

long long utime() {
  struct timeval now_time;

  gettimeofday(&now_time, NULL);

  if (gtd->time >= now_time.tv_sec * 1000000LL + now_time.tv_usec) {
    gtd->time++;
  } else {
    gtd->time = now_time.tv_sec * 1000000LL + now_time.tv_usec;
  }
  return gtd->time;
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

void show_message(struct session *ses, int index, char *format, ...) {
  struct listroot *root;
  char buf[STRING_SIZE];
  va_list args;

  va_start(args, format);

  vsprintf(buf, format, args);

  va_end(args);

  if (index == -1) {
    if (ses->input_level == 0) {
      display_puts2(ses, buf);
    }

    return;
  }

  root = ses->list[index];

  if (ses->input_level == 0) {
    display_puts2(ses, buf);

    return;
  }
}

void display_header(struct session *ses, char *format, ...) {
  char arg[BUFFER_SIZE], buf[BUFFER_SIZE];
  va_list args;

  va_start(args, format);
  vsprintf(arg, format, args);
  va_end(args);

  if ((int)strlen(arg) > gtd->ses->cols - 2) {
    arg[gtd->ses->cols - 2] = 0;
  }

  memset(buf, '#', gtd->ses->cols);

  memcpy(&buf[(gtd->ses->cols - strlen(arg)) / 2], arg, strlen(arg));

  buf[gtd->ses->cols] = 0;

  display_puts2(ses, buf);
}

void socket_printf(struct session *ses, size_t length, char *format, ...) {
  char buf[STRING_SIZE];
  va_list args;

  va_start(args, format);
  vsprintf(buf, format, args);
  va_end(args);

  if (HAS_BIT(ses->flags, SES_FLAG_CONNECTED)) {
    write(ses->socket, buf, length);
  }
}
void display_printf2(struct session *ses, char *format, ...) {
  char buf[STRING_SIZE];
  va_list args;

  va_start(args, format);
  vsprintf(buf, format, args);
  va_end(args);

  display_puts2(ses, buf);
}

void display_printf(struct session *ses, char *format, ...) {
  char buf[STRING_SIZE];
  va_list args;

  va_start(args, format);
  vsprintf(buf, format, args);
  va_end(args);

  display_puts(ses, buf);
}

// Like display_puts2, but no color codes added
void display_puts3(struct session *ses, char *string) {
  char output[STRING_SIZE];

  if (ses == NULL) {
    ses = gtd->ses;
  }

  if (gtd->quiet) {
    return;
  }

  if (strip_vt102_strlen(ses, ses->more_output) != 0) {
    sprintf(output, "\n%s", string);
  } else {
    sprintf(output, "%s", string);
  }

  if (ses != gtd->ses) {
    return;
  }

  printline(ses, output, FALSE);
}

// output to screen should go through this function
// the output is NOT checked for actions or anything
void display_puts2(struct session *ses, char *string) {
  char output[STRING_SIZE];

  if (ses == NULL) {
    ses = gtd->ses;
  }

  if (gtd->quiet) {

    return;
  }

  if (strip_vt102_strlen(ses, ses->more_output) != 0) {
    sprintf(output, "\n\033[0m%s\033[0m", string);
  } else {
    sprintf(output, "\033[0m%s\033[0m", string);
  }

  if (ses != gtd->ses) {

    return;
  }

  printline(ses, output, FALSE);

  return;
}

//  output to screen should go through this function
//  the output IS treated as though it came from the mud
void display_puts(struct session *ses, char *string) {
  if (ses == NULL) {
    ses = gtd->ses;
  }

  do_one_line(string, ses);

  display_puts2(ses, string);
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
  quitmsg(s);
}

// Whoops, strcasecmp wasn't found.
#if !defined(HAVE_STRCASECMP)
#define UPPER(c) (islower(c) ? toupper(c) : c)

int strcasecmp(char *string1, char *string2) {
  for (; UPPER(*string1) == UPPER(*string2); string1++, string2++)
    if (!*string1)
      return (0);
  return (UPPER(*string1) - UPPER(*string2));
}

int strncasecmp(char *string1, char *string2, size_t count) {
  if (count)
    do {
      if (UPPER(*string1) != UPPER(*string2))
        return (UPPER(*string1) - UPPER(*string2));
      if (!*string1++)
        break;
      string2++;
    } while (--count);

  return (0);
}

#endif
