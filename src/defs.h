/* This program is protected under the GNU GPL (See COPYING) */

#include <ctype.h>
#include <errno.h>
#include <pcre.h>
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <termios.h>
#include <unistd.h>
#include <wordexp.h>

#include "config.h"

#ifdef HAVE_UTIL_H
#include <util.h>
#else
#ifdef HAVE_FORKPTY
#include <pty.h>
#endif
#endif

#ifndef __DEFS_H__
#define __DEFS_H__

#define VERSION "0.06"

#define FALSE 0
#define TRUE 1

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 24

#define PRIORITY 0
#define ALPHA 1

#define DEFAULT_OPEN '{'
#define DEFAULT_CLOSE '}'

#define BUFFER_SIZE 20000

#define MUD_OUTPUT_MAX 500000

#define ESCAPE 27

/* Index for lists */
#define LIST_CONFIG 0
#define LIST_HIGHLIGHT 1
#define LIST_MAX 2

#define SES_FLAG_CONVERTMETA (1 << 0)
#define SES_FLAG_HIGHLIGHT (1 << 1)

/* Bit operations */
#define HAS_BIT(bitvector, bit) ((bitvector) & (bit))
#define SET_BIT(bitvector, bit) ((bitvector) |= (bit))
#define DEL_BIT(bitvector, bit) ((bitvector) &= (~(bit)))
#define TOG_BIT(bitvector, bit) ((bitvector) ^= (bit))

/* Generic */
#define UMAX(a, b) ((a) > (b) ? (a) : (b))

#define DO_COMMAND(command) void command(char *arg)
#define DO_CONFIG(config) int config(char *arg, int index)

/* Structures */
struct listroot {
  struct listnode **list;
  int size;
  int used;
  int type;
};

struct listnode {
  char left[BUFFER_SIZE];
  char right[BUFFER_SIZE];
  char pr[BUFFER_SIZE];
  pcre *compiled_regex;
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
  struct termios saved_terminal;
  char command_char;
  char mud_output_buf[MUD_OUTPUT_MAX];
  int mud_output_len;
  int quiet;
  int run_overriden;
};

/* Typedefs */
typedef void COMMAND(char *arg);
typedef int CONFIG(char *arg, int index);

/* Structures for tables.c */
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

struct help_type {
  char *name;
  char *text;
};

struct list_type {
  char *name;
  int mode;
};

#endif

/* Function declarations */

#ifndef __CONFIG_H__
#define __CONFIG_H__

DO_COMMAND(do_configure);
DO_CONFIG(config_commandchar);
DO_CONFIG(config_convertmeta);
DO_CONFIG(config_highlight);

#endif

#ifndef __DATA_H__
#define __DATA_H__

int bsearch_alpha_list(struct listroot *root, char *text, int seek);
int bsearch_priority_list(struct listroot *root, char *text, char *priority,
                          int seek);
void delete_index_list(struct listroot *root, int index);
struct listroot *init_list(int type, int size);
struct listnode *insert_index_list(struct listroot *root, struct listnode *node,
                                   int index);
struct listnode *insert_node_list(struct listroot *root, char *ltext,
                                  char *rtext, char *prtext);
int locate_index_list(struct listroot *root, char *text, char *priority);
int nsearch_list(struct listroot *root, char *text);
int search_index_list(struct listroot *root, char *text, char *priority);
struct listnode *search_node_list(struct listroot *root, char *text);
struct listnode *update_node_list(struct listroot *root, char *ltext,
                                  char *rtext, char *prtext);

#endif

#ifndef __FILES_H__
#define __FILES_H__

DO_COMMAND(do_read);
DO_COMMAND(do_write);

void write_node(int list, struct listnode *node, FILE *file);

#endif

#ifndef __HIGHLIGHT_H__
#define __HIGHLIGHT_H__

DO_COMMAND(do_highlight);
DO_COMMAND(do_unhighlight);

void check_all_highlights(char *original);
int get_highlight_codes(char *string, char *result);
int regex_compare(pcre *compiled_regex, char *str, char *result);
int skip_vt102_codes(char *str);
void strip_vt102_codes(char *str, char *buf);
void substitute(char *string, char *result);

#endif

#ifndef __IO_H__
#define __IO_H__

void convert_meta(char *input, char *output);
void print_backspace(int sig);
int read_buffer_mud(void);
void read_key(void);
void readmud(void);

#endif

#ifndef __MAIN_H__
#define __MAIN_H__

extern struct session gts;
extern struct global_data gtd;

extern pthread_t input_thread;
extern pthread_t output_thread;

int main(int argc, char **argv);
void init_program(void);
void help_menu(int error, char *proc_name);
void quit_void(void);
void quitmsg(char *message, int exit_signal);
void pipe_handler(int sig);
void trap_handler(int sig);
void winch_handler(int sig);

#endif

#ifndef __MISC_H__
#define __MISC_H__

DO_COMMAND(do_commands);
DO_COMMAND(do_exit);
DO_COMMAND(do_help);
DO_COMMAND(do_run);

#endif

#ifndef __SESSION_H__
#define __SESSION_H__

void *poll_input(void *);
void *poll_session(void *);
void script_driver(char *str);

#endif

#ifndef __TABLES_H__
#define __TABLES_H__

extern struct color_type color_table[];
extern struct command_type command_table[];
extern struct config_type config_table[];
extern struct list_type list_table[LIST_MAX];
extern struct help_type help_table[];

#endif

#ifndef __UTILS_H__
#define __UTILS_H__

char *capitalize(char *str);
int cat_sprintf(char *dest, char *fmt, ...);
void display_header(char *str);
void display_printf(char *format, ...);
char *get_arg(char *string, char *result);
int is_abbrev(char *s1, char *s2);
void printline(char *str, int isaprompt);
void socket_printf(unsigned int length, char *format, ...);
char *space_out(char *string);

#endif
