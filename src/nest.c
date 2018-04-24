// This program is protected under the GNU GPL (See COPYING)

#include "defs.h"

struct listroot *update_nest_root(struct listroot *root, char *arg) {
  struct listnode *node;

  node = search_node_list(root, arg);

  if (node == NULL) {
    node = update_node_list(root, arg, "", "");
  }

  if (node->root == NULL) {
    node->root = init_list(root->ses, root->type, LIST_SIZE);
  }

  return node->root;
}

void update_nest_node(struct listroot *root, char *arg) {
  char arg1[BUFFER_SIZE], arg2[BUFFER_SIZE];

  while (*arg) {
    arg = get_arg_in_braces(root->ses, arg, arg1, FALSE);
    arg = get_arg_in_braces(root->ses, arg, arg2, FALSE);

    if (*arg2 == DEFAULT_OPEN) {
      update_nest_node(update_nest_root(root, arg1), arg2);
    } else if (*arg1) {
      update_node_list(root, arg1, arg2, "");
    }

    if (*arg == COMMAND_SEPARATOR) {
      arg++;
    }
  }
}

// cats to result when initialize is 0
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

void copy_nest_node(struct listroot *dst_root, struct listnode *dst,
                    struct listnode *src) {
  int index;

  if (src->root == NULL) {
    return;
  }

  dst_root = dst->root =
      init_list(dst_root->ses, dst_root->type, src->root->size);

  for (index = 0; index < src->root->used; index++) {
    dst = insert_node_list(dst_root, src->root->list[index]->left,
                           src->root->list[index]->right,
                           src->root->list[index]->pr);

    if (src->root->list[index]->root) {
      copy_nest_node(dst_root, dst, src->root->list[index]);
    }
  }
}
