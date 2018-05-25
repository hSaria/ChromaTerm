/* This program is protected under the GNU GPL (See COPYING) */

#include "config.h"

#include <ctype.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <wordexp.h>

#ifdef HAVE_PCRE2_H
#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>
#else
#ifdef HAVE_PCRE_H
#include <pcre.h>
#endif
#endif

#define VERSION "0.2.0"

#define FALSE 0
#define TRUE 1

#define DEFAULT_OPEN '{'
#define DEFAULT_CLOSE '}'

#define BUFFER_SIZE 20000

#define INPUT_MAX 262144 /* 256 KiB */

/* Microseconds to wait before processing a line without \n at the end */
#define WAIT_FOR_NEW_LINE 500
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

#define DO_COMMAND(command) void command(char *arg)

/* Stores the shared data for CT-- */
struct global_data {
  struct highlight **highlights;
  int highlights_size;
  int highlights_used;

  char command_char;
  int flags;
  int quiet;

  char input_buffer[INPUT_MAX];
  int input_buffer_length;
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

/**** highlight.c ****/
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

/**** io.c ****/
void convert_meta(char *input, char *output);
void process_input(int wait_for_new_line);

/**** main.c ****/
extern struct global_data gd;

int main(int argc, char **argv);
void init_program(void);
void help_menu(char *proc_name);
void quit_with_signal(int exit_signal);

/**** misc.c ****/
DO_COMMAND(do_commands);
DO_COMMAND(do_configure);
DO_COMMAND(do_help);
DO_COMMAND(do_read);
DO_COMMAND(do_showme);
DO_COMMAND(do_write);

/**** tables.c ****/
extern struct color_type color_table[];
extern struct command_type command_table[];
extern struct help_type help_table[];

/**** utils.c ****/
void cat_sprintf(char *dest, char *fmt, ...);
void display_printf(char *format, ...);
char *get_arg(char *string, char *result);
int is_abbrev(char *s1, char *s2);
void script_driver(char *str);
