// This program is protected under the GNU GPL (See COPYING)

#include <ctype.h>
#include <errno.h>
#include <regex.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <util.h>
#include <wordexp.h>

#ifndef __DEFS_H__
#define __DEFS_H__

#define FALSE 0
#define TRUE 1

#define GET_ONE 0 // stop at a space
#define GET_ALL 1 // don't stop (believing)

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 24

#define PRIORITY 0
#define ALPHA 1

#define DEFAULT_OPEN '{'
#define DEFAULT_CLOSE '}'

#define BUFFER_SIZE 20000

#define ESCAPE 27

#define PULSE_PER_SECOND 500

// Index for lists
#define LIST_CONFIG 0
#define LIST_HIGHLIGHT 1
#define LIST_MAX 2

// Various flags
#define COL_BLD (1 << 0)
#define COL_UND (1 << 1)
#define COL_BLK (1 << 2)
#define COL_REV (1 << 3)
#define COL_XTF (1 << 4)
#define COL_XTB (1 << 5)
#define COL_256 (1 << 6)

#define SUB_NONE 0

#define GLOBAL_FLAG_CONVERTMETACHAR (1 << 0)
#define GLOBAL_FLAG_PROCESSINPUT (1 << 1)
#define GLOBAL_FLAG_INSERTINPUT (1 << 2)

#define SES_FLAG_CONNECTED (1 << 0)
#define SES_FLAG_CONVERTMETA (1 << 1)
#define SES_FLAG_UTF8 (1 << 2)
#define SES_FLAG_HIGHLIGHT (1 << 3)

// Bit operations
#define HAS_BIT(bitvector, bit) ((bitvector) & (bit))
#define SET_BIT(bitvector, bit) ((bitvector) |= (bit))
#define DEL_BIT(bitvector, bit) ((bitvector) &= (~(bit)))
#define TOG_BIT(bitvector, bit) ((bitvector) ^= (bit))

// Generic
#define URANGE(a, b, c) ((b) < (a) ? (a) : (b) > (c) ? (c) : (b))
#define UMAX(a, b) ((a) > (b) ? (a) : (b))

#define DO_COMMAND(command) void command(char *arg)
#define DO_CONFIG(config) int config(char *arg, int index)
#define DO_CURSOR(cursor) void cursor(void)

// Structures
struct listroot {
  struct listnode **list;
  int size;
  int used;
  int type;
  int update;
};

struct listnode {
  char *left;
  char *right;
  char *pr;
  regex_t compiled_regex;
};

struct session {
  struct listroot *list[LIST_MAX];
  int rows;
  int cols;
  int pid;
  int socket;
  int flags;
};

struct global_data {
  struct termios active_terminal;
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
  int flags;
  int quiet;
  char command_char;
};

// Typedefs
typedef void COMMAND(char *arg);
typedef int CONFIG(char *arg, int index);
typedef void CURSOR(void);

// Structures for tables.c
struct color_type {
  char *name;
  char *code;
};

struct command_type {
  char *name;
  COMMAND *command;
};

struct config_type {
  char *name;
  char *description;
  CONFIG *config;
};

struct list_type {
  char *name;
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

#ifndef __CONFIG_H__
#define __CONFIG_H__

DO_COMMAND(do_configure);
DO_CONFIG(config_commandchar);
DO_CONFIG(config_convertmeta);
DO_CONFIG(config_charset);
DO_CONFIG(config_highlight);

#endif

#ifndef __CURSOR_H__
#define __CURSOR_H__

DO_CURSOR(cursor_backspace);
DO_CURSOR(cursor_check_line);
DO_CURSOR(cursor_check_line_modified);
DO_CURSOR(cursor_clear_left);
DO_CURSOR(cursor_clear_line);
DO_CURSOR(cursor_clear_right);
DO_CURSOR(cursor_convert_meta);
DO_CURSOR(cursor_delete);
DO_CURSOR(cursor_delete_or_exit);
DO_CURSOR(cursor_delete_word_left);
DO_CURSOR(cursor_delete_word_right);
DO_CURSOR(cursor_echo_on);
DO_CURSOR(cursor_echo_off);
DO_CURSOR(cursor_end);
DO_CURSOR(cursor_enter);
DO_CURSOR(cursor_exit);
DO_CURSOR(cursor_home);
DO_CURSOR(cursor_insert);
DO_CURSOR(cursor_left);
DO_CURSOR(cursor_left_word);
DO_CURSOR(cursor_paste_buffer);
DO_CURSOR(cursor_redraw_input);
DO_CURSOR(cursor_redraw_line);
DO_CURSOR(cursor_right);
DO_CURSOR(cursor_right_word);
DO_CURSOR(cursor_suspend);
DO_CURSOR(cursor_test);

#endif

#ifndef __DATA_H__
#define __DATA_H__

struct listroot *init_list(int type, int size);
struct listnode *insert_node_list(struct listroot *root, char *ltext,
                                  char *rtext, char *prtext);
struct listnode *update_node_list(struct listroot *root, char *ltext,
                                  char *rtext, char *prtext);
struct listnode *insert_index_list(struct listroot *root, struct listnode *node,
                                   int index);
struct listnode *search_node_list(struct listroot *root, char *text);
void delete_node_with_wild(int index, char *string);
void delete_index_list(struct listroot *root, int index);
int search_index_list(struct listroot *root, char *text, char *priority);
int locate_index_list(struct listroot *root, char *text, char *priority);
int bsearch_alpha_list(struct listroot *root, char *text, int seek);
int bsearch_priority_list(struct listroot *root, char *text, char *priority,
                          int seek);
int nsearch_list(struct listroot *root, char *text);

#endif

#ifndef __FILES_H__
#define __FILES_H__

DO_COMMAND(do_read);
DO_COMMAND(do_write);

void write_node(int mode, struct listnode *node, FILE *file);

#endif

#ifndef __HELP_H__
#define __HELP_H__

DO_COMMAND(do_help);

#endif

#ifndef __HIGHLIGHT_H__
#define __HIGHLIGHT_H__

DO_COMMAND(do_highlight);
DO_COMMAND(do_unhighlight);

void check_all_highlights(char *original, char *line);
int get_highlight_codes(char *htype, char *result);

#endif

#ifndef __INPUT_H__
#define __INPUT_H__

void process_input(void);
void read_key(void);
void read_line(void);
void convert_meta(char *input, char *output);
void input_printf(char *format, ...);

#endif

#ifndef __MAIN_H__
#define __MAIN_H__

extern struct session *gts;
extern struct global_data *gtd;

int main(int argc, char **argv);
void init_program(void);
void help_menu(int error, char c, char *proc_name);
void quitmsg(char *message, int exit_signal);
void abort_and_trap_handler(int sig);
void pipe_handler(int sig);
void suspend_handler(int sig);
void winch_handler(int sig);

#endif

#ifndef __MISC_H__
#define __MISC_H__

DO_COMMAND(do_commands);
DO_COMMAND(do_exit);
DO_COMMAND(do_run);

#endif

#ifndef __NET_H__
#define __NET_H__

void write_line_socket(char *line, int size);
int read_buffer_mud(void);
void readmud(void);
void process_mud_output(char *linebuf, int prompt);

#endif

#ifndef __PARSE_H__
#define __PARSE_H__

char *get_arg_all(char *string, char *result, int with_spaces);
char *get_arg_in_braces(char *string, char *result, int flag);
char *get_arg_stop_spaces(char *string, char *result);
char *space_out(char *string);
void do_one_line(char *line);

#endif

#ifndef __REGEXP_H__
#define __REGEXP_H__

void substitute(char *string, char *result);
int regex_compare(regex_t *compiled_regex, char *str, char *result);

#endif

#ifndef __SESSION_H__
#define __SESSION_H__

struct session *new_session(int pid, int socket);
void cleanup_session(void);

#endif

#ifndef __TABLES_H__
#define __TABLES_H__

extern struct color_type color_table[];
extern struct command_type command_table[];
extern struct config_type config_table[];
extern struct cursor_type cursor_table[];
extern struct list_type list_table[LIST_MAX];

#endif

#ifndef __TERMINAL_H__
#define __TERMINAL_H__

void init_terminal(void);
void restore_terminal(void);
void init_screen_size(void);
int get_scroll_size(void);

#endif

#ifndef __TOKENIZE_H__
#define __TOKENIZE_H__

void script_driver(char *str);

#endif

#ifndef __UPDATE_H__
#define __UPDATE_H__

void mainloop(void);
void poll_input(void);
void poll_sessions(void);

#endif

#ifndef __UTILS_H__
#define __UTILS_H__

int is_abbrev(char *s1, char *s2);
char *capitalize(char *str);
int cat_sprintf(char *dest, char *fmt, ...);
void ins_sprintf(char *dest, char *fmt, ...);
void display_header(char *format, ...);
void socket_printf(size_t length, char *format, ...);
void display_printf(char *format, ...);
void printline(char *str, int isaprompt);

#endif

#ifndef __VT102_H__
#define __VT102_H__

int skip_vt102_codes(char *str);
void strip_vt102_codes(char *str, char *buf);
void get_color_codes(char *old, char *str, char *buf);
int find_non_color_codes(char *str);
int strip_vt102_strlen(char *str);

#endif
