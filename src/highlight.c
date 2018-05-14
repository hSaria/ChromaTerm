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
  char *pto, *ptl, *ptm;
  char match[BUFFER_SIZE], color[BUFFER_SIZE], line[BUFFER_SIZE],
      reset[BUFFER_SIZE], output[BUFFER_SIZE], plain[BUFFER_SIZE],
      result[BUFFER_SIZE];
  int i;

  strip_vt102_codes(original, line);

  for (i = 0; i < root->used; i++) {
    if (regex_compare(&root->list[i]->compiled_regex, line, result)) {
      get_highlight_codes(root->list[i]->right, color);

      *output = *reset = 0;

      pto = original;
      ptl = line;

      do {
        if (*result == 0) {
          break;
        }

        strcpy(match, result);

        strip_vt102_codes(match, plain);

        ptm = strstr(pto, match);

        if (ptm == NULL) {
          break;
        }

        ptl = strstr(ptl, match);
        ptl = ptl + strlen(match);

        *ptm = 0;

        get_color_codes(reset, pto, reset);

        cat_sprintf(output, "%s%s%s\033[0m%s", pto, color, plain, reset);

        pto = ptm + strlen(match);
      } while (regex_compare(&root->list[i]->compiled_regex, ptl, result));

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
