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

#define MAX_STACK_SIZE     51
#define MAX_DEBUG_SIZE     400

char debug_stack[MAX_STACK_SIZE][MAX_DEBUG_SIZE];

short debug_index;

int push_call(char *f, ...)
{
	va_list ap;

	if (debug_index < MAX_STACK_SIZE)
	{
		va_start(ap, f);

		vsnprintf(debug_stack[debug_index], MAX_DEBUG_SIZE - 1, f, ap);

		va_end(ap);
	}

	if (++debug_index == 10000)
	{
		dump_stack();

		return 1;
	}

	return 0;
}

void pop_call(void)
{
	if (debug_index > 0)
	{
		debug_index--;
	}
	else
	{
		display_printf2(gtd->ses, "pop_call: index is zero.");
		dump_full_stack();
	}
}

void dump_stack(void)
{
	unsigned char i;

	for (i = 0 ; i < debug_index && i < MAX_STACK_SIZE ; i++)
	{
		printf("\033[1;32mDEBUG_STACK[\033[1;31m%03d\033[1;32m] = \033[1;31m%s\n", i, debug_stack[i]);
	}
}

void dump_full_stack(void)
{
	unsigned char i;

	display_header(gtd->ses, " FULL DEBUG STACK ");

	for (i = 0 ; i < MAX_STACK_SIZE ; i++)
	{
		if (*debug_stack[i])
		{
			display_printf2(gtd->ses, "\033[1;31mDEBUG_STACK[%03d] = %s", i, debug_stack[i]);
		}
	}
	display_header(gtd->ses, "");
}
