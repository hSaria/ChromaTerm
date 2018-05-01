// This program is protected under the GNU GPL (See COPYING)

#include "defs.h"

DO_COMMAND(do_configure) {
  char left[BUFFER_SIZE], right[BUFFER_SIZE];
  struct listnode *node;
  int index;

  arg = get_arg_in_braces(arg, left, GET_ONE);
  get_arg_in_braces(arg, right, GET_ONE);

  if (*left == 0) {
    display_header(" CONFIGURATIONS ");

    for (index = 0; *config_table[index].name != 0; index++) {
      node = search_node_list(gts->list[LIST_CONFIG], config_table[index].name);

      if (node) {
        display_printf("[%-14s] [%9s] [%s]", node->left, node->right,
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
            display_printf("%cCONFIG: {%s} has been set to {%s}",
                           gtd->command_char, config_table[index].name,
                           node->right);
          }
        }
        return;
      }
    }
    display_printf("%cERROR: {%s} is not a valid option", gtd->command_char,
                   capitalize(left));
  }
}

DO_CONFIG(config_commandchar) {
  if (arg[0]) {
    gtd->command_char = arg[0];
  } else {
    display_printf("%cSYNTAX: %cCONFIG {%s} CHAR", gtd->command_char,
                   gtd->command_char, config_table[index].name);
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
    display_printf("%cSYNTAX: %cCONFIG {%s} {ON|OFF}", gtd->command_char,
                   gtd->command_char, config_table[index].name);
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
    display_printf("%cSYNTAX: %cCONFIG {%s} <ASCII|UTF-8>", gtd->command_char,
                   gtd->command_char, config_table[index].name);
    return FALSE;
  }

  update_node_list(gts->list[LIST_CONFIG], config_table[index].name,
                   capitalize(arg), "");
  return TRUE;
}

DO_CONFIG(config_highlight) {
  if (!strcasecmp(arg, "ON")) {
    SET_BIT(gts->flags, SES_FLAG_HIGHLIGHT);
  } else if (!strcasecmp(arg, "OFF")) {
    DEL_BIT(gts->flags, SES_FLAG_HIGHLIGHT);
  } else {
    display_printf("%cSYNTAX: %cCONFIG {%s} {ON|OFF}", gtd->command_char,
                   gtd->command_char, config_table[index].name);
    return FALSE;
  }

  update_node_list(gts->list[LIST_CONFIG], config_table[index].name,
                   capitalize(arg), "");
  return TRUE;
}
