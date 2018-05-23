/* This program is protected under the GNU GPL (See COPYING) */

#include "defs.h"

DO_COMMAND(do_commands) {
  char buf[BUFFER_SIZE] = {0}, add[BUFFER_SIZE];
  int cmd;

  for (cmd = 0; *command_table[cmd].name != 0; cmd++) {
    if (*arg && !is_abbrev(arg, command_table[cmd].name)) {
      continue;
    }

    sprintf(add, "%-14s", command_table[cmd].name);
    strcat(buf, add);
  }
  if (buf[0]) {
    display_printf(buf);
  }
}

DO_COMMAND(do_configure) {
  char left[BUFFER_SIZE];

  arg = get_arg(arg, left);
  get_arg(arg, arg);

  if (*left == 0) {
    display_printf("%-12s = %-3c    [%s]", "COMMAND CHAR", gd.command_char,
                   "The character used for CT-- commands");
    display_printf("%-12s = %-3s    [%s]", "CONVERT META",
                   HAS_BIT(gd.flags, SES_FLAG_CONVERTMETA) ? "ON" : "OFF",
                   "Convert meta and control characters");
    display_printf("%-12s = %-3s    [%s]", "HIGHLIGHT",
                   HAS_BIT(gd.flags, SES_FLAG_HIGHLIGHT) ? "ON" : "OFF",
                   "Highlight according to rules");
  } else {
    if (is_abbrev(left, "COMMAND CHAR")) {
      if (*arg == 0) {
        display_printf("%1$cSYNTAX: %1$cCONFIG {COMMAND CHAR} {CHAR}",
                       gd.command_char);
      } else if (!ispunct((int)arg[0])) {
        display_printf("%cERROR: Commad character must me a punctuation: "
                       "!@#$%%^&*-+=',.\"\\/:;?_`<>()[]{}|~",
                       gd.command_char);
      } else {
        gd.command_char = arg[0];
      }
    } else if (is_abbrev(left, "CONVERT META")) {
      if (!strcasecmp(arg, "ON")) {
        SET_BIT(gd.flags, SES_FLAG_CONVERTMETA);
      } else if (!strcasecmp(arg, "OFF")) {
        DEL_BIT(gd.flags, SES_FLAG_CONVERTMETA);
      } else {
        display_printf("%1$cSYNTAX: %1$cCONFIG {CONVERT META} {ON|OFF}",
                       gd.command_char);
      }
    } else if (is_abbrev(left, "HIGHLIGHT")) {
      if (!strcasecmp(arg, "ON")) {
        SET_BIT(gd.flags, SES_FLAG_HIGHLIGHT);
      } else if (!strcasecmp(arg, "OFF")) {
        DEL_BIT(gd.flags, SES_FLAG_HIGHLIGHT);
      } else {
        display_printf("%1$cSYNTAX: %1$cCONFIG {HIGHLIGHT} {ON|OFF}",
                       gd.command_char);
      }
    } else {
      display_printf("%cERROR: {%s} is not a valid option", gd.command_char,
                     left);
    }
  }
}

DO_COMMAND(do_exit) {
  if (*arg) {
    quit_with_msg(arg, 0);
  }
  quit_with_msg(NULL, 0);
}

DO_COMMAND(do_help) {
  char left[BUFFER_SIZE], add[BUFFER_SIZE];
  int cnt;

  get_arg(arg, left);

  if (*left == 0) {
    for (cnt = add[0] = 0; *help_table[cnt].name != 0; cnt++) {
      cat_sprintf(add, "%-14s", help_table[cnt].name);
    }
    display_printf(add);

  } else {
    int found = FALSE;
    for (cnt = 0; *help_table[cnt].name != 0; cnt++) {
      if (is_abbrev(left, help_table[cnt].name) || is_abbrev(left, "all")) {
        char buf[BUFFER_SIZE];
        found = TRUE;

        substitute(help_table[cnt].text, buf);

        display_printf(buf);
      }
    }

    if (!found) {
      display_printf("%cHELP: No help found for topic '%s'", gd.command_char,
                     left);
    }
  }
}

DO_COMMAND(do_showme) {
  char *pto = space_out(arg);

  check_all_highlights(pto);

  display_printf(pto);
}

void script_driver(char *str) {
  /* Skip any unnecessary command chars or spaces before the actual command */
  while (*str == gd.command_char || isspace((int)*str)) {
    str++;
  }

  if (*str != 0) {
    char *args, line[BUFFER_SIZE];
    int i;

    /* Command stored in line, the rest in args */
    args = get_arg(str, line);

    for (i = 0; *command_table[i].name != 0; i++) {
      if (is_abbrev(line, command_table[i].name)) {
        (*command_table[i].command)(args);
        *args = 0;
        return;
      }
    }

    display_printf("%cERROR: Unknown command '%s'", gd.command_char, line);
  }
}
