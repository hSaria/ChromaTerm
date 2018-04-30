// This program is protected under the GNU GPL (See COPYING)

#include "defs.h"

int regexp_compare(char *str, char *exp, char *result) {
  regex_t regexp;
  regmatch_t pmatch[1];

  if (regcomp(&regexp, exp, REG_EXTENDED | REG_NEWLINE) != 0) {
    return FALSE;
  }

  if (regexec(&regexp, str, 1, pmatch, 0) != 0) {
    return FALSE;
  }

  sprintf(result, "%.*s", (int)(pmatch[0].rm_eo - pmatch[0].rm_so),
          &str[pmatch[0].rm_so]);

  regfree(&regexp);

  return TRUE;
}

// copy *string into *result, but substitute the various expressions with the
// values they stand for.
int substitute(char *string, char *result, int flags) {
  char buffer[BUFFER_SIZE], *pti, *pto, *ptt;
  char old[6] = {0};
  int i, cnt;

  pti = string;
  pto = (string == result) ? buffer : result;

  while (TRUE) {
    switch (*pti) {
    case '\0':
      if (HAS_BIT(flags, SUB_EOL)) {
        *pto++ = '\r';
      }

      *pto = 0;

      if (string == result) {
        strcpy(result, buffer);

        return pto - buffer;
      } else {
        return pto - result;
      }
      break;
    case '<':
      if (HAS_BIT(flags, SUB_COL)) {
        if (isdigit((int)pti[1]) && isdigit((int)pti[2]) &&
            isdigit((int)pti[3]) && pti[4] == '>') {
          if (pti[1] != '8' || pti[2] != '8' || pti[3] != '8') {
            *pto++ = ESCAPE;
            *pto++ = '[';

            switch (pti[1]) {
            case '2':
              *pto++ = '2';
              *pto++ = '2';
              *pto++ = ';';
              break;
            case '8':
              break;
            default:
              *pto++ = pti[1];
              *pto++ = ';';
            }
            switch (pti[2]) {
            case '8':
              break;
            default:
              *pto++ = '3';
              *pto++ = pti[2];
              *pto++ = ';';
              break;
            }
            switch (pti[3]) {
            case '8':
              break;
            default:
              *pto++ = '4';
              *pto++ = pti[3];
              *pto++ = ';';
              break;
            }
            pto--;
            *pto++ = 'm';
          }
          pti += sprintf(old, "<%c%c%c>", pti[1], pti[2], pti[3]);
        } else if (pti[1] >= 'a' && pti[1] <= 'f' && pti[2] >= 'a' &&
                   pti[2] <= 'f' && pti[3] >= 'a' && pti[3] <= 'f' &&
                   pti[4] == '>') {
          *pto++ = ESCAPE;
          *pto++ = '[';
          *pto++ = '3';
          *pto++ = '8';
          *pto++ = ';';
          *pto++ = '5';
          *pto++ = ';';
          cnt = 16 + (pti[1] - 'a') * 36 + (pti[2] - 'a') * 6 + (pti[3] - 'a');
          *pto++ = '0' + cnt / 100;
          *pto++ = '0' + cnt % 100 / 10;
          *pto++ = '0' + cnt % 10;
          *pto++ = 'm';
          pti += sprintf(old, "<%c%c%c>", pti[1], pti[2], pti[3]);
        } else if (pti[1] >= 'A' && pti[1] <= 'F' && pti[2] >= 'A' &&
                   pti[2] <= 'F' && pti[3] >= 'A' && pti[3] <= 'F' &&
                   pti[4] == '>') {
          *pto++ = ESCAPE;
          *pto++ = '[';
          *pto++ = '4';
          *pto++ = '8';
          *pto++ = ';';
          *pto++ = '5';
          *pto++ = ';';
          cnt = 16 + (pti[1] - 'A') * 36 + (pti[2] - 'A') * 6 + (pti[3] - 'A');
          *pto++ = '0' + cnt / 100;
          *pto++ = '0' + cnt % 100 / 10;
          *pto++ = '0' + cnt % 10;
          *pto++ = 'm';
          pti += sprintf(old, "<%c%c%c>", pti[1], pti[2], pti[3]);
        } else if (pti[1] == 'g' && isdigit((int)pti[2]) &&
                   isdigit((int)pti[3]) && pti[4] == '>') {
          *pto++ = ESCAPE;
          *pto++ = '[';
          *pto++ = '3';
          *pto++ = '8';
          *pto++ = ';';
          *pto++ = '5';
          *pto++ = ';';
          cnt = 232 + (pti[2] - '0') * 10 + (pti[3] - '0');
          *pto++ = '0' + cnt / 100;
          *pto++ = '0' + cnt % 100 / 10;
          *pto++ = '0' + cnt % 10;
          *pto++ = 'm';
          pti += sprintf(old, "<%c%c%c>", pti[1], pti[2], pti[3]);
        } else if (pti[1] == 'G' && isdigit((int)pti[2]) &&
                   isdigit((int)pti[3]) && pti[4] == '>') {
          *pto++ = ESCAPE;
          *pto++ = '[';
          *pto++ = '4';
          *pto++ = '8';
          *pto++ = ';';
          *pto++ = '5';
          *pto++ = ';';
          cnt = 232 + (pti[2] - '0') * 10 + (pti[3] - '0');
          *pto++ = '0' + cnt / 100;
          *pto++ = '0' + cnt % 100 / 10;
          *pto++ = '0' + cnt % 10;
          *pto++ = 'm';
          pti += sprintf(old, "<%c%c%c>", pti[1], pti[2], pti[3]);
        } else {
          *pto++ = *pti++;
        }
      } else {
        *pto++ = *pti++;
      }
      break;
    case '%':
      if (HAS_BIT(flags, SUB_ARG) && (isdigit((int)pti[1]) || pti[1] == '%')) {
        if (pti[1] == '%') {
          while (pti[1] == '%') {
            *pto++ = *pti++;
          }
          pti++;
        } else {
          i = isdigit((int)pti[2]) ? (pti[1] - '0') * 10 + pti[2] - '0'
                                   : pti[1] - '0';

          ptt = gtd->vars[i];

          while (*ptt) {
            *pto++ = *ptt++;
          }
          pti += isdigit((int)pti[2]) ? 3 : 2;
        }
      } else {
        *pto++ = *pti++;
      }
      break;
    case '&':
      *pto++ = *pti++;
      break;
    case '\\':
      *pto++ = *pti++;
      break;
    case ESCAPE:
      *pto++ = *pti++;
      break;
    default:
      *pto++ = *pti++;
      break;
    }
  }
}
