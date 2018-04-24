// This program is protected under the GNU GPL (See COPYING)

#include "defs.h"

void printline(struct session *ses, char *str, int prompt) {
  char wrapped_str[STRING_SIZE];

  if (HAS_BIT(ses->flags, SES_FLAG_CONVERTMETA)) {
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
