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

void script_driver(char *str) {
  str = space_out(str);

  if (*str != 0) {
    char *args, line[BUFFER_SIZE];
    int cmd;

    while (*str == gtd->command_char) {
      str++;
    }

    /* Command stored in line, rest of string in args */
    args = get_arg_stop_spaces(str, line);

    cmd = find_command(line);

    if (cmd == -1) {
      display_printf("%cERROR: Unknown command '%s'", gtd->command_char, str);
    } else {
      (*command_table[cmd].command)(args);
      *args = 0;

      /* Repair the mud's current line */
      printline(gtd->mud_output_buf, TRUE);
    }
  }
}
