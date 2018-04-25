// This program is protected under the GNU GPL (See COPYING)

#include <ctype.h>
#include <pcre.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <termios.h>
#include <zlib.h>

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
#define BADSIG (RETSIGTYPE(*)(int)) - 1
#endif

#ifdef HAVE_NET_ERRNO_H
#include <net/errno.h>
#endif

#ifndef __DEFS_H__
#define __DEFS_H__

// A bunch of constants

#define FALSE 0
#define TRUE 1

#define GET_ONE 0 /* stop at spaces */
#define GET_ALL 1 /* stop at semicolon */
#define GET_NST 2 /* nest square brackets */

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 24

#define PRIORITY 0
#define ALPHA 1
#define APPEND 2

#define DEFAULT_OPEN '{'
#define DEFAULT_CLOSE '}'

#define COMMAND_SEPARATOR ';'

#define STRING_SIZE 45000
#define BUFFER_SIZE 20000
#define NUMBER_SIZE 100
#define LIST_SIZE 2

#define CLIENT_NAME "ChromaTerm--"
#define CLIENT_VERSION "0.02.0  "

#define ESCAPE 27

#define TIMER_POLL_INPUT 0
#define TIMER_POLL_SESSIONS 1
#define TIMER_UPDATE_PACKETS 4
#define TIMER_UPDATE_TERMINAL 5
#define TIMER_UPDATE_MEMORY 7
#define TIMER_STALL_PROGRAM 8
#define TIMER_CPU 9

#define PULSE_PER_SECOND 100

#define PULSE_POLL_INPUT 1
#define PULSE_POLL_SESSIONS 1
#define PULSE_UPDATE_PACKETS 2
#define PULSE_UPDATE_TERMINAL 1
#define PULSE_UPDATE_MEMORY 2

// Index for lists
#define LIST_CONFIG 0
#define LIST_HIGHLIGHT 1
#define LIST_MAX 2

// Command type
enum operators {
  TOKEN_TYPE_COMMAND,
  TOKEN_TYPE_DEFAULT,
  TOKEN_TYPE_END,
  TOKEN_TYPE_ELSE,
  TOKEN_TYPE_REGEX,
  TOKEN_TYPE_SESSION,
  TOKEN_TYPE_STRING
};

// generic define for show_message
#define LIST_MESSAGE -1

// Various flags
#define COL_BLD (1 << 1)
#define COL_UND (1 << 2)
#define COL_BLK (1 << 3)
#define COL_REV (1 << 4)
#define COL_XTF (1 << 5)
#define COL_XTB (1 << 6)
#define COL_256 (1 << 7)

#define SUB_NONE 0
#define SUB_ARG (1 << 0)
#define SUB_VAR (1 << 1)
#define SUB_FUN (1 << 2)
#define SUB_COL (1 << 3)
#define SUB_ESC (1 << 4)
#define SUB_CMD (1 << 5)
#define SUB_SEC (1 << 6)
#define SUB_EOL (1 << 7)
#define SUB_LNF (1 << 8)
#define SUB_FIX (1 << 9)
#define SUB_CMP (1 << 10)

#define GLOBAL_FLAG_CONVERTMETACHAR (1 << 1)
#define GLOBAL_FLAG_PROCESSINPUT (1 << 4)
#define GLOBAL_FLAG_INSERTINPUT (1 << 6)

#define SES_FLAG_READMUD (1 << 8)
#define SES_FLAG_CONNECTED (1 << 11)
#define SES_FLAG_CONVERTMETA (1 << 24)
#define SES_FLAG_RUN (1 << 25)
#define SES_FLAG_UTF8 (1 << 26)

#define NODE_FLAG_META (1 << 0)

// Some macros to deal with double linked lists
#define LINK(link, head, tail)                                                 \
  {                                                                            \
    if ((head) == NULL) {                                                      \
      (head) = (link);                                                         \
    } else {                                                                   \
      (tail)->next = (link);                                                   \
    }                                                                          \
    (link)->next = NULL;                                                       \
    (link)->prev = (tail);                                                     \
    (tail) = (link);                                                           \
  }

#define UNLINK(link, head, tail)                                               \
  {                                                                            \
    if (((link)->prev == NULL && (link) != head) ||                            \
        ((link)->next == NULL && (link) != tail)) {                            \
      display_printf2(NULL, "#UNLINK ERROR in file %s on line %d", __FILE__,   \
                      __LINE__);                                               \
    }                                                                          \
    if ((link)->prev == NULL) {                                                \
      (head) = (link)->next;                                                   \
    } else {                                                                   \
      (link)->prev->next = (link)->next;                                       \
    }                                                                          \
    if ((link)->next == NULL) {                                                \
      (tail) = (link)->prev;                                                   \
    } else {                                                                   \
      (link)->next->prev = (link)->prev;                                       \
    }                                                                          \
    (link)->next = NULL;                                                       \
    (link)->prev = NULL;                                                       \
  }

// Bit operations
#define HAS_BIT(bitvector, bit) ((bitvector) & (bit))
#define SET_BIT(bitvector, bit) ((bitvector) |= (bit))
#define DEL_BIT(bitvector, bit) ((bitvector) &= (~(bit)))
#define TOG_BIT(bitvector, bit) ((bitvector) ^= (bit))

// Generic
#define URANGE(a, b, c) ((b) < (a) ? (a) : (b) > (c) ? (c) : (b))
#define UMAX(a, b) ((a) > (b) ? (a) : (b))
#define UMIN(a, b) ((a) < (b) ? (a) : (b))

#define up(u) (u < 99 ? u++ : u)

#define DO_COMMAND(command)                                                    \
  struct session *command(struct session *ses, char *arg)
#define DO_CONFIG(config)                                                      \
  struct session *config(struct session *ses, char *arg, int index)
#define DO_CURSOR(cursor) void cursor(char *arg)

// Compatibility
#define atoll(str) (strtoll(str, NULL, 10))

// Structures
struct listroot {
  struct listnode **list;
  struct session *ses;
  int size;
  int used;
  int type;
  int update;
  int flags;
};

struct listnode {
  struct listroot *root;
  char *left;
  char *right;
  char *pr;
  char *group;
  pcre *regex;
  long long data;
  short flags;
};

struct session {
  struct session *next;
  struct session *prev;
  char *name;
  char *group;
  char *command;
  struct listroot *list[LIST_MAX];
  int rows;
  int cols;
  int cur_row;
  int sav_row;
  int cur_col;
  int sav_col;
  int fgc;
  int bgc;
  int vtc;
  int pid;
  int socket;
  int flags;
  int input_level;
  char more_output[BUFFER_SIZE * 2];
  char color[100];
  long long check_output;
};

struct global_data {
  struct session *ses;
  struct session *update;
  struct session *dispose_next;
  struct session *dispose_prev;
  struct termios old_terminal;
  char *mud_output_buf;
  int mud_output_max;
  int mud_output_len;
  char input_buf[BUFFER_SIZE];
  char macro_buf[BUFFER_SIZE];
  char paste_buf[BUFFER_SIZE];
  int input_off;
  int input_len;
  int input_cur;
  int input_pos;
  int input_hid;
  char *term;
  long long time;
  long long timer[TIMER_CPU][5];
  int command_ref[26];
  int flags;
  int quiet;
  char command_char;
  char *vars[100];
  char *cmds[100];
  int args[100];
};

struct link_data {
  struct link_data *next;
  struct link_data *prev;
  char *str1;
  char *str2;
  char *str3;
};

// Typedefs
typedef struct session *CONFIG(struct session *ses, char *arg, int index);
typedef struct session *COMMAND(struct session *ses, char *arg);
typedef void CURSOR(char *arg);
typedef struct session *LINE(struct session *ses, char *arg);

// Structures for tables.c
struct color_type {
  char *name;
  char *code;
};

struct command_type {
  char *name;
  COMMAND *command;
  int type;
};

struct config_type {
  char *name;
  char *msg_on;
  char *msg_off;
  CONFIG *config;
};

struct list_type {
  char *name;
  char *name_multi;
  int mode;
  int args;
};

struct cursor_type {
  char *name;
  char *desc;
  char *code;
  CURSOR *fun;
};

#endif

// Function declarations

#ifndef __CURSOR_H__
#define __CURSOR_H__

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
extern void read_key(void);
extern void read_line();
extern void convert_meta(char *input, char *output);
extern void echo_command(struct session *ses, char *line);
extern void input_printf(char *format, ...);

#endif

#ifndef __TERMINAL_H__
#define __TERMINAL_H__

extern void init_terminal(void);
extern void restore_terminal(void);
extern void init_screen_size(struct session *ses);
extern int get_scroll_size(struct session *ses);

#endif

#ifndef __MATH_H__
#define __MATH_H__

extern double get_number(struct session *ses, char *str);
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

DO_COMMAND(do_regexp);

extern int substitute(struct session *ses, char *string, char *result,
                      int flags);
extern int match(struct session *ses, char *str, char *exp, int flags);
extern int find(struct session *ses, char *str, char *exp, int sub);
extern int regexp_compare(pcre *regex, char *str, char *exp, int option,
                          int flag);
extern int check_one_regexp(struct session *ses, struct listnode *node,
                            char *line, char *original, int option);
extern int regexp(struct session *ses, pcre *pcre, char *str, char *exp,
                  int option, int flag);

#endif

#ifndef __CONFIG_H__
#define __CONFIG_H__

extern DO_COMMAND(do_configure);
extern DO_CONFIG(config_packetpatch);
extern DO_CONFIG(config_commandchar);
extern DO_CONFIG(config_convertmeta);
extern DO_CONFIG(config_charset);

#endif

#ifndef __DATA_H__
#define __DATA_H__

extern struct listroot *init_list(struct session *ses, int type, int size);
extern void kill_list(struct listroot *root);
extern void free_list(struct listroot *root);
extern struct listroot *copy_list(struct session *ses,
                                  struct listroot *sourcelist, int type);

extern struct listnode *insert_node_list(struct listroot *root, char *ltext,
                                         char *rtext, char *prtext);
extern struct listnode *update_node_list(struct listroot *root, char *ltext,
                                         char *rtext, char *prtext);
extern struct listnode *insert_index_list(struct listroot *root,
                                          struct listnode *node, int index);

extern int show_node_with_wild(struct session *ses, char *cptr, int type);
extern void show_node(struct listroot *root, struct listnode *node, int level);
extern void show_nest(struct listnode *node, char *result);
extern void show_list(struct listroot *root, int level);

extern struct listnode *search_node_list(struct listroot *root, char *text);

extern void delete_node_list(struct session *ses, int type,
                             struct listnode *node);
extern void delete_node_with_wild(struct session *ses, int index, char *string);
extern void delete_index_list(struct listroot *root, int index);
extern int search_index_list(struct listroot *root, char *text, char *priority);
extern int locate_index_list(struct listroot *root, char *text, char *priority);

extern int bsearch_alpha_list(struct listroot *root, char *text, int seek);
extern int bsearch_priority_list(struct listroot *root, char *text,
                                 char *priority, int seek);
extern int nsearch_list(struct listroot *root, char *text);

#endif

#ifndef __FILES_H__
#define __FILES_H__

extern DO_COMMAND(do_read);
extern DO_COMMAND(do_write);

extern void write_node(struct session *ses, int mode, struct listnode *node,
                       FILE *file);

#endif

#ifndef __HELP_H__
#define __HELP_H__

extern DO_COMMAND(do_help);

#endif

#ifndef __HIGHLIGHT_H__
#define __HIGHLIGHT_H__

extern DO_COMMAND(do_highlight);
extern DO_COMMAND(do_unhighlight);

extern void check_all_highlights(struct session *ses, char *original,
                                 char *line);
extern int get_highlight_codes(struct session *ses, char *htype, char *result);

#endif

#ifndef __MAIN_H__
#define __MAIN_H__

extern struct session *gts;
extern struct global_data *gtd;

extern int exit_after_session;

extern int main(int argc, char **argv);
extern void help_menu(int error, char c, char *proc_name);
extern void winch_handler(int signal);
extern void abort_and_trap_handler(int signal);
extern void pipe_handler(int signal);
extern void suspend_handler(int signal);
extern void init_program();
extern void quitmsg(char *message);

#endif

#ifndef __MEMORY_H__
#define __MEMORY_H__

extern char *refstring(char *point, char *fmt, ...);

#endif

#ifndef __MISC_H__
#define __MISC_H__

extern DO_COMMAND(do_commands);
extern DO_COMMAND(do_exit);
extern DO_COMMAND(do_showme);
extern DO_COMMAND(do_zap);

#endif

#ifndef __NEST_H__
#define __NEST_H__

extern struct listroot *update_nest_root(struct listroot *root, char *arg);
extern void update_nest_node(struct listroot *root, char *arg);
extern void show_nest_node(struct listnode *node, char *result, int initialize);
extern void copy_nest_node(struct listroot *dst_root, struct listnode *dst,
                           struct listnode *src);

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
extern char *get_arg_all(struct session *ses, char *string, char *result);
extern char *get_arg_in_braces(struct session *ses, char *string, char *result,
                               int flag);
extern char *sub_arg_in_braces(struct session *ses, char *string, char *result,
                               int flag, int sub);
extern char *get_arg_with_spaces(struct session *ses, char *string,
                                 char *result, int flag);
extern char *get_arg_stop_spaces(struct session *ses, char *string,
                                 char *result, int flag);
extern char *space_out(char *string);
extern char *get_arg_at_brackets(struct session *ses, char *string,
                                 char *result);
extern void write_mud(struct session *ses, char *command, int flags);
extern void do_one_line(char *line, struct session *ses);

#endif

#ifndef __SESSION_H__
#define __SESSION_H__

extern struct session *new_session(struct session *ses, char *name,
                                   char *command, int pid, int socket);
extern void cleanup_session(struct session *ses);

#endif

#ifndef __SYSTEM_H__
#define __SYSTEM_H__

extern DO_COMMAND(do_run);

#endif

#ifndef __TABLES_H__
#define __TABLES_H__

extern struct color_type color_table[];
extern struct command_type command_table[];
extern struct config_type config_table[];
extern struct cursor_type cursor_table[];
extern struct list_type list_table[LIST_MAX];

#endif

#ifndef __TEXT_H__
#define __TEXT_H__

extern void printline(struct session *ses, char *str, int isaprompt);

#endif

#ifndef __TOKENIZE_H__
#define __TOKENIZE_H__

extern struct session *script_driver(struct session *ses, int list, char *str);

#endif

#ifndef __UPDATE_H__
#define __UPDATE_H__

extern void mainloop(void);
extern void poll_input(void);
extern void poll_sessions(void);
extern void packet_update(void);
extern void terminal_update(void);
extern void memory_update(void);

#endif

#ifndef __UTILS_H__
#define __UTILS_H__

extern int is_abbrev(char *s1, char *s2);
extern int is_number(char *str);
extern int hex_number(char *str);
extern int oct_number(char *str);
extern long long utime(void);
extern char *capitalize(char *str);
extern int cat_sprintf(char *dest, char *fmt, ...);
extern void ins_sprintf(char *dest, char *fmt, ...);
extern void syserr(char *msg);
extern void show_message(struct session *ses, int index, char *format, ...);
extern void display_header(struct session *ses, char *format, ...);
extern void socket_printf(struct session *ses, size_t length, char *format,
                          ...);
extern void display_printf2(struct session *ses, char *format, ...);
extern void display_printf(struct session *ses, char *format, ...);
extern void display_puts3(struct session *ses, char *string);
extern void display_puts2(struct session *ses, char *string);
extern void display_puts(struct session *ses, char *string);

#endif

#ifndef __VT102_H__
#define __VT102_H__

extern int skip_vt102_codes(char *str);
extern void strip_vt102_codes(char *str, char *buf);
extern void get_color_codes(char *old, char *str, char *buf);
extern int find_non_color_codes(char *str);
extern int strip_vt102_strlen(struct session *ses, char *str);

#endif
