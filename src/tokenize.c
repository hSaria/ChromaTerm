// This program is protected under the GNU GPL (See COPYING)

#include "defs.h"

struct scriptnode {
  struct scriptnode *next;
  struct scriptnode *prev;
  struct script_regex *regex;
  char *str;
  short lvl;
  short type;
  short cmd;
};

struct script_regex {
  char *str;
  char *bod;
  char *buf;
  int val;
};

struct scriptroot {
  struct scriptnode *next;
  struct scriptnode *prev;
};

void addtoken(struct scriptroot *root, int lvl, int opr, int cmd, char *str) {
  struct scriptnode *token;

  token = (struct scriptnode *)calloc(1, sizeof(struct scriptnode));

  token->lvl = lvl;
  token->type = opr;
  token->cmd = cmd;
  token->str = strdup(str);

  LINK(token, root->next, root->prev);
}

char *addregextoken(struct scriptroot *root, int lvl, int type, int cmd,
                    char *str) {
  struct script_regex *regex;

  char arg1[BUFFER_SIZE], arg2[BUFFER_SIZE], arg3[BUFFER_SIZE];

  str = get_arg_in_braces(str, arg1, FALSE);
  str = get_arg_in_braces(str, arg2, FALSE);
  str = get_arg_in_braces(str, arg3, TRUE);

  addtoken(root, lvl, type, cmd, arg1);

  regex = (struct script_regex *)calloc(1, sizeof(struct script_regex));

  regex->str = strdup(arg2);
  regex->bod = strdup(arg3);
  regex->buf = calloc(1, BUFFER_SIZE);

  root->prev->regex = regex;

  return str;
}

void deltoken(struct scriptroot *root, struct scriptnode *token) {
  UNLINK(token, root->next, root->prev);

  free(token->str);

  if (token->type == TOKEN_TYPE_REGEX) {
    free(token->regex->str);
    free(token->regex->bod);
    free(token->regex->buf);
    free(token->regex);
  }

  free(token);
  return;
}

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

void tokenize_script(struct scriptroot *root, int lvl, char *str) {
  char *arg, *line;
  int cmd;

  if (*str == 0) {
    addtoken(root, lvl, TOKEN_TYPE_STRING, -1, "");

    return;
  }

  line = (char *)calloc(1, BUFFER_SIZE);

  while (*str) {
    str = space_out(str);

    if (*str != gtd->command_char) {
      str = get_arg_all(str, line);

      addtoken(root, lvl, TOKEN_TYPE_STRING, -1, line);
    } else {
      arg = get_arg_stop_spaces(str, line, 0);

      cmd = find_command(line + 1);

      if (cmd == -1) {
        str = get_arg_all(str, line);
        addtoken(root, lvl, TOKEN_TYPE_SESSION, -1, line + 1);
      } else {
        switch (command_table[cmd].type) {
        case TOKEN_TYPE_ELSE:
          addtoken(root, lvl++, TOKEN_TYPE_ELSE, cmd, "else");

          str = get_arg_in_braces(arg, line, TRUE);
          tokenize_script(root, lvl--, line);

          addtoken(root, lvl, TOKEN_TYPE_END, -1, "endelse");
          break;
        case TOKEN_TYPE_REGEX:
          str = addregextoken(root, lvl, TOKEN_TYPE_REGEX, cmd, arg);
          if (*str && *str != COMMAND_SEPARATOR) {
            addtoken(root, lvl++, TOKEN_TYPE_ELSE, -1, "else");

            str = get_arg_in_braces(str, line, TRUE);
            tokenize_script(root, lvl--, line);

            addtoken(root, lvl, TOKEN_TYPE_END, -1, "endregex");
          }
          break;
        default:
          str = get_arg_with_spaces(arg, line);
          addtoken(root, lvl, TOKEN_TYPE_COMMAND, cmd, line);
          break;
        }
      }
    }
    if (*str == COMMAND_SEPARATOR) {
      str++;
    }
  }

  free(line);
}

struct scriptnode *parse_script(int lvl, struct scriptnode *token) {
  struct scriptnode *split = NULL;

  while (token) {
    if (token->lvl < lvl) {
      return token;
    }

    switch (token->type) {
    case TOKEN_TYPE_COMMAND:
      (*command_table[token->cmd].command)(token->str);
      break;
    case TOKEN_TYPE_END:
      break;
    case TOKEN_TYPE_REGEX:
      split = NULL;

      token->regex->val = find(token->str, token->regex->str, SUB_CMD);

      if (token->regex->val) {
        substitute(token->regex->bod, token->regex->buf, SUB_CMD);

        gts = script_driver(token->regex->buf);
      }
      break;
    case TOKEN_TYPE_SESSION:
      gts = parse_command(token->str);
      break;
    case TOKEN_TYPE_STRING:
      gts = parse_input(token->str);
      break;
    }

    if (token) {
      token = token->next;
    }
  }
  if (lvl) {
    return NULL;
  } else {
    return (struct scriptnode *)gts;
  }
}

struct session *script_driver(char *str) {
  struct scriptroot *root;

  root = (struct scriptroot *)calloc(1, sizeof(struct scriptroot));

  tokenize_script(root, 0, str);

  gts = (struct session *)parse_script(0, root->next);

  while (root->prev) {
    deltoken(root, root->prev);
  }

  free(root);

  return gts;
}
