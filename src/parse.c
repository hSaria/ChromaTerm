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

struct session *parse_input(struct session *ses, char *input)
{
	char *line;

	push_call("parse_input(%s,%s)",ses->name,input);

	if (*input == 0)
	{
		write_mud(ses, input, SUB_EOL);

		pop_call();
		return ses;
	}

	line = (char *) malloc(BUFFER_SIZE);

	strcpy(line, input);

	if (check_all_aliases(ses, line))
	{
		ses = script_driver(ses, LIST_ALIAS, line);
	}
	else
	{
		write_mud(ses, line, SUB_EOL);
	}

	free(line);

	pop_call();
	return ses;
}

/*
	Deals with # stuff
*/

struct session *parse_command(struct session *ses, char *input)
{
	char line[BUFFER_SIZE];

	input = get_arg_stop_spaces(ses, input, line, 0);

	display_printf(ses, "#ERROR: #UNKNOWN COMMAND '%s'.", line);

	return ses;
}


/*
	get all arguments - only check for unescaped command separators
*/


char *get_arg_all(struct session *ses, char *string, char *result, int verbatim)
{
	char *pto, *pti;
	int nest = 0;

	pti = string;
	pto = result;

	while (*pti)
	{
		if (HAS_BIT(ses->flags, SES_FLAG_BIG5) && *pti & 128 && pti[1] != 0)
		{
			*pto++ = *pti++;
			*pto++ = *pti++;
			continue;
		}

		if (*pti == '\\' && pti[1] == COMMAND_SEPARATOR)
		{
			*pto++ = *pti++;
		}
		else if (*pti == COMMAND_SEPARATOR && nest == 0 && !verbatim)
		{
			break;
		}
		else if (*pti == DEFAULT_OPEN)
		{
			nest++;
		}
		else if (*pti == DEFAULT_CLOSE)
		{
			nest--;
		}
		*pto++ = *pti++;
	}
	*pto = '\0'; 

	return pti;
}

/*
	Braces are stripped in braced arguments leaving all else as is.
*/

char *get_arg_in_braces(struct session *ses, char *string, char *result, int flag)
{
	char *pti, *pto;
	int nest = 1;

	pti = space_out(string);
	pto = result;

	if (*pti != DEFAULT_OPEN)
	{
		if (!HAS_BIT(flag, GET_ALL))
		{
			pti = get_arg_stop_spaces(ses, pti, result, flag);
		}
		else
		{
			pti = get_arg_with_spaces(ses, pti, result, flag);
		}
		return pti;
	}

	pti++;

	while (*pti)
	{
		
		if (HAS_BIT(ses->flags, SES_FLAG_BIG5) && *pti & 128 && pti[1] != 0)
		{
			*pto++ = *pti++;
			*pto++ = *pti++;
			continue;
		}

		if (*pti == DEFAULT_OPEN)
		{
			nest++;
		}
		else if (*pti == DEFAULT_CLOSE)
		{
			nest--;

			if (nest == 0)
			{
				break;
			}
		}
		*pto++ = *pti++;
	}

	if (*pti == 0)
	{
		display_printf2(NULL, "#Unmatched braces error!");
	}
	else
	{
		pti++;
	}
	*pto = '\0';

	return pti;
}

char *sub_arg_in_braces(struct session *ses, char *string, char *result, int flag, int sub)
{
	char buffer[BUFFER_SIZE];

	string = get_arg_in_braces(ses, string, buffer, flag);

	substitute(ses, buffer, result, sub);

	return string;
}

/*
	get all arguments
*/

char *get_arg_with_spaces(struct session *ses, char *string, char *result, int flag)
{
	char *pto, *pti;
	int nest = 0;

	pti = space_out(string);
	pto = result;

	while (*pti)
	{
		if (HAS_BIT(ses->flags, SES_FLAG_BIG5) && *pti & 128 && pti[1] != 0)
		{
			*pto++ = *pti++;
			*pto++ = *pti++;
			continue;
		}

		if (*pti == '\\' && pti[1] == COMMAND_SEPARATOR)
		{
			*pto++ = *pti++;
		}
		else if (*pti == COMMAND_SEPARATOR && nest == 0)
		{
			break;
		}
		else if (*pti == DEFAULT_OPEN)
		{
			nest++;
		}
		else if (*pti == DEFAULT_CLOSE)
		{
			nest--;
		}
		*pto++ = *pti++;
	}
	*pto = '\0'; 

	return pti;
}

/*
	get one arg, stop at spaces
*/

char *get_arg_stop_spaces(struct session *ses, char *string, char *result, int flag)
{
	char *pto, *pti;
	int nest = 0;

	pti = space_out(string);
	pto = result;

	while (*pti)
	{
		if (HAS_BIT(ses->flags, SES_FLAG_BIG5) && *pti & 128 && pti[1] != 0)
		{
			*pto++ = *pti++;
			*pto++ = *pti++;
			continue;
		}

		if (*pti == '\\' && pti[1] == COMMAND_SEPARATOR)
		{
			*pto++ = *pti++;
		}
		else if (*pti == COMMAND_SEPARATOR && nest == 0)
		{
			break;
		}
		else if (isspace((int) *pti) && nest == 0)
		{
			pti++;
			break;
		}
		else if (*pti == DEFAULT_OPEN)
		{
			nest++;
		}
		else if (*pti == '[' && HAS_BIT(flag, GET_NST))
		{
			nest++;
		}
		else if (*pti == DEFAULT_CLOSE)
		{
			nest--;
		}
		else if (*pti == ']' && HAS_BIT(flag, GET_NST))
		{
			nest--;
		}
		*pto++ = *pti++;
	}
	*pto = '\0';

	return pti;
}

/*
	advance ptr to next none-space
*/

char *space_out(char *string)
{
	while (isspace((int) *string))
	{
		string++;
	}
	return string;
}

/*
	For list handling
*/

char *get_arg_to_brackets(struct session *ses, char *string, char *result)
{
	char *pti, *pto, *ptii, *ptoo;
	int nest1 = 0, nest2 = 0, nest3 = 0;

	pti = space_out(string);
	pto = result;
	ptii = ptoo = NULL;

	while (*pti)
	{
		if (HAS_BIT(ses->flags, SES_FLAG_BIG5) && *pti & 128 && pti[1] != 0)
		{
			*pto++ = *pti++;
			*pto++ = *pti++;
			continue;
		}

		if (*pti == '[')
		{
			nest2++;

			if (nest1 == 0 && ptii == NULL)
			{
				ptii = pti;
				ptoo = pto;
			}
		}
		else if (*pti == ']')
		{
			if (nest2)
			{
				nest2--;
			}
			else
			{
				nest3 = 1;
			}

			if (*(pti+1) == 0 && ptii && nest1 == 0 && nest2 == 0 && nest3 == 0)
			{
				*ptoo = 0;

				return ptii;
			}
		}
		else if (*pti == DEFAULT_OPEN)
		{
			nest1++;
		}
		else if (*pti == DEFAULT_CLOSE)
		{
			nest1--;
		}
		*pto++ = *pti++;
	}
	*pto = 0;

	return pti;
}

char *get_arg_at_brackets(struct session *ses, char *string, char *result)
{
	char *pti, *pto;
	int nest = 0;

	pti = string;
	pto = result;

	if (*pti != '[')
	{
		*pto = 0;

		return pti;
	}

	while (*pti)
	{
		if (HAS_BIT(ses->flags, SES_FLAG_BIG5) && *pti & 128 && pti[1] != 0)
		{
			*pto++ = *pti++;
			*pto++ = *pti++;
			continue;
		}

		if (*pti == '[')
		{
			nest++;
		}
		else if (*pti == ']')
		{
			if (nest)
			{
				nest--;
			}
			else
			{
				break;
			}
		}
		else if (nest == 0)
		{
			break;
		}
		*pto++ = *pti++;
	}

	if (nest)
	{
		display_printf2(NULL, "#UNMATCHED AT BRACKETS ERROR.");
	}
	*pto = 0;

	return pti;
}

char *get_arg_in_brackets(struct session *ses, char *string, char *result)
{
	char *pti, *pto;
	int nest = 1;

	pti = string;
	pto = result;

	if (*pti != '[')
	{
		*pto = 0;

		return pti;
	}

	pti++;

	while (*pti)
	{
		if (HAS_BIT(ses->flags, SES_FLAG_BIG5) && *pti & 128 && pti[1] != 0)
		{
			*pto++ = *pti++;
			*pto++ = *pti++;
			continue;
		}

		if (*pti == '[')
		{
			nest++;
		}
		else if (*pti == ']')
		{
			nest--;

			if (nest == 0)
			{
				break;
			}
		}
		*pto++ = *pti++;
	}

	if (*pti == 0)
	{
		display_printf2(NULL, "#UNMATCHED IN BRACKETS ERROR.");
	}
	else
	{
		pti++;
	}
	*pto = 0;

	return pti;
}

/*
	send command to the socket
*/

void write_mud(struct session *ses, char *command, int flags)
{
	char output[BUFFER_SIZE];
	int size;

	size = substitute(ses, command, output, flags);

	write_line_socket(ses, output, size);
}


/******************************************************************************
* do all of the functions to one line of buffer, VT102 codes and variables    *
* substituted beforehand                                                      *
******************************************************************************/

void do_one_line(char *line, struct session *ses)
{
	char strip[BUFFER_SIZE];

	push_call("[%s] do_one_line(%s)",ses->name,line);

	if (HAS_BIT(ses->flags, SES_FLAG_IGNORELINE))
	{
		pop_call();
		return;
	}

	strip_vt102_codes(line, strip);

	if (!HAS_BIT(ses->list[LIST_ACTION]->flags, LIST_FLAG_IGNORE))
	{
		check_all_actions(ses, line, strip);
	}

	if (!HAS_BIT(ses->list[LIST_GAG]->flags, LIST_FLAG_IGNORE))
	{
		check_all_gags(ses, line, strip);
	}

	if (!HAS_BIT(ses->list[LIST_SUBSTITUTE]->flags, LIST_FLAG_IGNORE))
	{
		check_all_substitutions(ses, line, strip);
	}

	if (!HAS_BIT(ses->list[LIST_HIGHLIGHT]->flags, LIST_FLAG_IGNORE))
	{
		check_all_highlights(ses, line, strip);
	}

	if (ses->logline)
	{
		logit(ses, line, ses->logline, TRUE);

		fclose(ses->logline);
		ses->logline = NULL;
	}
	pop_call();
	return;
}
