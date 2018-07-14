/* This program is protected under the GNU GPL (See COPYING) */

#include "config.h"

#include <ctype.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <wordexp.h>

/**** Compatibility interface ****/
#ifdef HAVE_PCRE2_H /* PCRE2 */
#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>
typedef pcre2_code PCRE_CODE;
typedef PCRE2_SIZE PCRE_ERR_P;
#define PCRE_FREE(code)                                                        \
  { pcre2_code_free(code); }
#define PCRE_COMPILE(compiled, regex, err_n, err_p)                            \
  {                                                                            \
    compiled = pcre2_compile((PCRE2_SPTR)regex, PCRE2_ZERO_TERMINATED, 0,      \
                             err_n, err_p, NULL);                              \
    if (pcre2_jit_compile(compiled, 0) == 0) {                                 \
      /* Accelerate pattern matching if JIT is supported on the platform */    \
      pcre2_jit_compile(compiled, PCRE2_JIT_COMPLETE);                         \
    }                                                                          \
  }
#else /* Legacy PCRE */
#include <pcre.h>
typedef pcre PCRE_CODE;
typedef const char *PCRE_ERR_P;
#define PCRE_FREE(code)                                                        \
  { pcre_free(code); }
#define PCRE_COMPILE(compiled, regex, err_n, err_p)                            \
  { compiled = pcre_compile(regex, 0, err_p, err_n, NULL); }
#endif

#define VERSION "0.2.5"

#define FALSE 0
#define TRUE 1

#define DEFAULT_OPEN '{'
#define DEFAULT_CLOSE '}'

#define BUFFER_SIZE 16384

#define INPUT_MAX 262144 /* 256 KiB */

/* Microseconds to wait before processing a line without \n at the end */
#define WAIT_FOR_NEW_LINE 500
/* Why: CT-- cannot determine if a line has finished printing or if CT--'s
 * processing speed is just faster than the output of the piping process.
 * Therefore, on lines that do not end with \n, CT-- will wait a fixed interval
 * prior to printing that line. However, in the case that the line does end with
 * a \n, process it right away and move on to the next line. The delay will only
 * affect the last line; if you're outputting thousands of lines back-to-back,
 * they'll be processed as soon as possible since each line will end with \n */

#define ESCAPE 27

struct global_data { /* Stores the shared data for CT-- */
  struct highlight **highlights;
  int highlights_size;
  int highlights_used;
  char input_buffer[INPUT_MAX];
  int input_buffer_length;
  int colliding_actions; /* Allow or disallow colliding actions */
};

struct highlight {
  char condition[BUFFER_SIZE];       /* Processed into compiled_regex */
  char action[BUFFER_SIZE];          /* Processed into compiled_action */
  char priority[BUFFER_SIZE];        /* Lower value = better priority */
  char compiled_action[BUFFER_SIZE]; /* Compiled once, used multiple times */
  PCRE_CODE *compiled_regex;         /* Compiled once, used multiple times */
};

struct regex_r {
  int start;
  int end;
};

/**** highlight.c ****/
extern PCRE_CODE *lookback_for_color;

void check_highlights(char *string);
int find_highlight_index(char *text);
int get_highlight_codes(char *string, char *result);
void highlight(char *condition, char *action, char *priority);
void substitute(char *string, char *result);
void unhighlight(char *condition);

/**** main.c ****/
extern struct global_data gd;

int main(int argc, char **argv);
void colordemo(void);
void quit_with_signal(int exit_signal);

/**** utils.c ****/
char *get_arg(char *string, char *argument);
int is_abbrev(char *s1, char *s2);
void process_input(int wait_for_new_line);
void read_config(char *file);
struct regex_r regex_compare(PCRE_CODE *compiled_regex, char *str);
