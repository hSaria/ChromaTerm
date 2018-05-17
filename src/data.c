/* This program is protected under the GNU GPL (See COPYING) */

#include "defs.h"

int bsearch_alpha_list(struct listroot *root, char *text, int seek) {
  int bot, top, val;

  bot = 0;
  top = root->used - 1;
  val = top;

  while (bot <= top) {
    double srt = strcmp(text, root->list[val]->left);

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

  bot = 0;
  top = root->used - 1;
  val = top;

  while (bot <= top) {
    double srt = atof(priority) - atof(root->list[val]->pr);

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

void delete_index_list(struct listroot *root, int index) {
  struct listnode *node = root->list[index];

  if (node->compiled_regex != NULL) {
    pcre_free(node->compiled_regex);
  }

  free(node);

  memmove(&root->list[index], &root->list[index + 1],
          (root->used - index) * sizeof(struct listnode *));

  root->used--;
}

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

/* create a node and stuff it into the list in the desired order */
struct listnode *insert_node_list(struct listroot *root, char *ltext,
                                  char *rtext, char *prtext) {
  const char *error_pointer;
  int error_offset;
  struct listnode *node;

  node = (struct listnode *)calloc(1, sizeof(struct listnode));

  strcpy(node->left, ltext);
  strcpy(node->right, rtext);
  if (list_table[root->type].mode == PRIORITY) {
    strcpy(node->pr, prtext);
  }

  if (root->type == LIST_HIGHLIGHT) {
    node->compiled_regex =
        pcre_compile(ltext, 0, &error_pointer, &error_offset, NULL);
    if (node->compiled_regex == NULL) {
      display_printf("%cWARNING: Couldn't compile regex at %i: %s",
                     gtd.command_char, error_offset, error_pointer);
    }
  }

  return insert_index_list(root, node, locate_index_list(root, ltext, prtext));
}

/* Return insertion index */
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

int nsearch_list(struct listroot *root, char *text) {
  int i;

  for (i = 0; i < root->used; i++) {
    if (!strcmp(text, root->list[i]->left)) {
      return i;
    }
  }
  return -1;
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

struct listnode *update_node_list(struct listroot *root, char *ltext,
                                  char *rtext, char *prtext) {
  int index;

  index = search_index_list(root, ltext, NULL);

  if (index != -1) {
    struct listnode *node = root->list[index];

    if (strcmp(node->right, rtext) != 0) {
      strcpy(node->right, rtext);
    }

    if (list_table[root->type].mode == PRIORITY) {
      if (atof(node->pr) != atof(prtext)) {
        delete_index_list(root, index);
        return insert_node_list(root, ltext, rtext, prtext);
      }
    }
    return node;
  } else {
    return insert_node_list(root, ltext, rtext, prtext);
  }
}
