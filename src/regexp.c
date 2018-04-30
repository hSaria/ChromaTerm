// This program is protected under the GNU GPL (See COPYING)

#include "defs.h"

int match(char *str, char *exp) {
  char expbuf[BUFFER_SIZE];

  sprintf(expbuf, "^%s$", exp);

  substitute(expbuf, expbuf, SUB_NONE);

  return regexp(NULL, str, expbuf, 0);
}

int regexp_compare(pcre *nodepcre, char *str, char *exp, int flag) {
  pcre *regex;
  const char *error;
  int i, j, matches, match[303];

  if (nodepcre == NULL) {
    regex = pcre_compile(exp, 0, &error, &i, NULL);
  } else {
    regex = nodepcre;
  }

  if (regex == NULL) {
    return FALSE;
  }

  matches = pcre_exec(regex, NULL, str, (int)strlen(str), 0, 0, match, 303);

  if (matches <= 0) {
    if (nodepcre == NULL) {
      free(regex);
    }
    return FALSE;
  }

  switch (flag) {
  case SUB_ARG:
    for (i = 0; i < matches; i++) {
      gtd->vars[i] =
          refstring(gtd->vars[i], "%.*s", match[i * 2 + 1] - match[i * 2],
                    &str[match[i * 2]]);
    }
    break;
  case SUB_ARG + SUB_FIX:
    for (i = 0; i < matches; i++) {
      j = gtd->args[i];

      gtd->vars[j] =
          refstring(gtd->vars[j], "%.*s", match[i * 2 + 1] - match[i * 2],
                    &str[match[i * 2]]);
    }
    break;
  }

  if (nodepcre == NULL) {
    free(regex);
  }

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

// Calls regexp checking if the string matches, and automatically fills
// in the text represented by the wildcards on success.
int check_one_regexp(struct listnode *node, char *line) {
  char *exp, *str;

  if (node->regex == NULL) {
    char result[BUFFER_SIZE];

    substitute(node->left, result, SUB_NONE);

    exp = result;
  } else {
    exp = node->left;
  }

  str = line;

  return regexp(node->regex, str, exp, SUB_ARG);
}

int regexp(pcre *nodepcre, char *str, char *exp, int flag) {
  char out[BUFFER_SIZE], *pti, *pto;
  int arg = 1, var = 1, fix = 0;

  pti = exp;
  pto = out;

  while (*pti == '^') {
    *pto++ = *pti++;
  }

  while (*pti) {
    switch (pti[0]) {
    case '\\':
      *pto++ = *pti++;
      *pto++ = *pti++;
      break;

    case '{':
      gtd->args[up(var)] = up(arg);
      *pto++ = '(';
      pti = get_arg_in_braces(pti, pto, TRUE);
      pto += strlen(pto);
      *pto++ = ')';
      break;

    case '[':
    case ']':
    case '(':
    case ')':
    case '|':
    case '.':
    case '?':
    case '+':
    case '*':
    case '^':
      *pto++ = '\\';
      *pto++ = *pti++;
      break;
    case '$':
      if (pti[1] != DEFAULT_OPEN && !isalnum((int)pti[1])) {
        int i = 0;

        while (pti[++i] == '$') {
          continue;
        }

        if (pti[i]) {
          *pto++ = '\\';
        }
      }
      *pto++ = *pti++;
      break;
    case '%':
      switch (pti[1]) {
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
        fix = SUB_FIX;
        arg = isdigit((int)pti[2]) ? (pti[1] - '0') * 10 + (pti[2] - '0')
                                   : pti[1] - '0';
        gtd->args[up(var)] = up(arg);
        pti += isdigit((int)pti[2]) ? 3 : 2;
        strcpy(pto, *pti == 0 ? "(.*)" : "(.*?)");
        pto += strlen(pto);
        break;
      case 'd':
        gtd->args[up(var)] = up(arg);
        pti += 2;
        strcpy(pto, *pti == 0 ? "([0-9]*)" : "([0-9]*?)");
        pto += strlen(pto);
        break;
      case 'D':
        gtd->args[up(var)] = up(arg);
        pti += 2;
        strcpy(pto, *pti == 0 ? "([^0-9]*)" : "([^0-9]*?)");
        pto += strlen(pto);
        break;
      case 'i':
        pti += 2;
        strcpy(pto, "(?i)");
        pto += strlen(pto);
        break;
      case 'I':
        pti += 2;
        strcpy(pto, "(?-i)");
        pto += strlen(pto);
        break;
      case 's':
        gtd->args[up(var)] = up(arg);
        pti += 2;
        strcpy(pto, *pti == 0 ? "(\\s*)" : "(\\s*?)");
        pto += strlen(pto);
        break;
      case 'S':
        gtd->args[up(var)] = up(arg);
        pti += 2;
        strcpy(pto, *pti == 0 ? "(\\S*)" : "(\\S*?)");
        pto += strlen(pto);
        break;
      case 'w':
        gtd->args[up(var)] = up(arg);
        pti += 2;
        strcpy(pto, *pti == 0 ? "([a-zA-Z]*)" : "([a-zA-Z]*?)");
        pto += strlen(pto);
        break;
      case 'W':
        gtd->args[up(var)] = up(arg);
        pti += 2;
        strcpy(pto, *pti == 0 ? "([^a-zA-Z]*)" : "([^a-zA-Z]*?)");
        pto += strlen(pto);
        break;
      case '?':
        gtd->args[up(var)] = up(arg);
        pti += 2;
        strcpy(pto, *pti == 0 ? "(.?)"
                              : "(.?"
                                "?)");
        pto += strlen(pto);
        break;
      case '*':
        gtd->args[up(var)] = up(arg);
        pti += 2;
        strcpy(pto, *pti == 0 ? "(.*)" : "(.*?)");
        pto += strlen(pto);
        break;
      case '+':
        gtd->args[up(var)] = up(arg);
        pti += 2;
        strcpy(pto, *pti == 0 ? "(.+)" : "(.+?)");
        pto += strlen(pto);
        break;
      case '.':
        gtd->args[up(var)] = up(arg);
        pti += 2;
        strcpy(pto, "(.)");
        pto += strlen(pto);
        break;
      case '%':
        *pto++ = *pti++;
        pti++;
        break;
      default:
        *pto++ = *pti++;
        break;
      }
      break;
    default:
      *pto++ = *pti++;
      break;
    }
  }
  *pto = 0;

  return regexp_compare(nodepcre, str, out, flag + fix);
}
