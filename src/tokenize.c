/* This program is protected under the GNU GPL (See COPYING) */

#include "defs.h"

struct scriptnode {
  char *str;
  short cmd;
};

int find_command(char *command) {
  int cmd;

  if (isalpha((int)*command)) {
    for (cmd = 0; *command_table[cmd].name != 0; cmd++) {
      if (is_abbrev(command, command_table[cmd].name)) {
        return cmd;
      }
    }
  }
  return -1;
}

void parse_script(struct scriptnode *token) {
  if (token->cmd == -1) {
    display_printf("%cERROR: Unknown command '%s'", gtd->command_char,
                   token->str);
  } else {
    (*command_table[token->cmd].command)(token->str);
  }
  free(token->str);
}

void script_driver(char *str) {
  struct scriptnode token;

  if (*str != 0) {
    str = space_out(str);

    if (*str == gtd->command_char) {
      char *args, line[BUFFER_SIZE];
      int cmd;
      /* Command stored in line, rest of string in args */
      args = get_arg_stop_spaces(str, line);
      cmd = find_command(line + 1);

      if (cmd == -1) {
        token.cmd = -1;
        token.str = strdup(line + 1);
      } else {
        get_arg_all(args, line, TRUE);
        token.cmd = cmd;
        token.str = strdup(args);
      }
      parse_script(&token);
    }
  }
}
