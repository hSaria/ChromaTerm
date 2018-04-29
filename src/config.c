// This program is protected under the GNU GPL (See COPYING)

#include "defs.h"

DO_COMMAND(do_configure) {
  char left[BUFFER_SIZE], right[BUFFER_SIZE];
  struct listnode *node;
  int index;

  arg = get_arg_in_braces(arg, left, FALSE);
  sub_arg_in_braces(arg, right, GET_ONE, SUB_NONE);

  if (*left == 0) {
    display_header(" CONFIGURATIONS ");

    for (index = 0; *config_table[index].name != 0; index++) {
      node = search_node_list(gts->list[LIST_CONFIG], config_table[index].name);

      if (node) {
        display_printf(FALSE, "[%-14s] [%9s] [%s]", node->left, node->right,
                       config_table[index].description);
      }
    }
    display_header("");
  } else {
    for (index = 0; *config_table[index].name != 0; index++) {
      if (is_abbrev(left, config_table[index].name)) {
        if (config_table[index].config(right, index)) {
          node = search_node_list(gts->list[LIST_CONFIG],
                                  config_table[index].name);
          if (node) {
            show_message("#CONFIG {%s} HAS BEEN SET TO {%s}",
                         config_table[index].name, node->right);
          }
        }
        return;
      }
    }
    display_printf(TRUE, "#ERROR: #CONFIG {%s} IS NOT A VALID OPTION",
                   capitalize(left));
  }
}

DO_CONFIG(config_commandchar) {
  if (arg[0]) {
    gtd->command_char = arg[0];
  } else {
    display_printf(TRUE, "#SYNTAX: #CONFIG {%s} CHAR",
                   config_table[index].name);
    return FALSE;
  }

  char single_char[] = {arg[0], 0};
  update_node_list(gts->list[LIST_CONFIG], config_table[index].name,
                   single_char, "");
  return TRUE;
}

DO_CONFIG(config_convertmeta) {
  if (!strcasecmp(arg, "ON")) {
    SET_BIT(gts->flags, SES_FLAG_CONVERTMETA);
  } else if (!strcasecmp(arg, "OFF")) {
    DEL_BIT(gts->flags, SES_FLAG_CONVERTMETA);
  } else {
    display_printf(TRUE, "#SYNTAX: #CONFIG {%s} <ON|OFF>",
                   config_table[index].name);
    return FALSE;
  }

  update_node_list(gts->list[LIST_CONFIG], config_table[index].name,
                   capitalize(arg), "");
  return TRUE;
}

DO_CONFIG(config_charset) {
  if (!strcasecmp(arg, "UTF-8")) {
    SET_BIT(gts->flags, SES_FLAG_UTF8);
  } else if (!strcasecmp(arg, "ASCII")) {
    DEL_BIT(gts->flags, SES_FLAG_UTF8);
  } else {
    display_printf(TRUE, "#SYNTAX: #CONFIG {%s} <ASCII|UTF-8>",
                   config_table[index].name);
    return FALSE;
  }

  update_node_list(gts->list[LIST_CONFIG], config_table[index].name,
                   capitalize(arg), "");
  return TRUE;
}
