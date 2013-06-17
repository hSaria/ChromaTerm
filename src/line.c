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

DO_COMMAND(do_line)
{
	char left[BUFFER_SIZE];
	int cnt;

	arg = get_arg_in_braces(ses, arg, left, FALSE);

	if (*left == 0)
	{
		display_printf(ses, "#SYNTAX: #LINE {<OPTION>} {argument}.");
	}
	else
	{
		for (cnt = 0 ; *line_table[cnt].name ; cnt++)
		{
			if (is_abbrev(left, line_table[cnt].name))
			{
				break;
			}
		}

		if (*line_table[cnt].name == 0)
		{
			do_line(ses, "");
		}
		else
		{
			ses = line_table[cnt].fun(ses, arg);
		}
	}
	return ses;
}


DO_LINE(line_gag)
{
	SET_BIT(ses->flags, SES_FLAG_GAG);

	return ses;
}

// Without an argument mark next line to be logged, otherwise log the given line to file.

DO_LINE(line_log)
{
	char left[BUFFER_SIZE], right[BUFFER_SIZE], temp[BUFFER_SIZE];
	FILE *logfile;

	arg = sub_arg_in_braces(ses, arg, left, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, temp, GET_ALL, SUB_VAR|SUB_FUN);

	if ((logfile = fopen(left, "a")))
	{
		if (HAS_BIT(ses->flags, SES_FLAG_LOGHTML))
		{
			fseek(logfile, 0, SEEK_END);

			if (ftell(logfile) == 0)
			{
				write_html_header(logfile);
			}
		}

		if (*temp)
		{
			substitute(ses, temp, right, SUB_ESC|SUB_COL|SUB_LNF);

			logit(ses, right, logfile, FALSE);

			fclose(logfile);
		}
		else
		{
			if (ses->logline)
			{
				fclose(ses->logline);
			}
			ses->logline = logfile;
		}
	}
	else
	{
		display_printf(ses, "#ERROR: #LINE LOG {%s} - COULDN'T OPEN FILE.", left);
	}
	return ses;
}


DO_LINE(line_logverbatim)
{
	char left[BUFFER_SIZE], right[BUFFER_SIZE];
	FILE *logfile;

	arg = sub_arg_in_braces(ses, arg, left,  GET_ONE, SUB_VAR|SUB_FUN);
	arg = get_arg_in_braces(ses, arg, right, 1);

	if ((logfile = fopen(left, "a")))
	{
		if (HAS_BIT(ses->flags, SES_FLAG_LOGHTML))
		{
			fseek(logfile, 0, SEEK_END);

			if (ftell(logfile) == 0)
			{
				write_html_header(logfile);
			}
		}

		logit(ses, right, logfile, TRUE);

		fclose(logfile);
	}
	else
	{
		display_printf(ses, "#ERROR: #LINE LOGVERBATIM {%s} - COULDN'T OPEN FILE.", left);
	}
	return ses;
}

DO_LINE(line_strip)
{
	char left[BUFFER_SIZE], strip[BUFFER_SIZE];

	arg = sub_arg_in_braces(ses, arg, left, GET_ALL, SUB_ESC|SUB_COL);

	if (*left == 0)
	{
		display_printf(ses, "#SYNTAX: #LINE {STRIP} {command}.");

		return ses;
	}

	strip_vt102_codes(left, strip);

	ses = script_driver(ses, -1, strip);

	return ses;
}

DO_LINE(line_substitute)
{
	char left[BUFFER_SIZE], right[BUFFER_SIZE], subs[BUFFER_SIZE];
	int i, flags = 0;

	arg = get_arg_in_braces(ses, arg, left,  0);
	arg = get_arg_in_braces(ses, arg, right, 1);

	if (*right == 0)
	{
		display_printf(ses, "#SYNTAX: #LINE {SUBSTITUTE} {argument} {command}.");

		return ses;
	}

	arg = left;

	while (*arg)
	{
		arg = get_arg_in_braces(ses, arg, subs, 0);

		for (i = 0 ; *substitution_table[i].name ; i++)
		{
			if (is_abbrev(subs, substitution_table[i].name))
			{
				SET_BIT(flags, substitution_table[i].bitvector);
			}
		}

		if (*arg == COMMAND_SEPARATOR)
		{
			arg++;
		}
	}

	substitute(ses, right, subs, flags);

	ses = script_driver(ses, -1, subs);

	return ses;
}

DO_LINE(line_verbose)
{
	struct session *sesptr;
	char left[BUFFER_SIZE];

	arg = get_arg_in_braces(ses, arg, left,  TRUE);

	if (*left == 0)
	{
		display_printf(ses, "#SYNTAX: #LINE {VERBOSE} {command}.");

		return ses;
	}

	sesptr = ses;

	SET_BIT(sesptr->flags, SES_FLAG_VERBOSELINE);

	ses = script_driver(ses, -1, left);

	DEL_BIT(sesptr->flags, SES_FLAG_VERBOSELINE);

	return ses;
}

DO_LINE(line_ignore)
{
	struct session *sesptr;
	char left[BUFFER_SIZE];

	arg = get_arg_in_braces(ses, arg, left,  TRUE);

	if (*left == 0)
	{
		display_printf(ses, "#SYNTAX: #LINE {IGNORE} {command}.");

		return ses;
	}

	sesptr = ses;

	SET_BIT(sesptr->flags, SES_FLAG_IGNORELINE);

	ses = script_driver(ses, -1, left);

	DEL_BIT(sesptr->flags, SES_FLAG_IGNORELINE);

	return ses;
}
