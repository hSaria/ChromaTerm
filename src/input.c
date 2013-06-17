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

void process_input(void)
{
	if (!HAS_BIT(gtd->ses->flags, SES_FLAG_LOCALECHO))
	{
		read_key();
	}
	else
	{
		read_line();
	}

	if (!HAS_BIT(gtd->flags, GLOBAL_FLAG_PROCESSINPUT))
	{
		return;
	}

	DEL_BIT(gtd->flags, GLOBAL_FLAG_PROCESSINPUT);

	if (HAS_BIT(gtd->ses->flags, SES_FLAG_LOCALECHO))
	{
		echo_command(gtd->ses, gtd->input_buf);
	}
	else
	{
		echo_command(gtd->ses, "");
	}

	check_all_events(gtd->ses, SUB_ARG|SUB_SEC, 0, 1, "RECEIVED INPUT", gtd->input_buf);

	gtd->ses = script_driver(gtd->ses, -1, gtd->input_buf);

	gtd->input_buf[0] = 0;
}

void read_line()
{
	char buffer[STRING_SIZE];
	struct listnode *node;
	struct listroot *root;
	int len, cnt, match;

	gtd->input_buf[gtd->input_len] = 0;

	len = read(0, buffer, 1);

	buffer[len] = 0;

	root = gtd->ses->list[LIST_MACRO];

	if (HAS_BIT(gtd->ses->flags, SES_FLAG_CONVERTMETA) || HAS_BIT(gtd->flags, GLOBAL_FLAG_CONVERTMETACHAR))
	{
		convert_meta(buffer, &gtd->macro_buf[strlen(gtd->macro_buf)]);
	}
	else
	{
		strcat(gtd->macro_buf, buffer);
	}

	if (!HAS_BIT(gtd->ses->flags, SES_FLAG_CONVERTMETA))
	{
		match = 0;

		if (!HAS_BIT(root->flags, LIST_FLAG_IGNORE))
		{
			for (root->update = 0 ; root->update < root->used ; root->update++)
			{
				node = root->list[root->update];

				if (!strcmp(gtd->macro_buf, node->pr))
				{
					script_driver(gtd->ses, LIST_MACRO, node->right);

					gtd->macro_buf[0] = 0;

					return;
				}
				else if (!strncmp(gtd->macro_buf, node->pr, strlen(gtd->macro_buf)))
				{
					match = 1;
				}
			}
		}

		for (cnt = 0 ; *cursor_table[cnt].fun != NULL ; cnt++)
		{
			if (!strcmp(gtd->macro_buf, cursor_table[cnt].code))
			{
				cursor_table[cnt].fun("");
				gtd->macro_buf[0] = 0;

				return;
			}
			else if (!strncmp(gtd->macro_buf, cursor_table[cnt].code, strlen(gtd->macro_buf)))
			{
				match = 1;
			}
		}

		if (match)
		{
			return;
		}
	}

	if (gtd->macro_buf[0] == ESCAPE)
	{
		strcpy(buffer, gtd->macro_buf);

		convert_meta(buffer, gtd->macro_buf);
	}

	for (cnt = 0 ; gtd->macro_buf[cnt] ; cnt++)
	{
		switch (gtd->macro_buf[cnt])
		{
			case 10:
				cursor_enter("");
				break;

			default:
				if (HAS_BIT(gtd->flags, GLOBAL_FLAG_INSERTINPUT) && gtd->input_len != gtd->input_cur)
				{
					if (!HAS_BIT(gtd->ses->flags, SES_FLAG_UTF8) || (gtd->macro_buf[cnt] & 192) != 128)
					{
						cursor_delete("");
					}
				}

				ins_sprintf(&gtd->input_buf[gtd->input_cur], "%c", gtd->macro_buf[cnt]);

				gtd->input_len++;
				gtd->input_cur++;

				if (!HAS_BIT(gtd->ses->flags, SES_FLAG_UTF8) || (gtd->macro_buf[cnt] & 192) != 128)
				{
					gtd->input_pos++;
				}

				if (gtd->input_len != gtd->input_cur)
				{
					if (HAS_BIT(gtd->ses->flags, SES_FLAG_UTF8) && (gtd->macro_buf[cnt] & 192) == 128)
					{
						input_printf("%c", gtd->macro_buf[cnt]);
					}
					else
					{
						input_printf("\033[1@%c", gtd->macro_buf[cnt]);
					}
				}
				else
				{
					input_printf("%c", gtd->macro_buf[cnt]);
				}

				gtd->macro_buf[0] = 0;
				gtd->input_tmp[0] = 0;
				gtd->input_buf[gtd->input_len] = 0;

				cursor_check_line_modified("");

				break;
		}
	}
}

void read_key(void)
{
	char buffer[BUFFER_SIZE];
	struct listnode *node;
	struct listroot *root;
	int len, cnt, match;

	if (!HAS_BIT(gtd->ses->flags, SES_FLAG_VERBATIM) && gtd->input_buf[0] == gtd->command_char)
	{
		read_line();

		return;
	}

	len = read(0, buffer, 1);

	buffer[len] = 0;

	root = gtd->ses->list[LIST_MACRO];

	if (HAS_BIT(gtd->ses->flags, SES_FLAG_CONVERTMETA) || HAS_BIT(gtd->flags, GLOBAL_FLAG_CONVERTMETACHAR))
	{
		convert_meta(buffer, &gtd->macro_buf[strlen(gtd->macro_buf)]);
	}
	else
	{
		strcat(gtd->macro_buf, buffer);
	}

	if (!HAS_BIT(gtd->ses->flags, SES_FLAG_CONVERTMETA) && !HAS_BIT(gtd->ses->flags, SES_FLAG_VERBATIM))
	{
		match = 0;

		if (!HAS_BIT(root->flags, LIST_FLAG_IGNORE))
		{
			for (root->update = 0 ; root->update < root->used ; root->update++)
			{
				node = root->list[root->update];

				if (!strcmp(gtd->macro_buf, node->pr))
				{
					script_driver(gtd->ses, LIST_MACRO, node->right);

					gtd->macro_buf[0] = 0;
					return;
				}
				else if (!strncmp(gtd->macro_buf, node->pr, strlen(gtd->macro_buf)))
				{
					match = 1;
				}
			}

			if (match)
			{
				return;
			}
		}
	}

	for (cnt = 0 ; gtd->macro_buf[cnt] ; cnt++)
	{
		switch (gtd->macro_buf[cnt])
		{
			case '\n':
				gtd->input_buf[0] = 0;
				gtd->macro_buf[0] = 0;
				gtd->input_len = 0;

				if (HAS_BIT(gtd->ses->flags, SES_FLAG_RUN))
				{
					socket_printf(gtd->ses, 1, "%c", '\r');
				}
				else
				{
					socket_printf(gtd->ses, 2, "%c%c", '\r', '\n');
				}
				break;

			default:
				if (!HAS_BIT(gtd->ses->flags, SES_FLAG_VERBATIM) && gtd->macro_buf[cnt] == gtd->command_char && gtd->input_buf[0] == 0)
				{
					if (gtd->input_len != gtd->input_cur)
					{
						printf("\033[1@%c", gtd->macro_buf[cnt]);
					}
					else
					{
						printf("%c", gtd->macro_buf[cnt]);
					}
					gtd->input_buf[0] = gtd->command_char;
					gtd->input_buf[1] = 0;
					gtd->macro_buf[0] = 0;
					gtd->input_len = 1;
					gtd->input_cur = 1;
					gtd->input_pos = 1;
				}
				else
				{
					socket_printf(gtd->ses, 1, "%c", gtd->macro_buf[cnt]);
					gtd->input_buf[0] = 127;
					gtd->macro_buf[0] = 0;
					gtd->input_len = 0;
				}
				break;
		}
	}
}

void convert_meta(char *input, char *output)
{
	char *pti, *pto;

	DEL_BIT(gtd->flags, GLOBAL_FLAG_CONVERTMETACHAR);

	pti = input;
	pto = output;

	while (*pti)
	{
		switch (*pti)
		{
			case ESCAPE:
				*pto++ = '\\';
				*pto++ = 'e';
				pti++;
				break;

			case 127:
				*pto++ = '\\';
				*pto++ = 'b';
				pti++;
				break;

			case '\a':
				*pto++ = '\\';
				*pto++ = 'a';
				pti++;
				break;

			case '\b':
				*pto++ = '\\';
				*pto++ = 'b';
				pti++;
				break;

			case '\t':
				*pto++ = '\\';
				*pto++ = 't';
				pti++;
				break;

			case '\r':
				*pto++ = '\\';
				*pto++ = 'r';
				pti++;
				break;

			case '\n':
				*pto++ = *pti++;
				break;

			default:
				if (*pti > 0 && *pti < 32)
				{
					*pto++ = '\\';
					*pto++ = 'c';
					if (*pti <= 26)
					{
						*pto++ = 'a' + *pti - 1;
					}
					else
					{
						*pto++ = 'A' + *pti - 1;
					}
					pti++;
					break;
				}
				else
				{
					*pto++ = *pti++;
				}
				break;
		}
	}
	*pto = 0;
}

void unconvert_meta(char *input, char *output)
{
	char *pti, *pto;

	pti = input;
	pto = output;

	while (*pti)
	{
		switch (pti[0])
		{
			case '\\':
				switch (pti[1])
				{
					case 'C':
						if (pti[2] == '-' && pti[3])
						{
							*pto++  = pti[3] - 'a' + 1;
							pti    += 4;
						}
						else
						{
							*pto++ = *pti++;
						}
						break;

					case 'c':
						*pto++ = pti[2] % 32;
						pti += 3;
						break;

					case 'a':
						*pto++  = '\a';
						pti += 2;
						break;

					case 'b':
						*pto++  = 127;
						pti    += 2;
						break;

					case 'e':
						*pto++  = ESCAPE;
						pti    += 2;
						break;

					case 't':
						*pto++  = '\t';
						pti    += 2;
						break;

					case 'x':
						if (pti[2] && pti[3])
						{
							*pto++ = hex_number(&pti[2]);
							pti += 4;
						}
						else
						{
							*pto++ = *pti++;
						}
						break;
					default:
						*pto++ = *pti++;
						break;
				}
				break;

			default:
				*pto++ = *pti++;
				break;
		}
	}
	*pto = 0;
}

/*
	Currenly only used in split mode.
*/

void echo_command(struct session *ses, char *line)
{
	char buffer[STRING_SIZE], result[STRING_SIZE];

	sprintf(buffer, "%s", line);

	/*
		Deal with pending output
	*/

	if (ses->more_output[0])
	{
		if (ses->check_output)
		{
			strcpy(result, ses->more_output);
			ses->more_output[0] = 0;

			process_mud_output(ses, result, FALSE);
		}
	}
}

void input_printf(char *format, ...)
{
	char buf[STRING_SIZE];
	va_list args;

	if (!HAS_BIT(gtd->ses->flags, SES_FLAG_LOCALECHO) && gtd->input_buf[0] != gtd->command_char)
	{
		return;
	}

	va_start(args, format);
	vsprintf(buf, format, args);
	va_end(args);

	printf("%s", buf);
}

void modified_input(void)
{
	return;
}
