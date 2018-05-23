/* This program is protected under the GNU GPL (See COPYING) */

#include "defs.h"

DO_COMMAND(do_configure) {
  int sel = -1;
  char *config[3] = {"COMMAND CHAR = ", "CONVERT META = ", "HIGHLIGHT    = "};

  strcat(config[0], &gd.command_char);
  strcat(config[1], HAS_BIT(gd.flags, SES_FLAG_CONVERTMETA) ? "ON" : "OFF");
  strcat(config[2], HAS_BIT(gd.flags, SES_FLAG_HIGHLIGHT) ? "ON" : "OFF");

  sel = show_menu(config, 3);

  switch (sel) {
  case 0:
    break;
  case 1:
    TOG_BIT(gd.flags, SES_FLAG_CONVERTMETA);
    break;
  case 2: /* Toggle the highlight option */
    TOG_BIT(gd.flags, SES_FLAG_HIGHLIGHT);
    break;
  default:
    return;
  }

  do_configure(NULL);

  return;

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
  (void)arg; /* To make a warning shut up */
  quit_with_msg(EXIT_SUCCESS);
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
      if (is_abbrev(left, help_table[cnt].name)) {
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

/* Read and process a CT-- command */
#ifdef HAVE_CURSES_H
#ifdef HAVE_MENU_H
void read_command(void) { // TEMP
  int len = 0;
  char command_buffer[BUFFER_SIZE];

  /* Remove the trailing \n */
  command_buffer[strlen(command_buffer) - 1] = 0;
  script_driver(command_buffer);
  memset(&command_buffer, 0, len);
}
#endif
#endif

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

#ifdef HAVE_CURSES_H
#ifdef HAVE_MENU_H
int show_menu(char **item_strings, int count) {
  int c, i, selection = 0;
  MENU *menu;
  ITEM **items = (ITEM **)calloc(count, sizeof(ITEM *));

  for (i = 0; i < count; ++i) {
    items[i] = new_item(item_strings[i], "");
  }

  menu = new_menu(items);
  set_menu_mark(menu, "* ");

  mvprintw(LINES - 2, 0, "Q to close menu");

  post_menu(menu);
  refresh();

  while ((c = getch())) {
    if (c == KEY_UP || tolower(c) == 'w') {
      menu_driver(menu, REQ_UP_ITEM);
      selection = selection > 0 ? selection - 1 : selection;
    } else if (c == KEY_DOWN || tolower(c) == 's') {
      menu_driver(menu, REQ_DOWN_ITEM);
      selection = selection < count ? selection + 1 : selection;
    } else if (c == '\n') {
      break;
    } else if (tolower(c) == 'q') {
      selection = -1;
      break;
    }
  }

  unpost_menu(menu);

  for (i = 0; i < count; ++i) {
    free_item(items[i]);
  }

  free_menu(menu);
  endwin();

  return selection;
}
#endif
#endif
