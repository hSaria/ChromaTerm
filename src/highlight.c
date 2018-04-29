// This program is protected under the GNU GPL (See COPYING)

#include "defs.h"

DO_COMMAND(do_highlight) {
  char arg1[BUFFER_SIZE], arg2[BUFFER_SIZE], arg3[BUFFER_SIZE],
      temp[BUFFER_SIZE];

  arg = sub_arg_in_braces(arg, arg1, GET_ONE, SUB_NONE);
  arg = sub_arg_in_braces(arg, arg2, GET_ALL, SUB_NONE);
  get_arg_in_braces(arg, arg3, TRUE);

  if (*arg3 == 0) {
    strcpy(arg3, "5");
  }

  if (*arg1 == 0) {
    show_list(gts->list[LIST_HIGHLIGHT], 0);
  } else if (*arg1 && *arg2 == 0) {
    if (show_node_with_wild(arg1, LIST_HIGHLIGHT) == FALSE) {
      show_message("#HIGHLIGHT: NO MATCH(ES) FOUND FOR {%s}", arg1);
    }
  } else {
    if (get_highlight_codes(arg2, temp) == FALSE) {
      display_printf(FALSE, "#HIGHLIGHT: VALID COLORS ARE:\n");
      display_printf(
          FALSE,
          "reset, bold, light, faint, dim, dark, underscore, blink, reverse, "
          "black, red, green, yellow, blue, magenta, cyan, white, b black, b "
          "red, b green, b yellow, b blue, b magenta, b cyan, b white, azure, "
          "ebony, jade, lime, orange, pink, silver, tan, violet");
    } else {
      update_node_list(gts->list[LIST_HIGHLIGHT], arg1, arg2, arg3);

      show_message("#OK. {%s} NOW HIGHLIGHTS {%s} {%s}", arg1, arg2, arg3);
    }
  }
}

DO_COMMAND(do_unhighlight) { delete_node_with_wild(LIST_HIGHLIGHT, arg); }

void check_all_highlights(char *original, char *line) {
  struct listroot *root = gts->list[LIST_HIGHLIGHT];
  struct listnode *node;
  char *pto, *ptl, *ptm;
  char match[BUFFER_SIZE], color[BUFFER_SIZE], reset[BUFFER_SIZE],
      output[BUFFER_SIZE], plain[BUFFER_SIZE];

  for (root->update = 0; root->update < root->used; root->update++) {
    if (check_one_regexp(root->list[root->update], line, 0)) {
      node = root->list[root->update];

      get_highlight_codes(node->right, color);

      *output = *reset = 0;

      pto = original;
      ptl = line;

      do {
        if (*gtd->vars[0] == 0) {
          break;
        }

        strcpy(match, gtd->vars[0]);

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
      } while (check_one_regexp(node, ptl, 0));

      strcat(output, pto);

      strcpy(original, output);
    }
  }
}

int get_highlight_codes(char *string, char *result) {
  int cnt;

  *result = 0;

  if (*string == '<') {
    substitute(string, result, SUB_COL);

    return TRUE;
  }

  while (*string) {
    if (isalpha((int)*string)) {
      for (cnt = 0; *color_table[cnt].name; cnt++) {
        if (is_abbrev(color_table[cnt].name, string)) {
          substitute(color_table[cnt].code, result, SUB_COL);

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
