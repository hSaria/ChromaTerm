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

#ifdef HAVE_SYS_IOCTL_H
  #include <sys/ioctl.h>
#endif
#include <termios.h>
#include <errno.h>

void init_terminal()
{
	struct termios io;

	if (tcgetattr(0, &gtd->old_terminal))
	{
		perror("tcgetattr");

		exit(errno);
	}

	io = gtd->old_terminal;

	/*
		Canonical mode off
	*/

	DEL_BIT(io.c_lflag, ICANON);

	io.c_cc[VMIN]   = 1;
	io.c_cc[VTIME]  = 0;
	io.c_cc[VSTART] = 255;
	io.c_cc[VSTOP]  = 255;

	/*
		Make the terminalal as raw as possible
	*/

/*
	DEL_BIT(io.c_iflag, IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON);
	DEL_BIT(io.c_oflag, OPOST);
	DEL_BIT(io.c_cflag, CSIZE|PARENB);
*/

	DEL_BIT(io.c_lflag, ECHO|ECHONL|IEXTEN|ISIG);

	SET_BIT(io.c_cflag, CS8);

	if (tcsetattr(0, TCSANOW, &io))
	{
		perror("tcsetattr");

		exit(errno);
	}

	if (tcgetattr(0, &gtd->new_terminal))
	{
		perror("tcgetattr");

		exit(errno);
	}
}

void restore_terminal(void)
{
	tcsetattr(0, TCSANOW, &gtd->old_terminal);
}

void refresh_terminal(void)
{
	tcsetattr(0, TCSANOW, &gtd->new_terminal);
}

void echo_off(struct session *ses)
{
	struct termios io;

	tcgetattr(STDIN_FILENO, &io);

	DEL_BIT(io.c_lflag, ECHO|ECHONL);

	tcsetattr(STDIN_FILENO, TCSADRAIN, &io);
}

void echo_on(struct session *ses)
{
	struct termios io;

	tcgetattr(STDIN_FILENO, &io);

	SET_BIT(io.c_lflag, ECHO|ECHONL);

	tcsetattr(STDIN_FILENO, TCSADRAIN, &io);
}

void init_screen_size(struct session *ses)
{
	struct winsize screen;

	if (ses == gts)
	{
		if (ioctl(0, TIOCGWINSZ, &screen) == -1)
		{
			ses->rows = SCREEN_HEIGHT;
			ses->cols = SCREEN_WIDTH;
		}
		else
		{
			ses->rows = screen.ws_row;
			ses->cols = screen.ws_col;
		}
	}
	else
	{
		ses->rows = gts->rows;
		ses->cols = gts->cols;
	}

	check_all_events(ses, SUB_ARG|SUB_SEC, 0, 2, "SCREEN RESIZE", ntos(ses->cols), ntos(ses->rows));
}

int get_scroll_size(struct session *ses)
{
	return ses->rows;
}
