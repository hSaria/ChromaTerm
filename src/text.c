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


void printline(struct session *ses, char *str, int prompt)
{
	char wrapped_str[STRING_SIZE];

	push_call("printline(%p,%p,%d)",ses,str,prompt);

	if (HAS_BIT(ses->flags, SES_FLAG_CONVERTMETA))
	{
		convert_meta(str, wrapped_str);
	}
	else
	{
		strcpy(wrapped_str, str);
	}

	if (!HAS_BIT(ses->flags, SES_FLAG_LOGLEVEL))
	{
		if (ses->logfile)
		{
			logit(ses, wrapped_str, ses->logfile, !prompt);
		}
	}

	if (prompt)
	{
		printf("%s", wrapped_str);
	}
	else
	{
		printf("%s\n", wrapped_str);
	}
	pop_call();
	return;
}

