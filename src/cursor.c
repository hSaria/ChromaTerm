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

DO_COMMAND(do_cursor)
{
	char left[BUFFER_SIZE], right[BUFFER_SIZE], temp[BUFFER_SIZE];
	int cnt;

	arg = get_arg_in_braces(ses, arg, left,  TRUE);
	arg = get_arg_in_braces(ses, arg, right, TRUE);
	substitute(ses, right, right, SUB_VAR|SUB_FUN);

	if (*left == 0)
	{
		display_header(ses, " CURSOR OPTIONS ");

		for (cnt = 0 ; *cursor_table[cnt].fun ; cnt++)
		{
			if (*cursor_table[cnt].name)
			{
				convert_meta(cursor_table[cnt].code, temp);

				display_printf2(ses, "  [%-18s] [%-6s] %s", 
					cursor_table[cnt].name,
					temp,
					cursor_table[cnt].desc);
			}
		}

		display_header(ses, "");
	}
	else
	{
		for (cnt = 0 ; *cursor_table[cnt].fun ; cnt++)
		{
			if (is_abbrev(left, cursor_table[cnt].name))
			{
				cursor_table[cnt].fun(right);

				return ses;
			}
		}
		display_printf(ses, "#ERROR: #CURSOR {%s} IS NOT A VALID OPTION.", capitalize(left));
	}
	return ses;
}

// Multi-Byte support routines

int inputline_str_str_len(int start, int end)
{
	int raw_cnt, str_cnt, ret_cnt;

	raw_cnt = str_cnt = ret_cnt = 0;

	while (raw_cnt < gtd->input_len)
	{
		if (str_cnt >= end)
		{
			break;
		}

		if (HAS_BIT(gtd->ses->flags, SES_FLAG_UTF8) && (gtd->input_buf[raw_cnt] & 192) == 192)
		{
			raw_cnt++;

			while (raw_cnt < gtd->input_len && (gtd->input_buf[raw_cnt] & 192) == 128)
			{
				raw_cnt++;
			}
		}
		else
		{
			raw_cnt++;
		}
		str_cnt++;

		if (str_cnt > start)
		{
			ret_cnt++;
		}
	}
	return ret_cnt;
}

int inputline_raw_str_len(int start, int end)
{
	int raw_cnt, ret_cnt;

	raw_cnt = start;
	ret_cnt = 0;

	while (raw_cnt < gtd->input_len)
	{
		if (raw_cnt >= end)
		{
			break;
		}

		if (HAS_BIT(gtd->ses->flags, SES_FLAG_UTF8) && (gtd->input_buf[raw_cnt] & 192) == 192)
		{
			raw_cnt++;

			while (raw_cnt < gtd->input_len && (gtd->input_buf[raw_cnt] & 192) == 128)
			{
				raw_cnt++;
			}
		}
		else
		{
			raw_cnt++;
		}
		ret_cnt++;
	}
	return ret_cnt;
}

int inputline_str_raw_len(int start, int end)
{
	int raw_cnt, str_cnt, ret_cnt;

	raw_cnt = str_cnt = ret_cnt = 0;

	while (raw_cnt < gtd->input_len)
	{
		if (str_cnt >= end)
		{
			break;
		}

		if (HAS_BIT(gtd->ses->flags, SES_FLAG_UTF8) && (gtd->input_buf[raw_cnt] & 192) == 192)
		{
			ret_cnt += (str_cnt >= start) ? 1 : 0;
			raw_cnt++;

			while (raw_cnt < gtd->input_len && (gtd->input_buf[raw_cnt] & 192) == 128)
			{
				ret_cnt += (str_cnt >= start) ? 1 : 0;
				raw_cnt++;
			}
		}
		else
		{
			ret_cnt += (str_cnt >= start) ? 1 : 0;
			raw_cnt++;
		}
		str_cnt++;
	}
	return ret_cnt;
}

int inputline_raw_raw_len(int start, int end)
{
	int raw_cnt, ret_cnt;

	raw_cnt = ret_cnt = 0;

	while (raw_cnt < gtd->input_len)
	{
		if (raw_cnt >= end)
		{
			break;
		}

		if (HAS_BIT(gtd->ses->flags, SES_FLAG_UTF8) && (gtd->input_buf[raw_cnt] & 192) == 192)
		{
			ret_cnt += (raw_cnt >= start) ? 1 : 0;
			raw_cnt++;

			while (raw_cnt < gtd->input_len && (gtd->input_buf[raw_cnt] & 192) == 128)
			{
				ret_cnt += (raw_cnt >= start) ? 1 : 0;
				raw_cnt++;
			}
		}
		else
		{
			ret_cnt += (raw_cnt >= start) ? 1 : 0;
			raw_cnt++;
		}
	}
	return ret_cnt;
}

// Get string length of the input area

int inputline_max_str_len(void)
{
	return gtd->ses->cols + 1 - gtd->input_off;
}

int inputline_cur_str_len(void)
{
	return inputline_str_str_len(gtd->input_hid, gtd->input_hid + inputline_max_str_len());
}

// Check for invalid characters.

int inputline_str_chk(int offset, int totlen)
{
	while (offset < totlen)
	{
		if (HAS_BIT(gtd->ses->flags, SES_FLAG_BIG5))
		{
			if (gtd->input_buf[offset] & 128)
			{
				if (offset + 1 >= totlen || gtd->input_buf[offset+1] == 0)
				{
					return FALSE;
				}
				offset += 2;
			}
			else
			{
				offset += 1;
			}
		}
		else if (HAS_BIT(gtd->ses->flags, SES_FLAG_UTF8))
		{
			switch (gtd->input_buf[offset] & (128+64+32+16))
			{
				case 128+64:
				case 128+64+16:
					if (offset + 1 >= totlen || (gtd->input_buf[offset+1] & 192) != 128)
					{
						return FALSE;
					}
					offset += 2;
					break;

				case 128+64+32:
					if (offset + 2 >= totlen || (gtd->input_buf[offset+1] & 192) != 128 || (gtd->input_buf[offset+2] & 192) != 128)
					{
						return FALSE;
					}
					offset += 3;
					break;

				case 128+64+32+16:
					if (offset + 3 >= totlen || (gtd->input_buf[offset+1] & 192) != 128 || (gtd->input_buf[offset+2] & 192) != 128 || (gtd->input_buf[offset+3] & 192) != 128)
					{
						return FALSE;
					}
					offset += 4;
					break;

				default:
					offset += 1;
					break;
			}
		}
	}
	return TRUE;
}

DO_CURSOR(cursor_backspace)
{
	if (gtd->input_cur == 0)
	{
		return;
	}

	cursor_left("");
	cursor_delete("");

	modified_input();
}

DO_CURSOR(cursor_check_line)
{
	if (gtd->input_pos - gtd->input_hid > inputline_max_str_len() - 3)
	{
		return cursor_redraw_line("");
	}

	if (gtd->input_hid && gtd->input_pos - gtd->input_hid < 2)
	{
		return cursor_redraw_line("");
	}
}

DO_CURSOR(cursor_check_line_modified)
{
	if (gtd->input_hid + inputline_max_str_len() < inputline_raw_str_len(0, gtd->input_len))
	{
		return cursor_redraw_line("");
	}

	return cursor_check_line("");
}

DO_CURSOR(cursor_clear_left)
{
	if (gtd->input_cur == 0)
	{
		return;
	}

	sprintf(gtd->paste_buf, "%.*s", gtd->input_cur, gtd->input_buf);

	input_printf("\033[%dG\033[%dP", gtd->input_off, gtd->input_pos - gtd->input_hid);

	memmove(&gtd->input_buf[0], &gtd->input_buf[gtd->input_cur], gtd->input_len - gtd->input_cur);

	gtd->input_len -= gtd->input_cur;

	gtd->input_buf[gtd->input_len] = 0;

	gtd->input_cur  = 0;
	gtd->input_pos  = 0;

	cursor_check_line_modified("");

	modified_input();
}

DO_CURSOR(cursor_clear_line)
{
	if (gtd->input_len == 0)
	{
		return;
	}

	input_printf("\033[%dG\033[%dP", gtd->input_off, inputline_cur_str_len());

	gtd->input_len = 0;
	gtd->input_cur = 0;
	gtd->input_hid = 0;
	gtd->input_pos = 0;
	gtd->input_buf[0] = 0;

	modified_input();
}

DO_CURSOR(cursor_clear_right)
{
	if (gtd->input_cur == gtd->input_len)
	{
		return;
	}

	strcpy(gtd->paste_buf, &gtd->input_buf[gtd->input_cur]);

	input_printf("\033[%dP", inputline_max_str_len() - inputline_str_str_len(gtd->input_hid, gtd->input_pos));

	gtd->input_buf[gtd->input_cur] = 0;

	gtd->input_len = gtd->input_cur;

	modified_input();
}

DO_CURSOR(cursor_convert_meta)
{
	SET_BIT(gtd->flags, GLOBAL_FLAG_CONVERTMETACHAR);
}

DO_CURSOR(cursor_delete_or_exit)
{
	if (gtd->input_len == 0)
	{
		cursor_exit("");
	}
	else
	{
		cursor_delete("");
	}
}

DO_CURSOR(cursor_delete)
{
	if (gtd->input_len == 0)
	{
		return;
	}

	if (gtd->input_len == gtd->input_cur)
	{
		return;
	}

	if (HAS_BIT(gtd->ses->flags, SES_FLAG_BIG5) && (gtd->input_buf[gtd->input_cur] & 128) == 128)
	{
		if (gtd->input_cur + 1 < gtd->input_len && gtd->input_buf[gtd->input_cur+1])
		{
			memmove(&gtd->input_buf[gtd->input_cur+1], &gtd->input_buf[gtd->input_cur+2], gtd->input_len - gtd->input_cur);

			gtd->input_len--;

			input_printf("\033[1P");
		}
	}
	else if (HAS_BIT(gtd->ses->flags, SES_FLAG_UTF8) && (gtd->input_buf[gtd->input_cur] & 192) == 192)
	{
		while (gtd->input_cur + 1 < gtd->input_len && (gtd->input_buf[gtd->input_cur+1] & 192) == 128)
		{
			memmove(&gtd->input_buf[gtd->input_cur+1], &gtd->input_buf[gtd->input_cur+2], gtd->input_len - gtd->input_cur);

			gtd->input_len--;
		}
	}

	memmove(&gtd->input_buf[gtd->input_cur], &gtd->input_buf[gtd->input_cur+1], gtd->input_len - gtd->input_cur);

	gtd->input_len--;

	if (gtd->input_hid + inputline_max_str_len() <= inputline_raw_str_len(0, gtd->input_len))
	{
		cursor_redraw_line("");
	}
	else
	{
		input_printf("\033[1P");
	}

	modified_input();
}

DO_CURSOR(cursor_delete_word_left)
{
	int index_cur;

	if (gtd->input_cur == 0)
	{
		return;
	}

	index_cur = gtd->input_cur;

	while (gtd->input_cur > 0 && gtd->input_buf[gtd->input_cur - 1] == ' ')
	{
		gtd->input_pos--;
		gtd->input_cur--;
	}

	while (gtd->input_cur > 0 && gtd->input_buf[gtd->input_cur - 1] != ' ')
	{
		if (!HAS_BIT(gtd->ses->flags, SES_FLAG_UTF8) || (gtd->input_buf[gtd->input_cur] & 192) != 128)
		{
			gtd->input_pos--;
		}

		gtd->input_cur--;
	}

	sprintf(gtd->paste_buf, "%.*s", index_cur - gtd->input_cur, &gtd->input_buf[gtd->input_cur]);

	memmove(&gtd->input_buf[gtd->input_cur], &gtd->input_buf[index_cur], gtd->input_len - index_cur + 1);

//	input_printf("\033[%dD\033[%dP", index_cur - gtd->input_cur, index_cur - gtd->input_cur);

	gtd->input_len -= index_cur - gtd->input_cur;

	cursor_redraw_line("");

	modified_input();
}


DO_CURSOR(cursor_delete_word_right)
{
	int index_cur;

	if (gtd->input_cur == gtd->input_len)
	{
		return;
	}

	index_cur = gtd->input_cur;

	while (gtd->input_cur != gtd->input_len && gtd->input_buf[gtd->input_cur] == ' ')
	{
		gtd->input_cur++;
	}

	while (gtd->input_cur != gtd->input_len && gtd->input_buf[gtd->input_cur] != ' ')
	{
		gtd->input_cur++;
	}

	sprintf(gtd->paste_buf, "%.*s", gtd->input_cur - index_cur, &gtd->input_buf[gtd->input_cur]);

	memmove(&gtd->input_buf[index_cur], &gtd->input_buf[gtd->input_cur], gtd->input_len - gtd->input_cur + 1);

//	input_printf("\033[%dP", gtd->input_cur - index_cur);

	gtd->input_len -= gtd->input_cur - index_cur;

	gtd->input_cur = index_cur;

	cursor_redraw_line("");
	
	modified_input();
}

DO_CURSOR(cursor_end)
{
	gtd->input_cur = gtd->input_len;

	gtd->input_pos = inputline_raw_str_len(0, gtd->input_len);

	cursor_redraw_line("");
}

DO_CURSOR(cursor_enter)
{
	input_printf("\n");

	gtd->input_buf[gtd->input_len] = 0;

	gtd->input_len    = 0;
	gtd->input_cur    = 0;
	gtd->input_pos    = 0;
	gtd->input_off    = 1;
	gtd->macro_buf[0] = 0;
	gtd->input_tmp[0] = 0;

	SET_BIT(gtd->flags, GLOBAL_FLAG_PROCESSINPUT);

	modified_input();
}

DO_CURSOR(cursor_exit)
{
	if (gtd->ses == gts)
	{
		do_end(NULL, "");
	}
	else
	{
		cleanup_session(gtd->ses);
	}
}

DO_CURSOR(cursor_home)
{
	if (gtd->input_cur == 0)
	{
		return;
	}

	input_printf("\033[%dD", gtd->input_pos - gtd->input_hid);

	gtd->input_cur = 0;
	gtd->input_pos = 0;

	if (gtd->input_hid)
	{
		gtd->input_hid = 0;

		cursor_redraw_line("");
	}
}

DO_CURSOR(cursor_insert)
{
	if (*arg == 0)
	{
		TOG_BIT(gtd->flags, GLOBAL_FLAG_INSERTINPUT);
	}
	else if (!strcasecmp(arg, "ON"))
	{
		SET_BIT(gtd->flags, GLOBAL_FLAG_INSERTINPUT);
	}
	else if (!strcasecmp(arg, "OFF"))
	{
		DEL_BIT(gtd->flags, GLOBAL_FLAG_INSERTINPUT);
	}
	else
	{
		display_printf(NULL, "#SYNTAX: #CURSOR {INSERT} {ON|OFF}.");
	}
}


DO_CURSOR(cursor_left)
{
	if (gtd->input_cur > 0)
	{
		gtd->input_cur--;
		gtd->input_pos--;

		if (HAS_BIT(gtd->ses->flags, SES_FLAG_BIG5) && inputline_str_chk(0, gtd->input_cur) == FALSE)
		{
			gtd->input_cur--;
			gtd->input_pos--;

			input_printf("\033[1D");
		}
		else if (HAS_BIT(gtd->ses->flags, SES_FLAG_UTF8))
		{
			while (gtd->input_cur > 0 && (gtd->input_buf[gtd->input_cur] & 192) == 128)
			{
				gtd->input_cur--;
			}
		}

		input_printf("\033[1D");

		cursor_check_line("");
	}
}

DO_CURSOR(cursor_left_word)
{
//	int index_pos;

	if (gtd->input_cur == 0)
	{
		return;
	}

//	index_pos = gtd->input_pos;

	while (gtd->input_cur > 0 && gtd->input_buf[gtd->input_cur - 1] == ' ')
	{
		gtd->input_pos--;
		gtd->input_cur--;
	}

	while (gtd->input_cur > 0 && gtd->input_buf[gtd->input_cur - 1] != ' ')
	{
		if (!HAS_BIT(gtd->ses->flags, SES_FLAG_UTF8) || (gtd->input_buf[gtd->input_cur] & 192) != 128)
		{
			gtd->input_pos--;
		}
		gtd->input_cur--;
	}

//	input_printf("\033[%dD", index_pos - gtd->input_pos);

	cursor_redraw_line("");
}


DO_CURSOR(cursor_paste_buffer)
{
	if (*gtd->paste_buf == 0)
	{
		return;
	}

	ins_sprintf(&gtd->input_buf[gtd->input_cur], "%s", gtd->paste_buf);

	gtd->input_len += strlen(gtd->paste_buf);
	gtd->input_cur += strlen(gtd->paste_buf);

	gtd->input_pos += inputline_raw_str_len(gtd->input_cur - strlen(gtd->paste_buf), gtd->input_cur);

	cursor_redraw_line("");

	modified_input();
}


DO_CURSOR(cursor_redraw_input)
{
	input_printf("\033[1G\033[0K%s%s\033[0K", gtd->ses->more_output, gtd->input_buf);

	gtd->input_cur = gtd->input_len;

	gtd->input_pos = gtd->input_len % gtd->ses->cols;
}


DO_CURSOR(cursor_redraw_line)
{
	if (HAS_BIT(gtd->ses->flags, SES_FLAG_UTF8|SES_FLAG_BIG5))
	{
		if (inputline_str_chk(0, gtd->input_len) == FALSE)
		{
			return;
		}
	}

	// Erase current input

	input_printf("\033[%dG\033[%dP", gtd->input_off, inputline_max_str_len());

	// Center long lines of input

	if (gtd->input_pos > inputline_max_str_len() - 3)
	{
		while (gtd->input_pos - gtd->input_hid > inputline_max_str_len() - 3)
		{
			gtd->input_hid += inputline_max_str_len() / 2;
		}

		while (gtd->input_pos - gtd->input_hid < 2)
		{
			gtd->input_hid -= inputline_max_str_len() / 2;
		}
	}
	else
	{
		if (gtd->input_hid && gtd->input_pos - gtd->input_hid < 2)
		{
			gtd->input_hid = 0;
		}
	}

	// Print the entire thing

	if (gtd->input_hid)
	{
		if (gtd->input_hid + inputline_max_str_len() >= inputline_raw_str_len(0, gtd->input_len))
		{
			input_printf("<%.*s\033[%dG",  inputline_str_raw_len(gtd->input_hid + 1, gtd->input_hid + inputline_max_str_len() - 0), &gtd->input_buf[inputline_str_raw_len(0, gtd->input_hid + 1)], gtd->input_off + gtd->input_pos - gtd->input_hid);
		}
		else
		{
			input_printf("<%.*s>\033[%dG", inputline_str_raw_len(gtd->input_hid + 1, gtd->input_hid + inputline_max_str_len() - 1), &gtd->input_buf[inputline_str_raw_len(0, gtd->input_hid + 1)], gtd->input_off + gtd->input_pos - gtd->input_hid);
		}
	}
	else
	{
		if (gtd->input_hid + inputline_max_str_len() >= inputline_raw_str_len(0, gtd->input_len))
		{
			input_printf("%.*s\033[%dG",   inputline_str_raw_len(gtd->input_hid + 0, gtd->input_hid + inputline_max_str_len() - 0), &gtd->input_buf[inputline_str_raw_len(0, gtd->input_hid + 0)], gtd->input_off + gtd->input_pos - gtd->input_hid);
		}
		else
		{
			input_printf("%.*s>\033[%dG",  inputline_str_raw_len(gtd->input_hid + 0, gtd->input_hid + inputline_max_str_len() - 1), &gtd->input_buf[inputline_str_raw_len(0, gtd->input_hid + 0)], gtd->input_off + gtd->input_pos - gtd->input_hid);
		}
	}
}

DO_CURSOR(cursor_right)
{
	if (gtd->input_cur < gtd->input_len)
	{
		gtd->input_cur++;
		gtd->input_pos++;

		if (HAS_BIT(gtd->ses->flags, SES_FLAG_BIG5))
		{
			if (gtd->input_cur < gtd->input_len && inputline_str_chk(0, gtd->input_cur) == FALSE)
			{
				gtd->input_cur++;
				gtd->input_pos++;

				input_printf("\033[1C");
			}
		}
		else if (HAS_BIT(gtd->ses->flags, SES_FLAG_UTF8))
		{
			while (gtd->input_cur < gtd->input_len && (gtd->input_buf[gtd->input_cur] & 192) == 128)
			{
				gtd->input_cur++;
			}
		}

		input_printf("\033[1C");
	}

	cursor_check_line("");
}

DO_CURSOR(cursor_right_word)
{
	if (gtd->input_cur == gtd->input_len)
	{
		return;
	}

	while (gtd->input_cur < gtd->input_len && gtd->input_buf[gtd->input_cur] == ' ')
	{
		gtd->input_cur++;
		gtd->input_pos++;
	}

	while (gtd->input_cur < gtd->input_len && gtd->input_buf[gtd->input_cur] != ' ')
	{
		if (!HAS_BIT(gtd->ses->flags, SES_FLAG_UTF8) || (gtd->input_buf[gtd->input_cur] & 192) != 128)
		{
			gtd->input_pos++;
		}
		gtd->input_cur++;
	}

	cursor_redraw_line("");
}

DO_CURSOR(cursor_suspend)
{
	do_suspend(gtd->ses, "");
}

DO_CURSOR(cursor_test)
{
	display_printf2(gtd->ses, "bar_len %d off %d pos %d hid %d cur %d len %d max %d tot %d", inputline_max_str_len(), gtd->input_off, gtd->input_pos, gtd->input_hid, gtd->input_cur, gtd->input_len, inputline_max_str_len(), inputline_cur_str_len());
}

