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

void write_line_socket(struct session *ses, char *line, int size)
{
	static int retry;

	push_call("write_line_socket(%p,%p,%d)",line,ses,size);

	if (!HAS_BIT(ses->flags, SES_FLAG_CONNECTED))
	{
		display_printf2(ses, "#THIS SESSION IS NOT CONNECTED. USE: %cRUN {name} {command} TO CONNECT.", gtd->command_char);

		pop_call();
		return;
	}

	if (write(ses->socket, line, size) == -1)
	{
		if (retry++ < 10)
		{
			usleep(100000);

			write_line_socket(ses, line, size);

			pop_call();
			return;
		}
		perror("write in write_line_socket");
		cleanup_session(ses);

		pop_call();
		return;
	}

	retry = 0;

	check_all_events(ses, SUB_ARG|SUB_SEC, 0, 1, "SEND OUTPUT", line);

	pop_call();
	return;
}


int read_buffer_mud(struct session *ses)
{
	push_call("read_buffer_mud(%p)",ses);

	gtd->mud_output_len += read(ses->socket, &gtd->mud_output_buf[gtd->mud_output_len], gtd->mud_output_max - gtd->mud_output_len - 1);

	if (gtd->mud_output_len <= 0)
	{
		pop_call();
		return FALSE;
	}

	gtd->mud_output_buf[gtd->mud_output_len] = 0;

	if (HAS_BIT(ses->flags, SES_FLAG_LOGLEVEL) && ses->logfile)
	{
		fwrite(gtd->mud_output_buf, 1, gtd->mud_output_len, ses->logfile);

		fflush(ses->logfile);
	}

	pop_call();
	return TRUE;
}


void readmud(struct session *ses)
{
	char *line, *next_line;
	char linebuf[STRING_SIZE];

	push_call("readmud(%p)", ses);

	if (gtd->mud_output_len < BUFFER_SIZE)
	{
		check_all_events(ses, SUB_ARG|SUB_SEC, 0, 1, "RECEIVED OUTPUT", gtd->mud_output_buf);
	}

	gtd->mud_output_len = 0;

	/* separate into lines and print away */

	SET_BIT(gtd->ses->flags, SES_FLAG_READMUD);

	for (line = gtd->mud_output_buf ; line && *line ; line = next_line)
	{
		next_line = strchr(line, '\n');

		if (next_line)
		{
			*next_line = 0;
			next_line++;
		}
		else if (*line == 0)
		{
			break;
		}

		if (next_line == NULL && strlen(ses->more_output) < BUFFER_SIZE / 2)
		{
			if (gts->check_output)
			{
				strcat(ses->more_output, line);
				ses->check_output = utime() + gts->check_output;
				break;
			}
		}

		if (ses->more_output[0])
		{
			if (ses->check_output)
			{
				sprintf(linebuf, "%s%s", ses->more_output, line);

				ses->more_output[0] = 0;
			}
			else
			{
				strcpy(linebuf, line);
			}
		}
		else
		{
			strcpy(linebuf, line);
		}

		process_mud_output(ses, linebuf, next_line == NULL);
	}
	DEL_BIT(gtd->ses->flags, SES_FLAG_READMUD);

	pop_call();
	return;
}


void process_mud_output(struct session *ses, char *linebuf, int prompt)
{
	char line[STRING_SIZE];

	ses->check_output = 0;

	strip_vt102_codes(linebuf, line);

	check_all_events(ses, SUB_ARG|SUB_SEC, 0, 2, "RECEIVED LINE", linebuf, line);

	if (prompt)
	{
		check_all_events(ses, SUB_ARG|SUB_SEC, 0, 2, "RECEIVED PROMPT", linebuf, line);
	}

	if (HAS_BIT(ses->flags, SES_FLAG_COLORPATCH))
	{
		sprintf(line, "%s%s", ses->color, linebuf);

		get_color_codes(ses->color, linebuf, ses->color);

		linebuf = line;
	}

	do_one_line(linebuf, ses);   /* changes linebuf */

	/*
		Take care of gags, vt102 support still goes
	*/

	if (HAS_BIT(ses->flags, SES_FLAG_GAG))
	{
		strip_non_vt102_codes(linebuf, ses->more_output);

		printf("%s", ses->more_output);

		ses->more_output[0] = 0;

		DEL_BIT(ses->flags, SES_FLAG_GAG);

		return;
	}

	if (ses == gtd->ses)
	{
		printline(ses, linebuf, prompt);
	}
	else if (HAS_BIT(ses->flags, SES_FLAG_SNOOP))
	{
		strip_vt102_codes_non_graph(linebuf, linebuf);

		display_printf2(gtd->ses, "[%s] %s", ses->name, linebuf);
	}
}

