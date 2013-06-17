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

int skip_vt102_codes(char *str)
{
	int skip;

	push_call("skip_vt102_codes(%p)",str);

	switch (str[0])
	{
		case   5:   /* ENQ */
		case   7:   /* BEL */
		case   8:   /* BS  */
	/*	case   9: *//* HT  */
	/*	case  10: *//* LF  */
		case  11:   /* VT  */
		case  12:   /* FF  */
		case  13:   /* CR  */
		case  14:   /* SO  */
		case  15:   /* SI  */
		case  17:   /* DC1 */
		case  19:   /* DC3 */
		case  24:   /* CAN */
		case  26:   /* SUB */
		case 127:   /* DEL */
			pop_call();
			return 1;

		case  27:   /* ESC */
			break;

		default:
			pop_call();
			return 0;
	}

	switch (str[1])
	{
		case '\0':
			pop_call();
			return 1;

		case '%':
		case '#':
		case '(':
		case ')':
			pop_call();
			return str[2] ? 3 : 2;

		case ']':
			switch (str[2])
			{
				case 'P':
					for (skip = 3 ; skip < 10 ; skip++)
					{
						if (str[skip] == 0)
						{
							break;
						}
					}
					pop_call();
					return skip;

				case 'R':
					pop_call();
					return 3;
			}
			pop_call();
			return 2;

		case '[':
			break;

		default:
			pop_call();
			return 2;
	}

	for (skip = 2 ; str[skip] != 0 ; skip++)
	{
		if (isalpha((int) str[skip]))
		{
			pop_call();
			return skip + 1;
		}

		switch (str[skip])
		{
			case '@':
			case '`':
			case ']':
				pop_call();
				return skip + 1;
		}
	}
	pop_call();
	return skip;
}

int skip_escaped_vt102_codes(char *str)
{
	int skip;

	if (str[0] != '\\' || str[1] != 'e')
	{
		return 0;
	}

	switch (str[2])
	{
		case '\0':
			return 2;

		case '%':
		case '#':
		case '(':
		case ')':
			return str[2] ? 4 : 3;

		case ']':
			switch (str[3])
			{
				case 'P':
					for (skip = 4 ; skip < 11 ; skip++)
					{
						if (str[skip] == 0)
						{
							break;
						}
					}
					return skip;

				case 'R':
					return 4;
			}
			return 3;

		case '[':
			break;

		default:
			return 3;
	}

	for (skip = 3 ; str[skip] != 0 ; skip++)
	{
		if (isalpha((int) str[skip]))
		{
			return skip + 1;
		}

		switch (str[skip])
		{
			case '@':
			case '`':
			case ']':
				return skip + 1;
		}
	}
	return skip;
}

int find_non_color_codes(char *str)
{
	int skip;

	switch (str[0])
	{
		case  27:   /* ESC */
			break;

		default:
			return 0;
	}

	switch (str[1])
	{
		case '[':
			break;

		default:
			return 0;
	}

	for (skip = 2 ; str[skip] != 0 ; skip++)
	{
		switch (str[skip])
		{
			case 'm':
				return skip + 1;
			case '@':
			case '`':
			case ']':
				return 0;
		}

		if (isalpha((int) str[skip]))
		{
			return 0;
		}
	}
	return 0;
}


int skip_vt102_codes_non_graph(char *str)
{
	int skip = 0;

	switch (str[skip])
	{
		case   5:   /* ENQ */
		case   7:   /* BEL */
	/*	case   8: *//* BS  */
	/*	case   9: *//* HT  */
	/*	case  10: *//* LF  */
		case  11:   /* VT  */
		case  12:   /* FF  */
		case  13:   /* CR  */
		case  14:   /* SO  */
		case  15:   /* SI  */
		case  17:   /* DC1 */
		case  19:   /* DC3 */
		case  24:   /* CAN */
		case  26:   /* SUB */
		case 127:   /* DEL */
			return 1;

		case  27:   /* ESC */
			break;

		default:
			return 0;
	}

	switch (str[1])
	{
		case '\0':
			return 0;

		case 'c':
		case 'D':
		case 'E':
		case 'H':
		case 'M':
		case 'Z':
		case '7':
		case '8':
		case '>':
		case '=':
			return 2;

		case '%':
		case '#':
		case '(':
		case ')':
			return str[2] ? 3 : 2;

		case ']':
			switch (str[2])
			{
				case 'P':
					for (skip = 3 ; skip < 10 ; skip++)
					{
						if (str[skip] == 0)
						{
							break;
						}
					}
					return skip;
				case 'R':
					return 3;
			}
			return 2;

		case '[':
			break;

		default:
			return 2;
	}

	for (skip = 2 ; str[skip] != 0 ; skip++)
	{
		switch (str[skip])
		{
			case 'm':
				return 0;
			case '@':
			case '`':
			case ']':
				return skip + 1;
		}

		if (isalpha((int) str[skip]))
		{
			return skip + 1;
		}
	}
	return 0;
}



void strip_vt102_codes(char *str, char *buf)
{
	char *pti, *pto;

	pti = (char *) str;
	pto = (char *) buf;

	while (*pti)
	{
		while (skip_vt102_codes(pti))
		{
			pti += skip_vt102_codes(pti);
		}

		if (*pti)
		{
			*pto++ = *pti++;
		}
	}
	*pto = 0;
}


void strip_vt102_codes_non_graph(char *str, char *buf)
{
	char *pti, *pto;

	pti = str;
	pto = buf;

	while (*pti)
	{
		while (skip_vt102_codes_non_graph(pti))
		{
			pti += skip_vt102_codes_non_graph(pti);
		}

		if (*pti)
		{
			*pto++ = *pti++;
		}
	}
	*pto = 0;
}

void strip_non_vt102_codes(char *str, char *buf)
{
	char *pti, *pto;
	int len;

	pti = str;
	pto = buf;

	while (*pti)
	{
		while ((len = skip_vt102_codes(pti)) != 0)
		{
			memcpy(pto, pti, len);
			pti += len;
			pto += len;
		}

		if (*pti)
		{
			pti++;
		}
	}
	*pto = 0;
}

// mix old and str, then copy compressed color string to buf which can point to old.

void get_color_codes(char *old, char *str, char *buf)
{
	char *pti, *pto, col[100], tmp[BUFFER_SIZE];
	int len, vtc, fgc, bgc, cnt;

	pto = tmp;

	pti = old;

	while (*pti)
	{
		while ((len = find_non_color_codes(pti)) != 0)
		{
			memcpy(pto, pti, len);
			pti += len;
			pto += len;
		}

		if (*pti)
		{
			pti++;
		}
	}

	pti = str;

	while (*pti)
	{
		while ((len = find_non_color_codes(pti)) != 0)
		{
			memcpy(pto, pti, len);
			pti += len;
			pto += len;
		}

		if (*pti)
		{
			pti++;
		}
	}

	*pto = 0;

	if (strlen(tmp) == 0)
	{
		buf[0] = 0;
		return;
	}

	vtc =  0;
	fgc = -1;
	bgc = -1;

	pti = tmp;

	while (*pti)
	{
		switch (*pti)
		{
			case 27:
				pti += 2;

				if (pti[-1] == 'm')
				{
					vtc =  0;
					fgc = -1;
					bgc = -1;
					break;
				}

				for (cnt = 0 ; pti[cnt] ; cnt++)
				{
					col[cnt] = pti[cnt];

					if (pti[cnt] == ';' || pti[cnt] == 'm')
					{
						col[cnt] = 0;

						cnt = -1;
						pti += 1 + strlen(col);

						if (HAS_BIT(vtc, COL_256) && (HAS_BIT(vtc, COL_XTF) || HAS_BIT(vtc, COL_XTB)))
						{
							if (HAS_BIT(vtc, COL_XTF))
							{
								fgc = URANGE(0, atoi(col), 255);
							}

							if (HAS_BIT(vtc, COL_XTB))
							{
								bgc = URANGE(0, atoi(col), 255);
							}
							DEL_BIT(vtc, COL_XTF|COL_XTB);
						}
						else
						{
							switch (atoi(col))
							{
								case 0:
									vtc = 0;
									fgc = -1;
									bgc = -1;
									break;
								case 1:
									SET_BIT(vtc, COL_BLD);
									break;
								case 4:
									SET_BIT(vtc, COL_UND);
									break;
								case 5:
									if (HAS_BIT(vtc, COL_XTF) || HAS_BIT(vtc, COL_XTB))
									{
										SET_BIT(vtc, COL_256);
									}
									else
									{
										SET_BIT(vtc, COL_BLK);
									}
									break;
								case 7:
									SET_BIT(vtc, COL_REV);
									break;
								case  2:
								case 21:
								case 22:
									DEL_BIT(vtc, COL_BLD);
									break;
								case 24:
									DEL_BIT(vtc, COL_UND);
									break;
								case 25:
									DEL_BIT(vtc, COL_UND);
									break;
								case 27:
									DEL_BIT(vtc, COL_BLK);
									break;
								case 38:
									DEL_BIT(vtc, COL_XTB);
									DEL_BIT(vtc, COL_256);
									SET_BIT(vtc, COL_XTF);
									fgc = -1;
									break;
								case 39:
									DEL_BIT(vtc, COL_UND);
									fgc = -1;
									break;
								case 48:
									DEL_BIT(vtc, COL_XTF);
									DEL_BIT(vtc, COL_256);
									SET_BIT(vtc, COL_XTB);
									bgc = -1;
									break;
								default:
									DEL_BIT(vtc, COL_256);

									/*
										Use 256 color's 16 color notation
									*/

									if (atoi(col) / 10 == 4)
									{
										bgc = atoi(col) % 10;
									}
									if (atoi(col) / 10 == 9)
									{
										bgc = atoi(col) % 10 + 8;
									}

									if (atoi(col) / 10 == 3)
									{
										fgc = atoi(col) % 10;
									}
									if (atoi(col) / 10 == 10)
									{
										fgc = atoi(col) % 10 + 8;
									}
									break;
							}
						}
					}

					if (pti[-1] == 'm')
					{
						break;
					}
				}
				break;

			default:
				pti++;
				break;
		}
	}

	strcpy(buf, "\033[0");

	if (HAS_BIT(vtc, COL_BLD))
	{
		strcat(buf, ";1");
	}
	if (HAS_BIT(vtc, COL_UND))
	{
		strcat(buf, ";4");
	}
	if (HAS_BIT(vtc, COL_BLK))
	{
		strcat(buf, ";5");
	}
	if (HAS_BIT(vtc, COL_REV))
	{
		strcat(buf, ";7");
	}

	if (fgc >= 16)
	{
		cat_sprintf(buf, ";38;5;%d", fgc);
	}
	else if (fgc >= 8)
	{
		cat_sprintf(buf, ";%d", fgc + 100);
	}
	else if (fgc >= 0)
	{
		cat_sprintf(buf, ";%d", fgc + 30);
	}

	if (bgc >= 16)
	{
		cat_sprintf(buf, ";48;5;%d", bgc);
	}
	else if (bgc >= 8)
	{
		cat_sprintf(buf, ";%d", fgc + 90);
	}
	else if (bgc >= 0)
	{
		cat_sprintf(buf, ";%d", bgc + 40);
	}

	strcat(buf, "m");
}

int strip_vt102_strlen(struct session *ses, char *str)
{
	char *pti;
	int i = 0;

	pti = str;

	while (*pti)
	{
		if (skip_vt102_codes(pti))
		{
			pti += skip_vt102_codes(pti);

			continue;
		}

		if (HAS_BIT(ses->flags, SES_FLAG_UTF8) && (*pti & 192) == 192)
		{
			pti++;

			while ((*pti & 192) == 128)
			{
				pti++;
			}
		}
		else
		{
			pti++;
		}
		i++;
	}
	return i;
}

int strip_color_strlen(struct session *ses, char *str)
{
	char *pti;
	int i = 0;

	pti = str;

	while (*pti)
	{
		if (pti[0] == '<' && isalnum((int) pti[1]) && isalnum((int) pti[2]) && isalnum((int) pti[3]) && pti[4] == '>')
		{
			pti += 5;
			continue;
		}

		if (skip_vt102_codes(pti))
		{
			pti += skip_vt102_codes(pti);
			continue;
		}

		if (skip_escaped_vt102_codes(pti))
		{
			pti += skip_escaped_vt102_codes(pti);
			continue;
		}

		if (HAS_BIT(ses->flags, SES_FLAG_UTF8) && (*pti & 192) == 192)
		{
			pti++;

			while ((*pti & 192) == 128)
			{
				pti++;
			}
		}
		else
		{
			pti++;
		}
		i++;
	}
	return i;
}
