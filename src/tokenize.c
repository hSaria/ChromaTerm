// This program is protected under the GNU GPL (See COPYING)

#include "defs.h"

struct scriptdata {
  long long min;
  long long max;
  long long cnt;
  int inc;
  char *cpy;
  char *str;
  char *arg;
};

struct scriptnode {
  struct scriptnode *next;
  struct scriptnode *prev;
  union {
    struct scriptdata *data;
    struct script_regex *regex;
  };
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
  struct session *ses;
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

  str = get_arg_in_braces(root->ses, str, arg1, FALSE);
  str = get_arg_in_braces(root->ses, str, arg2, FALSE);
  str = get_arg_in_braces(root->ses, str, arg3, TRUE);

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

  switch (token->type) {
  case TOKEN_TYPE_REGEX:
    free(token->regex->str);
    free(token->regex->bod);
    free(token->regex->buf);
    free(token->regex);
    break;
  }

  free(token);
  return;
}

int find_command(char *command) {
  struct session *ses;
  int cmd;

  for (ses = gts; ses; ses = ses->next) {
    if (!strcmp(ses->name, command)) {
      return -1;
    }
  }

  if (isalpha((int)*command)) {
    for (cmd = gtd->command_ref[tolower((int)*command) - 'a'];
         *command_table[cmd].name; cmd++) {
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
      str = get_arg_all(root->ses, str, line);

      addtoken(root, lvl, TOKEN_TYPE_STRING, -1, line);
    } else {
      arg = get_arg_stop_spaces(root->ses, str, line, 0);

      cmd = find_command(line + 1);

      if (cmd == -1) {
        str = get_arg_all(root->ses, str, line);
        addtoken(root, lvl, TOKEN_TYPE_SESSION, -1, line + 1);
      } else {
        switch (command_table[cmd].type) {
        case TOKEN_TYPE_DEFAULT:
          addtoken(root, lvl++, TOKEN_TYPE_DEFAULT, cmd, "");

          str = get_arg_in_braces(root->ses, arg, line, TRUE);
          tokenize_script(root, lvl--, line);

          addtoken(root, lvl, TOKEN_TYPE_END, -1, "enddefault");
          break;
        case TOKEN_TYPE_ELSE:
          addtoken(root, lvl++, TOKEN_TYPE_ELSE, cmd, "else");

          str = get_arg_in_braces(root->ses, arg, line, TRUE);
          tokenize_script(root, lvl--, line);

          addtoken(root, lvl, TOKEN_TYPE_END, -1, "endelse");
          break;
        case TOKEN_TYPE_REGEX:
          str = addregextoken(root, lvl, TOKEN_TYPE_REGEX, cmd, arg);
          if (*str && *str != COMMAND_SEPARATOR) {
            addtoken(root, lvl++, TOKEN_TYPE_ELSE, -1, "else");

            str = get_arg_in_braces(root->ses, str, line, TRUE);
            tokenize_script(root, lvl--, line);

            addtoken(root, lvl, TOKEN_TYPE_END, -1, "endregex");
          }
          break;
        default:
          str = get_arg_with_spaces(root->ses, arg, line, 1);
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

struct scriptnode *parse_script(struct scriptroot *root, int lvl,
                                struct scriptnode *token,
                                struct scriptnode *shift) {
  struct scriptnode *split = NULL;

  while (token) {
    if (token->lvl < lvl) {
      return token;
    }

    switch (token->type) {
    case TOKEN_TYPE_COMMAND:
      root->ses = (*command_table[token->cmd].command)(root->ses, token->str);
      break;
    case TOKEN_TYPE_DEFAULT:
      token = token->next;

      token = parse_script(root, lvl + 1, token, shift);

      while (token && token->lvl >= lvl) {
        token = token->next;
      }
      continue;
    case TOKEN_TYPE_END:
      break;
    case TOKEN_TYPE_REGEX:
      split = NULL;

      token->regex->val =
          find(root->ses, token->str, token->regex->str, SUB_CMD);

      if (token->regex->val) {
        substitute(root->ses, token->regex->bod, token->regex->buf, SUB_CMD);

        root->ses = script_driver(root->ses, -1, token->regex->buf);
      } else {
        split = token;
      }
      break;
    case TOKEN_TYPE_SESSION:
      root->ses = parse_command(root->ses, token->str);
      break;
    case TOKEN_TYPE_STRING:
      root->ses = parse_input(root->ses, token->str);
      break;
    }

    if (token) {
      token = token->next;
    }
  }
  if (lvl) {
    return NULL;
  } else {
    return (struct scriptnode *)root->ses;
  }
}

struct session *script_driver(struct session *ses, int list, char *str) {
  struct scriptroot *root;
  struct session *cur_ses;
  int ilevel;

  root = (struct scriptroot *)calloc(1, sizeof(struct scriptroot));

  root->ses = cur_ses = ses;

  ilevel = (list >= 0) ? 1 : 0;

  cur_ses->input_level += ilevel;

  tokenize_script(root, 0, str);

  ses = (struct session *)parse_script(root, 0, root->next, root->prev);

  cur_ses->input_level -= ilevel;

  while (root->prev) {
    deltoken(root, root->prev);
  }

  free(root);

  return ses;
}
