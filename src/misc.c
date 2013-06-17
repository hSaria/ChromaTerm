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

DO_COMMAND(do_all)
{
	char left[BUFFER_SIZE];
	struct session *sesptr, *next_ses;

	if (gts->next)
	{
		get_arg_in_braces(ses, arg, left, TRUE);

		substitute(ses, left, left, SUB_VAR|SUB_FUN);

		for (sesptr = gts->next ; sesptr ; sesptr = next_ses)
		{
			next_ses = sesptr->next;

			script_driver(sesptr, -1, left);
		}
	}
	else
	{
		display_puts2(ses, "#BUT THERE AREN'T ANY SESSIONS AT ALL!");
	}
	return ses;
}

DO_COMMAND(do_commands)
{
	char buf[BUFFER_SIZE] = { 0 }, add[BUFFER_SIZE];
	int cmd;

	display_header(ses, " %s ", "COMMANDS");

	for (cmd = 0 ; *command_table[cmd].name != 0 ; cmd++)
	{
		if (*arg && !is_abbrev(arg, command_table[cmd].name))
		{
			continue;
		}
		if ((int) strlen(buf) + 20 > ses->cols)
		{
			display_puts2(ses, buf);
			buf[0] = 0;
		}
		sprintf(add, "%20s", command_table[cmd].name);
		strcat(buf, add);
	}
	if (buf[0])
	{
		display_puts2(ses, buf);
	}
	display_header(ses, "");

	return ses;
}

DO_COMMAND(do_echo)
{
	char temp[BUFFER_SIZE], output[BUFFER_SIZE];
	int lnf;

	sprintf(temp, "{result} %s", arg);

	set_nest_node(ses->list[LIST_VARIABLE], "result", "");

	do_format(ses, temp);

	substitute(ses, "$result", temp, SUB_VAR|SUB_FUN);

	arg = temp;

	lnf = !str_suffix(arg, "\\");

	substitute(ses, arg, temp, SUB_COL|SUB_ESC);

	if (strip_vt102_strlen(ses, ses->more_output) != 0)
	{
		sprintf(output, "\n\033[0m%s\033[0m", temp);
	}
	else
	{
		sprintf(output, "\033[0m%s\033[0m", temp);
	}

	if (ses != gtd->ses)
	{
		return ses;
	}

	printline(ses, output, lnf);

	return ses;
}

DO_COMMAND(do_end)
{
	if (*arg)
	{
		quitmsg(arg);
	}
	else
	{
		quitmsg(NULL);
	}
	return NULL;
}

DO_COMMAND(do_info)
{
	int cnt;

	if (*arg == 'c')
	{
		show_cpu(ses);

		return ses;
	}

	if (*arg == 's')
	{
		dump_stack();

		return ses;
	}

	display_header(ses, " INFORMATION ");

	for (cnt = 0 ; cnt < LIST_MAX ; cnt++)
	{
		if (!HAS_BIT(ses->list[cnt]->flags, LIST_FLAG_SHOW))
		{
			continue;
		}
		display_printf2(ses, "%-20s  %5d  IGNORE %3s  MESSAGE %3s  DEBUG %3s",
			list_table[cnt].name_multi,
			ses->list[cnt]->used,
			HAS_BIT(ses->list[cnt]->flags, LIST_FLAG_IGNORE) ? "ON" : "OFF",
			HAS_BIT(ses->list[cnt]->flags, LIST_FLAG_MESSAGE) ? "ON" : "OFF",
			HAS_BIT(ses->list[cnt]->flags, LIST_FLAG_DEBUG)   ? "ON" : "OFF");
	}
	display_header(ses, "");

	return ses;
}


DO_COMMAND(do_nop)
{
	return ses;
}

DO_COMMAND(do_send)
{
	char left[BUFFER_SIZE];

	push_call("do_send(%p,%p)",ses,arg);

	get_arg_in_braces(ses, arg, left, TRUE);

	write_mud(ses, left, SUB_VAR|SUB_FUN|SUB_ESC|SUB_EOL);

	pop_call();
	return ses;
}

DO_COMMAND(do_showme)
{
	char left[BUFFER_SIZE], right[BUFFER_SIZE], temp[STRING_SIZE];
	int lnf;

	arg = get_arg_in_braces(ses, arg, left, TRUE);

	lnf = !str_suffix(left, "\\");

	substitute(ses, left, temp, SUB_VAR|SUB_FUN);
	substitute(ses, temp, left, SUB_COL|SUB_ESC);

	do_one_line(left, ses);

	if (HAS_BIT(ses->flags, SES_FLAG_GAG))
	{
		DEL_BIT(ses->flags, SES_FLAG_GAG);

		return ses;
	}

	if (strip_vt102_strlen(ses, ses->more_output) != 0)
	{
		sprintf(right, "\n\033[0m%s\033[0m", left);
	}
	else
	{
		sprintf(right, "\033[0m%s\033[0m", left);
	}

	if (ses != gtd->ses)
	{
		return ses;
	}

	printline(ses, right, lnf);

	return ses;
}

DO_COMMAND(do_snoop)
{
	struct session *sesptr = ses;
	char left[BUFFER_SIZE];

	get_arg_in_braces(ses, arg, left, 1);
	substitute(ses, left, left, SUB_VAR|SUB_FUN);

	if (*left)
	{
		for (sesptr = gts->next ; sesptr ; sesptr = sesptr->next)
  		{
  			if (!strcmp(sesptr->name, left))
  			{
  				break;
  			}
  		}
		if (sesptr == NULL)
		{
			display_puts2(ses, "#NO SESSION WITH THAT NAME!");

			return ses;
		}
	}
	else
	{
		sesptr = ses;
	}

	if (HAS_BIT(sesptr->flags, SES_FLAG_SNOOP))
	{
		display_printf2(ses, "#UNSNOOPING SESSION '%s'", sesptr->name);
	}
	else
	{
		display_printf2(ses, "#SNOOPING SESSION '%s'", sesptr->name);
	}
	TOG_BIT(sesptr->flags, SES_FLAG_SNOOP);

	return ses;
}

DO_COMMAND(do_suspend)
{
	suspend_handler(0);

	return ses;
}


DO_COMMAND(do_test)
{
	return ses;
}

DO_COMMAND(do_zap)
{
	struct session *sesptr = ses;
	char left[BUFFER_SIZE];

	sub_arg_in_braces(ses, arg, left, GET_ALL, SUB_VAR|SUB_FUN);

	sesptr = ses;

	if (*left)
	{
		for (sesptr = gts->next ; sesptr ; sesptr = sesptr->next)
  		{
  			if (!strcmp(sesptr->name, left))
  			{
  				break;
  			}
  		}

		if (sesptr == NULL)
		{
			display_puts2(ses, "#NO SESSION WITH THAT NAME!");

			return ses;
		}
	}

	display_puts(sesptr, "");

	display_puts(sesptr, "#ZZZZZZZAAAAAAAAPPPP!!!!!!!!! LET'S GET OUTTA HERE!!!!!!!!");

	if (sesptr == gts)
	{
		return do_end(NULL, "");
	}
	cleanup_session(sesptr);

	return gtd->ses;
}
