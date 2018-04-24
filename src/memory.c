// This program is protected under the GNU GPL (See COPYING)

#include "defs.h"

char *refstring(char *point, char *fmt, ...) {
  char string[STRING_SIZE];
  va_list args;

  va_start(args, fmt);
  vsprintf(string, fmt, args);
  va_end(args);

  if (point) {
    free(point);
  }

  return strdup(string);
}
