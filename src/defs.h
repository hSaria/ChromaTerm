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
*                                                                             *
*              (T)he K(I)cki(N) (T)ickin D(I)kumud Clie(N)t                   *
*                                                                             *
*                        coded by peter unold 1992                            *
*                       modified by Bill Reiss 1993                           *
*                    updated by Igor van den Hoven 2004                       *
******************************************************************************/

#include <stdio.h>
#include <zlib.h>
#include <signal.h>
#include <ctype.h>
#include <stdarg.h>
#include <termios.h>
#include <pcre.h>

/******************************************************************************
*   Autoconf patching by David Hedbor                                         *
*******************************************************************************/

#include "config.h"

#if defined(HAVE_STRING_H)
#include <string.h>
#elif defined(HAVE_STRINGS_H)
#include <strings.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_TIME_H
#include <time.h>
#endif

#ifdef TIME_WITH_SYS_TIME
#include <sys/time.h>
#endif

#ifdef SOCKS
#include <socks.h>
#endif

#ifndef BADSIG
#define BADSIG (RETSIGTYPE (*)(int))-1
#endif

#ifdef HAVE_NET_ERRNO_H
#include <net/errno.h>
#endif

#ifndef __DEFS_H__
#define __DEFS_H__

/*
	Typedefs
*/

/*
	A bunch of constants
*/

#define FALSE                            0
#define TRUE                             1

#define GET_ONE                          0 /* stop at spaces */
#define GET_ALL                          1 /* stop at semicolon */
#define GET_NST                          2 /* nest square brackets */
#define GET_VBT                          4 /* ignore semicolon for verbatim mode */

#define SCREEN_WIDTH                    80
#define SCREEN_HEIGHT                   24

#define PRIORITY                         0
#define ALPHA                            1
#define APPEND                           2

#define DEFAULT_OPEN                   '{'
#define DEFAULT_CLOSE                  '}'

#define COMMAND_SEPARATOR              ';'

#define STRING_SIZE                  45000
#define BUFFER_SIZE                  20000
#define NUMBER_SIZE                    100
#define LIST_SIZE                        2

#define CLIENT_NAME          "ChromaTerm++"
#define CLIENT_VERSION           "0.01.0  "

#define ESCAPE                          27

#define TIMER_POLL_INPUT                 0
#define TIMER_POLL_SESSIONS              1
#define TIMER_UPDATE_TICKS               2
#define TIMER_UPDATE_DELAYS              3
#define TIMER_UPDATE_PACKETS             4
#define TIMER_UPDATE_TERMINAL            5
#define TIMER_UPDATE_TIME                6
#define TIMER_UPDATE_MEMORY              7
#define TIMER_STALL_PROGRAM              8
#define TIMER_CPU                        9


#define PULSE_PER_SECOND                20

#define PULSE_POLL_INPUT                 1
#define PULSE_POLL_SESSIONS              1
#define PULSE_UPDATE_TICKS               1
#define PULSE_UPDATE_DELAYS              1
#define PULSE_UPDATE_PACKETS             2
#define PULSE_UPDATE_TERMINAL            1
#define PULSE_UPDATE_MEMORY              2
#define PULSE_UPDATE_TIME               20

/*
	Index for lists
*/


#define LIST_ACTION                      0
#define LIST_ALIAS                       1
#define LIST_CLASS                       2
#define LIST_CONFIG                      3
#define LIST_DELAY                       4
#define LIST_EVENT                       5
#define LIST_FUNCTION                    6
#define LIST_GAG                         7
#define LIST_HIGHLIGHT                   8
#define LIST_MACRO                       9
#define LIST_SUBSTITUTE                 10
#define LIST_TICKER                     11
#define LIST_VARIABLE                   12
#define LIST_MAX                        13

/*
	Command type
*/

enum operators
{
	TOKEN_TYPE_BREAK,
	TOKEN_TYPE_CASE,
	TOKEN_TYPE_COMMAND,
	TOKEN_TYPE_CONTINUE,
	TOKEN_TYPE_DEFAULT,
	TOKEN_TYPE_END,
	TOKEN_TYPE_ELSE,
	TOKEN_TYPE_ELSEIF,
	TOKEN_TYPE_FOREACH,
	TOKEN_TYPE_BROKEN_FOREACH,
	TOKEN_TYPE_IF,
	TOKEN_TYPE_LOOP,
	TOKEN_TYPE_BROKEN_LOOP,
	TOKEN_TYPE_PARSE,
	TOKEN_TYPE_BROKEN_PARSE,
	TOKEN_TYPE_REGEX,
	TOKEN_TYPE_RETURN,
	TOKEN_TYPE_SESSION,
	TOKEN_TYPE_STRING,
	TOKEN_TYPE_SWITCH,
	TOKEN_TYPE_WHILE,
	TOKEN_TYPE_BROKEN_WHILE
};


/*
	generic define for show_message
*/

#define LIST_MESSAGE                    -1


/*
	Various flags
*/

#define COL_BLD                       (1 << 1)
#define COL_UND                       (1 << 2)
#define COL_BLK                       (1 << 3)
#define COL_REV                       (1 << 4)
#define COL_XTF                       (1 << 5)
#define COL_XTB                       (1 << 6)
#define COL_256                       (1 << 7)

#define SUB_NONE                      0
#define SUB_ARG                       (1 <<  0)
#define SUB_VAR                       (1 <<  1)
#define SUB_FUN                       (1 <<  2)
#define SUB_COL                       (1 <<  3)
#define SUB_ESC                       (1 <<  4)
#define SUB_CMD                       (1 <<  5)
#define SUB_SEC                       (1 <<  6)
#define SUB_EOL                       (1 <<  7)
#define SUB_LNF                       (1 <<  8)
#define SUB_FIX                       (1 <<  9)
#define SUB_CMP                       (1 << 10)

#define GLOBAL_FLAG_RESETBUFFER       (1 <<  0) /* Unused */
#define GLOBAL_FLAG_CONVERTMETACHAR   (1 <<  1)
#define GLOBAL_FLAG_HISTORYBROWSE     (1 <<  2) // Unused 
#define GLOBAL_FLAG_HISTORYSEARCH     (1 <<  3) // Unused
#define GLOBAL_FLAG_PROCESSINPUT      (1 <<  4)
#define GLOBAL_FLAG_USERCOMMAND       (1 <<  5) /* Unused */
#define GLOBAL_FLAG_INSERTINPUT       (1 <<  6)
#define GLOBAL_FLAG_VERBATIM          (1 <<  7) /* Unused */
#define GLOBAL_FLAG_TERMINATE         (1 <<  8) // Unused

#define SES_FLAG_LOCALECHO            (1 <<  1)
#define SES_FLAG_SNOOP                (1 <<  2)
#define SES_FLAG_MCCP                 (1 <<  3) //
#define SES_FLAG_MAPPING              (1 <<  4) //
#define SES_FLAG_SPLIT                (1 <<  5) //
#define SES_FLAG_SPEEDWALK            (1 <<  6) //
#define SES_FLAG_READMUD              (1 <<  8)
#define SES_FLAG_WORDWRAP             (1 <<  9) //
#define SES_FLAG_VERBATIM             (1 << 10) 
#define SES_FLAG_CONNECTED            (1 << 11)
#define SES_FLAG_REPEATENTER          (1 << 12) //
#define SES_FLAG_VERBOSE              (1 << 13)
#define SES_FLAG_VERBOSELINE          (1 << 14)
#define SES_FLAG_LOGLEVEL             (1 << 15)
#define SES_FLAG_LOGPLAIN             (1 << 16)
#define SES_FLAG_LOGHTML              (1 << 17)
#define SES_FLAG_GAG                  (1 << 18)
#define SES_FLAG_UPDATEVTMAP          (1 << 19) //
#define SES_FLAG_COLORPATCH           (1 << 20)
#define SES_FLAG_SCROLLLOCK           (1 << 21) //
#define SES_FLAG_SCAN                 (1 << 22) //
#define SES_FLAG_SCROLLSTOP           (1 << 23)
#define SES_FLAG_CONVERTMETA          (1 << 24)
#define SES_FLAG_RUN                  (1 << 25)
#define SES_FLAG_UTF8                 (1 << 26)
#define SES_FLAG_BIG5                 (1 << 27)
#define SES_FLAG_256COLOR             (1 << 28)
#define SES_FLAG_IGNORELINE           (1 << 29)

#define LIST_FLAG_IGNORE              (1 <<  0)
#define LIST_FLAG_MESSAGE             (1 <<  1)
#define LIST_FLAG_DEBUG               (1 <<  2)
#define LIST_FLAG_LOG                 (1 <<  3)
#define LIST_FLAG_CLASS               (1 <<  4)
#define LIST_FLAG_READ                (1 <<  5)
#define LIST_FLAG_WRITE               (1 <<  6)
#define LIST_FLAG_SHOW                (1 <<  7)
#define LIST_FLAG_INHERIT             (1 <<  8)
#define LIST_FLAG_NEST                (1 <<  9)
#define LIST_FLAG_DEFAULT             LIST_FLAG_MESSAGE

#define NODE_FLAG_META                (1 <<  0)
#define NODE_FLAG_PCRE                (1 <<  1)
#define NODE_FLAG_VARS                (1 <<  2)

#define TELOPT_FLAG_SGA               (1 <<  0)
#define TELOPT_FLAG_ECHO              (1 <<  1)

/*
	Some macros to deal with double linked lists
*/

#define LINK(link, head, tail) \
{ \
	if ((head) == NULL) \
	{ \
		(head) = (link); \
	} \
	else \
	{ \
		(tail)->next = (link); \
	} \
	(link)->next = NULL; \
	(link)->prev = (tail); \
	(tail)				    = (link); \
}

#define UNLINK(link, head, tail) \
{ \
	if (((link)->prev == NULL && (link) != head) \
	||  ((link)->next == NULL && (link) != tail)) \
	{ \
		display_printf2(NULL, "#UNLINK ERROR in file %s on line %d", __FILE__, __LINE__); \
		dump_stack(); \
	} \
	if ((link)->prev == NULL) \
	{ \
		(head)			   = (link)->next; \
	} \
	else \
	{ \
		(link)->prev->next	  = (link)->next; \
	} \
	if ((link)->next == NULL) \
	{ \
		(tail)			    = (link)->prev; \
	} \
	else \
	{ \
		(link)->next->prev	  = (link)->prev; \
	} \
	(link)->next = NULL; \
	(link)->prev = NULL; \
}

/*
	string allocation
*/

#define RESTRING(point, value) \
{ \
	free(point); \
	point = strdup((value)); \
}

#define STRFREE(point) \
{ \
	free((point)); \
	point = NULL; \
}

/*
	Bit operations
*/

#define HAS_BIT(bitvector, bit)   ((bitvector)  & (bit))
#define SET_BIT(bitvector, bit)   ((bitvector) |= (bit))
#define DEL_BIT(bitvector, bit)   ((bitvector) &= (~(bit)))
#define TOG_BIT(bitvector, bit)   ((bitvector) ^= (bit))

/*
	Generic
*/

#define URANGE(a, b, c)           ((b) < (a) ? (a) : (b) > (c) ? (c) : (b))
#define UMAX(a, b)                ((a) > (b) ? (a) : (b))
#define UMIN(a, b)                ((a) < (b) ? (a) : (b))

#define up(u)                     (u < 99 ? u++ : u)

#define VERBATIM(ses)             ((ses)->input_level == 0 && HAS_BIT((ses)->flags, SES_FLAG_VERBATIM))

#define DO_ARRAY(array) struct session *array (struct session *ses, struct listnode *list, char *arg)
#define DO_CLASS(class) struct session *class (struct session *ses, char *left, char *right)
#define DO_COMMAND(command) struct session  *command (struct session *ses, char *arg)
#define DO_CONFIG(config) struct session *config (struct session *ses, char *arg, int index)
#define DO_LINE(line) struct session *line (struct session *ses, char *arg)
#define DO_CURSOR(cursor) void cursor (char *arg)





/*
	Compatibility
*/


#define atoll(str) (strtoll(str, NULL, 10))


/************************ structures *********************/

struct listroot
{
	struct listnode  ** list;
	struct session    * ses;
	int                 size;
	int                 used;
	int                 type;
	int                 update;
	int                 flags;
};


struct listnode
{
	struct listroot   * root;
	char              * left;
	char              * right;
	char              * pr;
	char              * group;
	pcre              * regex;
	long long           data;
	short               flags;
};


struct session
{
	struct session        * next;
	struct session        * prev;
	char                  * name;
	char                  * group;
	char                  * command;
	FILE                  * logfile;
	FILE                  * logline;
	struct listroot       * list[LIST_MAX];
	int                     rows;
	int                     cols;
	int                     cur_row;
	int                     sav_row;
	int                     cur_col;
	int                     sav_col;
	int                     fgc;
	int                     bgc;
	int                     vtc;
	int                     pid;
	int                     socket;
	int                     flags;
	int                     debug_level;
	int                     input_level;
	char                    more_output[BUFFER_SIZE * 2];
	char                    color[100];
	long long               check_output;
};


struct global_data
{
	struct session        * ses;
	struct session        * update;
	struct session        * dispose_next;
	struct session        * dispose_prev;
	struct termios          old_terminal;
	struct termios          new_terminal;
	char                  * mud_output_buf;
	int                     mud_output_max;
	int                     mud_output_len;
	char                    input_buf[BUFFER_SIZE];
	char                    input_tmp[BUFFER_SIZE];
	char                    macro_buf[BUFFER_SIZE];
	char                    paste_buf[BUFFER_SIZE];
	int                     input_off;
	int                     input_len;
	int                     input_cur;
	int                     input_pos;
	int                     input_hid;
	char                  * term;
	long long               time;
	long long               timer[TIMER_CPU][5];
	long long               total_io_ticks;
	long long               total_io_exec;
	long long               total_io_delay;
	int                     command_ref[26];
	int                     flags;
	int                     quiet;
	char                    command_char;
	char                  * vars[100];
	char                  * cmds[100];
	int                     args[100];
};

struct link_data
{
	struct link_data     * next;
	struct link_data     * prev;
	char                 * str1;
	char                 * str2;
	char                 * str3;
};

/*
	Typedefs
*/

typedef struct session *ARRAY   (struct session *ses, struct listnode *list, char *arg);
typedef struct session *CLASS   (struct session *ses, char *left, char *right);
typedef struct session *CONFIG  (struct session *ses, char *arg, int index);
typedef struct session *COMMAND (struct session *ses, char *arg);
typedef void            CURSOR  (char *arg);
typedef struct session *LINE    (struct session *ses, char *arg);

/*
	Structures for tables.c
*/

struct array_type
{
	char                  * name;
	ARRAY                 * array;
	int                     lval;
	int                     rval;
};

struct class_type
{
	char                  * name;
	CLASS                 * group;
};

struct color_type
{
	char                  * name;
	char                  * code;
};

struct command_type
{
	char                  * name;
	COMMAND               * command;
	int                     type;
};

struct config_type
{
	char                  * name;
	char                  * msg_on;
	char                  * msg_off;
	CONFIG                * config;
};

struct event_type
{
	char                  * name;
	char                  * desc;
};

struct list_type
{
	char                  * name;
	char                  * name_multi;
	int                     mode;
	int                     args;
	int                     flags;
};

struct substitution_type
{
	char                  * name;
	int                     bitvector;
};

struct cursor_type
{
	char                  * name;
	char                  * desc;
	char                  * code;
	CURSOR                * fun;
};

struct timer_type
{
	char                  * name;
};

struct line_type
{
	char                  * name;
	LINE                  * fun;
};

#endif


/*
	Function declarations
*/

#ifndef __ACTION_H__
#define __ACTION_H__

extern struct session *do_action(struct session *ses, char *arg);
extern struct session *do_unaction(struct session *ses, char *arg);

extern void check_all_actions(struct session *ses, char *original, char *line);

#endif

#ifndef __CURSOR_H__
#define __CURSOR_H__

extern DO_COMMAND(do_cursor);

extern DO_CURSOR(cursor_backspace);
extern DO_CURSOR(cursor_check_line);
extern DO_CURSOR(cursor_check_line_modified);
extern DO_CURSOR(cursor_clear_left);
extern DO_CURSOR(cursor_clear_line);
extern DO_CURSOR(cursor_clear_right);
extern DO_CURSOR(cursor_convert_meta);
extern DO_CURSOR(cursor_delete);
extern DO_CURSOR(cursor_delete_or_exit);
extern DO_CURSOR(cursor_delete_word_left);
extern DO_CURSOR(cursor_delete_word_right);
extern DO_CURSOR(cursor_echo_on);
extern DO_CURSOR(cursor_echo_off);
extern DO_CURSOR(cursor_end);
extern DO_CURSOR(cursor_enter);
extern DO_CURSOR(cursor_exit);
extern DO_CURSOR(cursor_home);
extern DO_CURSOR(cursor_insert);
extern DO_CURSOR(cursor_left);
extern DO_CURSOR(cursor_left_word);
extern DO_CURSOR(cursor_paste_buffer);
extern DO_CURSOR(cursor_redraw_input);
extern DO_CURSOR(cursor_redraw_line);
extern DO_CURSOR(cursor_right);
extern DO_CURSOR(cursor_right_word);
extern DO_CURSOR(cursor_suspend);
extern DO_CURSOR(cursor_test);


#endif

#ifndef __INPUT_H__
#define __INPUT_H__

extern void process_input(void);
extern void read_line();
extern void read_key(void);
extern void convert_meta(char *input, char *output);
extern void unconvert_meta(char *input, char *output);
extern void echo_command(struct session *ses, char *line);
extern void input_printf(char *format, ...);
extern void modified_input(void);

#endif

#ifndef __ARRAY_H__
#define __ARRAY_H__

extern DO_COMMAND(do_list);
extern DO_ARRAY(array_add);
extern DO_ARRAY(array_clear);
extern DO_ARRAY(array_create);
extern DO_ARRAY(array_insert);
extern DO_ARRAY(array_delete);
extern DO_ARRAY(array_find);
extern DO_ARRAY(array_get);
extern DO_ARRAY(array_size);
extern DO_ARRAY(array_set);
extern DO_ARRAY(array_sort);
extern DO_ARRAY(array_tokenize);

#endif

#ifndef __TERMINAL_H__
#define __TERMINAL_H__

extern void init_terminal(void);
extern void restore_terminal(void);
extern void refresh_terminal(void);

extern void echo_on(struct session *ses);
extern void echo_off(struct session *ses);
extern void init_screen_size(struct session *ses);
extern int get_scroll_size(struct session *ses);

#endif

#ifndef __CLASS_H__
#define __CLASS_H__

extern DO_COMMAND(do_class);
extern int count_class(struct session *ses, struct listnode *group);
extern DO_CLASS(class_open);
extern DO_CLASS(class_close);
extern DO_CLASS(class_read);
extern DO_CLASS(class_write);
extern DO_CLASS(class_kill);
extern void parse_class(struct session *ses, char *input, struct listnode *group);

#endif

#ifndef __MATH_H__
#define __MATH_H__

extern DO_COMMAND(do_math);
extern double get_number(struct session *ses, char *str);
extern void get_number_string(struct session *ses, char *str, char *result);
extern double mathswitch(struct session *ses, char *left, char *right);
extern void mathexp(struct session *ses, char *str, char *result);
extern int mathexp_tokenize(struct session *ses, char *str);
extern void mathexp_level(struct session *ses, struct link_data *node);
extern void mathexp_compute(struct session *ses, struct link_data *node);
extern double tintoi(char *str);
extern double tincmp(char *left, char *right);
extern double tineval(struct session *ses, char *left, char *right);
extern double tindice(char *left, char *right);

#endif

#ifndef __TINEXP_H__
#define __TINEXP_H__

extern int substitute(struct session *ses, char *string, char *result, int flags);

extern int match(struct session *ses, char *str, char *exp, int flags);
extern int find(struct session *ses, char *str, char *exp, int sub);
DO_COMMAND(do_regexp);
extern int regexp_compare(pcre *regex, char *str, char *exp, int option, int flag);
extern int check_one_regexp(struct session *ses, struct listnode *node, char *line, char *original, int option);
extern int regexp_check(struct session *ses, char *exp);
extern int regexp(struct session *ses, pcre *pcre, char *str, char *exp, int option, int flag);
extern pcre *regexp_precompile(struct session *ses, struct listnode *node, char *exp, int option);

#endif


#ifndef __CONFIG_H__
#define __CONFIG_H__

extern DO_COMMAND(do_configure);

extern DO_CONFIG(config_verbatim);
extern DO_CONFIG(config_verbose);
extern DO_CONFIG(config_log);
extern DO_CONFIG(config_packetpatch);
extern DO_CONFIG(config_commandchar);
extern DO_CONFIG(config_convertmeta);
extern DO_CONFIG(config_loglevel);
extern DO_CONFIG(config_colorpatch);
extern DO_CONFIG(config_charset);
extern DO_CONFIG(config_256color);
extern DO_CONFIG(config_localecho);

#endif


#ifndef __MACRO_H__
#define __MACRO_H__

extern DO_COMMAND(do_macro);
extern DO_COMMAND(do_unmacro);
extern void macro_update(void);

#endif

#ifndef __DATA_H__
#define __DATA_H__

extern struct listroot *init_list(struct session *ses, int type, int size);
extern void kill_list(struct listroot *root);
extern void free_list(struct listroot *root);
extern struct listroot *copy_list(struct session *ses, struct listroot *sourcelist, int type);

extern struct listnode *insert_node_list(struct listroot *root, char *ltext, char *rtext, char *prtext);
extern struct listnode *update_node_list(struct listroot *root, char *ltext, char *rtext, char *prtext);
extern struct listnode *insert_index_list(struct listroot *root, struct listnode *node, int index);

extern  int show_node_with_wild(struct session *ses, char *cptr, int type);
extern void show_node(struct listroot *root, struct listnode *node, int level);
extern void show_nest(struct listnode *node, char *result);
extern void show_list(struct listroot *root, int level);

extern struct listnode *search_node_list(struct listroot *root, char *text);

extern void delete_node_list(struct session *ses, int type, struct listnode *node);
extern void delete_node_with_wild(struct session *ses, int index, char *string);

extern void delete_index_list(struct listroot *root, int index);
extern  int search_index_list(struct listroot *root, char *text, char *priority);
extern  int locate_index_list(struct listroot *root, char *text, char *priority);


extern int bsearch_alpha_list(struct listroot *root, char *text, int seek);
extern int bsearch_priority_list(struct listroot *root, char *text, char *priority, int seek);
extern int nsearch_list(struct listroot *root, char *text);

extern DO_COMMAND(do_kill);
extern DO_COMMAND(do_message);
extern DO_COMMAND(do_ignore);
extern DO_COMMAND(do_debug);
#endif

#ifndef __DEBUG_H__
#define __DEBUG_H__

extern int push_call(char *f, ...);
extern void pop_call(void);
extern void dump_stack(void);
extern void dump_full_stack(void);

#endif

#ifndef __EVENT_H__
#define __EVENT_H__

extern DO_COMMAND(do_event);
extern DO_COMMAND(do_unevent);
extern int check_all_events(struct session *ses, int flags, int args, int vars, char *fmt, ...);

#endif

#ifndef __ALIAS_H__
#define __ALIAS_H__

extern struct session *do_alias(struct session *ses, char *arg);
extern struct session *do_unalias(struct session *ses, char *arg);
int                    check_all_aliases(struct session *ses, char *input);

#endif


#ifndef __FILES_H__
#define __FILES_H__

extern DO_COMMAND(do_read);
extern DO_COMMAND(do_write);

extern void write_node(struct session *ses, int mode, struct listnode *node, FILE *file);

#endif 

#ifndef __FUNCTION_H__
#define __FUNCTION_H__

extern DO_COMMAND(do_function);
extern DO_COMMAND(do_unfunction);

#endif


#ifndef __GAG_H__
#define __GAG_H__

extern DO_COMMAND(do_gag);
extern DO_COMMAND(do_ungag);
extern void check_all_gags(struct session *ses, char *original, char *line);

#endif


#ifndef __HELP_H__
#define __HELP_H__

extern DO_COMMAND(do_help);

#endif

#ifndef __HIGHLIGHT_H__
#define __HIGHLIGHT_H__

extern DO_COMMAND(do_highlight);
extern int is_high_arg(char *s);
extern DO_COMMAND(do_unhighlight);
extern void check_all_highlights(struct session *ses, char *original, char *line);
extern int get_highlight_codes(struct session *ses, char *htype, char *result);

#endif

#ifndef __LINE_H__
#define __LINE_H__
extern DO_COMMAND(do_line);
extern DO_LINE(line_gag);
extern DO_LINE(line_ignore);
extern DO_LINE(line_log);
extern DO_LINE(line_logverbatim);
extern DO_LINE(line_strip);
extern DO_LINE(line_substitute);
extern DO_LINE(line_verbose);

#endif


#ifndef __LOG_H__
#define __LOG_H__

extern void logit(struct session *ses, char *txt, FILE *file, int newline);
extern DO_COMMAND(do_log);
extern void write_html_header(FILE *fp);
extern void vt102_to_html(struct session *ses, char *txt, char *out);
#endif


#ifndef __MAIN_H__
#define __MAIN_H__

extern struct session *gts;
extern struct global_data *gtd;


extern void winch_handler(int signal);
extern void abort_handler(int signal);
extern void pipe_handler(int signal);
extern void suspend_handler(int signal);
extern void trap_handler(int signal);

extern int main(int argc, char **argv);
extern void init_program(int greeting);
extern void quitmsg(char *message);

#endif


#ifndef __MEMORY_H__
#define __MEMORY_H__

extern char *restring(char *point, char *string);
extern char *refstring(char *point, char *fmt, ...);

#endif

#ifndef __MISC_H__
#define __MISC_H__

extern DO_COMMAND(do_all);
extern DO_COMMAND(do_commands);
extern DO_COMMAND(do_echo);
extern DO_COMMAND(do_end);
extern DO_COMMAND(do_info);
extern DO_COMMAND(do_nop);
extern DO_COMMAND(do_run);
extern DO_COMMAND(do_send);
extern DO_COMMAND(do_showme);
extern DO_COMMAND(do_snoop);
extern DO_COMMAND(do_suspend);
extern DO_COMMAND(do_test);
extern DO_COMMAND(do_zap);

#endif

#ifndef __NEST_H__
#define __NEST_H__

extern struct listroot *search_nest_root(struct listroot *root, char *arg);
extern struct listnode *search_base_node(struct listroot *root, char *variable);
extern struct listnode *search_nest_node(struct listroot *root, char *variable);
extern int search_nest_index(struct listroot *root, char *variable);
extern struct listroot *update_nest_root(struct listroot *root, char *arg);
extern void update_nest_node(struct listroot *root, char *arg);
extern int delete_nest_node(struct listroot *root, char *variable);
extern int get_nest_size(struct listroot *root, char *variable, char *result);
extern struct listnode *get_nest_node(struct listroot *root, char *variable, char *result, int def);
extern int get_nest_index(struct listroot *root, char *variable, char *result, int def);
extern struct listnode *set_nest_node(struct listroot *root, char *arg1, char *format, ...);
extern struct listnode *add_nest_node(struct listroot *root, char *arg1, char *format, ...);
extern void show_nest_node(struct listnode *node, char *result, int initialize);
extern void copy_nest_node(struct listroot *dst_root, struct listnode *dst, struct listnode *src);

#endif

#ifndef __NET_H__
#define __NET_H__

extern void write_line_socket(struct session *ses, char *line, int size);
extern int read_buffer_mud(struct session *ses);
extern void readmud(struct session *ses);
extern void process_mud_output(struct session *ses, char *linebuf, int prompt);

#endif

#ifndef __PARSE_H__
#define __PARSE_H__

extern struct session *parse_input(struct session *ses, char *input);
extern struct session *parse_command(struct session *ses, char *input);

extern struct session *parse_command(struct session *ses, char *input);
extern char *get_arg_all(struct session *ses, char *string, char *result, int verbatim);
extern char *get_arg_in_braces(struct session *ses, char *string, char *result, int flag);
extern char *sub_arg_in_braces(struct session *ses, char *string, char *result, int flag, int sub);
extern char *get_arg_with_spaces(struct session *ses, char *string, char *result, int flag);
extern char *get_arg_stop_spaces(struct session *ses, char *string, char *result, int flag);
extern char *space_out(char *string);
extern char *get_arg_to_brackets(struct session *ses, char *string, char *result);
extern char *get_arg_at_brackets(struct session *ses, char *string, char *result);
extern char *get_arg_in_brackets(struct session *ses, char *string, char *result);
extern void write_mud(struct session *ses, char *command, int flags);
extern void do_one_line(char *line, struct session *ses);

#endif


#ifndef __SESSION_H__
#define __SESSION_H__

extern DO_COMMAND(do_session);
extern struct session *session_command(char *arg, struct session *ses);
extern void show_session(struct session *ses, struct session *ptr);
extern struct session *newactive_session(void);
extern struct session *activate_session(struct session *ses);
extern struct session *new_session(struct session *ses, char *name, char *command, int pid, int socket);
extern void cleanup_session(struct session *ses);
extern void dispose_session(struct session *ses);

#endif


#ifndef __SUBSTITUTE_H__
#define __SUBSTITUTE_H__

extern DO_COMMAND(do_substitute);
extern DO_COMMAND(do_unsubstitute);
extern void check_all_substitutions(struct session *ses, char *original, char *line);

#endif


#ifndef __SYSTEM_H__
#define __SYSTEM_H__

extern DO_COMMAND(do_run);
extern DO_COMMAND(do_script);
extern DO_COMMAND(do_system);
#endif


#ifndef __TABLES_H__
#define __TABLES_H__

extern struct array_type array_table[];
extern struct class_type class_table[];
extern struct color_type color_table[];
extern struct command_type command_table[];
extern struct config_type config_table[];
extern struct cursor_type cursor_table[];
extern struct event_type event_table[];
extern struct list_type list_table[LIST_MAX];
extern struct substitution_type substitution_table[];
extern struct timer_type timer_table[];
extern struct line_type line_table[];

#endif

#ifndef __TEXT_H__
#define __TEXT_H__

extern void printline(struct session *ses, char *str, int isaprompt);
int word_wrap(struct session *ses, char *textin, char *textout, int display);
int word_wrap_split(struct session *ses, char *textin, char *textout, int skip, int keep);

#endif

#ifndef __TICKS_H__
#define __TICKS_H__

extern DO_COMMAND(do_tick);
extern DO_COMMAND(do_untick);
extern DO_COMMAND(do_delay);
extern DO_COMMAND(do_undelay);

#endif

#ifndef __TOKENIZE_H__
#define __TOKENIZE_H__

extern struct session *script_driver(struct session *ses, int list, char *str);
extern char *script_writer(struct session *ses, char *str);

#endif

#ifndef __UPDATE_H__
#define __UPDATE_H__

extern void mainloop(void);
extern void poll_input(void);
extern void poll_sessions(void);
extern void tick_update(void);
extern void delay_update(void);
extern void packet_update(void);
extern void terminal_update(void);
extern void memory_update(void);
extern void time_update(void);
#endif

#ifndef __UTILS_H__
#define __UTILS_H__

extern int is_abbrev(char *s1, char *s2);
extern int is_color_code(char *str);
extern int is_number(char *str);
extern int hex_number(char *str);
extern int oct_number(char *str);
extern long long utime(void);
extern char *capitalize(char *str);
extern char *ntos(long long number);
extern char *indent(int cnt);
extern int cat_sprintf(char *dest, char *fmt, ...);
extern void ins_sprintf(char *dest, char *fmt, ...);
extern int str_suffix(char *str1, char *str2);
extern void syserr(char *msg);
extern void show_message(struct session *ses, int index, char *format, ...);
extern void show_debug(struct session *ses, int index, char *format, ...);
extern void display_header(struct session *ses, char *format, ...);
extern void socket_printf(struct session *ses, size_t length, char *format, ...);

extern void display_printf2(struct session *ses, char *format, ...);
extern void display_printf(struct session *ses, char *format, ...);

extern void display_puts3(struct session *ses, char *string);
extern void display_puts2(struct session *ses, char *string);
extern void display_puts(struct session *ses, char *string);

extern void show_cpu(struct session *ses);
extern long long display_timer(struct session *ses, int timer);
extern void open_timer(int timer);
extern void close_timer(int timer);

#endif


#ifndef __VARIABLE_H__
#define __VARIABLE_H__

extern DO_COMMAND(do_variable);
extern DO_COMMAND(do_unvariable);

extern int delete_variable(struct session *ses, char *variable);
extern struct listnode *search_variable(struct session *ses, char *variable);

extern struct listnode *get_variable(struct session *ses, char *variable, char *result);
extern struct listnode *set_variable(struct session *ses, char *variable, char *format, ...);

extern int get_variable_index(struct session *ses, char *variable, char *result);

extern DO_COMMAND(do_format);
extern DO_COMMAND(do_tolower);
extern DO_COMMAND(do_toupper);
extern DO_COMMAND(do_postpad);
extern DO_COMMAND(do_prepad);
extern DO_COMMAND(do_replace);

#endif


#ifndef __VT102_H__
#define __VT102_H__

extern int skip_vt102_codes(char *str);
extern int skip_vt102_codes_non_graph(char *str);
extern void strip_vt102_codes(char *str, char *buf);
extern void strip_vt102_codes_non_graph(char *str, char *buf);
extern void strip_non_vt102_codes(char *str, char *buf);
extern void get_color_codes(char *old, char *str, char *buf);
extern int strip_vt102_strlen(struct session *ses, char *str);
extern int strip_color_strlen(struct session *ses, char *str);

#endif
