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

#include <sys/param.h>
#include <errno.h>


/**********************************************/
/* return: TRUE if s1 is an abbrevation of s2 */
/**********************************************/


int is_abbrev(char *s1, char *s2)
{
	if (*s1 == 0)
	{
		return FALSE;
	}
	return !strncasecmp(s2, s1, strlen(s1));
}


int is_color_code(char *str)
{
	if (str[0] == '<')
	{
		if (!isalnum((int) str[1]) || !isalnum((int) str[2]) || !isalnum((int) str[3]) || str[4] != '>')
		{
			return FALSE;
		}
		else if (str[1] >= '0' && str[1] <= '9' && str[2] >= '0' && str[2] <= '9' && str[3] >= '0' && str[3] <= '9')
		{
			return TRUE;
		}
		else if (str[1] >= 'a' && str[1] <= 'f' && str[2] >= 'a' && str[2] <= 'f' && str[3] >= 'a' && str[3] <= 'f')
		{
			return TRUE;
		}
		else if (str[1] >= 'A' && str[1] <= 'F' && str[2] >= 'A' && str[2] <= 'F' && str[3] >= 'A' && str[3] <= 'F')
		{
			return TRUE;
		}
		else if (str[1] == 'g' && str[1] <= '9' && str[2] >= '0' && str[2] <= '9' && str[3] >= '0' && str[3] <= '9')
		{
			return TRUE;
		}
		else if (str[1] == 'G' && str[1] <= '9' && str[2] >= '0' && str[2] <= '9' && str[3] >= '0' && str[3] <= '9')
		{
			return TRUE;
		}
	}
	return FALSE;
}

/*
	Keep synched with tintoi()
*/

int is_number(char *str)
{
	char *ptr = str;
	int i = 1, d = 0;

	if (*ptr == 0)
	{
		return FALSE;
	}

	ptr = str + strlen(str);

	while (TRUE)
	{
		ptr--;

		switch (*ptr)
		{
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				break;

			case '.':
				if (d)
				{
					return FALSE;
				}
				d = 1;
				break;

			case ':':
				if (i == 4)
				{
					return FALSE;
				}
				i++;
				break;

			case '!':
			case '~':
			case '+':
			case '-':
				if (ptr != str)
				{
					return FALSE;
				}
				break;

			default:
				return FALSE;
		}

		if (ptr == str)
		{
			break;
		}
	}
	return TRUE;
}

int hex_number(char *str)
{
	int value = 0;

	if (str)
	{
		if (isdigit((int) *str))
		{
			value += 16 * (*str - '0');
		}
		else
		{
			value += 16 * (toupper((int) *str) - 'A' + 10);
		}
		str++;
	}

	if (str)
	{
		if (isdigit((int) *str))
		{
			value += *str - '0';
		}
		else
		{
			value += toupper((int) *str) - 'A' + 10;
		}
		str++;
	}

	return value;
}

int oct_number(char *str)
{
	int value = 0;

	if (str)
	{
		if (isdigit((int) *str))
		{
			value += 8 * (*str - '0');
		}
		str++;
	}

	if (str)
	{
		if (isdigit((int) *str))
		{
			value += *str - '0';
		}
		str++;
	}

	return value;
}

long long utime()
{
	struct timeval now_time;

	gettimeofday(&now_time, NULL);

	if (gtd->time >= now_time.tv_sec * 1000000LL + now_time.tv_usec)
	{
		gtd->time++;
	}
	else
	{
		gtd->time = now_time.tv_sec * 1000000LL + now_time.tv_usec;
	}
	return gtd->time;
}

char *capitalize(char *str)
{
	static char outbuf[BUFFER_SIZE];
	int cnt;

	for (cnt = 0 ; str[cnt] != 0 ; cnt++)
	{
		outbuf[cnt] = toupper((int) str[cnt]);
	}
	outbuf[cnt] = 0;

	return outbuf;
}

char *ntos(long long number)
{
	static char outbuf[100][NUMBER_SIZE];
	static int cnt;

	cnt = (cnt + 1) % 100;

	sprintf(outbuf[cnt], "%lld", number);

	return outbuf[cnt];
}

char *indent(int cnt)
{
	static char outbuf[BUFFER_SIZE];

	memset(outbuf, '\t', cnt);

	outbuf[cnt] = 0;

	return outbuf;
}

int cat_sprintf(char *dest, char *fmt, ...)
{
	char buf[STRING_SIZE];
	int size;

	va_list args;

	va_start(args, fmt);
	size = vsprintf(buf, fmt, args);
	va_end(args);

	strcat(dest, buf);

	return size;
}

void ins_sprintf(char *dest, char *fmt, ...)
{
	char buf[STRING_SIZE], tmp[STRING_SIZE];

	va_list args;

	va_start(args, fmt);
	vsprintf(buf, fmt, args);
	va_end(args);

	strcpy(tmp, dest);
	strcpy(dest, buf);
	strcat(dest, tmp);
}

int str_suffix(char *str1, char *str2)
{
	int len1, len2;

	len1 = strlen(str1);
	len2 = strlen(str2);

	if (len1 >= len2)
	{
		if (!strcasecmp(str1 + len1 - len2, str2))
		{
			return FALSE;
		}
	}
	return TRUE;
}

void show_message(struct session *ses, int index, char *format, ...)
{
	struct listroot *root;
	char buf[STRING_SIZE];
	va_list args;

	push_call("show_message(%p,%p,%p)",ses,index,format);

	va_start(args, format);

	vsprintf(buf, format, args);

	va_end(args);

	if (HAS_BIT(ses->flags, SES_FLAG_VERBOSELINE))
	{
		display_puts2(ses, buf);

		pop_call();
		return;
	}

	if (index == -1)
	{
		if (ses->input_level == 0)
		{
			display_puts2(ses, buf);
		}
		pop_call();
		return;
	}

	root = ses->list[index];

	if (HAS_BIT(root->flags, LIST_FLAG_DEBUG))
	{
		display_puts2(ses, buf);

		pop_call();
		return;
	}

	if (HAS_BIT(root->flags, LIST_FLAG_MESSAGE))
	{
		if (ses->input_level == 0)
		{
			display_puts2(ses, buf);

			pop_call();
			return;
		}
	}

	if (HAS_BIT(root->flags, LIST_FLAG_LOG))
	{
		if (ses->logfile)
		{
			logit(ses, buf, ses->logfile, TRUE);
		}
	}
	pop_call();
	return;
}

void show_debug(struct session *ses, int index, char *format, ...)
{
	struct listroot *root;
	char buf[STRING_SIZE];
	va_list args;

	push_call("show_debug(%p,%p,%p)",ses,index,format);

	root = ses->list[index];

	if (!HAS_BIT(root->flags, LIST_FLAG_DEBUG) && !HAS_BIT(root->flags, LIST_FLAG_LOG))
	{
		pop_call();
		return;
	}

	va_start(args, format);

	vsprintf(buf, format, args);

	va_end(args);

	if (HAS_BIT(root->flags, LIST_FLAG_DEBUG))
	{
		display_puts2(ses, buf);

		pop_call();
		return;
	}

	if (HAS_BIT(root->flags, LIST_FLAG_LOG))
	{
		if (ses->logfile)
		{
			logit(ses, buf, ses->logfile, TRUE);
		}
	}
	pop_call();
	return;
}

void display_header(struct session *ses, char *format, ...)
{
	char arg[BUFFER_SIZE], buf[BUFFER_SIZE];
	va_list args;

	va_start(args, format);
	vsprintf(arg, format, args);
	va_end(args);

	if ((int) strlen(arg) > gtd->ses->cols - 2)
	{
		arg[gtd->ses->cols - 2] = 0;
	}

	memset(buf, '#', gtd->ses->cols);

	memcpy(&buf[(gtd->ses->cols - strlen(arg)) / 2], arg, strlen(arg));

	buf[gtd->ses->cols] = 0;

	display_puts2(ses, buf);
}


void socket_printf(struct session *ses, size_t length, char *format, ...)
{
	char buf[STRING_SIZE];
	va_list args;

	va_start(args, format);
	vsprintf(buf, format, args);
	va_end(args);

	if (HAS_BIT(ses->flags, SES_FLAG_CONNECTED))
	{
		write(ses->socket, buf, length);
	}
}

void display_printf2(struct session *ses, char *format, ...)
{
	char buf[STRING_SIZE];
	va_list args;

	va_start(args, format);
	vsprintf(buf, format, args);
	va_end(args);

	display_puts2(ses, buf);
}

void display_printf(struct session *ses, char *format, ...)
{
	char buf[STRING_SIZE];
	va_list args;

	va_start(args, format);
	vsprintf(buf, format, args);
	va_end(args);

	display_puts(ses, buf);
}

/*
	Like display_puts2, but no color codes added
*/

void display_puts3(struct session *ses, char *string)
{
	char output[STRING_SIZE];

	if (ses == NULL)
	{
		ses = gtd->ses;
	}

	if (!HAS_BIT(gtd->ses->flags, SES_FLAG_VERBOSE) && gtd->quiet)
	{
		return;
	}

	if (strip_vt102_strlen(ses, ses->more_output) != 0)
	{
		sprintf(output, "\n%s", string);
	}
	else
	{
		sprintf(output, "%s", string);
	}

	if (ses != gtd->ses)
	{
		return;
	}

	printline(ses, output, FALSE);
}

/*
	output to screen should go through this function
	the output is NOT checked for actions or anything
*/

void display_puts2(struct session *ses, char *string)
{
	char output[STRING_SIZE];

	push_call("display_puts2(%p,%p)",ses,string);

	if (ses == NULL)
	{
		ses = gtd->ses;
	}

	if (!HAS_BIT(gtd->ses->flags, SES_FLAG_VERBOSE) && gtd->quiet)
	{
		pop_call();
		return;
	}

	if (strip_vt102_strlen(ses, ses->more_output) != 0)
	{
		sprintf(output, "\n\033[0m%s\033[0m", string);
	}
	else
	{
		sprintf(output, "\033[0m%s\033[0m", string);
	}

	if (ses != gtd->ses)
	{
		pop_call();
		return;
	}

	printline(ses, output, FALSE);

	pop_call();
	return;
}

/*
	output to screen should go through this function
	the output IS treated as though it came from the mud
*/

void display_puts(struct session *ses, char *string)
{
	if (ses == NULL)
	{
		ses = gtd->ses;
	}

	do_one_line(string, ses);

	if (!HAS_BIT(ses->flags, SES_FLAG_GAG))
	{
		display_puts2(ses, string);
	}
	else
	{
		DEL_BIT(ses->flags, SES_FLAG_GAG);
	}
}


/*************************************************/
/* print system call error message and terminate */
/*************************************************/

void syserr(char *msg)
{
	char s[128], *syserrmsg;

	syserrmsg = strerror(errno);

	if (syserrmsg)
	{
		sprintf(s, "ERROR: %s (%d: %s)", msg, errno, syserrmsg);
	}
	else
	{
		sprintf(s, "ERROR: %s (%d)", msg, errno);
	}
	quitmsg(s);
}

void show_cpu(struct session *ses)
{
	long long total_cpu;
	int timer;

	display_printf2(ses, "Section                           Time (usec)    Freq (msec)  %%Prog         %%CPU");

	display_printf2(ses, "");

	for (total_cpu = timer = 0 ; timer < TIMER_CPU ; timer++)
	{
		total_cpu += display_timer(ses, timer);
	}

	display_printf2(ses, "");

	display_printf2(ses, "Unknown CPU Usage:              %6.2f percent", (gtd->total_io_exec - total_cpu) * 100.0 / (gtd->total_io_delay + gtd->total_io_exec));
	display_printf2(ses, "Average CPU Usage:              %6.2f percent", (gtd->total_io_exec)             * 100.0 / (gtd->total_io_delay + gtd->total_io_exec));
}


long long display_timer(struct session *ses, int timer)
{
	long long total_usage, indicated_usage;

	total_usage = gtd->total_io_exec + gtd->total_io_delay;

	if (total_usage == 0)
	{
		return 0;
	}

	if (gtd->timer[timer][1] == 0 || gtd->timer[timer][4] == 0)
	{
		return 0;
	}

	indicated_usage = gtd->timer[timer][0] / gtd->timer[timer][1] * gtd->timer[timer][4];

	display_printf2(ses, "%-30s%8lld       %8lld      %8.2f     %8.2f",
		timer_table[timer].name,
		gtd->timer[timer][0] / gtd->timer[timer][1],
		gtd->timer[timer][3] / gtd->timer[timer][4] / 1000,
		100.0 * (double) indicated_usage / (double) gtd->total_io_exec,
		100.0 * (double) indicated_usage / (double) total_usage);

	return indicated_usage;
}


void open_timer(int timer)
{
	struct timeval last_time;
	long long current_time;

	gettimeofday(&last_time, NULL);

	current_time = (long long) last_time.tv_usec + 1000000LL * (long long) last_time.tv_sec;

	if (gtd->timer[timer][2] == 0)
	{
		gtd->timer[timer][2] = current_time ;
	}
	else
	{
		gtd->timer[timer][3] += current_time - gtd->timer[timer][2];
		gtd->timer[timer][2]  = current_time;
		gtd->timer[timer][4] ++;
	}
}


void close_timer(int timer)
{
	struct timeval last_time;
	long long current_time;

	gettimeofday(&last_time, NULL);

	current_time = (long long) last_time.tv_usec + 1000000LL * (long long) last_time.tv_sec;

	gtd->timer[timer][0] += (current_time - gtd->timer[timer][2]);
	gtd->timer[timer][1] ++;
}
		
/*
	Whoops, strcasecmp wasn't found.
*/

#if !defined(HAVE_STRCASECMP)
#define UPPER(c) (islower(c) ? toupper(c) : c)

int strcasecmp(char *string1, char *string2)
{
	for ( ; UPPER(*string1) == UPPER(*string2) ; string1++, string2++)
		if (!*string1)
			return(0);
	return(UPPER(*string1) - UPPER(*string2));
}

int strncasecmp(char *string1, char *string2, size_t count)
{
	if (count)
		do
		{
			if (UPPER(*string1) != UPPER(*string2))
				return(UPPER(*string1) - UPPER(*string2));
					if (!*string1++)
						break;
		string2++;
	}
	while (--count);

	return(0);
}

#endif
