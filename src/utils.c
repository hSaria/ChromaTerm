/* This program is protected under the GNU GPL (See COPYING) */

#include "defs.h"

/* TRUE if s1 is an abbrevation of s2 (case-insensitive) */
int isAbbrev(char *s1, char *s2) {
  if (*s1 == 0) {
    return FALSE;
  }
  return !strncasecmp(s2, s1, strlen(s1));
}

/* if waitForNewLine, process lines until the one without \n at the end */
void processInput(int waitForNewLine) {
  char *line, *nextLine;

  gd.inputBuf[gd.inputBufLen] = 0;

  /* separate into lines and process. Next interation = next line */
  for (line = gd.inputBuf; line && *line; line = nextLine) {
    char lineBuf[INPUT_MAX * 2];

    nextLine = strchr(line, '\n');

    if (nextLine) {
      *nextLine = 0;             /* Replace \n with a null-terminator */
      nextLine++;                /* Move the pointer to just after that \n */
    } else if (waitForNewLine) { /* Reached the last line */
      strcpy(lineBuf, line);
      strcpy(gd.inputBuf, lineBuf);
      gd.inputBufLen = (int)strlen(lineBuf);

      return; /* Leave and wait until called again without having to wait */
    }

    /* Print the output after processing it */
    strcpy(lineBuf, line);
    highlightString(lineBuf);

    printf("%s%s", lineBuf, nextLine ? "\n" : "");

    fflush(stdout);
  }

  /* If we reached this point, then there's no more output in the buffer */
  gd.inputBufLen = 0;
}

struct regExRes regExCompare(PCRE_CODE *compiledRegEx, char *str) {
  struct regExRes res;
#ifdef HAVE_PCRE2_H
  PCRE2_SIZE *resPos;
  pcre2_match_data *match =
      pcre2_match_data_create_from_pattern(compiledRegEx, NULL);

  if (pcre2_match(compiledRegEx, (PCRE2_SPTR)str, (int)strlen(str), 0, 0, match,
                  NULL) <= 0) {
    pcre2_match_data_free(match);
    res.start = -1;
    return res;
  }

  resPos = pcre2_get_ovector_pointer(match);

  res.start = (int)resPos[0];
  res.end = (int)resPos[1];

  pcre2_match_data_free(match);
#else
  int resPos[600];

  if (pcre_exec(compiledRegEx, NULL, str, (int)strlen(str), 0, 0, resPos,
                600) <= 0) {
    res.start = -1;
    return res;
  }

  res.start = (int)resPos[0];
  res.end = (int)resPos[1];
#endif

  if (res.start > res.end) {
    res.start = -1;
    return res;
  }

  return res;
}
