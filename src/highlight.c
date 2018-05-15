/* This program is protected under the GNU GPL (See COPYING) */

#include "defs.h"

DO_COMMAND(do_highlight) {
  char arg1[BUFFER_SIZE], arg2[BUFFER_SIZE], arg3[BUFFER_SIZE];

  arg = get_arg_in_braces(arg, arg1, GET_ONE);
  arg = get_arg_in_braces(arg, arg2, GET_ONE);
  get_arg_in_braces(arg, arg3, GET_ONE);

  if (*arg3 == 0) {
    strcpy(arg3, "5");
  }

  if (*arg1 == 0 || *arg2 == 0) {
    int i;

    display_header(" HIGHLIGHTS ");

    for (i = 0; i < gts->list[LIST_HIGHLIGHT]->used; i++) {
      struct listnode *node = gts->list[LIST_HIGHLIGHT]->list[i];
      display_printf("%c%s "
                     "\033[1;31m{\033[0m%s\033[1;31m}\033[1;36m "
                     "\033[1;31m{\033[0m%s\033[1;31m}\033[1;36m "
                     "\033[1;31m{\033[0m%s\033[1;31m}\033[0m",
                     gtd->command_char, list_table[LIST_HIGHLIGHT].name,
                     node->left, node->right, node->pr);
    }

    display_header("");
  } else {
    char temp[BUFFER_SIZE];
    if (get_highlight_codes(arg2, temp) == FALSE) {
      display_printf(
          "%cHIGHLIGHT: Named codes are:\n"
          "bold, dim, underscore, blink, azure, black, blue, cyan, ebony, "
          "green, jade,\nlime, magenta, orange, pink, red, silver, tan, "
          "violet, white, yellow, b azure,\nb black, b blue, b cyan, b ebony, "
          "b green, b jade, b lime, b magenta, b orange,\nb pink, b red, b "
          "silver, b tan, b violet, b white, b yellow, light azure,\nlight "
          "ebony, light jade, light lime, light orange, light pink, light "
          "silver,\nlight tan, light violet\n",
          gtd->command_char);

    } else {
      update_node_list(gts->list[LIST_HIGHLIGHT], arg1, arg2, arg3);

      display_printf("%cHIGHLIGHT: {%s} now highlighted with {%s} {%s}",
                     gtd->command_char, arg1, arg2, arg3);
    }
  }
}

DO_COMMAND(do_unhighlight) {
  get_arg_in_braces(arg, arg, GET_ONE);
  if (*arg == 0) {
    display_printf("%cSYNTAX: %cUNHIGHLIGHT {MATCH CONDITION TO REMOVE}",
                   gtd->command_char, gtd->command_char);
    return;
  }

  delete_node_with_wild(LIST_HIGHLIGHT, arg);
}

void check_all_highlights(char *original) {
  struct listroot *root = gts->list[LIST_HIGHLIGHT];
  char *pto, *pts, *ptm;
  char match[BUFFER_SIZE], color[BUFFER_SIZE], stripped[BUFFER_SIZE],
      output[BUFFER_SIZE];
  int i;

  strip_vt102_codes(original, stripped, -1);

  /* Apply from the bottom since the top ones may overwrite them */
  for (i = root->used - 1; i > -1; i--) {
    int start_position =
        regex_compare(root->list[i]->compiled_regex, stripped, match);
    if (start_position != -1) {
      get_highlight_codes(root->list[i]->right, color);

      *output = 0;

      pto = ptm = original;
      pts = stripped;

      do {
        int count_inc_skipped = 0;   /* Skipped bytes until the match */
        int count_match_skipped = 0; /* Skipped inside of the match */
        int to_skip = strlen(match); /* Number of chars to skip in match */
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
            count_match_skipped += skip_vt102_codes(ptt);
            ptt += skip_vt102_codes(ptt);
          }

          if (*ptt) {
            ptt++;
            to_skip--;
          }
        }

        cat_sprintf(output, "%.*s%s%s\033[0m", count_inc_skipped, pto, color,
                    match);

        /* Move pto to after the match, and skip any vt102 codes, too. */
        pto = ptt;

        /* Move to the remaining of the stripped string */
        pts = strstr(pts, match);
        pts = pts + strlen(match);

        start_position =
            regex_compare(root->list[i]->compiled_regex, pts, match);
      } while (start_position != -1);

      /* Add the remainder of the string and then copy it to*/
      strcat(output, pto);
      strcpy(original, output);
    }
  }
}

int get_highlight_codes(char *string, char *result) {
  int cnt;

  *result = 0;

  if (*string == '<') {
    substitute(string, result);
    return TRUE;
  }

  while (*string) {
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

  if (pcre_exec(compiled_regex, NULL, str, strlen(str), 0, 0, match, 2000) <=
      0) {
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
void strip_vt102_codes(char *str, char *buf, int n) {
  char *pti, *pto;

  pti = str;
  pto = buf;

  while (*pti && (n < 0 || n > 0)) {
    while (skip_vt102_codes(pti)) {
      pti += skip_vt102_codes(pti);
    }

    if (*pti) {
      *pto++ = *pti++;
      n--;
    }
  }

  /* Only add null-termination if it isn't in seek-mode */
  if (n < 0) {
    *pto = 0;
  }
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
