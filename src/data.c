// This program is protected under the GNU GPL (See COPYING)

#include "defs.h"

struct listroot *init_list(struct session *ses, int type, int size) {
  struct listroot *listhead;

  if ((listhead = (struct listroot *)calloc(1, sizeof(struct listroot))) ==
      NULL) {
    fprintf(stderr, "couldn't alloc listhead\n");
    exit(1);
  }

  listhead->ses = ses;
  listhead->list = (struct listnode **)calloc(size, sizeof(struct listnode *));
  listhead->size = size;
  listhead->type = type;

  return listhead;
}

void kill_list(struct listroot *root) {
  while (root->used) {
    delete_index_list(root, root->used - 1);
  }
}

void free_list(struct listroot *root) {
  kill_list(root);

  free(root->list);
  free(root);
}

// create a node and stuff it into the list in the desired order
struct listnode *insert_node_list(struct listroot *root, char *ltext,
                                  char *rtext, char *prtext) {
  int index;
  struct listnode *node;

  node = (struct listnode *)calloc(1, sizeof(struct listnode));

  node->left = strdup(ltext);
  node->right = strdup(rtext);
  node->pr = strdup(prtext);

  node->group = strdup("");

  switch (root->type) {
  case LIST_HIGHLIGHT:
    break;
  }

  index = locate_index_list(root, ltext, prtext);

  return insert_index_list(root, node, index);
}

struct listnode *update_node_list(struct listroot *root, char *ltext,
                                  char *rtext, char *prtext) {
  int index;
  struct listnode *node;

  index = search_index_list(root, ltext, NULL);

  if (index != -1) {
    node = root->list[index];

    if (strcmp(node->right, rtext) != 0) {
      free(node->right);
      node->right = strdup(rtext);
    }

    switch (list_table[root->type].mode) {
    case PRIORITY:
      if (atof(node->pr) != atof(prtext)) {
        delete_index_list(root, index);
        return insert_node_list(root, ltext, rtext, prtext);
      }
      break;
    case ALPHA:
      if (strcmp(node->pr, prtext) != 0) {
        free(node->pr);
        node->pr = strdup(prtext);
      }
      break;
    default:
      display_printf2(root->ses, "#BUG: update_node_list: unknown mode: %d",
                      list_table[root->type].mode);
      break;
    }
    return node;
  } else {
    return insert_node_list(root, ltext, rtext, prtext);
  }
}

struct listnode *insert_index_list(struct listroot *root, struct listnode *node,
                                   int index) {
  root->used++;

  if (root->used == root->size) {
    root->size *= 2;

    root->list = (struct listnode **)realloc(
        root->list, (root->size) * sizeof(struct listnode *));
  }

  memmove(&root->list[index + 1], &root->list[index],
          (root->used - index) * sizeof(struct listnode *));

  root->list[index] = node;

  return node;
}

void delete_node_list(struct session *ses, int type, struct listnode *node) {
  int index = search_index_list(ses->list[type], node->left, node->pr);

  delete_index_list(ses->list[type], index);
}

void delete_index_list(struct listroot *root, int index) {
  struct listnode *node = root->list[index];

  if (node->root) {
    free_list(node->root);
  }

  if (index <= root->update) {
    root->update--;
  }

  free(node->left);
  free(node->right);
  free(node->pr);
  free(node->group);

  if (node->regex) {
    free(node->regex);
  }
  free(node);

  memmove(&root->list[index], &root->list[index + 1],
          (root->used - index) * sizeof(struct listnode *));

  root->used--;

  return;
}

struct listnode *search_node_list(struct listroot *root, char *text) {
  int index;

  switch (list_table[root->type].mode) {
  case ALPHA:
    index = bsearch_alpha_list(root, text, 0);
    break;

  default:
    index = nsearch_list(root, text);
    break;
  }

  if (index != -1) {
    return root->list[index];
  } else {
    return NULL;
  }
}

int search_index_list(struct listroot *root, char *text, char *priority) {
  if (list_table[root->type].mode == ALPHA) {
    return bsearch_alpha_list(root, text, 0);
  }

  if (list_table[root->type].mode == PRIORITY && priority) {
    return bsearch_priority_list(root, text, priority, 0);
  }

  return nsearch_list(root, text);
}

// Return insertion index.
int locate_index_list(struct listroot *root, char *text, char *priority) {
  switch (list_table[root->type].mode) {
  case ALPHA:
    return bsearch_alpha_list(root, text, 1);

  case PRIORITY:
    return bsearch_priority_list(root, text, priority, 1);

  default:
    return root->used;
  }
}

// Yup, all this for a bloody binary search.
int bsearch_alpha_list(struct listroot *root, char *text, int seek) {
  int bot, top, val;
  double srt;

  bot = 0;
  top = root->used - 1;
  val = top;

  while (bot <= top) {
    srt = strcmp(text, root->list[val]->left);

    if (srt == 0) {
      return val;
    }

    if (srt < 0) {
      top = val - 1;
    } else {
      bot = val + 1;
    }

    val = bot + (top - bot) / 2;
  }

  if (seek) {
    return UMAX(0, val);
  } else {
    return -1;
  }
}

int bsearch_priority_list(struct listroot *root, char *text, char *priority,
                          int seek) {
  int bot, top, val;
  double srt;

  bot = 0;
  top = root->used - 1;
  val = top;

  while (bot <= top) {
    srt = atof(priority) - atof(root->list[val]->pr);

    if (!srt) {
      srt = strcmp(text, root->list[val]->left);
    }

    if (srt == 0) {
      return val;
    }

    if (srt < 0) {
      top = val - 1;
    } else {
      bot = val + 1;
    }

    val = bot + (top - bot) / 2;
  }

  if (seek) {
    return UMAX(0, val);
  } else {
    return -1;
  }
}

int nsearch_list(struct listroot *root, char *text) {
  int i;

  for (i = 0; i < root->used; i++) {
    if (!strcmp(text, root->list[i]->left)) {
      return i;
    }
  }
  return -1;
}

// show contens of a node on screen
void show_node(struct listroot *root, struct listnode *node, int level) {
  char arg[STRING_SIZE];

  show_nest_node(node, arg, TRUE);

  switch (list_table[root->type].args) {
  case 3:
    display_printf2(
        root->ses,
        "%*s#%s "
        "\033[1;31m{\033[0m%s\033[1;31m}\033[1;36m \033[1;31m{\033[0m%s\033[1;"
        "31m} \033[1;36m\033[1;31m{\033[0m%s\033[1;31m}",
        level * 2, "", list_table[root->type].name, node->left, arg, node->pr);
    break;
  case 2:
    display_printf2(root->ses,
                    "%*s#%s "
                    "\033[1;31m{\033[0m%s\033[1;31m}\033[1;36m=\033[1;31m{\033["
                    "0m%s\033[1;31m}",
                    level * 2, "", list_table[root->type].name, node->left,
                    arg);
    break;
  case 1:
    display_printf2(root->ses, "%*s#%s \033[1;31m{\033[0m%s\033[1;31m}",
                    level * 2, "", list_table[root->type].name, node->left);
    break;
  }
}

void show_nest_node(struct listnode *node, char *result, int initialize) {
  if (initialize) {
    *result = 0;
  }

  if (node->root == NULL) {
    if (initialize) {
      strcat(result, node->right);
    } else {
      cat_sprintf(result, "{%s}", node->right);
    }
  } else {
    struct listroot *root = node->root;
    int i;

    if (!initialize) {
      strcat(result, "{");
    }

    for (i = 0; i < root->used; i++) {
      cat_sprintf(result, "{%s}", root->list[i]->left);

      show_nest_node(root->list[i], result, FALSE);
    }
    if (!initialize) {
      strcat(result, "}");
    }
  }
}

// list contens of a list on screen
void show_list(struct listroot *root, int level) {
  int i;

  if (root == root->ses->list[root->type]) {
    display_header(root->ses, " %s ", list_table[root->type].name_multi);
  }

  for (i = 0; i < root->used; i++) {
    show_node(root, root->list[i], level);
  }
}

int show_node_with_wild(struct session *ses, char *text, int type) {
  struct listroot *root = ses->list[type];
  struct listnode *node;
  int i, flag = FALSE;

  node = search_node_list(root, text);

  if (node) {
    show_node(root, node, 0);

    return TRUE;
  }

  for (i = 0; i < root->used; i++) {
    if (match(ses, root->list[i]->left, text, SUB_NONE)) {
      show_node(root, root->list[i], 0);

      flag = TRUE;
    }
  }
  return flag;
}

void delete_node_with_wild(struct session *ses, int type, char *text) {
  struct listroot *root = ses->list[type];
  struct listnode *node;
  char arg1[BUFFER_SIZE];
  int i, found = FALSE;

  sub_arg_in_braces(ses, text, arg1, 1, SUB_NONE);

  node = search_node_list(root, arg1);

  if (node) {
    show_message(
        ses, type, "#OK. {%s} IS NO LONGER %s %s", node->left,
        (*list_table[type].name == 'A' || *list_table[type].name == 'E') ? "AN"
                                                                         : "A",
        list_table[type].name);

    delete_node_list(ses, type, node);

    return;
  }

  for (i = root->used - 1; i >= 0; i--) {
    if (match(ses, root->list[i]->left, arg1, SUB_NONE)) {
      show_message(
          ses, type, "#OK. {%s} IS NO LONGER %s %s", root->list[i]->left,
          (*list_table[type].name == 'A' || *list_table[type].name == 'E')
              ? "AN"
              : "A",
          list_table[type].name);

      delete_index_list(root, i);

      found = TRUE;
    }
  }

  if (found == 0) {
    show_message(ses, type, "#ERROR: NO MATCHES FOUND FOR %s {%s}",
                 list_table[type].name, arg1);
  }
}
