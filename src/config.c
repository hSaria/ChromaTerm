// This program is protected under the GNU GPL (See COPYING)

#include "defs.h"

DO_COMMAND(do_configure) {
  char left[BUFFER_SIZE], right[BUFFER_SIZE];
  struct listnode *node;
  int index;

  arg = get_arg_in_braces(ses, arg, left, FALSE);
  sub_arg_in_braces(ses, arg, right, GET_ONE, SUB_NONE);

  if (*left == 0) {
    display_header(ses, " CONFIGURATIONS ");

    for (index = 0; *config_table[index].name != 0; index++) {
      node = search_node_list(ses->list[LIST_CONFIG], config_table[index].name);

      if (node) {
        display_printf2(ses, "[%-14s] [%9s] [%s]", node->left, node->right,
                        config_table[index].description);
      }
    }
    display_header(ses, "");
  } else {
    for (index = 0; *config_table[index].name != 0; index++) {
      if (is_abbrev(left, config_table[index].name)) {
        if (config_table[index].config(ses, right, index) != NULL) {
          node = search_node_list(ses->list[LIST_CONFIG],
                                  config_table[index].name);

          if (node) {
            show_message(ses, LIST_CONFIG, "#CONFIG {%s} HAS BEEN SET TO {%s}.",
                         config_table[index].name, node->right);
          }
        }
        return ses;
      }
    }
    display_printf(ses, "#ERROR: #CONFIG {%s} IS NOT A VALID OPTION.",
                   capitalize(left));
  }
  return ses;
}

DO_CONFIG(config_commandchar) {
  if (arg[0]) {
    gtd->command_char = arg[0];
  } else {
    display_printf(ses, "#SYNTAX: #CONFIG {%s} CHAR", config_table[index].name);
    return NULL;
  }

  char single_char[] = {arg[0], 0};
  update_node_list(ses->list[LIST_CONFIG], config_table[index].name,
                   single_char, "");

  return ses;
}

DO_CONFIG(config_convertmeta) {
  if (!strcasecmp(arg, "ON")) {
    SET_BIT(ses->flags, SES_FLAG_CONVERTMETA);
  } else if (!strcasecmp(arg, "OFF")) {
    DEL_BIT(ses->flags, SES_FLAG_CONVERTMETA);
  } else {
    display_printf(ses, "#SYNTAX: #CONFIG {%s} <ON|OFF>",
                   config_table[index].name);
    return NULL;
  }

  update_node_list(ses->list[LIST_CONFIG], config_table[index].name,
                   capitalize(arg), "");

  return ses;
}

DO_CONFIG(config_charset) {
  if (!strcasecmp(arg, "UTF-8")) {
    SET_BIT(ses->flags, SES_FLAG_UTF8);
  } else if (!strcasecmp(arg, "ASCII")) {
    DEL_BIT(ses->flags, SES_FLAG_UTF8);
  } else {
    display_printf(ses, "#SYNTAX: #CONFIG {%s} <ASCII|UTF-8>",
                   config_table[index].name);
    return NULL;
  }

  update_node_list(ses->list[LIST_CONFIG], config_table[index].name,
                   capitalize(arg), "");

  return ses;
}
