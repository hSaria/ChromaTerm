// This program is protected under the GNU GPL (See COPYING)

#include "defs.h"

struct scriptnode {
  char *str;
  short type;
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
  switch (token->type) {
  case TOKEN_TYPE_STRING:
    parse_input(token->str);
    break;
  case TOKEN_TYPE_SESSION:
    display_printf(TRUE, "#ERROR: #UNKNOWN COMMAND '%s'", token->str);
    break;
  case TOKEN_TYPE_COMMAND:
    (*command_table[token->cmd].command)(token->str);
    break;
  }
}

void add_token(struct scriptnode *token, int type, int cmd, char *str) {
  token->type = type;
  token->cmd = cmd;
  token->str = strdup(str);
}

void script_driver(char *str) {
  struct scriptnode token;

  if (*str == 0) {
    add_token(&token, TOKEN_TYPE_STRING, -1, "");
  } else {
    str = space_out(str);

    if (*str != gtd->command_char) {
      add_token(&token, TOKEN_TYPE_STRING, -1, str);
    } else {
      char *args, line[BUFFER_SIZE];
      int cmd;

      // Command stored in line, rest of string in args
      args = get_arg_stop_spaces(str, line);
      cmd = find_command(line + 1);

      if (cmd == -1) {
        add_token(&token, TOKEN_TYPE_SESSION, -1, line + 1);
      } else {
        get_arg_all(args, line, TRUE);
        add_token(&token, TOKEN_TYPE_COMMAND, cmd, args);
      }
    }
  }

  parse_script(&token);
}
