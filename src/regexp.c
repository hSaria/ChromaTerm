/* This program is protected under the GNU GPL (See COPYING) */

#include "defs.h"

int regex_compare(regex_t *compiled_regex, char *str, char *result) {
  regmatch_t pmatch[1];

  if (regexec(compiled_regex, str, 1, pmatch, 0) != 0) {
    return FALSE;
  }

  sprintf(result, "%.*s", (int)(pmatch[0].rm_eo - pmatch[0].rm_so),
          &str[pmatch[0].rm_so]);

  return TRUE;
}

/* copy *string into *result, but substitute the various colors with the
 * values they stand for */
void substitute(char *string, char *result) {
  char buffer[BUFFER_SIZE], *pti, *pto;
  char old[6] = {0};
  int cnt;

  pti = string;
  pto = (string == result) ? buffer : result;

  while (TRUE) {
    switch (*pti) {
    case '\0':
      *pto = 0;

      if (string == result) {
        strcpy(result, buffer);
      }
      return;
    case '<':
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
      break;
    default:
      *pto++ = *pti++;
      break;
    }
  }
}
