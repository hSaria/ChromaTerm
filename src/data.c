// This program is protected under the GNU GPL (See COPYING)

#include "defs.h"

struct listroot *init_list(int type, int size) {
  struct listroot *listhead;

  if ((listhead = (struct listroot *)calloc(1, sizeof(struct listroot))) ==
      NULL) {
    fprintf(stderr, "couldn't alloc listhead\n");
    exit(1);
  }

  listhead->list = (struct listnode **)calloc(size, sizeof(struct listnode *));
  listhead->size = size;
  listhead->type = type;

  return listhead;
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
      display_printf(FALSE, "#BUG: update_node_list: unknown mode: %d",
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

void delete_node_list(int type, struct listnode *node) {
  delete_index_list(gts->list[type],
                    search_index_list(gts->list[type], node->left, node->pr));
}

void delete_index_list(struct listroot *root, int index) {
  struct listnode *node = root->list[index];

  if (index <= root->update) {
    root->update--;
  }

  free(node->left);
  free(node->right);
  free(node->pr);
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

void delete_node_with_wild(int type, char *text) {
  struct listroot *root = gts->list[type];
  struct listnode *node;
  char arg1[BUFFER_SIZE];
  int i, found = FALSE;

  get_arg_in_braces(text, arg1, GET_ALL);

  node = search_node_list(root, arg1);

  if (node) {
    show_message(
        "#OK. {%s} IS NO LONGER %s %s", node->left,
        (*list_table[type].name == 'A' || *list_table[type].name == 'E') ? "AN"
                                                                         : "A",
        list_table[type].name);

    delete_node_list(type, node);

    return;
  }

  for (i = root->used - 1; i >= 0; i--) {
    if (root->list[i]->left == arg1) {
      show_message(
          "#OK. {%s} IS NO LONGER %s %s", root->list[i]->left,
          (*list_table[type].name == 'A' || *list_table[type].name == 'E')
              ? "AN"
              : "A",
          list_table[type].name);

      delete_index_list(root, i);

      found = TRUE;
    }
  }

  if (found == 0) {
    show_message("#ERROR: NO MATCHES FOUND FOR %s {%s}", list_table[type].name,
                 arg1);
  }
}
