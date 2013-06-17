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

DO_COMMAND(do_variable)
{
	char arg1[BUFFER_SIZE], arg2[BUFFER_SIZE], *str;

	struct listroot *root = ses->list[LIST_VARIABLE];

	arg = sub_arg_in_braces(ses, arg, arg1, GET_NST, SUB_VAR|SUB_FUN);

	str = arg;

	arg = sub_arg_in_braces(ses, arg, arg2, GET_ALL, SUB_VAR|SUB_FUN);

	if (*arg1 == 0)
	{
		show_list(root, 0);
	}
	else if (*arg1 && *str == 0)
	{
		struct listnode *node = search_nest_node(root, arg1);

		if (node)
		{
			show_node(root, node, 0);
		}
		else if (show_node_with_wild(ses, arg1, LIST_VARIABLE) == FALSE)
		{
			show_message(ses, LIST_VARIABLE, "#VARIABLE: NO MATCH(ES) FOUND FOR {%s}.", arg1);
		}
	}
	else
	{
		struct listnode *node = set_nest_node(root, arg1, "%s", arg2);

		while (*arg)
		{
			arg = sub_arg_in_braces(ses, arg, arg2, GET_ALL, SUB_VAR|SUB_FUN);

			if (*arg2)
			{
				add_nest_node(root, arg1, "%s", arg2);
			}
		}
		show_nest_node(node, arg2, 1);

		show_message(ses, LIST_VARIABLE, "#OK. VARIABLE {%s} HAS BEEN SET TO {%s}.", arg1, arg2);
	}
	return ses;
}

DO_COMMAND(do_unvariable)
{
	char arg1[BUFFER_SIZE], arg2[BUFFER_SIZE];

	sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	sub_arg_in_braces(ses, arg, arg2, GET_ALL, SUB_VAR|SUB_FUN);

	if (delete_nest_node(ses->list[LIST_VARIABLE], arg2))
	{
		show_message(ses, LIST_VARIABLE, "#OK. {%s} IS NO LONGER A VARIABLE.", arg2);
	}
	else
	{
		delete_node_with_wild(ses, LIST_VARIABLE, arg1);
	}
	return ses;
}


DO_COMMAND(do_replace)
{
	char arg1[BUFFER_SIZE], arg2[BUFFER_SIZE], arg3[BUFFER_SIZE], buf[BUFFER_SIZE], tmp[BUFFER_SIZE], *pti, *ptm;
	struct listroot *root;
	struct listnode *node;

	root = ses->list[LIST_VARIABLE];

	arg = get_arg_in_braces(ses, arg, arg1, GET_NST);
	arg = sub_arg_in_braces(ses, arg, arg2, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg3, GET_ALL, SUB_VAR|SUB_FUN);

	if (*arg1 == 0 || *arg2 == 0)
	{
		show_message(ses, LIST_VARIABLE, "#Syntax: #replace <var> <old text> <new text>", ses);
	}
	else if ((node = search_nest_node(root, arg1)) == NULL)
	{
		show_message(ses, LIST_VARIABLE, "#REPLACE: VARIABLE {%s} NOT FOUND.", arg1);
	}
	else if (regexp(ses, NULL, node->right, arg2, 0, SUB_CMD) == FALSE)
	{
		show_message(ses, LIST_VARIABLE, "#REPLACE: {%s} NOT FOUND IN {%s}.", arg2, node->right);
	}
	else
	{
		pti = node->right;
		*buf = 0;

		do
		{
			if (*gtd->cmds[0] == 0) // Set by regexp
			{
				break;
			}

			ptm = strstr(pti, gtd->cmds[0]);

			if (ptm == NULL)
			{
				break;
			}

			*ptm = 0;

			substitute(ses, arg3, tmp, SUB_CMD);

			cat_sprintf(buf, "%s%s", pti, tmp);

			pti = ptm + strlen(gtd->cmds[0]);
		}
		while (regexp(ses, NULL, pti, arg2, 0, SUB_CMD));

		strcat(buf, pti);

		set_nest_node(ses->list[LIST_VARIABLE], arg1, "%s", buf);
	}
	return ses;
}

/*
	support routines for #format - Igor
*/

void charactertonumber(struct session *ses, char *str)
{
	unsigned int result;

	if (HAS_BIT(ses->flags, SES_FLAG_BIG5) && str[0] & 128)
	{
		result = (unsigned char) str[0] + (unsigned char) str[1] * 256;
	}
	else if (HAS_BIT(ses->flags, SES_FLAG_UTF8) && (str[0] & 192) == 192)
	{
		result = (unsigned char) str[0];

		if ((str[1] & 192) == 128)
		{
			result += (unsigned char) str[1] * 256;
		}
		if ((str[2] & 192) == 128)
		{
			result += (unsigned char) str[2] * 256 * 256;
		}
		if ((str[3] & 192) == 128)
		{
			result += (unsigned char) str[3] * 256 * 256 * 256;
		}
	}
	else
	{
		result = (unsigned char) str[0];
	}
	sprintf(str, "%d", result);
}

void colorstring(struct session *ses, char *str)
{
	char result[BUFFER_SIZE] = { 0 };

	get_highlight_codes(ses, str, result);

	strcpy(str, result);

	strcpy(str, result);
}

void headerstring(char *str)
{
	char buf[BUFFER_SIZE];

	if ((int) strlen(str) > gtd->ses->cols - 2)
	{
		str[gtd->ses->cols - 2] = 0;
	}

	memset(buf, '#', gtd->ses->cols);

	memcpy(&buf[(gtd->ses->cols - strlen(str)) / 2], str, strlen(str));

	buf[gtd->ses->cols] = 0;

	strcpy(str, buf);
}

void lowerstring(char *str)
{
	char *pts;

	for (pts = str ; *pts ; pts++)
	{
		*pts = tolower((int) *pts);
	}
}

void upperstring(char *str)
{
	char *pts;

	for (pts = str ; *pts ; pts++)
	{
		*pts = toupper((int) *pts);
	}
}

void reversestring(char *str)
{
	char t;
	int a = 0, z = strlen(str) - 1;

	while (z > a)
	{
		t = str[z];
		str[z--] = str[a];
		str[a++] = t;
	}
}

void mathstring(struct session *ses, char *str)
{
	get_number_string(ses, str, str);
}

void thousandgroupingstring(struct session *ses, char *str)
{
	char result[BUFFER_SIZE], strold[BUFFER_SIZE];
	int cnt1, cnt2, cnt3, cnt4;

	get_number_string(ses, str, strold);

	cnt1 = strlen(strold);
	cnt2 = BUFFER_SIZE / 2;
	cnt4 = strchr(strold, '.') ? 0 : 1;

	result[cnt2+1] = 0;

	for (cnt3 = 0 ; cnt1 >= 0 ; cnt1--, cnt2--)
	{
		if (cnt3++ % 3 == 0 && cnt3 != 1 && cnt4 == 0 && isdigit((int) strold[cnt1]))
		{
			result[cnt2--] = ',';
		}

		result[cnt2] = strold[cnt1];

		if (!isdigit((int) result[cnt2]))
		{
			cnt4 = 1;
			cnt3 = 0;
			continue;
		}
	}

	strcpy(str, result + cnt2 + 1);
}


void stripspaces(char *str)
{
	int cnt;

	for (cnt = strlen(str) - 1 ; cnt >= 0 ; cnt--)
	{
		if (!isspace((int) str[cnt]))
		{
			break;
		}
		str[cnt] = 0;
	}

	for (cnt = 0 ; str[cnt] != 0 ; cnt++)
	{
		if (!isspace((int) str[cnt]))
		{
			break;
		}
	}
	strcpy(str, &str[cnt]);
}

void wrapstring(struct session *ses, char *str)
{
	char *pti, *lis, *sos, *soc, buf[BUFFER_SIZE], color[BUFFER_SIZE], tmp[BUFFER_SIZE];
	int col = 1, cnt = 1;

	push_call("wrapstring(%p,%p)",ses,str);

	pti = lis = sos = soc = str;

	buf[0] = color[0] = 0;

	while (*pti != 0)
	{
		if (skip_vt102_codes(pti))
		{
			pti += skip_vt102_codes(pti);

			continue;
		}

		if (*pti == ' ')
		{
			lis = pti;
		}

		if (col > ses->cols)
		{
			col = 1;

			if (pti - lis > 15)
			{
				sprintf(tmp, "%.*s", (int) (sos - soc), soc);

				get_color_codes(color, tmp, color);

				cat_sprintf(buf, "{%d}{%s%.*s}", cnt++, color, pti - sos, sos);

				soc = sos;
				lis = sos = pti;
			}
			else
			{
				sprintf(tmp, "%.*s", (int) (sos - soc), soc);

				get_color_codes(color, tmp, color);

				cat_sprintf(buf, "{%d}{%s%.*s}", cnt++, color, lis - sos, sos);

				lis++;

				soc = sos;
				pti = sos = lis;
			}
		}
		else
		{
			pti++;
			col++;
		}
	}
	sprintf(tmp, "%.*s", (int) (sos - soc), soc);

	get_color_codes(color, tmp, color);

	cat_sprintf(buf, "{%d}{%s%s}", cnt, color, sos);

	strcpy(str, buf);

	pop_call();
	return;
}

int stringlength(struct session *ses, char *str)
{
	char temp[BUFFER_SIZE];

	substitute(ses, str, temp, SUB_COL|SUB_ESC);

	return strip_vt102_strlen(ses, temp);
}


// Returns the raw length for the stripped range

int string_str_raw_len(struct session *ses, char *str, int start, int end)
{
	int raw_cnt, str_cnt, ret_cnt, tot_len;

	raw_cnt = str_cnt = ret_cnt = 0;

	tot_len = strlen(str);

	while (raw_cnt < tot_len)
	{
		if (skip_vt102_codes(&str[raw_cnt]))
		{
			ret_cnt += (str_cnt >= start) ? skip_vt102_codes(&str[raw_cnt]) : 0;
			raw_cnt += skip_vt102_codes(&str[raw_cnt]);

			continue;
		}

		if (is_color_code(&str[raw_cnt]))
		{
			ret_cnt += (str_cnt >= start) ? 5 : 0;
			raw_cnt += 5;

			continue;
		}

		if (str_cnt >= end)
		{
			break;
		}

		if (HAS_BIT(ses->flags, SES_FLAG_UTF8) && (str[raw_cnt] & 192) == 192)
		{
			ret_cnt += (str_cnt >= start) ? 1 : 0;
			raw_cnt++;

			while (raw_cnt < tot_len && (str[raw_cnt] & 192) == 128)
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

int string_raw_str_len(struct session *ses, char *str, int start, int end)
{
	int raw_cnt, ret_cnt, tot_len;

	raw_cnt = start;
	ret_cnt = 0;
	tot_len = strlen(str);

	while (raw_cnt < tot_len)
	{
		if (raw_cnt >= end)
		{
			break;
		}

		if (skip_vt102_codes(&str[raw_cnt]))
		{
			raw_cnt += skip_vt102_codes(&str[raw_cnt]);

			continue;
		}

		if (is_color_code(&str[raw_cnt]))
		{
			raw_cnt += 5;

			continue;
		}

		if (HAS_BIT(gtd->ses->flags, SES_FLAG_UTF8) && (str[raw_cnt] & 192) == 192)
		{
			raw_cnt++;

			while (raw_cnt < tot_len && (str[raw_cnt] & 192) == 128)
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

void timestring(struct session *ses, char *str)
{
	char left[BUFFER_SIZE], right[BUFFER_SIZE], *arg;

	struct tm timeval_tm;
	time_t    timeval_t;

	arg = get_arg_in_braces(ses, str, left, TRUE);

	if (*arg == COMMAND_SEPARATOR)
	{
		arg++;
	}
	arg = get_arg_in_braces(ses, arg, right, TRUE);

	if (*right)
	{
		timeval_t = (time_t) get_number(ses, right);
	}
	else
	{
		timeval_t = (time_t) time(NULL);
	}

	timeval_tm = *localtime(&timeval_t);

	strftime(str, BUFFER_SIZE, left, &timeval_tm);
}

DO_COMMAND(do_format)
{
	char temp[BUFFER_SIZE], destvar[BUFFER_SIZE], format[BUFFER_SIZE], newformat[BUFFER_SIZE], arglist[20][BUFFER_SIZE], *ptf, *ptt, *pts, *ptn;
	struct tm timeval_tm;
	time_t    timeval_t;
	int i;

	arg = sub_arg_in_braces(ses, arg, destvar,  GET_NST, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, format,   GET_ONE, SUB_VAR|SUB_FUN);

	if (*destvar == 0)
	{
		display_printf2(ses, "#SYNTAX: #format {variable} {format} {arg1} {arg2}");

		return ses;
	}

	for (i = 0 ; i < 20 ; i++)
	{
		arg = sub_arg_in_braces(ses, arg, arglist[i], GET_ONE, SUB_VAR|SUB_FUN);
	}

	i = 0;

	ptf = format;
	ptn = newformat;

	while (*ptf)
	{
		if (i == 20)
		{
			break;
		}

		if (*ptf == '%')
		{
			pts = ptn;

			*ptn++ = *ptf++;

			if (*ptf == 0)
			{
				break;
			}
			else if (*ptf == '%')
			{
				*ptn++ = *ptf++;
			}
			else
			{
				while (!isalpha((int) *ptf))
				{
					*ptn++ = *ptf++;
				}

				if (*ptf == 0)
				{
					break;
				}

				*ptn = 0;

				if (*ptf == 'd' || *ptf == 'f')
				{
					strcpy(temp, pts);

					ptn = pts + 1;
					*ptn = 0;
				}
				else if (pts[1])
				{
					char arg1[BUFFER_SIZE], arg2[BUFFER_SIZE];

					ptt = arg1;
					ptn = pts + 1;

					while (*ptn && *ptn != '.')
					{
						*ptt++ = *ptn++;
					}

					*ptt = 0;

					if (*ptn == 0)
					{
						if (atoi(arg1) < 0)
						{
							sprintf(temp, "%%%d",
								atoi(arg1) - ((int) strlen(arglist[i]) - string_raw_str_len(ses, arglist[i], 0, BUFFER_SIZE)));
						}
						else
						{
							sprintf(temp, "%%%d",
								atoi(arg1) + ((int) strlen(arglist[i]) - string_raw_str_len(ses, arglist[i], 0, BUFFER_SIZE)));
						}
					}
					else
					{
						ptt = arg2;
						ptn = ptn + 1;

						while (*ptn)
						{
							*ptt++ = *ptn++;
						}

						*ptt = 0;

						if (atoi(arg1) < 0)
						{
							sprintf(temp, "%%%d.%d",
								atoi(arg1) - ((int) strlen(arglist[i]) - string_raw_str_len(ses, arglist[i], 0, BUFFER_SIZE)),
								string_str_raw_len(ses, arglist[i], 0, atoi(arg2)));
						}
						else
						{
							sprintf(temp, "%%%d.%d",
								atoi(arg1) + ((int) strlen(arglist[i]) - string_raw_str_len(ses, arglist[i], 0, BUFFER_SIZE)),
								string_str_raw_len(ses, arglist[i], 0, atoi(arg2)));
						}
					}

					ptt = temp;
					ptn = pts;

					while (*ptt)
					{
						*ptn++ = *ptt++;
					}

					*ptn = 0;
				}

				switch (*ptf)
				{
					case 'a':
						sprintf(arglist[i], "%c", (char) get_number(ses, arglist[i]));
						break;

					case 'c':
						colorstring(ses, arglist[i]);
						break;

					case 'd':
						strcat(temp, "lld");
						sprintf(arglist[i], temp, (long long) get_number(ses, arglist[i]));
						break;

					case 'f':
						strcat(temp, "f");
						sprintf(arglist[i], temp, get_number(ses, arglist[i]));
						break;

					case 'g':
						thousandgroupingstring(ses, arglist[i]);
						break;

					case 'h':
						headerstring(arglist[i]);
						break;

					case 'l':
						lowerstring(arglist[i]);
						break;

					case 'm':
						mathstring(ses, arglist[i]);
						break;

					case 'n':
						arglist[i][0] = toupper((int) arglist[i][0]);
						break;

					case 'p':
						stripspaces(arglist[i]);
						break;

					case 'r':
						reversestring(arglist[i]);
						break;

					case 's':
						break;

					case 't':
						timestring(ses, arglist[i]);
						break;

					case 'u':
						upperstring(arglist[i]);
						break;

					case 'w':
						substitute(ses, arglist[i], arglist[i], SUB_VAR|SUB_FUN|SUB_COL|SUB_ESC);
						wrapstring(ses, arglist[i]);
						break;

					case 'A':
						charactertonumber(ses, arglist[i]);
						break;

					case 'C':
						sprintf(arglist[i], "%d", ses->cols);
						break;

					case 'D':
						timeval_t  = (time_t) *arglist[i] ? atoll(arglist[i]) : time(NULL);
						timeval_tm = *localtime(&timeval_t);
						strftime(arglist[i], BUFFER_SIZE, "%d", &timeval_tm);
						break;

					case 'G':
						thousandgroupingstring(ses, arglist[i]);
						break;

					case 'L':
						sprintf(arglist[i], "%d", stringlength(ses, arglist[i]));
						break;

					case 'M':
						timeval_t  = (time_t) *arglist[i] ? atoll(arglist[i]) : time(NULL);
						timeval_tm = *localtime(&timeval_t);
						strftime(arglist[i], BUFFER_SIZE, "%m", &timeval_tm);
						break;

					case 'R':
						sprintf(arglist[i], "%d", ses->rows);
						break;

					case 'T':
						sprintf(arglist[i], "%d", (int) time(NULL));
						break;

					case 'U':
						sprintf(arglist[i], "%lld", utime());
						break;

					case 'Y':
						timeval_t  = (time_t) *arglist[i] ? atoll(arglist[i]) : time(NULL);
						timeval_tm = *localtime(&timeval_t);
						strftime(arglist[i], BUFFER_SIZE, "%Y", &timeval_tm);
						break;

					default:
						show_message(ses, LIST_VARIABLE, "#FORMAT: UNKNOWN ARGUMENT {%%%c}.", *ptf);
						break;
				}
				*ptn++ = 's';
				i++;
				ptf++;
			}
		}
		else
		{
			*ptn++ = *ptf++;
		}
	}
	*ptn = 0;

	sprintf(temp, newformat, arglist[0], arglist[1], arglist[2], arglist[3], arglist[4], arglist[5], arglist[6], arglist[7], arglist[8], arglist[9], arglist[10], arglist[11], arglist[12], arglist[13], arglist[14], arglist[15], arglist[16], arglist[17], arglist[18], arglist[19]);

	set_nest_node(ses->list[LIST_VARIABLE], destvar, "%s", temp);

	return ses;
}
