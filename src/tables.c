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

struct command_type command_table[] =
{
	{    "action",            do_action,            TOKEN_TYPE_COMMAND },
	{    "alias",             do_alias,             TOKEN_TYPE_COMMAND },
	{    "all",               do_all,               TOKEN_TYPE_COMMAND },
	{    "break",             do_nop,               TOKEN_TYPE_BREAK   },
	{    "case",              do_nop,               TOKEN_TYPE_CASE    },
	{    "class",             do_class,             TOKEN_TYPE_COMMAND },
	{    "commands",          do_commands,          TOKEN_TYPE_COMMAND },
	{    "config",            do_configure,         TOKEN_TYPE_COMMAND },
	{    "continue",          do_nop,               TOKEN_TYPE_CONTINUE},
	{    "cursor",            do_cursor,            TOKEN_TYPE_COMMAND },
	{    "debug",             do_debug,             TOKEN_TYPE_COMMAND },
	{    "default",           do_nop,               TOKEN_TYPE_DEFAULT },
	{    "delay",             do_delay,             TOKEN_TYPE_COMMAND },
	{    "echo",              do_echo,              TOKEN_TYPE_COMMAND },
	{    "else",              do_nop,               TOKEN_TYPE_ELSE    },
	{    "elseif",            do_nop,               TOKEN_TYPE_ELSEIF  },
	{    "end",               do_end,               TOKEN_TYPE_COMMAND },
	{    "event",             do_event,             TOKEN_TYPE_COMMAND },
	{    "foreach",           do_nop,               TOKEN_TYPE_FOREACH },
	{    "format",            do_format,            TOKEN_TYPE_COMMAND },
	{    "function",          do_function,          TOKEN_TYPE_COMMAND },
	{    "gag",               do_gag,               TOKEN_TYPE_COMMAND },
	{    "help",              do_help,              TOKEN_TYPE_COMMAND },
	{    "highlight",         do_highlight,         TOKEN_TYPE_COMMAND },
	{    "if",                do_nop,               TOKEN_TYPE_IF      },
	{    "ignore",            do_ignore,            TOKEN_TYPE_COMMAND },
	{    "info",              do_info,              TOKEN_TYPE_COMMAND },
	{    "killall",           do_kill,              TOKEN_TYPE_COMMAND },
	{    "line",              do_line,              TOKEN_TYPE_COMMAND },
	{    "list",              do_list,              TOKEN_TYPE_COMMAND },
	{    "log",               do_log,               TOKEN_TYPE_COMMAND },
	{    "loop",              do_nop,               TOKEN_TYPE_LOOP    },
	{    "macro",             do_macro,             TOKEN_TYPE_COMMAND },
	{    "math",              do_math,              TOKEN_TYPE_COMMAND },
	{    "message",           do_message,           TOKEN_TYPE_COMMAND },
	{    "nop",               do_nop,               TOKEN_TYPE_COMMAND },
	{    "read",              do_read,              TOKEN_TYPE_COMMAND },
	{    "regexp",            do_regexp,            TOKEN_TYPE_REGEX   },
	{    "replace",           do_replace,           TOKEN_TYPE_COMMAND },
	{    "return",            do_nop,               TOKEN_TYPE_RETURN  },
	{    "run",               do_run,               TOKEN_TYPE_COMMAND },
	{    "script",            do_script,            TOKEN_TYPE_COMMAND },
	{    "send",              do_send,              TOKEN_TYPE_COMMAND },
	{    "session",           do_session,           TOKEN_TYPE_COMMAND },
	{    "showme",            do_showme,            TOKEN_TYPE_COMMAND },
	{    "snoop",             do_snoop,             TOKEN_TYPE_COMMAND },
	{    "substitute",        do_substitute,        TOKEN_TYPE_COMMAND },
	{    "switch",            do_nop,               TOKEN_TYPE_SWITCH  },
	{    "system",            do_system,            TOKEN_TYPE_COMMAND },
//	{    "test",              do_test,              TOKEN_TYPE_COMMAND },
	{    "ticker",            do_tick,              TOKEN_TYPE_COMMAND },
	{    "unaction",          do_unaction,          TOKEN_TYPE_COMMAND },
	{    "unalias",           do_unalias,           TOKEN_TYPE_COMMAND },
	{    "undelay",           do_undelay,           TOKEN_TYPE_COMMAND },
	{    "unevent",           do_unevent,           TOKEN_TYPE_COMMAND },
	{    "unfunction",        do_unfunction,        TOKEN_TYPE_COMMAND },
	{    "ungag",             do_ungag,             TOKEN_TYPE_COMMAND },
	{    "unhighlight",       do_unhighlight,       TOKEN_TYPE_COMMAND },
	{    "unmacro",           do_unmacro,           TOKEN_TYPE_COMMAND },
	{    "unsubstitute",      do_unsubstitute,      TOKEN_TYPE_COMMAND },
	{    "unticker",          do_untick,            TOKEN_TYPE_COMMAND },
	{    "unvariable",        do_unvariable,        TOKEN_TYPE_COMMAND },
	{    "variable",          do_variable,          TOKEN_TYPE_COMMAND },
	{    "while",             do_nop,               TOKEN_TYPE_WHILE   },
	{    "write",             do_write,             TOKEN_TYPE_COMMAND },
	{    "zap",               do_zap,               TOKEN_TYPE_COMMAND },
	{    "",                  NULL,                 TOKEN_TYPE_COMMAND }
};


struct list_type list_table[LIST_MAX] =
{
	{    "ACTION",            "ACTIONS",            PRIORITY,    3,  LIST_FLAG_SHOW|LIST_FLAG_MESSAGE|LIST_FLAG_READ|LIST_FLAG_WRITE|LIST_FLAG_CLASS|LIST_FLAG_INHERIT },
	{    "ALIAS",             "ALIASES",            PRIORITY,    3,  LIST_FLAG_SHOW|LIST_FLAG_MESSAGE|LIST_FLAG_READ|LIST_FLAG_WRITE|LIST_FLAG_CLASS|LIST_FLAG_INHERIT },
	{    "CLASS",             "CLASSES",            ALPHA,       2,  LIST_FLAG_SHOW|LIST_FLAG_MESSAGE|LIST_FLAG_READ|LIST_FLAG_INHERIT                                 },
	{    "CONFIG",            "CONFIGURATIONS",     ALPHA,       2,  LIST_FLAG_SHOW|LIST_FLAG_MESSAGE|LIST_FLAG_READ|LIST_FLAG_WRITE|LIST_FLAG_INHERIT                 },
	{    "DELAY",             "DELAYS",             ALPHA,       3,  LIST_FLAG_SHOW|LIST_FLAG_MESSAGE|LIST_FLAG_READ                                                   },
	{    "EVENT",             "EVENTS",             ALPHA,       2,  LIST_FLAG_SHOW|LIST_FLAG_MESSAGE|LIST_FLAG_READ|LIST_FLAG_WRITE|LIST_FLAG_CLASS|LIST_FLAG_INHERIT },
	{    "FUNCTION",          "FUNCTIONS",          ALPHA,       2,  LIST_FLAG_SHOW|LIST_FLAG_MESSAGE|LIST_FLAG_READ|LIST_FLAG_WRITE|LIST_FLAG_CLASS|LIST_FLAG_INHERIT },
	{    "GAG",               "GAGS",               ALPHA,       1,  LIST_FLAG_SHOW|LIST_FLAG_MESSAGE|LIST_FLAG_READ|LIST_FLAG_WRITE|LIST_FLAG_CLASS|LIST_FLAG_INHERIT },
	{    "HIGHLIGHT",         "HIGHLIGHTS",         PRIORITY,    3,  LIST_FLAG_SHOW|LIST_FLAG_MESSAGE|LIST_FLAG_READ|LIST_FLAG_WRITE|LIST_FLAG_CLASS|LIST_FLAG_INHERIT },
	{    "MACRO",             "MACROS",             ALPHA,       2,  LIST_FLAG_SHOW|LIST_FLAG_MESSAGE|LIST_FLAG_READ|LIST_FLAG_WRITE|LIST_FLAG_CLASS|LIST_FLAG_INHERIT },
	{    "SUBSTITUTE",        "SUBSTITUTIONS",      PRIORITY,    3,  LIST_FLAG_SHOW|LIST_FLAG_MESSAGE|LIST_FLAG_READ|LIST_FLAG_WRITE|LIST_FLAG_CLASS|LIST_FLAG_INHERIT },
	{    "TICKER",            "TICKERS",            ALPHA,       3,  LIST_FLAG_SHOW|LIST_FLAG_MESSAGE|LIST_FLAG_READ|LIST_FLAG_WRITE|LIST_FLAG_CLASS|LIST_FLAG_INHERIT },
	{    "VARIABLE",          "VARIABLES",          ALPHA,       2,  LIST_FLAG_SHOW|LIST_FLAG_MESSAGE|LIST_FLAG_READ|LIST_FLAG_WRITE|LIST_FLAG_CLASS|LIST_FLAG_INHERIT|LIST_FLAG_NEST }
};

struct substitution_type substitution_table[] =
{
//	{    "ARGUMENTS",            1  },
	{    "VARIABLES",            2  },
	{    "FUNCTIONS",            4  },
	{    "COLORS",               8  },
	{    "ESCAPES",             16  },
//	{    "COMMANDS",            32  },
	{    "SECURE",              64  },
	{    "EOL",                128  },
	{    "LNF",                256  },
//	{    "FIX",               1024  },
	{    "",                  0     }
};

struct config_type config_table[] =
{
	{
		"CHARSET",
		"",
		"The character set encoding used",
		config_charset
	},

	{
		"COLOR PATCH",
		"Properly color the start of each line",
		"Leave color handling to the server",
		config_colorpatch
	},

	{
		"COMMAND CHAR",
		"",
		"The character used for commands",
		config_commandchar
	},

	{
		"CONVERT META",
		"Convert meta and control characters",
		"Do not convert meta and control characters",
		config_convertmeta
	},

	{
		"LOCAL ECHO",
		"Line mode is enabled.",
		"Character mode is enabled.",
		config_localecho
	},

	{
		"LOG",
		"",
		"The data format of the log files",
		config_log
	},

	{
		"LOG LEVEL",
		"Log low level mud data",
		"Log high level mud data",
		config_loglevel
	},

	{
		"PACKET PATCH",
		"",
		"Seconds to wait to patch broken packets",
		config_packetpatch
	},


	{
		"VERBATIM",
		"Your keyboard input is send as is",
		"Your keyboard input is parsed",
		config_verbatim
	},

	{
		"VERBOSE",
		"Messages while reading in a script file are echoed",
		"Messages while reading in a script file are gagged",
		config_verbose
	},

	{
		"256 COLORS",
		"Your terminal is 256 color capable",
		"Your terminal is not 256 color capable",
		config_256color
	},

	{
		"",
		"",
		0,
		0
	}
};

struct color_type color_table[] =
{

	{    "azure",         "<abd>" },
	{    "ebony",         "<g04>" },
	{    "jade",          "<adb>" },
	{    "lime",          "<bda>" },
	{    "orange",        "<dba>" },
	{    "pink",          "<dab>" },
	{    "silver",        "<ccc>" },
	{    "tan",           "<cba>" },
	{    "violet",        "<bad>" },

	{    "light azure",   "<acf>" },
	{    "light ebony",   "<bbb>" },
	{    "light jade",    "<afc>" },
	{    "light lime",    "<cfa>" },
	{    "light orange",  "<fca>" },
	{    "light pink",    "<fac>" },
	{    "light silver",  "<eee>" },
	{    "light tan",     "<eda>" },
	{    "light violet",  "<caf>" },

	{    "reset",         "<088>" },
	{    "light",         "<188>" },
	{    "bold",          "<188>" },
	{    "faint",         "<288>" },
	{    "dim",           "<288>" },
	{    "dark",          "<288>" },
	{    "underscore",    "<488>" },
	{    "blink",         "<588>" },
	{    "reverse",       "<788>" },

	{    "black",         "<808>" },
	{    "red",           "<818>" },
	{    "green",         "<828>" },
	{    "yellow",        "<838>" },
	{    "blue",          "<848>" },
	{    "magenta",       "<858>" },
	{    "cyan",          "<868>" },
	{    "white",         "<878>" },

	{    "b black",       "<880>" },
	{    "b red",         "<881>" },
	{    "b green",       "<882>" },
	{    "b yellow",      "<883>" },
	{    "b blue",        "<884>" },
	{    "b magenta",     "<885>" },
	{    "b cyan",        "<886>" },
	{    "b white",       "<887>" },

	{    "b azure",       "<ABD>" },
	{    "b ebony",       "<G04>" },
	{    "b jade",        "<ADB>" },
	{    "b lime",        "<BDA>" },
	{    "b orange",      "<DBA>" },
	{    "b pink",        "<DAB>" },
	{    "b silver",      "<CCC>" },
	{    "b tan",         "<CBA>" },
	{    "b violet",      "<BAD>" },

	{    "",              "<888>" }
};

struct class_type class_table[] =
{
	{    "OPEN",              class_open             },
	{    "CLOSE",             class_close            },
	{    "READ",              class_read             },
	{    "WRITE",             class_write            },
	{    "KILL",              class_kill             },
	{    "",                  NULL                   },
};

struct array_type array_table[] =
{
	{     "ADD",              array_add           },
	{     "CLEAR",            array_clear         },
	{     "CREATE",           array_create        },
	{     "DELETE",           array_delete        },
	{     "FIND",             array_find          },
	{     "GET",              array_get           },
	{     "INSERT",           array_insert        },
	{     "LENGTH",           array_size          },
	{     "SET",              array_set           },
	{     "SIZE",             array_size          },
	{     "SORT",             array_sort          },
	{     "TOKENIZE",         array_tokenize      },
	{     "",                 NULL                }
};

struct cursor_type cursor_table[] =
{
	{
		"BACKSPACE",
		"Delete backward character",
		"",
		cursor_backspace
	},
	{
		"BACKWARD",
		"Move cursor backward",
		"",
		cursor_left
	},
	{
		"CLEAR LEFT",
		"Delete from cursor to start of input",
		"",
		cursor_clear_left
	},
	{
		"CLEAR LINE",
		"Delete the input line",
		"",
		cursor_clear_line
	},
	{
		"CLEAR RIGHT",
		"Delete from cursor to end of input",
		"",
		cursor_clear_right
	},
	{
		"CONVERT META",
		"Meta convert the next character",
		"",
		cursor_convert_meta
	},
	{
		"CTRL DELETE",
		"Delete one character, exit on an empty line",
		"",
		cursor_delete_or_exit
	},
	{
		"DELETE",
		"Delete character at cursor",
		"[3~",
		cursor_delete
	},
	{
		"DELETE WORD LEFT",
		"Delete backwards till next space",
		"",
		cursor_delete_word_left
	},
	{
		"DELETE WORD RIGHT",
		"Delete forwards till next space",
		"",
		cursor_delete_word_right
	},
	{
		"END",
		"Move cursor to end of input",
		"",
		cursor_end
	},
	{
		"ENTER",
		"Process the input line",
		"",
		cursor_enter
	},
	{
		"EXIT",
		"Exit current session",
		"",
		cursor_exit
	},
	{
		"FORWARD",
		"Move cursor forward",
		"",
		cursor_right
	},
	{
		"HOME",
		"Move the cursor to start of input",
		"",
		cursor_home
	},
	{
		"INSERT",
		"Turn insert mode on or off",
		"",
		cursor_insert
	},
	{
		"NEXT WORD",
		"Move cursor to the next word",
		"f",
		cursor_right_word
	},
	{
		"PASTE BUFFER",
		"Paste the previously deleted input text",
		"",
		cursor_paste_buffer
	},
	{
		"PREV WORD",
		"Move cursor to the previous word",
		"b",
		cursor_left_word
	},
	{
		"REDRAW INPUT",
		"Redraw the input line",
		"",
		cursor_redraw_input
	},
	{
		"SUSPEND",
		"Suspend program, return with fg",
		"",
		cursor_suspend
	},
	{
		"TEST",
		"Print debugging information",
		"",
		cursor_test
	},

	{
		"", "", "OM",    cursor_enter
	},
	{
		"", "", "[7~",   cursor_home
	},
	{
		"", "", "[1~",   cursor_home
	},
	{
		"", "", "OH",    cursor_home
	},
	{
		"", "", "[H",    cursor_home
	},
	{
		"", "", "OD",    cursor_left
	},
	{
		"", "", "[D",    cursor_left
	},
	{
		"", "", "[8~",   cursor_end
	},
	{
		"", "", "[4~",   cursor_end
	},
	{
		"", "", "OF",    cursor_end
	},
	{
		"", "", "[F",    cursor_end
	},
	{
		"", "", "OC",    cursor_right
	},
	{
		"", "", "[C",    cursor_right
	},
	{
		"", "", "",      cursor_backspace
	},
	{
		"", "", "",     cursor_delete_word_left
	},
	{
		"", "", "d",     cursor_delete_word_right
	},
	{
		"",
		"",
		"",
		NULL
	}
};

struct timer_type timer_table[] =
{
	{    "Poll Stdin"                  },
	{    "Poll Sessions"               },
	{    "Update Tickers"              },
	{    "Update Delays"               },
	{    "Update Packet Patcher"       },
	{    "Update Terminal"             },
	{    "Update Time Events"          },
	{    "Update Memory"               },
	{    "Stall Program"               }
};

struct event_type event_table[] =
{
	{    "DATE",                                   "Triggers on the given date."             },
	{    "DAY",                                    "Triggers each day or given day."         },
	{    "HOUR",                                   "Triggers each hour or given hour."       },
	{    "MINUTE",                                 "Triggers each minute or given minute."   },
	{    "MONTH",                                  "Triggers each month or given month."     },
	{    "PROGRAM START",                          "Triggers when main session starts."      },
	{    "PROGRAM TERMINATION",                    "Triggers when main session exists."      },
	{    "RECEIVED INPUT",                         "Triggers when new input is received."    },
	{    "RECEIVED LINE",                          "Triggers when a new line is received."   },
	{    "RECEIVED OUTPUT",                        "Triggers when new output is received."   },
	{    "RECEIVED PROMPT",                        "Triggers when a prompt is received."     },
	{    "SCREEN RESIZE",                          "Triggers when the screen is resized."    },
	{    "SECOND",                                 "Trigers each second or given second."    },
	{    "SEND OUTPUT",                            "Triggers when sending output."           },
	{    "SESSION ACTIVATED",                      "Triggers when a session is activated."   },
	{    "SESSION CONNECTED",                      "Triggers when a new session connects."   },
	{    "SESSION DEACTIVATED",                    "Triggers when a session is deactivated." },
	{    "SESSION DISCONNECTED",                   "Triggers when a session disconnects."    },
	{    "TIME",                                   "Triggers on the given time."             },
	{    "VARIABLE UPDATE ",                       "Triggers on a variable update."          },
	{    "WEEK",                                   "Triggers each week or given week."       },
	{    "YEAR",                                   "Triggers each year or given year."       },
	{    "",                                       ""                                        }
};


struct line_type line_table[] =
{
	{    "GAG",               line_gag               },
	{    "IGNORE",            line_ignore            },
	{    "LOG",               line_log               },
	{    "LOGVERBATIM",       line_logverbatim       },
	{    "STRIP",             line_strip             },
	{    "SUBSTITUTE",        line_substitute        },
	{    "VERBOSE",           line_verbose           },
	{    "",                  NULL                   }
};

