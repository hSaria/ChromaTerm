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

#include <errno.h>
#include <signal.h>

DO_COMMAND(do_session)
{
	char left[BUFFER_SIZE], right[BUFFER_SIZE];
	struct session *sesptr;

	arg = sub_arg_in_braces(ses, arg, left,  GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, right, GET_ALL, SUB_VAR|SUB_FUN);

	if (*left == 0)
	{
		display_puts(ses, "#THESE SESSIONS HAVE BEEN DEFINED:");

		for (sesptr = gts->next ; sesptr ; sesptr = sesptr->next)
		{
			show_session(ses, sesptr);
		}
		return ses;
	}

	for (sesptr = gts ; sesptr ; sesptr = sesptr->next)
	{
		if (!strcmp(sesptr->name, left))
		{
			break;
		}
	}

	if (sesptr == NULL)
	{
		display_puts(ses, "#THAT SESSION IS NOT DEFINED.");

		return ses;
	}

	if (*left && *right == 0)
	{
		return activate_session(sesptr);
	}

	script_driver(sesptr, -1, right);

	return ses;
}


void show_session(struct session *ses, struct session *ptr)
{
	char temp[BUFFER_SIZE];

	sprintf(temp, "%3d %-12s %20s:%-5d", ptr->socket, ptr->name, ptr->command, ptr->pid);

	if (ptr == gtd->ses)
	{
		strcat(temp, " (active)");
	}
	else
	{
		strcat(temp, "         ");
	}

	if (HAS_BIT(ptr->flags, SES_FLAG_SNOOP))
	{
		strcat(temp, " (snooped)");
	}

	if (ptr->logfile)
	{
		strcat(temp, " (logging)");
	}

	display_puts2(ses, temp);
}

struct session *newactive_session(void)
{
	push_call("newactive_session(void)");

	if (gts->next)
	{
		activate_session(gts->next);
	}
	else
	{
		activate_session(gts);
	}
	pop_call();
	return gtd->ses;
}

struct session *activate_session(struct session *ses)
{
	check_all_events(gtd->ses, SUB_ARG|SUB_SEC, 0, 1, "SESSION DEACTIVATED", gtd->ses->name);

	gtd->ses = ses;

	display_printf(ses, "#SESSION '%s' ACTIVATED.", ses->name);

	check_all_events(ses, SUB_ARG|SUB_SEC, 0, 1, "SESSION ACTIVATED", ses->name);

	return ses;
}

struct session *new_session(struct session *ses, char *name, char *command, int pid, int socket)
{
	int cnt = 0;
	struct session *newsession;

	push_call("new_session(%p,%p,%p,%d,%d)",ses,name,command,pid,socket);

	for (newsession = gts ; newsession ; newsession = newsession->next)
	{
		if (!strcmp(newsession->name, name))
		{
			display_puts(ses, "THERE'S A SESSION WITH THAT NAME ALREADY.");

			if (close(socket) == -1)
			{
				syserr("close in new_session");
			}

			kill(pid, SIGKILL);

			pop_call();
			return ses;
		}
	}

	newsession                = (struct session *) calloc(1, sizeof(struct session));

	newsession->name          = strdup(name);
	newsession->command       = strdup(command);
	newsession->pid           = pid;

	newsession->group         = strdup(gts->group);
	newsession->flags         = gts->flags;

	LINK(newsession, gts->next, gts->prev);

	for (cnt = 0 ; cnt < LIST_MAX ; cnt++)
	{
		newsession->list[cnt] = copy_list(newsession, gts->list[cnt], cnt);
	}

	newsession->rows          = gts->rows;
	newsession->cols          = gts->cols;

	display_printf(ses, "#TRYING TO LAUNCH '%s' RUNNING '%s'.", newsession->name, newsession->command);

	gtd->ses = newsession;

	SET_BIT(newsession->flags, SES_FLAG_CONNECTED|SES_FLAG_RUN);

	DEL_BIT(newsession->flags, SES_FLAG_LOCALECHO);

	gtd->ses = newsession;

	gtd->ses->socket = socket;

	check_all_events(ses, SUB_ARG|SUB_SEC, 0, 3, "SESSION CONNECTED", ses->name, ses->command, ntos(ses->pid));

	pop_call();
	return gtd->ses;
}

void cleanup_session(struct session *ses)
{
	push_call("cleanup_session(%p)",ses);

	if (ses == gtd->update)
	{
		gtd->update = ses->next;
	}

	UNLINK(ses, gts->next, gts->prev);

	if (ses->socket)
	{
		if (close(ses->socket) == -1)
		{
			syserr("close in cleanup");
		}

		if (HAS_BIT(ses->flags, SES_FLAG_RUN))
		{
			kill(ses->pid, SIGKILL);
		}

		DEL_BIT(ses->flags, SES_FLAG_CONNECTED);
	}

	check_all_events(ses, SUB_ARG|SUB_SEC, 0, 3, "SESSION DISCONNECTED", ses->name, ses->command, ntos(ses->pid));

	display_printf(gtd->ses, "#SESSION '%s' DIED.", ses->name);

	if (ses == gtd->ses)
	{
		gtd->ses = newactive_session();
	}

	if (ses->logfile)
	{
		fclose(ses->logfile);
	}

	if (ses->logline)
	{
		fclose(ses->logline);
	}

	LINK(ses, gtd->dispose_next, gtd->dispose_prev);

	pop_call();
	return;
}

void dispose_session(struct session *ses)
{
	int index;

	push_call("dispose_session(%p)", ses);

	UNLINK(ses, gtd->dispose_next, gtd->dispose_prev);

	for (index = 0 ; index < LIST_MAX ; index++)
	{
		free_list(ses->list[index]);
	}

	free(ses->name);
	free(ses->command);
	free(ses->group);

	free(ses);

	pop_call();
	return;
}
