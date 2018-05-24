/* This program is protected under the GNU GPL (See COPYING) */

#include "defs.h"

void MENU_ACTION_CONFIG(void) {
  int sel;
  char convert_meta[BUFFER_SIZE], highlight[BUFFER_SIZE], read[BUFFER_SIZE],
      write[BUFFER_SIZE], value[BUFFER_SIZE], *config[4];

  sprintf(convert_meta, "%-12s = %-3s    [%s]", "CONVERT META",
          HAS_BIT(gd.flags, SES_FLAG_CONVERTMETA) ? "ON" : "OFF",
          "Convert meta and control characters");
  sprintf(highlight, "%-12s = %-3s    [%s]", "HIGHLIGHT",
          HAS_BIT(gd.flags, SES_FLAG_HIGHLIGHT) ? "ON" : "OFF",
          "Highlight according to rules");
  sprintf(read, "%-21s [%s]", "READ", "Load (merge) a configuration file");
  sprintf(write, "%-21s [%s]", "WRITE",
          "Save the current configuration and highlight rules");

  config[0] = convert_meta;
  config[1] = highlight;
  config[2] = read;
  config[3] = write;

  sel = show_menu(config, 4);

  switch (sel) {
  case 0: /* Toggle the convert meta option */
    if (HAS_BIT(gd.flags, SES_FLAG_CONVERTMETA)) {
      do_configure("{CONVERT META} {OFF}");
    } else {
      do_configure("{CONVERT META} {ON}");
    }
    break;
  case 1: /* Toggle the highlight option */
    if (HAS_BIT(gd.flags, SES_FLAG_HIGHLIGHT)) {
      do_configure("{HIGHLIGHT} {OFF}");
    } else {
      do_configure("{HIGHLIGHT} {ON}");
    }
    break;
  case 2: /* Read config file */
    sprintf(value, "{READ} {%s}", ask_for_input());
    do_configure(value);
    break;
  case 3: /* Write config file */
    sprintf(value, "{WRITE} {%s}", ask_for_input());
    do_configure(value);
    break;
  default:
    break;
  }
}

void MENU_ACTION_HELP(void) {
  int cnt = 0, sel = -1;

  while (*help_table[cnt].name != 0) {
    cnt++;
  }

  char *help_subjects[cnt];

  for (cnt = 0; *help_table[cnt].name != 0; cnt++) {
    help_subjects[cnt] = help_table[cnt].name;
  }

  sel = show_menu(help_subjects, cnt);

  if (sel > -1) {
    char buf[BUFFER_SIZE];
    substitute(help_table[sel].text, buf);

    show_msg(buf);
  }
}

void MENU_ACTION_HIGHLIGHT_ADD(void) {
  char value[BUFFER_SIZE];
  strcpy(value, ask_for_input());
  do_highlight(value);
}

void MENU_ACTION_HIGHLIGHT_REMOVE(void) {
  int cnt = 0, sel = -1;

  char *highlights[gd.highlights_used];

  for (cnt = 0; cnt < gd.highlights_used; cnt++) {
    char temp[BUFFER_SIZE];
    sprintf(temp, "{%s} {%s} {%s}", gd.highlights[cnt]->condition,
            gd.highlights[cnt]->action, gd.highlights[cnt]->priority);
    highlights[cnt] = temp;
  }

  sel = show_menu(highlights, gd.highlights_used);

  if (sel > -1) {
    do_unhighlight(gd.highlights[cnt]->condition);
  }
}

char *ask_for_input(void) { return 0; }

void main_menu(void) {
  char *items[4] = {"HIGHLIGHT->ADD", "HIGHLIGHT->REMOVE", "CONFIG", "HELP"};

  int sel = show_menu(items, 4);

  switch (sel) {
  case 0:
    MENU_ACTION_HIGHLIGHT_ADD();
    break;
  case 1:
    MENU_ACTION_HIGHLIGHT_REMOVE();
    break;
  case 2:
    MENU_ACTION_CONFIG();
    break;
  case 3:
    MENU_ACTION_HELP();
    break;
  }
}

/* Display the  */
int show_menu(char **item_strings, int count) {
  int c, i, selection = 0;
  MENU *menu;
  ITEM **items = (ITEM **)calloc(count + 1, sizeof(ITEM *));

  initscr();            /* initialise default ncurses screen (stdscr) */
  cbreak();             /* Disable line buffering */
  noecho();             /* Don't print input to the CT menu back */
  keypad(stdscr, TRUE); /* Accept special characters (e.g. arrow keys) */

  for (i = 0; i < count; ++i) {
    items[i] = new_item(item_strings[i], "");
  }
  items[count] = NULL;

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
  free_item(items[count]);

  free_menu(menu);
  endwin();

  return selection;
}

void show_msg(char *msg) { (void)msg; }
