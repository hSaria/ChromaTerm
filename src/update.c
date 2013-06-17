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

#include <sys/types.h>
#include <sys/time.h>
#include <termios.h>
#include <errno.h>


void mainloop(void)
{
	static struct timeval curr_time, wait_time, last_time;
	int usec_loop, usec_wait;

	short int pulse_poll_input      = 0 + PULSE_POLL_INPUT;
	short int pulse_poll_sessions   = 0 + PULSE_POLL_SESSIONS;
	short int pulse_update_ticks    = 0 + PULSE_UPDATE_TICKS;
	short int pulse_update_delays   = 0 + PULSE_UPDATE_DELAYS;
	short int pulse_update_packets  = 0 + PULSE_UPDATE_PACKETS;
	short int pulse_update_terminal = 0 + PULSE_UPDATE_TERMINAL;
	short int pulse_update_memory   = 0 + PULSE_UPDATE_MEMORY;
	short int pulse_update_time     = 1 + PULSE_UPDATE_TIME;

	wait_time.tv_sec = 0;

	while (TRUE)
	{
		gettimeofday(&last_time, NULL);

		if (--pulse_poll_input == 0)
		{
			open_timer(TIMER_POLL_INPUT);

			pulse_poll_input = PULSE_POLL_INPUT;

			poll_input();

			close_timer(TIMER_POLL_INPUT);
		}

		if (--pulse_poll_sessions == 0)
		{
			pulse_poll_sessions = PULSE_POLL_SESSIONS;

			poll_sessions();
		}

		if (--pulse_update_ticks == 0)
		{
			pulse_update_ticks = PULSE_UPDATE_TICKS;

			tick_update();
		}

		if (--pulse_update_delays == 0)
		{
			pulse_update_delays = PULSE_UPDATE_DELAYS;

			delay_update();
		}

		if (--pulse_update_packets == 0)
		{
			pulse_update_packets = PULSE_UPDATE_PACKETS;

			packet_update();
		}

		if (--pulse_update_terminal == 0)
		{
			pulse_update_terminal = PULSE_UPDATE_TERMINAL;

			terminal_update();
		}

		if (--pulse_update_memory == 0)
		{
			pulse_update_memory = PULSE_UPDATE_MEMORY;

			memory_update();
		}

		if (--pulse_update_time == 0)
		{
			pulse_update_time = PULSE_UPDATE_TIME;

			time_update();
		}

		gettimeofday(&curr_time, NULL);

		if (curr_time.tv_sec == last_time.tv_sec)
		{
			usec_loop = curr_time.tv_usec - last_time.tv_usec;
		}
		else
		{
			usec_loop = 1000000 - last_time.tv_usec + curr_time.tv_usec;
		}

		usec_wait = 1000000 / PULSE_PER_SECOND - usec_loop;

		wait_time.tv_usec = usec_wait;

		gtd->total_io_exec  += usec_loop;
		gtd->total_io_delay += usec_wait;

		if (usec_wait > 0)
		{
			select(0, NULL, NULL, NULL, &wait_time);
		}
	}
}

void poll_input(void)
{
	fd_set readfds;
	static struct timeval to;

	while (TRUE)
	{
		FD_ZERO(&readfds);

		FD_SET(0, &readfds);

		if (select(FD_SETSIZE, &readfds, NULL, NULL, &to) <= 0)
		{
			return;
		}

		if (FD_ISSET(0, &readfds))
		{
			process_input();
		}
		else
		{
			return;
		}
	}
}

void poll_sessions(void)
{
	fd_set readfds, excfds;
	static struct timeval to;
	struct session *ses;
	int rv;

	open_timer(TIMER_POLL_SESSIONS);

	if (gts->next)
	{
		FD_ZERO(&readfds);
		FD_ZERO(&excfds);

		for (ses = gts->next ; ses ; ses = gtd->update)
		{
			gtd->update = ses->next;

			if (HAS_BIT(ses->flags, SES_FLAG_CONNECTED))
			{
				while (TRUE)
				{
					FD_SET(ses->socket, &readfds);
					FD_SET(ses->socket, &excfds);

					rv = select(FD_SETSIZE, &readfds, NULL, &excfds, &to);

					if (rv <= 0)
					{
						break;
					}

					if (FD_ISSET(ses->socket, &readfds))
					{
						if (read_buffer_mud(ses) == FALSE)
						{
							readmud(ses);

							cleanup_session(ses);

							gtd->mud_output_len = 0;

							break;
						}
					}

					if (FD_ISSET(ses->socket, &excfds))
					{
						FD_CLR(ses->socket, &readfds);

						cleanup_session(ses);

						gtd->mud_output_len = 0;

						break;
					}
				}

				if (gtd->mud_output_len)
				{
					readmud(ses);
				}
			}
		}
	}
	close_timer(TIMER_POLL_SESSIONS);
}

void tick_update(void)
{
	struct session *ses;
	struct listnode *node;
	struct listroot *root;

	open_timer(TIMER_UPDATE_TICKS);

	utime();

	for (ses = gts->next ; ses ; ses = gtd->update)
	{
		gtd->update = ses->next;

		root = ses->list[LIST_TICKER];

		for (root->update = 0 ; root->update < root->used ; root->update++)
		{
			node = root->list[root->update];

			if (node->data == 0)
			{
				node->data = gtd->time + (long long) (get_number(ses, node->pr) * 1000000LL);
			}

			if (node->data <= gtd->time)
			{
				node->data += (long long) (get_number(ses, node->pr) * 1000000LL);

				show_debug(ses, LIST_TICKER, "#DEBUG TICKER {%s}", node->right);

				script_driver(ses, LIST_TICKER, node->right);
			}
		}
	}
	close_timer(TIMER_UPDATE_TICKS);
}

void delay_update(void)
{
	struct session *ses;
	struct listnode *node;
	struct listroot *root;
	char buf[BUFFER_SIZE];

	open_timer(TIMER_UPDATE_DELAYS);

	for (ses = gts ; ses ; ses = gtd->update)
	{
		gtd->update = ses->next;

		root = ses->list[LIST_DELAY];	

		for (root->update = 0 ; root->update < root->used ; root->update++)
		{
			node = root->list[root->update];

			if (node->data == 0)
			{
				node->data = gtd->time + (long long) (get_number(ses, node->pr) * 1000000LL);
			}

			if (node->data <= gtd->time)
			{
				strcpy(buf, node->right);

				show_debug(ses, LIST_DELAY, "#DEBUG DELAY {%s}", buf);

				delete_node_list(ses, LIST_DELAY, node);

				script_driver(ses, LIST_DELAY, buf);
			}
		}
	}
	close_timer(TIMER_UPDATE_DELAYS);
}

void packet_update(void)
{
	char result[STRING_SIZE];
	struct session *ses;

	open_timer(TIMER_UPDATE_PACKETS);

	for (ses = gts->next ; ses ; ses = gtd->update)
	{
		gtd->update = ses->next;

		if (ses->check_output && gtd->time > ses->check_output)
		{
			SET_BIT(ses->flags, SES_FLAG_READMUD);

			strcpy(result, ses->more_output);

			ses->more_output[0] = 0;

			process_mud_output(ses, result, TRUE);

			DEL_BIT(ses->flags, SES_FLAG_READMUD);
		}
	}
	close_timer(TIMER_UPDATE_PACKETS);
}

void terminal_update(void)
{
	open_timer(TIMER_UPDATE_TERMINAL);

	fflush(stdout);

	close_timer(TIMER_UPDATE_TERMINAL);
}

void memory_update(void)
{
	open_timer(TIMER_UPDATE_MEMORY);

	while (gtd->dispose_next)
	{
		dispose_session(gtd->dispose_next);
	}

	close_timer(TIMER_UPDATE_MEMORY);
}

void time_update(void)
{
	static char sec[3], min[3], hrs[3], day[3], wks[3], mon[3], yrs[5];
	static char old_sec[3], old_min[3], old_hrs[3], old_day[3], old_wks[3], old_mon[3], old_yrs[5];

	time_t timeval_t = (time_t) time(NULL);
	struct tm timeval_tm = *localtime(&timeval_t);

	open_timer(TIMER_UPDATE_TIME);

	// Initialize on the first call.

	if (old_sec[0] == 0)
	{
		strftime(old_sec, 3, "%S", &timeval_tm);
		strftime(old_min, 3, "%M", &timeval_tm);
		strftime(old_hrs, 3, "%H", &timeval_tm);
		strftime(old_day, 3, "%d", &timeval_tm);
		strftime(old_wks, 3, "%W", &timeval_tm);
		strftime(old_mon, 3, "%m", &timeval_tm);
		strftime(old_yrs, 5, "%Y", &timeval_tm);

		strftime(sec, 3, "%S", &timeval_tm);
		strftime(min, 3, "%M", &timeval_tm);
		strftime(hrs, 3, "%H", &timeval_tm);
		strftime(day, 3, "%d", &timeval_tm);
		strftime(wks, 3, "%W", &timeval_tm);
		strftime(mon, 3, "%m", &timeval_tm);
		strftime(yrs, 5, "%Y", &timeval_tm);
	}

	strftime(sec, 3, "%S", &timeval_tm);
	strftime(min, 3, "%M", &timeval_tm);

	if (min[0] == old_min[0] && min[1] == old_min[1])
	{
		goto time_event_sec;
	}

	strcpy(old_min, min);

	strftime(hrs, 3, "%H", &timeval_tm);

	if (hrs[0] == old_hrs[0] && hrs[1] == old_hrs[1])
	{
		goto time_event_min;
	}

	strcpy(old_hrs, hrs);

	strftime(day, 3, "%d", &timeval_tm);
	strftime(wks, 3, "%W", &timeval_tm);

	if (day[0] == old_day[0] && day[1] == old_day[1])
	{
		goto time_event_hrs;
	}

	strcpy(old_day, day);

	strftime(mon, 3, "%m", &timeval_tm);

	if (mon[0] == old_mon[0] && mon[1] == old_mon[1])
	{
		goto time_event_day;
	}

	strcpy(old_mon, mon);

	strftime(yrs, 5, "%Y", &timeval_tm);

	if (yrs[0] == old_yrs[0] && yrs[1] == old_yrs[1] && yrs[2] == old_yrs[2] && yrs[3] == old_yrs[3])
	{
		goto time_event_mon;
	}

	strcpy(old_yrs, yrs);

	check_all_events(NULL, SUB_ARG|SUB_SEC, 0, 7, "YEAR", yrs, mon, wks, day, hrs, min, sec);
	check_all_events(NULL, SUB_ARG|SUB_SEC, 1, 7, "YEAR %s", yrs, yrs, mon, wks, day, hrs, min, sec);


	time_event_mon:

	check_all_events(NULL, SUB_ARG|SUB_SEC, 0, 7, "MONTH", yrs, mon, wks, day, hrs, min, sec);
	check_all_events(NULL, SUB_ARG|SUB_SEC, 1, 7, "MONTH %s", mon, yrs, mon, wks, day, hrs, min, sec);


	time_event_day:

	if (wks[0] != old_wks[0] || wks[1] != old_wks[1])
	{
		strcpy(old_wks, wks);

		check_all_events(NULL, SUB_ARG|SUB_SEC, 0, 7, "WEEK", yrs, mon, wks, day, hrs, min, sec);
		check_all_events(NULL, SUB_ARG|SUB_SEC, 1, 7, "WEEK %s", wks, yrs, mon, wks, day, hrs, min, sec);
	}

	check_all_events(NULL, SUB_ARG|SUB_SEC, 2, 7, "DATE %s-%s", mon, day, yrs, mon, wks, day, hrs, min, sec);

	check_all_events(NULL, SUB_ARG|SUB_SEC, 0, 7, "DAY", yrs, mon, wks, day, hrs, min, sec);
	check_all_events(NULL, SUB_ARG|SUB_SEC, 1, 7, "DAY %s", day, yrs, mon, wks, day, hrs, min, sec);


	time_event_hrs:

	check_all_events(NULL, SUB_ARG|SUB_SEC, 0, 7, "HOUR", yrs, mon, wks, day, hrs, min, sec);
	check_all_events(NULL, SUB_ARG|SUB_SEC, 1, 7, "HOUR %s", hrs, yrs, mon, wks, day, hrs, min, sec);


	time_event_min:

	check_all_events(NULL, SUB_ARG|SUB_SEC, 4, 7, "DATE %s-%s %s:%s", mon, day, hrs, min, yrs, mon, wks, day, hrs, min, sec);

	check_all_events(NULL, SUB_ARG|SUB_SEC, 2, 7, "TIME %s:%s", hrs, min, yrs, mon, wks, day, hrs, min, sec);

	check_all_events(NULL, SUB_ARG|SUB_SEC, 0, 7, "MINUTE", yrs, mon, wks, day, hrs, min, sec);
	check_all_events(NULL, SUB_ARG|SUB_SEC, 1, 7, "MINUTE %s", min, yrs, mon, wks, day, hrs, min, sec);


	time_event_sec:

	check_all_events(NULL, SUB_ARG|SUB_SEC, 3, 7, "TIME %s:%s:%s", hrs, min, sec, yrs, mon, wks, day, hrs, min, sec);

	check_all_events(NULL, SUB_ARG|SUB_SEC, 0, 7, "SECOND", yrs, mon, wks, day, hrs, min, sec);
	check_all_events(NULL, SUB_ARG|SUB_SEC, 1, 7, "SECOND %s", sec, yrs, mon, wks, day, hrs, min, sec);

	close_timer(TIMER_UPDATE_TIME);
}
