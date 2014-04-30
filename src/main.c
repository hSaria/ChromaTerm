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

#include <signal.h>
#include <sys/socket.h>

/*************** globals ******************/

struct session *gts;
struct global_data *gtd;

void pipe_handler(int signal)
{
	restore_terminal();

	display_printf(NULL, "broken_pipe: dumping stack");

	dump_stack();
}

/*
	when the screen size changes, take note of it
*/

void winch_handler(int signal)
{
	struct session *ses;

	init_screen_size(gts);

	for (ses = gts->next ; ses ; ses = ses->next)
	{
		init_screen_size(ses);
	}

	/*
		we have to reinitialize the signals for sysv machines

	if (signal(SIGWINCH, winch_handler) == BADSIG)
	{
		syserr("signal SIGWINCH");
	}
	*/
}


void abort_handler(int signal)
{
	static char crashed = FALSE;

	if (crashed)
	{
		exit(-1);
	}
	crashed = TRUE;

	restore_terminal();

	dump_stack();

	fflush(NULL);

	exit(-1);

	if (!HAS_BIT(gtd->ses->flags, SES_FLAG_LOCALECHO))
	{
		socket_printf(gtd->ses, 1, "%c", 3);
	}
	else
	{
		do_zap(gtd->ses, "");
	}
}

void suspend_handler(int signal)
{
	printf("\033[r\033[%d;%dH", gtd->ses->rows, 1);

	fflush(stdout);

	restore_terminal();

	kill(0, SIGSTOP);

	init_terminal();

	display_puts(NULL, "#WELCOME BACK.");
}

void trap_handler(int signal)
{
	static char crashed = FALSE;

	if (crashed)
	{
		exit(-1);
	}
	crashed = TRUE;

	restore_terminal();

	dump_stack();

	fflush(NULL);

	exit(-1);
}


/****************************************************************************/
/* main() - show title - setup signals - init lists - readcoms - mainloop() */
/****************************************************************************/


int main(int argc, char **argv)
{
	int greeting = TRUE;
	int shell = TRUE;

	#ifdef SOCKS
		SOCKSinit(argv[0]);
	#endif

	if (signal(SIGTERM, trap_handler) == BADSIG)
	{
		syserr("signal SIGTERM");
	}

	if (signal(SIGSEGV, trap_handler) == BADSIG)
	{
		syserr("signal SIGSEGV");
	}

	if (signal(SIGHUP, trap_handler) == BADSIG)
	{
		syserr("signal SIGHUP");
	}

	if (signal(SIGABRT, abort_handler) == BADSIG)
	{
		syserr("signal SIGTERM");
	}

	if (signal(SIGINT, abort_handler) == BADSIG)
	{
		syserr("signal SIGINT");
	}

	if (signal(SIGTSTP, suspend_handler) == BADSIG)
	{
		syserr("signal SIGSTOP");
	}

	if (signal(SIGPIPE, pipe_handler) == BADSIG)
	{
		syserr("signal SIGPIPE");
	}

	if (signal(SIGWINCH, winch_handler) == BADSIG)
	{
		syserr("signal SIGWINCH");
	}

	srand(time(NULL));

	if (argc > 1)
	{
		int c;

		while ((c = getopt(argc, argv, "c e: G h r: s: t: v")) != EOF)
		{
			if (c == 'G')
			{
				greeting = FALSE;
			}
		}

		optind = 1;
	}

	init_program(greeting);

	if (argc > 1)
	{
		int c;

		optind = 1;

		while ((c = getopt(argc, argv, "c e: G h r: s: t: v")) != EOF)
		{
			switch (c)
			{
				case 'c':
					shell = FALSE;
					break;

				case 'e':
					gtd->ses = script_driver(gtd->ses, -1, optarg);
					break;

				case 'G':
					break;

				case 'h':
					display_printf(NULL, "Usage: %s [OPTION]... [FILE]...", argv[0]);
					display_printf(NULL, "");
					display_printf(NULL, "  -c  Stay in interactive command shell.");
					display_printf(NULL, "  -e  Execute given command.");
					display_printf(NULL, "  -G  Do not show the greeting screen.");
					display_printf(NULL, "  -h  This help section.");
					display_printf(NULL, "  -r  Read given command file.");
					display_printf(NULL, "  -s  Use given rand seed."); 
					display_printf(NULL, "  -t  Set given title.");
					display_printf(NULL, "  -v  Enable verbose mode.");

					restore_terminal();
					exit(1);
					break;

				case 'r':
					gtd->ses = do_read(gtd->ses, optarg);
					break;

				case 't':
					printf("\033]0;%s\007", optarg);
					break;

				case 's':
					srand((unsigned int) atoll(optarg));
					break;

				case 'v':
					do_configure(gtd->ses, "{VERBOSE} {ON}");
					break;

				default:
					display_printf(NULL, "Unknown option '%c'.", c);
					break;
			}
		}

		if (argv[optind] != NULL)
		{
			gtd->ses = do_read(gtd->ses, argv[optind]);
		}
	}

	if (getenv("HOME") != NULL)
	{
		char filename[256];

		sprintf(filename, "%s", ".chromatermrc");

		if (access(filename, R_OK) != -1)
		{
			gtd->ses = do_read(gtd->ses, filename);
		}
		else
		{
			sprintf(filename, "%s/%s", getenv("HOME"), ".chromatermrc");

			if (access(filename, R_OK) != -1)
			{
				gtd->ses = do_read(gtd->ses, filename);
			}
		}
	}

	check_all_events(gts, SUB_ARG|SUB_SEC, 0, 2, "PROGRAM START", CLIENT_NAME, CLIENT_VERSION);
	check_all_events(gts, SUB_ARG|SUB_SEC, 0, 2, "SCREEN RESIZE", ntos(gts->cols), ntos(gts->rows));

	if (shell && getenv("SHELL") != NULL)
	{
		gtd->ses = do_run(gtd->ses, "shell");
	}

	mainloop();

	return 0;
}


void init_program(int greeting)
{
	int ref, index;

	gts = (struct session *) calloc(1, sizeof(struct session));

	for (index = 0 ; index < LIST_MAX ; index++)
	{
		gts->list[index] = init_list(gts, index, 32);
	}

	gts->name           = strdup("gts");
	gts->group          = strdup("");
	gts->socket         = 1;

	gtd                 = (struct global_data *) calloc(1, sizeof(struct global_data));

	gtd->ses            = gts;

	gtd->mud_output_max = 16384;
	gtd->mud_output_buf = (char *) calloc(1, gtd->mud_output_max);

	gtd->input_off      = 1;

	gtd->term           = strdup(getenv("TERM") ? getenv("TERM") : "UNKNOWN");

	for (index = 0 ; index < 100 ; index++)
	{
		gtd->vars[index] = strdup("");
		gtd->cmds[index] = strdup("");
	}

	for (ref = 0 ; ref < 26 ; ref++)
	{
		for (index = 0 ; *command_table[index].name != 0 ; index++)
		{
			if (*command_table[index].name == 'a' + ref)
			{
				gtd->command_ref[ref] = index;
				break;
			}
		}
	}

	init_screen_size(gts);

	gts->input_level++;

	do_configure(gts, "{CHARSET}         {UTF-8}");
	do_configure(gts, "{LOCAL ECHO}         {ON}");
	do_configure(gts, "{LOG}               {RAW}");
	do_configure(gts, "{PACKET PATCH}     {0.00}");
	do_configure(gts, "{COMMAND CHAR}        {#}");
	do_configure(gts, "{VERBATIM}          {OFF}");
	do_configure(gts, "{VERBOSE}           {OFF}");
	do_configure(gts, "{256 COLORS}       {AUTO}");

	gts->input_level--;

	init_terminal();

	if (greeting)
	{
		do_help(gts, "GREETING");
	}
}


void quitmsg(char *message)
{
	struct session *ses;

	push_call("quitmsg(%s)",message);

	while ((ses = gts->next) != NULL)
	{
		cleanup_session(ses);
	}

	check_all_events(gts, SUB_ARG|SUB_SEC, 0, 0, "PROGRAM TERMINATION");

	restore_terminal();

	if (message)
	{
		printf("\n%s\n", message);
	}
	else
	{
		printf("\nChromaTerm ended.\n");
	}

	fflush(NULL);

	pop_call();
	exit(0);
}
