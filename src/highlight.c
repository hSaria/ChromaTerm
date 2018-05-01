/* This program is protected under the GNU GPL (See COPYING) */

#include "defs.h"

DO_COMMAND(do_highlight) {
  char arg1[BUFFER_SIZE], arg2[BUFFER_SIZE], arg3[BUFFER_SIZE],
      temp[BUFFER_SIZE];

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

void check_all_highlights(char *original, char *line) {
  struct listroot *root = gts->list[LIST_HIGHLIGHT];
  char *pto, *ptl, *ptm;
  char match[BUFFER_SIZE], color[BUFFER_SIZE], reset[BUFFER_SIZE],
      output[BUFFER_SIZE], plain[BUFFER_SIZE], result[BUFFER_SIZE];

  for (root->update = 0; root->update < root->used; root->update++) {
    if (regex_compare(&root->list[root->update]->compiled_regex, line,
                      result)) {
      get_highlight_codes(root->list[root->update]->right, color);

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
      } while (regex_compare(&root->list[root->update]->compiled_regex, ptl,
                             result));

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
