/* This program is protected under the GNU GPL (See COPYING) */

#include "defs.h"

int find_command(char *command) {
  if (isalpha((int)*command)) {
    int cmd;
    for (cmd = 0; *command_table[cmd].name != 0; cmd++) {
      if (is_abbrev(command, command_table[cmd].name)) {
        return cmd;
      }
    }
  }
  return -1;
}

void parse_script(char *str, int cmd) {
  if (cmd == -1) {
    display_printf("%cERROR: Unknown command '%s'", gtd->command_char, str);
  } else {
    (*command_table[cmd].command)(str);

    /* Repair the mud's current line */
    printline(gtd->mud_output_buf, TRUE);
  }
}

void script_driver(char *str) {
  if (*str != 0) {
    str = space_out(str);

    if (*str == gtd->command_char) {
      char *args, line[BUFFER_SIZE];
      int cmd;

      /* Command stored in line, rest of string in args */
      args = get_arg_stop_spaces(str, line);

      cmd = find_command(line + 1);
      if (cmd == -1) {
        parse_script(line + 1, cmd);
      } else {
        get_arg_all(args, line, TRUE);
        parse_script(args, cmd);
      }
    }
  }
}
