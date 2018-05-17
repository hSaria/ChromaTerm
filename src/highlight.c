/* This program is protected under the GNU GPL (See COPYING) */

#include "defs.h"

DO_COMMAND(do_highlight) {
  char arg1[BUFFER_SIZE], arg2[BUFFER_SIZE], arg3[BUFFER_SIZE];

  arg = get_arg(arg, arg1);
  arg = get_arg(arg, arg2);
  get_arg(arg, arg3);

  if (*arg3 == 0) {
    strcpy(arg3, "1000");
  }

  if (*arg1 == 0 || *arg2 == 0) {
    int i;

    for (i = 0; i < gts.list[LIST_HIGHLIGHT]->used; i++) {
      struct listnode *node = gts.list[LIST_HIGHLIGHT]->list[i];
      display_printf("%c%s "
                     "\033[1;31m{\033[0m%s\033[1;31m}\033[1;36m "
                     "\033[1;31m{\033[0m%s\033[1;31m}\033[1;36m "
                     "\033[1;31m{\033[0m%s\033[1;31m}\033[0m",
                     gtd.command_char, list_table[LIST_HIGHLIGHT].name,
                     node->left, node->right, node->pr);
    }

  } else {
    char temp[BUFFER_SIZE];
    if (get_highlight_codes(arg2, temp) == FALSE) {
      display_printf("%cERROR: Invalid color code; see %chelp color",
                     gtd.command_char, gtd.command_char);

    } else {
      update_node_list(gts.list[LIST_HIGHLIGHT], arg1, arg2, arg3);

      display_printf("%cHIGHLIGHT: {%s} now highlighted with {%s} {%s}",
                     gtd.command_char, arg1, arg2, arg3);
    }
  }
}

DO_COMMAND(do_unhighlight) {
  struct listnode *node;

  get_arg(arg, arg);
  if (*arg == 0) {
    display_printf("%cSYNTAX: %cUNHIGHLIGHT {MATCH CONDITION TO REMOVE}",
                   gtd.command_char, gtd.command_char);
    return;
  }

  node = search_node_list(gts.list[LIST_HIGHLIGHT], arg);

  if (node) {
    delete_index_list(
        gts.list[LIST_HIGHLIGHT],
        search_index_list(gts.list[LIST_HIGHLIGHT], node->left, node->pr));
    display_printf("%cUNHIGHLIGHT: Removed", gtd.command_char);
    return;
  }

  display_printf("%cUNHIGHLIGHT: Not found", gtd.command_char);
}

void check_all_highlights(char *original) {
  struct listroot *root = gts.list[LIST_HIGHLIGHT];
  char match[BUFFER_SIZE], stripped[BUFFER_SIZE];
  int i;

  strip_vt102_codes(original, stripped);

  /* Apply from the bottom since the top ones may overwrite them */
  for (i = root->used - 1; i > -1; i--) {
    int start_position;

    if ((start_position = regex_compare(root->list[i]->compiled_regex, stripped,
                                        match)) != -1) {
      char *pto, *pts, *ptm;
      char output[BUFFER_SIZE];

      *output = 0;

      pto = ptm = original;
      pts = stripped;

      do {
        int count_inc_skipped = 0;        /* Skipped bytes until the match */
        int to_skip = (int)strlen(match); /* Number of chars to skip in match */
        char *ptt;

        /* Seek ptm (original with vt102 codes) until beginning of match */
        while (*ptm && start_position > 0) {
          while (skip_vt102_codes(ptm)) {
            count_inc_skipped += skip_vt102_codes(ptm);
            ptm += skip_vt102_codes(ptm);
          }

          if (*ptm) {
            ptm++;
            pts++;
            start_position--;
            count_inc_skipped++;
          }
        }

        ptt = ptm;

        while (*ptt && to_skip > 0) {
          while (skip_vt102_codes(ptt)) {
            ptt += skip_vt102_codes(ptt);
          }

          if (*ptt) {
            ptt++;
            to_skip--;
          }
        }

        cat_sprintf(output, "%.*s%s%s\033[0m", count_inc_skipped, pto,
                    root->list[i]->processed_color, match);

        /* Move pto to after the match, and skip any vt102 codes, too. */
        pto = ptt;
        ptm = pto; /* Sync: next iteration should simulate a fresh call */

        /* Move to the remaining of the stripped string */
        pts = strstr(pts, match);
        pts = pts + strlen(match);

      } while ((start_position = regex_compare(root->list[i]->compiled_regex,
                                               pts, match)) != -1);

      /* Add the remainder of the string and then copy it to*/
      strcat(output, pto);
      strcpy(original, output);
    }
  }
}

int get_highlight_codes(char *string, char *result) {
  *result = 0;

  if (*string == '<') {
    substitute(string, result);
    return TRUE;
  }

  while (*string) {
    int cnt;

    if (isalpha((int)*string)) {
      for (cnt = 0; *color_table[cnt].name; cnt++) {
        if (is_abbrev(color_table[cnt].name, string)) {
          substitute(color_table[cnt].code, result);

          result += strlen(result);
          break;
        }
      }

      if (*color_table[cnt].name == 0) {
        return FALSE;
      }

      string += strlen(color_table[cnt].name);
    }

    switch (*string) {
    case ' ':
    case ',':
      string++;
      break;
    case 0:
      return TRUE;
    default:
      return FALSE;
    }
  }
  return TRUE;
}

int regex_compare(pcre *compiled_regex, char *str, char *result) {
  int match[2000];

  if (pcre_exec(compiled_regex, NULL, str, (int)strlen(str), 0, 0, match,
                2000) <= 0) {
    return -1;
  }

  sprintf(result, "%.*s", match[1] - match[0], &str[match[0]]);

  return match[0];
}

int skip_vt102_codes(char *str) {
  int skip;

  switch (str[0]) {
  case 5:   /* ENQ */
  case 7:   /* BEL */
  case 8:   /* BS  */
  case 11:  /* VT  */
  case 12:  /* FF  */
  case 13:  /* CR  */
  case 14:  /* SO  */
  case 15:  /* SI  */
  case 17:  /* DC1 */
  case 19:  /* DC3 */
  case 24:  /* CAN */
  case 26:  /* SUB */
  case 127: /* DEL */
    return 1;
  case 27: /* ESC */
    break;
  default:
    return 0;
  }

  switch (str[1]) {
  case '\0':
    return 1;
  case '%':
  case '#':
  case '(':
  case ')':
    return str[2] ? 3 : 2;
  case ']':
    switch (str[2]) {
    case 'P':
      for (skip = 3; skip < 10; skip++) {
        if (str[skip] == 0) {
          break;
        }
      }
      return skip;
    case 'R':
      return 3;
    }
    return 2;
  case '[':
    break;
  default:
    return 2;
  }

  for (skip = 2; str[skip] != 0; skip++) {
    if (isalpha((int)str[skip])) {
      return skip + 1;
    }

    switch (str[skip]) {
    case '@':
    case '`':
    case ']':
      return skip + 1;
    }
  }
  return skip;
}

/* If n is not null, then this function seeks n times, exluding vt102 codes */
void strip_vt102_codes(char *str, char *buf) {
  char *pti, *pto;

  pti = str;
  pto = buf;

  while (*pti) {
    int skip;
    while ((skip = skip_vt102_codes(pti))) {
      pti += skip;
    }

    if (*pti) {
      *pto++ = *pti++;
    }
  }

  *pto = 0;
}

/* copy *string into *result, but substitute the various colors with the
 * values they stand for */
void substitute(char *string, char *result) {
  char *pti = string, *pto = result;
  char old[6] = {0};

  while (TRUE) {
    switch (*pti) {
    case '\0':
      *pto = 0;
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
      } else if (((pti[1] >= 'a' && pti[1] <= 'f' && pti[2] >= 'a' &&
                   pti[2] <= 'f' && pti[3] >= 'a' && pti[3] <= 'f') ||
                  (pti[1] >= 'A' && pti[1] <= 'F' && pti[2] >= 'A' &&
                   pti[2] <= 'F' && pti[3] >= 'A' && pti[3] <= 'F') ||
                  ((pti[1] == 'g' || pti[1] == 'G') && isdigit((int)pti[2]) &&
                   isdigit((int)pti[3]))) &&
                 pti[4] == '>') {
        int cnt, grayscale = (pti[1] == 'g' || pti[1] == 'G') ? TRUE : FALSE;
        char g = (pti[1] >= 'a' && pti[1] <= 'f') || pti[1] == 'g' ? '3' : '4';

        *pto++ = ESCAPE;
        *pto++ = '[';
        *pto++ = g;
        *pto++ = '8';
        *pto++ = ';';
        *pto++ = '5';
        *pto++ = ';';

        if (grayscale) {
          cnt = 232 + (pti[2] - '0') * 10 + (pti[3] - '0');
        } else {
          g = g == '3' ? 'a' : 'A'; /* 3 means foreground */
          cnt = 16 + (pti[1] - g) * 36 + (pti[2] - g) * 6 + (pti[3] - g);
        }

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
