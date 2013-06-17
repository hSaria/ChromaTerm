/******************************************************************************
*   This program is protected under the GNU GPL (See COPYING)                 *
*                                                                             *
*   This program is free software; you can redistribute it and/or modify      *
*   it under the terms of the GNU General Public License as published by      *
*   the Free Software Foundation; either version 2 of the License, or         *
*   (at your option) any later version.                                       *
*                                                                             *
*   This program is distributed in the hope that it will be useful,           *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of            *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
*   GNU General Public License for more details.                              *
*                                                                             *
*   You should have received a copy of the GNU General Public License         * 
*   along with this program; if not, write to the Free Software               *
*   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA *
*******************************************************************************/

/******************************************************************************
*                  C H R O M A T E R M (C) 2013 (See CREDITS)                 *
******************************************************************************/

#include "defs.h"

struct listroot *search_nest_root(struct listroot *root, char *arg)
{
	struct listnode *node;

	node = search_node_list(root, arg);

	if (node == NULL || node->root == NULL)
	{
		return NULL;
	}
	return node->root;
}

struct listnode *search_base_node(struct listroot *root, char *variable)
{
	char name[BUFFER_SIZE];

	get_arg_to_brackets(root->ses, variable, name);

	return search_node_list(root, name);
}

struct listnode *search_nest_node(struct listroot *root, char *variable)
{
	char name[BUFFER_SIZE], *arg;

	arg = get_arg_to_brackets(root->ses, variable, name);

	while (root && *arg)
	{
		root = search_nest_root(root, name);

		if (root)
		{
			arg = get_arg_in_brackets(root->ses, arg, name);
		}
	}

	if (root)
	{
		return search_node_list(root, name);
	}

	return NULL;
}

int search_nest_index(struct listroot *root, char *variable)
{
	char name[BUFFER_SIZE], *arg;

	arg = get_arg_to_brackets(root->ses, variable, name);

	while (root && *arg)
	{
		root = search_nest_root(root, name);

		if (root)
		{
			arg = get_arg_in_brackets(root->ses, arg, name);
		}
	}

	if (root)
	{
		return search_index_list(root, name, NULL);
	}

	return -1;
}

struct listroot *update_nest_root(struct listroot *root, char *arg)
{
	struct listnode *node;

	node = search_node_list(root, arg);

	if (node == NULL)
	{
		node = update_node_list(root, arg, "", "");
	}

	if (node->root == NULL)
	{
		node->root = init_list(root->ses, root->type, LIST_SIZE);
	}

	return node->root;
}

void update_nest_node(struct listroot *root, char *arg)
{
	char arg1[BUFFER_SIZE], arg2[BUFFER_SIZE];

	while (*arg)
	{
		arg = get_arg_in_braces(root->ses, arg, arg1, FALSE);
		arg = get_arg_in_braces(root->ses, arg, arg2, FALSE);

		if (*arg2 == DEFAULT_OPEN)
		{
			update_nest_node(update_nest_root(root, arg1), arg2);
		}
		else if (*arg1)
		{
			update_node_list(root, arg1, arg2, "");
		}

		if (*arg == COMMAND_SEPARATOR)
		{
			arg++;
		}
	}
}

int delete_nest_node(struct listroot *root, char *variable)
{
	char name[BUFFER_SIZE], *arg;
	int index;

	arg = get_arg_to_brackets(root->ses, variable, name);

	while (root && *arg)
	{
		root = search_nest_root(root, name);

		if (root)
		{
			arg = get_arg_in_brackets(root->ses, arg, name);
		}
	}

	if (root)
	{
		index = search_index_list(root, name, NULL);

		if (index != -1)
		{
			delete_index_list(root, index);

			return TRUE;
		}
	}

	return FALSE;
}

// Return the number of indices of a node.

int get_nest_size(struct listroot *root, char *variable, char *result)
{
	char name[BUFFER_SIZE], *arg;
	int index, count;

	arg = get_arg_to_brackets(root->ses, variable, name);

	*result = 0;

	if (!strcmp(arg, "[]"))
	{
		if (*name == 0)
		{
			for (index = 0 ; index < root->used ; index++)
			{
				cat_sprintf(result, "{%s}", root->list[index]->left);
			}
			return root->used + 1;
		}

		if (search_nest_root(root, name) == NULL)
		{
			if (search_node_list(root, name))
			{
				return 1;
			}
		}
	}



	while (root && *name)
	{
		// Handle regex queries

		if (search_nest_root(root, name) == NULL)
		{
			if (search_node_list(root, name) == NULL)
			{
				if (regexp_check(root->ses, name))
				{
					for (index = count = 0 ; index < root->used ; index++)
					{
						if (match(root->ses, root->list[index]->left, name, SUB_NONE))
						{
							show_nest_node(root->list[index], result, 0); // behaves like strcat
							count++;
						}
					}
					return count + 1;
				}
				else
				{
					return 0;
				}
			}
		}

		root = search_nest_root(root, name);

		if (root)
		{
			if (!strcmp(arg, "[]"))
			{
				for (index = 0 ; index < root->used ; index++)
				{
					cat_sprintf(result, "{%s}", root->list[index]->left);
				}
				return root->used + 1;
			}
			arg = get_arg_in_brackets(root->ses, arg, name);
		}
	}

	return 0;
}


struct listnode *get_nest_node(struct listroot *root, char *variable, char *result, int def)
{
	struct listnode *node;
	int size;

	size = get_nest_size(root, variable, result);

	if (size)
	{
		return NULL;
	}

	node = search_nest_node(root, variable);

	if (node)
	{
		show_nest_node(node, result, TRUE);

		return node;
	}

	node = search_base_node(root, variable);

	if (node || def)
	{
		strcpy(result, "");
	}
	else
	{
		sprintf(result, "$%s", variable);
	}
	return NULL;
}

int get_nest_index(struct listroot *root, char *variable, char *result, int def)
{
	struct listnode *node;
	int index, size;

	size = get_nest_size(root, variable, result);

	if (size)
	{
		sprintf(result, "%d", size - 1);

		return -1;
	}

	index = search_nest_index(root, variable);

	if (index >= 0)
	{
		sprintf(result, "%d", index + 1);

		return index;
	}

	node = search_base_node(root, variable);

	if (node || def)
	{
		strcpy(result, "0");
	}
	else
	{
		sprintf(result, "&%s", variable);
	}
	return -1;
}


struct listnode *set_nest_node(struct listroot *root, char *arg1, char *format, ...)
{
	struct listnode *node;
	char arg2[BUFFER_SIZE], name[BUFFER_SIZE], *arg;
	va_list args;

	va_start(args, format);
	vsprintf(arg2, format, args);
	va_end(args);

	arg = get_arg_to_brackets(root->ses, arg1, name);

	check_all_events(root->ses, SUB_ARG, 1, 0, "VARIABLE UPDATE %s", name);

	while (*arg)
	{
		root = update_nest_root(root, name);

		if (root)
		{
			arg = get_arg_in_brackets(root->ses, arg, name);
		}
	}

	node = search_node_list(root, name);

	if (node && node->root)
	{
		free_list(node->root);

		node->root = NULL;
	}

	if (*space_out(arg2) == DEFAULT_OPEN)
	{
		update_nest_node(update_nest_root(root, name), arg2);

		return search_node_list(root, name);
	}
	else
	{
		return update_node_list(root, name, arg2, "");
	}
}

// Like set, but don't erase old data.

struct listnode *add_nest_node(struct listroot *root, char *arg1, char *format, ...)
{
//	struct listnode *node;
	char arg2[BUFFER_SIZE], name[BUFFER_SIZE], *arg;
	va_list args;

	va_start(args, format);
	vsprintf(arg2, format, args);
	va_end(args);

	arg = get_arg_to_brackets(root->ses, arg1, name);

	while (*arg)
	{
		root = update_nest_root(root, name);

		if (root)
		{
			arg = get_arg_in_brackets(root->ses, arg, name);
		}
	}

/*
	Adding here, so don't clear the variable.

	node = search_node_list(root, name);

	if (node && node->root)
	{
		free_list(node->root);

		node->root = NULL;
	}
*/
	if (*space_out(arg2) == DEFAULT_OPEN)
	{
		update_nest_node(update_nest_root(root, name), arg2);

		return search_node_list(root, name);
	}
	else
	{
		return update_node_list(root, name, arg2, "");
	}
}

// cats to result when initialize is 0

void show_nest_node(struct listnode *node, char *result, int initialize)
{
	if (initialize)
	{
		*result = 0;
	}

	if (node->root == NULL)
	{
		if (initialize)
		{
			strcat(result, node->right);
		}
		else
		{
			cat_sprintf(result, "{%s}", node->right);
		}
	}
	else
	{
		struct listroot *root = node->root;
		int i;

		if (!initialize)
		{
			strcat(result, "{");
		}

		for (i = 0 ; i < root->used ; i++)
		{
			cat_sprintf(result, "{%s}", root->list[i]->left);

			show_nest_node(root->list[i], result, FALSE);
		}
		if (!initialize)
		{
			strcat(result, "}");
		}
	}
}

void copy_nest_node(struct listroot *dst_root, struct listnode *dst, struct listnode *src)
{
	int index;

	if (src->root == NULL)
	{
		return;
	}

	dst_root = dst->root = init_list(dst_root->ses, dst_root->type, src->root->size);

	for (index = 0 ; index < src->root->used ; index++)
	{
		dst = insert_node_list(dst_root, src->root->list[index]->left, src->root->list[index]->right, src->root->list[index]->pr);

		if (src->root->list[index]->root)
		{
			copy_nest_node(dst_root, dst, src->root->list[index]);
		}
	}
}
