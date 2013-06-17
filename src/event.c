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
******************************************************************************/

/******************************************************************************
*                  C H R O M A T E R M (C) 2013 (See CREDITS)                 *
******************************************************************************/

#include "defs.h"


DO_COMMAND(do_event)
{
	char arg1[BUFFER_SIZE], arg2[BUFFER_SIZE];
	int cnt;

	arg = sub_arg_in_braces(ses, arg, arg1, 0, SUB_VAR|SUB_FUN);
	arg = get_arg_in_braces(ses, arg, arg2, 1);

	if (*arg1 == 0)
	{
		display_header(ses, " EVENTS ");

		for (cnt = 0 ; *event_table[cnt].name != 0 ; cnt++)
		{
			display_printf2(ses, "  [%-20s] [%s] %s", event_table[cnt].name, search_node_list(ses->list[LIST_EVENT], event_table[cnt].name) ? "X" : " ", event_table[cnt].desc);
		}
		display_header(ses, "");
	}
	else if (*arg2 == 0)
	{
		if (show_node_with_wild(ses, arg1, LIST_EVENT) == FALSE)
		{
			show_message(ses, LIST_ALIAS, "#EVENT: NO MATCH(ES) FOUND FOR {%s}.", arg1);
		}
	}
	else
	{
		for (cnt = 0 ; *event_table[cnt].name != 0 ; cnt++)
		{
			if (!strncmp(event_table[cnt].name, arg1, strlen(event_table[cnt].name)))
			{
				show_message(ses, LIST_EVENT, "#EVENT {%s} HAS BEEN SET TO {%s}.", arg1, arg2);

				update_node_list(ses->list[LIST_EVENT], arg1, arg2, "");

				return ses;
			}
		}
		display_printf(ses, "#EVENT {%s} IS NOT AN EXISTING EVENT.", capitalize(arg1));
	}
	return ses;
}

DO_COMMAND(do_unevent)
{
	delete_node_with_wild(ses, LIST_EVENT, arg);

	return ses;
}

int check_all_events(struct session *ses, int flags, int args, int vars, char *fmt, ...)
{
	struct session *ses_ptr;
	struct listnode *node;
	char buf[BUFFER_SIZE];
	va_list list;
	int cnt;

	push_call("check_all_events(%p,%d,%d,%d, ...)",ses,flags,args,vars);

	va_start(list, fmt);

	vsprintf(buf, fmt, list);

	va_end(list); 

	for (ses_ptr = ses ? ses : gts ; ses_ptr ; ses_ptr = ses_ptr->next)
	{
		if (!HAS_BIT(ses_ptr->list[LIST_EVENT]->flags, LIST_FLAG_IGNORE))
		{
			node = search_node_list(ses_ptr->list[LIST_EVENT], buf);

			if (node)
			{
				va_start(list, fmt);

				for (cnt = 0 ; cnt < args ; cnt++)
				{
					va_arg(list, char *);
				}

				for (cnt = 0 ; cnt < vars ; cnt++)
				{
					RESTRING(gtd->vars[cnt], va_arg(list, char *));
				}

				substitute(ses_ptr, node->right, buf, flags);

				if (HAS_BIT(ses_ptr->list[LIST_EVENT]->flags, LIST_FLAG_DEBUG))
				{
					show_debug(ses_ptr, LIST_ACTION, "#DEBUG EVENT {%s}", node->left);
				}

				script_driver(ses_ptr, LIST_EVENT, buf);

				va_end(list);

				if (ses)
				{
					pop_call();
					return 1;
				}
			}
		}

		if (ses)
		{
			pop_call();
			return 0;
		}
	}
	pop_call();
	return 0;
}
