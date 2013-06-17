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


DO_COMMAND(do_alias)
{
	char arg1[BUFFER_SIZE], arg2[BUFFER_SIZE], arg3[BUFFER_SIZE];

	arg = sub_arg_in_braces(ses, arg, arg1, 0, SUB_VAR|SUB_FUN);
	arg = get_arg_in_braces(ses, arg, arg2, 1);
	arg = get_arg_in_braces(ses, arg, arg3, 1);

	if (*arg3 == 0)
	{
		strcpy(arg3, "5");
	}

	if (*arg1 == 0)
	{
		show_list(ses->list[LIST_ALIAS], 0);
	}
	else if (*arg2 == 0)
	{
		if (show_node_with_wild(ses, arg1, LIST_ALIAS) == FALSE)
		{
			show_message(ses, LIST_ALIAS, "#ALIAS: NO MATCH(ES) FOUND FOR {%s}.", arg1);
		}
	}
	else
	{
		update_node_list(ses->list[LIST_ALIAS], arg1, arg2, arg3);

		show_message(ses, LIST_ALIAS, "#OK. {%s} NOW ALIASES {%s} @ {%s}.", arg1, arg2, arg3);
	}
	return ses;
}


DO_COMMAND(do_unalias)
{
	delete_node_with_wild(ses, LIST_ALIAS, arg);

	return ses;
}

int check_all_aliases(struct session *ses, char *input)
{
	struct listnode *node;
	struct listroot *root;
	char line[BUFFER_SIZE], tmp[BUFFER_SIZE], *arg;
	int i;

	root = ses->list[LIST_ALIAS];

	if (HAS_BIT(root->flags, LIST_FLAG_IGNORE))
	{
		return FALSE;
	}

	for (i = 1 ; i < 100 ; i++)
	{
		if (*gtd->vars[i])
		{
			RESTRING(gtd->vars[i], "");
		}
	}

	substitute(ses, input, line, SUB_VAR|SUB_FUN);

	for (root->update = 0 ; root->update < root->used ; root->update++)
	{
		if (check_one_regexp(ses, root->list[root->update], line, line, PCRE_ANCHORED))
		{
			node = root->list[root->update];

			i = strlen(node->left);

			if (!strncmp(node->left, line, i))
			{
				if (line[i] && line[i] != ' ')
				{
					continue;
				}
				
				arg = get_arg_in_braces(ses, line, tmp, FALSE);

				RESTRING(gtd->vars[0], arg)

				for (i = 1 ; i < 100 && *arg ; i++)
				{
					arg = get_arg_in_braces(ses, arg, tmp, FALSE);

					RESTRING(gtd->vars[i], tmp);
				}
			}

			substitute(ses, node->right, tmp, SUB_ARG);

			if (!strncmp(node->left, line, strlen(node->left)) && !strcmp(node->right, tmp) && *gtd->vars[0])
			{
				sprintf(input, "%s %s", tmp, gtd->vars[0]);
			}
			else
			{
				sprintf(input, "%s", tmp);
			}

			show_debug(ses, LIST_ALIAS, "#DEBUG ALIAS {%s} {%s}", node->left, gtd->vars[0]);

			return TRUE;
		}
	}
	return FALSE;
}
