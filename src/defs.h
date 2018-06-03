/* This program is protected under the GNU GPL (See COPYING) */

#include "config.h"

#include <ctype.h>
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
#include <pcre.h>
#endif

#define VERSION "0.2.0"

#define FALSE 0
#define TRUE 1

#define DEFAULT_OPEN '{'
#define DEFAULT_CLOSE '}'

#define BUFFER_SIZE 16384

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

/* Stores the shared data for CT-- */
struct global_data {
  struct highlight **highlights;
  int highlights_size;
  int highlights_used;

  char input_buffer[INPUT_MAX];
  int input_buffer_length;

  int debug;
};

struct highlight {
  char condition[BUFFER_SIZE];       /* Processed into compiled_action */
  char action[BUFFER_SIZE];          /* Processed into compiled_regex */
  char priority[BUFFER_SIZE];        /* Lower value overwrites higher value */
  char compiled_action[BUFFER_SIZE]; /* Compiled once, used multiple times */

#ifdef HAVE_PCRE2_H
  pcre2_code *compiled_regex; /* Compiled once, used multiple times */
#else
  pcre *compiled_regex; /* Compiled once, used multiple times */
#endif
};

struct regex_result {
  int start;
  int end;
};

/**** highlight.c ****/
void check_all_highlights(char *original);
int find_highlight_index(char *text);
int get_highlight_codes(char *string, char *result);
void highlight(char *args);

#ifdef HAVE_PCRE2_H
struct regex_result regex_compare(pcre2_code *compiled_regex, char *str);
#else
struct regex_result regex_compare(pcre *compiled_regex, char *str);
#endif

int skip_vt102_codes(char *str);
void strip_vt102_codes(char *str, char *buf);
void substitute(char *string, char *result);
void unhighlight(char *args);

/**** main.c ****/
extern struct global_data gd;

int main(int argc, char **argv);
void colordemo(void);
void init_program(void);
void quit_with_signal(int exit_signal);

/**** utils.c ****/
void convert_meta(char *dest, char *src);
void display_printf(char *format, ...);
char *get_arg(char *string, char *argument);
int is_abbrev(char *s1, char *s2);
void process_input(int wait_for_new_line);
void read_config(char *file);
