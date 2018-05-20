/* This program is protected under the GNU GPL (See COPYING) */

#include <ctype.h>
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>
#include <wordexp.h>

#include "config.h"

#ifdef HAVE_UTIL_H
#include <util.h>
#else
#ifdef HAVE_PTY_H
#include <pty.h>
#endif
#endif

#ifdef HAVE_PCRE2_H
#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>
#else
#ifdef HAVE_PCRE_H
#include <pcre.h>
#endif
#endif

#ifndef __DEFS_H__
#define __DEFS_H__

#define VERSION "0.08"

#define FALSE 0
#define TRUE 1

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 24

#define DEFAULT_OPEN '{'
#define DEFAULT_CLOSE '}'

#define BUFFER_SIZE 20000

#define MUD_OUTPUT_MAX 262144 /* 256 KiB */

/* Microseconds to wait before processing a line without \n at the end */
#define WAIT_FOR_NEW_LINE 10000
/* Why: CT-- cannot determine if a line has finished printing or if CT--'s
 * processing speed is just faster than the output of the child process.
 * Therefore, on lines that do not end with \n, CT-- will wait a fixed interval
 * prior to printing that line. However, in the case that the line does end with
 * a \n, process it right away and move on to the next line. The delay will only
 * affect the last line; if you're outputting thousands of lines back-to-back,
 * they'll be processed as soon as possible since each line will end with \n */

#define ESCAPE 27

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

/* Stores the shared data for CT-- */
struct global_data {
  struct termios active_terminal;
  struct termios saved_terminal;
  struct highlight **highlights;
  int highlights_size;
  int highlights_used;
  char command_char;
  int flags;
  char mud_output_buf[MUD_OUTPUT_MAX];
  char mud_current_line[MUD_OUTPUT_MAX];
  int mud_output_len;
  int cols;
  int rows;
  int pid;
  int socket;
  int run_overriden;
  int quiet;
};

struct highlight {
  char condition[BUFFER_SIZE];       /* Processed into compiled_action */
  char action[BUFFER_SIZE];          /* Processed into compiled_regex */
  char priority[BUFFER_SIZE];        /* Lower value overwrites higher value */
  char compiled_action[BUFFER_SIZE]; /* Compiled once, used multiple times */

#ifdef HAVE_PCRE2_H
  pcre2_code *compiled_regex; /* Compiled once, used multiple times */
#else
#ifdef HAVE_PCRE_H
  pcre *compiled_regex; /* Compiled once, used multiple times */
#endif
#endif
};

struct regex_result {
  int start;
  char match[BUFFER_SIZE];
};

/* Typedefs */
typedef void COMMAND(char *arg);

/* Structures for tables.c */
struct color_type {
  char *name;
  char *code;
};

struct command_type {
  char *name;
  COMMAND *command;
};

struct help_type {
  char *name;
  char *text;
};

#endif

/* Function declarations */
#ifndef __FILES_H__
#define __FILES_H__

DO_COMMAND(do_read);
DO_COMMAND(do_write);

#endif

#ifndef __HIGHLIGHT_H__
#define __HIGHLIGHT_H__

DO_COMMAND(do_highlight);
DO_COMMAND(do_unhighlight);

void check_all_highlights(char *original);
int find_highlight_index(char *text);
int get_highlight_codes(char *string, char *result);

#ifdef HAVE_PCRE2_H
struct regex_result regex_compare(pcre2_code *compiled_regex, char *str);
#else
#ifdef HAVE_PCRE_H
struct regex_result regex_compare(pcre *compiled_regex, char *str);
#endif
#endif

int skip_vt102_codes(char *str);
void strip_vt102_codes(char *str, char *buf);
void substitute(char *string, char *result);

#endif

#ifndef __IO_H__
#define __IO_H__

void convert_meta(char *input, char *output);
void sigint_handler_during_read(int sig);
void read_key(void);
void read_output_buffer(int wait_for_new_line);

#endif

#ifndef __MAIN_H__
#define __MAIN_H__

extern struct global_data gd;

int main(int argc, char **argv);
void init_program(void);
void help_menu(int error, char *proc_name);
void quit_void(void);
void quit_with_msg(char *message, int exit_signal);
void pipe_handler(int sig);
void trap_handler(int sig);
void winch_handler(int sig);

#endif

#ifndef __MISC_H__
#define __MISC_H__

DO_COMMAND(do_commands);
DO_COMMAND(do_configure);
DO_COMMAND(do_exit);
DO_COMMAND(do_help);
DO_COMMAND(do_run);
DO_COMMAND(do_showme);

#endif

#ifndef __SESSION_H__
#define __SESSION_H__

extern pthread_t input_thread;
extern pthread_t output_thread;

void *poll_input(void *);
void *poll_output(void *);
void script_driver(char *str);

#endif

#ifndef __TABLES_H__
#define __TABLES_H__

extern struct color_type color_table[];
extern struct command_type command_table[];
extern struct help_type help_table[];

#endif

#ifndef __UTILS_H__
#define __UTILS_H__

void cat_sprintf(char *dest, char *fmt, ...);
void display_printf(char *format, ...);
char *get_arg(char *string, char *result);
int is_abbrev(char *s1, char *s2);
void printline(char *str, int isaprompt);
char *space_out(char *string);

#endif
