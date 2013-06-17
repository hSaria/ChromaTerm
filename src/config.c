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

DO_COMMAND(do_configure)
{
	char left[BUFFER_SIZE], right[BUFFER_SIZE];
	struct listnode *node;
	int index;

	arg = get_arg_in_braces(ses, arg, left,  FALSE);
	arg = sub_arg_in_braces(ses, arg, right, GET_ONE, SUB_VAR|SUB_FUN);

	if (*left == 0)
	{
		display_header(ses, " CONFIGURATIONS ");

		for (index = 0 ; *config_table[index].name != 0 ; index++)
		{
			node = search_node_list(ses->list[LIST_CONFIG], config_table[index].name);

			if (node)
			{
				display_printf2(ses, "[%-13s] [%8s] %s", 
					node->left,
					node->right,
					strcmp(node->right, "ON") == 0 ? config_table[index].msg_on : config_table[index].msg_off);
			}
		}

		display_header(ses, "");
	}
	else
	{
		for (index = 0 ; *config_table[index].name != 0 ; index++)
		{
			if (is_abbrev(left, config_table[index].name))
			{
				if (config_table[index].config(ses, right, index) != NULL)
				{
					node = search_node_list(ses->list[LIST_CONFIG], config_table[index].name);

					if (node)
					{
						show_message(ses, LIST_CONFIG, "#CONFIG {%s} HAS BEEN SET TO {%s}.", config_table[index].name, node->right);
					}
				}
				return ses;
			}
		}
		display_printf(ses, "#ERROR: #CONFIG {%s} IS NOT A VALID OPTION.", capitalize(left));
	}
	return ses;
}

DO_CONFIG(config_verbatim)
{
	if (!strcasecmp(arg, "ON"))
	{
		SET_BIT(ses->flags, SES_FLAG_VERBATIM);
	}
	else if (!strcasecmp(arg, "OFF"))
	{
		DEL_BIT(ses->flags, SES_FLAG_VERBATIM);
	}
	else
	{
		display_printf(ses, "#SYNTAX: #CONFIG {%s} <ON|OFF>", config_table[index].name);

		return NULL;
	}
	update_node_list(ses->list[LIST_CONFIG], config_table[index].name, capitalize(arg), "");

	return ses;
}

DO_CONFIG(config_verbose)
{
	if (!strcasecmp(arg, "ON"))
	{
		SET_BIT(ses->flags, SES_FLAG_VERBOSE);
	}
	else if (!strcasecmp(arg, "OFF"))
	{
		DEL_BIT(ses->flags, SES_FLAG_VERBOSE);
	}
	else
	{
		display_printf(ses, "#SYNTAX: #CONFIG {%s} <ON|OFF>", config_table[index].name);

		return NULL;
	}
	update_node_list(ses->list[LIST_CONFIG], config_table[index].name, capitalize(arg), "");

	return ses;
}

DO_CONFIG(config_log)
{
	if (!strcasecmp(arg, "HTML"))
	{
		DEL_BIT(ses->flags, SES_FLAG_LOGPLAIN);
		SET_BIT(ses->flags, SES_FLAG_LOGHTML);
	}
	else if (!strcasecmp(arg, "PLAIN"))
	{
		SET_BIT(ses->flags, SES_FLAG_LOGPLAIN);
		DEL_BIT(ses->flags, SES_FLAG_LOGHTML);
	}
	else if (!strcasecmp(arg, "RAW"))
	{
		DEL_BIT(ses->flags, SES_FLAG_LOGPLAIN);
		DEL_BIT(ses->flags, SES_FLAG_LOGHTML);
	}
	else
	{
		display_printf(ses, "#SYNTAX: #CONFIG LOG <HTML|PLAIN|RAW>");

		return NULL;
	}
	update_node_list(ses->list[LIST_CONFIG], config_table[index].name, capitalize(arg), "");

	return ses;
}

DO_CONFIG(config_packetpatch)
{
	if (!is_number(arg))
	{
		display_printf(ses, "#SYNTAX: #CONFIG {PACKET PATCH} <NUMBER>");

		return NULL;
	}

	if (atof(arg) < 0 || atof(arg) > 10)
	{
		display_printf(ses, "#ERROR: #CONFIG PACKET PATCH: PROVIDE A NUMBER BETWEEN 0.00 and 10.00");

		return NULL;
	}

	gts->check_output = (long long) (get_number(ses, arg) * 1000000LL);

	update_node_list(ses->list[LIST_CONFIG], config_table[index].name, capitalize(arg), "");

	return ses;
}

DO_CONFIG(config_commandchar)
{
	gtd->command_char = arg[0];

	update_node_list(ses->list[LIST_CONFIG], config_table[index].name, arg, "");

	return ses;
}

DO_CONFIG(config_convertmeta)
{
	if (!strcasecmp(arg, "ON"))
	{
		SET_BIT(ses->flags, SES_FLAG_CONVERTMETA);
	}
	else if (!strcasecmp(arg, "OFF"))
	{
		DEL_BIT(ses->flags, SES_FLAG_CONVERTMETA);
	}
	else
	{
		display_printf(ses, "#SYNTAX: #CONFIG {%s} <ON|OFF>", config_table[index].name);

		return NULL;
	}
	update_node_list(ses->list[LIST_CONFIG], config_table[index].name, capitalize(arg), "");

	return ses;
}

DO_CONFIG(config_loglevel)
{
	if (!strcasecmp(arg, "LOW"))
	{
		SET_BIT(ses->flags, SES_FLAG_LOGLEVEL);
	}
	else if (!strcasecmp(arg, "HIGH"))
	{
		DEL_BIT(ses->flags, SES_FLAG_LOGLEVEL);
	}
	else
	{
		display_printf(ses, "#SYNTAX: #CONFIG {%s} <LOW|HIGH>", config_table[index].name);

		return NULL;
	}
	update_node_list(ses->list[LIST_CONFIG], config_table[index].name, capitalize(arg), "");

	return ses;
}

DO_CONFIG(config_colorpatch)
{
	if (!strcasecmp(arg, "ON"))
	{
		SET_BIT(ses->flags, SES_FLAG_COLORPATCH);
	}
	else if (!strcasecmp(arg, "OFF"))
	{
		DEL_BIT(ses->flags, SES_FLAG_COLORPATCH);
	}
	else
	{
		display_printf(ses, "#SYNTAX: #CONFIG {%s} <ON|OFF>", config_table[index].name);

		return NULL;
	}
	update_node_list(ses->list[LIST_CONFIG], config_table[index].name, capitalize(arg), "");

	return ses;
}

DO_CONFIG(config_charset)
{
	if (!strcasecmp(arg, "BIG5"))
	{
		SET_BIT(ses->flags, SES_FLAG_BIG5);
		DEL_BIT(ses->flags, SES_FLAG_UTF8);
	}
	else if (!strcasecmp(arg, "UTF-8"))
	{
		SET_BIT(ses->flags, SES_FLAG_UTF8);
		DEL_BIT(ses->flags, SES_FLAG_BIG5);
	}
	else if (!strcasecmp(arg, "ASCII"))
	{
		DEL_BIT(ses->flags, SES_FLAG_BIG5);
		DEL_BIT(ses->flags, SES_FLAG_UTF8);
	}
	else
	{
		display_printf(ses, "#SYNTAX: #CONFIG {%s} <ASCII|BIG5|UTF-8>", config_table[index].name);

		return NULL;
	}
	update_node_list(ses->list[LIST_CONFIG], config_table[index].name, capitalize(arg), "");

	return ses;
}

DO_CONFIG(config_256color)
{
	if (!strcasecmp(arg, "AUTO"))
	{
		if (!strcasecmp(gtd->term, "xterm") || strstr(gtd->term, "256color") || strstr(gtd->term, "256COLOR"))
		{
			SET_BIT(ses->flags, SES_FLAG_256COLOR);
		}
		else
		{
			DEL_BIT(ses->flags, SES_FLAG_256COLOR);
		}
	}
	else if (!strcasecmp(arg, "ON"))
	{
		SET_BIT(ses->flags, SES_FLAG_256COLOR);
	}
	else if (!strcasecmp(arg, "OFF"))
	{
		DEL_BIT(ses->flags, SES_FLAG_256COLOR);
	}
	else
	{
		display_printf(ses, "#SYNTAX: #CONFIG {%s} <AUTO|ON|OFF>", config_table[index].name);

		return NULL;
	}
	update_node_list(ses->list[LIST_CONFIG], config_table[index].name, HAS_BIT(ses->flags, SES_FLAG_256COLOR) ? "ON" : "OFF", "");

	return ses;
}

DO_CONFIG(config_localecho)
{
	if (!strcasecmp(arg, "ON"))
	{
		SET_BIT(ses->flags, SES_FLAG_LOCALECHO);
	}
	else if (!strcasecmp(arg, "OFF"))
	{
		DEL_BIT(ses->flags, SES_FLAG_LOCALECHO);
	}
	else
	{
		display_printf(ses, "#SYNTAX: #CONFIG {%s} <ON|OFF>", config_table[index].name);

		return NULL;
	}
	update_node_list(ses->list[LIST_CONFIG], config_table[index].name, capitalize(arg), "");

	return ses;
}
