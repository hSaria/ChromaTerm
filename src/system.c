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

#ifdef HAVE_PTY_H
#include <pty.h>
#else
#ifdef HAVE_UTIL_H
#include <util.h>
#endif
#endif

DO_COMMAND(do_run)
{
	char left[BUFFER_SIZE], right[BUFFER_SIZE], temp[BUFFER_SIZE];
	int desc, pid;
	struct winsize size;

	char *argv[4] = {"sh", "-c", "", NULL};

	arg = sub_arg_in_braces(ses, arg, left,  GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, right, GET_ALL, SUB_VAR|SUB_FUN);

	if (*left == 0)
	{
		display_printf2(ses, "#RUN: PROVIDE A SESSION NAME.");

		return ses;
	}

	if (*right == 0)
	{
		strcpy(right, getenv("SHELL") ? getenv("SHELL") : "");
	}

	size.ws_row = get_scroll_size(ses);
	size.ws_col = ses->cols;

	pid = forkpty(&desc, NULL, &gtd->old_terminal, &size);

	switch (pid)
	{
		case -1:
			perror("forkpty");
			break;

		case 0:
			sprintf(temp, "exec %s", right);
			argv[2] = temp;
			execv("/bin/sh", argv);
			break;

		default:
			ses = new_session(ses, left, right, pid, desc);
			break;
	}
	return gtd->ses;
}

DO_COMMAND(do_script)
{
	char arg1[BUFFER_SIZE], arg2[BUFFER_SIZE], *cptr, buf[BUFFER_SIZE], var[BUFFER_SIZE], tmp[BUFFER_SIZE];
	FILE *script;
	int index;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg2, GET_ALL, SUB_VAR|SUB_FUN);

	if (*arg1 == 0)
	{
		show_message(ses, LIST_MESSAGE, "#SCRIPT: ONE ARGUMENT REQUIRED.");
	}
	else if (*arg2 == 0)
	{
		script = popen(arg1, "r");

		while (fgets(buf, BUFFER_SIZE - 1, script))
		{
			cptr = strchr(buf, '\n');

			if (cptr)
			{
				*cptr = 0;
			}

			ses = script_driver(ses, -1, buf);
		}

		pclose(script);
	}
	else
	{
		index = 1;

		script = popen(arg2, "r");

		var[0] = 0;

		while (fgets(buf, BUFFER_SIZE - 1, script))
		{
			cptr = strchr(buf, '\n');

			if (cptr)
			{
				*cptr = 0;
			}

			substitute(ses, buf, tmp, SUB_SEC);

			cat_sprintf(var, "{%d}{%s}", index++, tmp);
		}


		set_nest_node(ses->list[LIST_VARIABLE], arg1, "%s", var);

		pclose(script);
	}
	refresh_terminal();

	return ses;
}

DO_COMMAND(do_system)
{
	char left[BUFFER_SIZE];

	get_arg_in_braces(ses, arg, left, TRUE);
	substitute(ses, left, left, SUB_VAR|SUB_FUN);

	if (*left == 0)
	{
		display_printf(ses, "#SYNTAX: #SYSTEM {COMMAND}.");
		return ses;
	}

	show_message(ses, LIST_MESSAGE, "#OK: EXECUTING '%s'", left);

	fflush(stdout);

	system(left);

	fflush(stdout);

	refresh_terminal();

	return ses;
}
