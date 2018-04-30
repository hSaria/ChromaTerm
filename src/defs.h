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

#define GET_ONE 0 // stop at spaces
#define GET_ALL 1 // stop at semicolon
#define GET_NST 2 // nest square brackets

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 24

#define PRIORITY 0
#define ALPHA 1

#define DEFAULT_OPEN '{'
#define DEFAULT_CLOSE '}'

#define COMMAND_SEPARATOR ';'

#define STRING_SIZE 45000
#define BUFFER_SIZE 20000

#define ESCAPE 27

#define PULSE_PER_SECOND 500

// Index for lists
#define LIST_CONFIG 0
#define LIST_HIGHLIGHT 1
#define LIST_MAX 2

// Command type
enum operators { TOKEN_TYPE_COMMAND, TOKEN_TYPE_SESSION, TOKEN_TYPE_STRING };

// Various flags
#define COL_BLD (1 << 0)
#define COL_UND (1 << 1)
#define COL_BLK (1 << 2)
#define COL_REV (1 << 3)
#define COL_XTF (1 << 4)
#define COL_XTB (1 << 5)
#define COL_256 (1 << 6)

#define SUB_NONE 0
#define SUB_COL (1 << 0)
#define SUB_EOL (1 << 1)

#define GLOBAL_FLAG_CONVERTMETACHAR (1 << 0)
#define GLOBAL_FLAG_PROCESSINPUT (1 << 1)
#define GLOBAL_FLAG_INSERTINPUT (1 << 2)

#define SES_FLAG_CONNECTED (1 << 0)
#define SES_FLAG_CONVERTMETA (1 << 1)
#define SES_FLAG_UTF8 (1 << 2)

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
};

struct session {
  struct listroot *list[LIST_MAX];
  int rows;
  int cols;
  int pid;
  int socket;
  int flags;
  int input_level;
  char more_output[BUFFER_SIZE * 2];
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

#ifndef __CONFIG_H__
#define __CONFIG_H__

extern DO_COMMAND(do_configure);
extern DO_CONFIG(config_commandchar);
extern DO_CONFIG(config_convertmeta);
extern DO_CONFIG(config_charset);

#endif

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

#ifndef __DATA_H__
#define __DATA_H__

extern struct listroot *init_list(int type, int size);
extern void kill_list(struct listroot *root);
extern void free_list(struct listroot *root);
extern struct listnode *insert_node_list(struct listroot *root, char *ltext,
                                         char *rtext, char *prtext);
extern struct listnode *update_node_list(struct listroot *root, char *ltext,
                                         char *rtext, char *prtext);
extern struct listnode *insert_index_list(struct listroot *root,
                                          struct listnode *node, int index);
extern struct listnode *search_node_list(struct listroot *root, char *text);
extern void delete_node_list(int type, struct listnode *node);
extern void delete_node_with_wild(int index, char *string);
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

extern void write_node(int mode, struct listnode *node, FILE *file);

#endif

#ifndef __HELP_H__
#define __HELP_H__

extern DO_COMMAND(do_help);

#endif

#ifndef __HIGHLIGHT_H__
#define __HIGHLIGHT_H__

extern DO_COMMAND(do_highlight);
extern DO_COMMAND(do_unhighlight);

extern void check_all_highlights(char *original, char *line);
extern int get_highlight_codes(char *htype, char *result);

#endif

#ifndef __INPUT_H__
#define __INPUT_H__

extern void process_input(void);
extern void read_key(void);
extern void read_line(void);
extern void convert_meta(char *input, char *output);
extern void input_printf(char *format, ...);

#endif

#ifndef __MAIN_H__
#define __MAIN_H__

extern struct session *gts;
extern struct global_data *gtd;

extern int main(int argc, char **argv);
extern void init_program(void);
extern void help_menu(int error, char c, char *proc_name);
extern void quitmsg(char *message, int exit_signal);
extern void abort_and_trap_handler(int sig);
extern void pipe_handler(int sig);
extern void suspend_handler(int sig);
extern void winch_handler(int sig);

#endif

#ifndef __MISC_H__
#define __MISC_H__

extern DO_COMMAND(do_commands);
extern DO_COMMAND(do_exit);
extern DO_COMMAND(do_run);

#endif

#ifndef __NET_H__
#define __NET_H__

extern void write_line_socket(char *line, int size);
extern int read_buffer_mud(void);
extern void readmud(void);
extern void process_mud_output(char *linebuf, int prompt);

#endif

#ifndef __PARSE_H__
#define __PARSE_H__

extern void parse_input(char *input);
extern char *get_arg_all(char *string, char *result, int with_spaces);
extern char *get_arg_in_braces(char *string, char *result, int flag);
extern char *sub_arg_in_braces(char *string, char *result, int flag, int sub);
extern char *get_arg_stop_spaces(char *string, char *result);
extern char *space_out(char *string);
extern void write_mud(char *command, int flags);
extern void do_one_line(char *line);

#endif

#ifndef __REGEXP_H__
#define __REGEXP_H__

extern int substitute(char *string, char *result, int flags);
extern int regexp_compare(char *str, char *exp, char *result);

#endif

#ifndef __SESSION_H__
#define __SESSION_H__

extern struct session *new_session(int pid, int socket);
extern void cleanup_session(void);

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

extern void init_terminal(void);
extern void restore_terminal(void);
extern void init_screen_size(void);
extern int get_scroll_size(void);

#endif

#ifndef __TOKENIZE_H__
#define __TOKENIZE_H__

extern void script_driver(char *str);

#endif

#ifndef __UPDATE_H__
#define __UPDATE_H__

extern void mainloop(void);
extern void poll_input(void);
extern void poll_sessions(void);

#endif

#ifndef __UTILS_H__
#define __UTILS_H__

extern int is_abbrev(char *s1, char *s2);
extern char *capitalize(char *str);
extern int cat_sprintf(char *dest, char *fmt, ...);
extern void ins_sprintf(char *dest, char *fmt, ...);
extern void syserr(char *msg);
extern void show_message(char *format, ...);
extern void display_header(char *format, ...);
extern void socket_printf(size_t length, char *format, ...);
extern void display_printf(int came_from_command, char *format, ...);
extern void display_puts(int came_from_mud, int with_color, char *string);
extern void printline(char *str, int isaprompt);

#endif

#ifndef __VT102_H__
#define __VT102_H__

extern int skip_vt102_codes(char *str);
extern void strip_vt102_codes(char *str, char *buf);
extern void get_color_codes(char *old, char *str, char *buf);
extern int find_non_color_codes(char *str);
extern int strip_vt102_strlen(char *str);

#endif
