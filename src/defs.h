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
#define PCRE_COMPILE(compiled, regEx, errN, errP)                              \
  {                                                                            \
    compiled = pcre2_compile((PCRE2_SPTR)regEx, PCRE2_ZERO_TERMINATED, 0,      \
                             errN, errP, NULL);                                \
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
#define PCRE_COMPILE(compiled, regEx, errN, errP)                              \
  { compiled = pcre_compile(regEx, 0, errP, errN, NULL); }
#endif

#define VERSION "0.2.7"

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

struct globalData { /* Stores the shared data for CT-- */
  struct highlight **highlights;
  int highlightsSize;
  int highlightsUsed;
  char inputBuf[INPUT_MAX];
  int inputBufLen;
  int collidingActions; /* Allow or disallow colliding actions */
};

struct highlight {
  char condition[BUFFER_SIZE];      /* Processed into compiledRegEx */
  char action[BUFFER_SIZE];         /* Processed into compiledAction */
  char priority[BUFFER_SIZE];       /* Lower value = better priority */
  char compiledAction[BUFFER_SIZE]; /* Compiled once, used multiple times */
  PCRE_CODE *compiledRegEx;         /* Compiled once, used multiple times */
};

struct regExRes {
  int start;
  int end;
};

/**** highlight.c ****/
extern PCRE_CODE *colorLookback;

void highlightString(char *string);
int findHighlightIndex(char *text);
int getHighlightCodes(char *string, char *result);
void highlight(char *condition, char *action, char *priority);
void substitute(char *string, char *result);
void unhighlight(char *condition);

/**** main.c ****/
extern struct globalData gd;

int main(int argc, char **argv);
void colorDemo(void);
void exitWithSignal(int exitSignal);

/**** utils.c ****/
char *getArg(char *string, char *argument);
int isAbbrev(char *s1, char *s2);
void processInput(int waitForNewLine);
void readConfig(char *file);
struct regExRes regExCompare(PCRE_CODE *compiledRegEx, char *str);
