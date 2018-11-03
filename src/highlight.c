/* This program is protected under the GNU GPL (See COPYING) */

#include "defs.h"

struct color {
  char *name;
  char *code;
};

struct color colorTable[] = {
    {"bold", "<188>"},         {"dim", "<288>"},
    {"underscore", "<488>"},   {"blink", "<588>"},
    {"b azure", "<ABD>"},      {"b black", "<880>"},
    {"b blue", "<884>"},       {"b cyan", "<886>"},
    {"b ebony", "<G04>"},      {"b green", "<882>"},
    {"b jade", "<ADB>"},       {"b lime", "<BDA>"},
    {"b magenta", "<885>"},    {"b orange", "<DBA>"},
    {"b pink", "<DAB>"},       {"b red", "<881>"},
    {"b silver", "<CCC>"},     {"b tan", "<CBA>"},
    {"b violet", "<BAD>"},     {"b white", "<887>"},
    {"b yellow", "<883>"},     {"azure", "<abd>"},
    {"black", "<808>"},        {"blue", "<848>"},
    {"cyan", "<868>"},         {"ebony", "<g04>"},
    {"green", "<828>"},        {"jade", "<adb>"},
    {"light azure", "<acf>"},  {"light ebony", "<bbb>"},
    {"light jade", "<afc>"},   {"light lime", "<cfa>"},
    {"light orange", "<fca>"}, {"light pink", "<fac>"},
    {"light silver", "<eee>"}, {"light tan", "<eda>"},
    {"light violet", "<caf>"}, {"lime", "<bda>"},
    {"magenta", "<858>"},      {"orange", "<dba>"},
    {"pink", "<dab>"},         {"red", "<818>"},
    {"silver", "<ccc>"},       {"tan", "<cba>"},
    {"violet", "<bad>"},       {"white", "<878>"},
    {"yellow", "<838>"},       {"", "<088>"}};

/* Used to search for the start of a color */
PCRE_CODE *colorLookback;

void addHighlight(char *condition, char *action, char *priority) {
  if (*priority == 0) {
    strcpy(priority, "1000");
  }

  if (*condition == 0 || *action == 0) {
    if (gd.highlightsUsed == 0) {
      fprintf(stderr, "HIGHLIGHT: No rules configured\n");
    } else {
      int i;
      for (i = 0; i < gd.highlightsUsed; i++) {
        fprintf(stderr, "HIGHLIGHT {%s} {%s} {%s}\n",
                gd.highlights[i]->condition, gd.highlights[i]->action,
                gd.highlights[i]->priority);
      }
    }
  } else {
    char temp[BUFFER_SIZE];
    if (getHighlightCodes(action, temp) == FALSE) {
      fprintf(stderr, "ERROR: Invalid color code {%s}; see `man ct`\n", action);
    } else {
      PCRE_ERR_P errP;
      struct highlight *highlight;
      int errN, index, insertIndex;

      /* Remove if already exists */
      if ((index = findHighlightIndex(condition)) != -1) {
        delHighlight(gd.highlights[index]->condition);
      }

      highlight = (struct highlight *)calloc(1, sizeof(struct highlight));

      strcpy(highlight->condition, condition);
      strcpy(highlight->action, action);
      strcpy(highlight->priority, priority);

      getHighlightCodes(action, highlight->compiledAction);

      PCRE_COMPILE(highlight->compiledRegEx, condition, &errN, &errP);

      if (highlight->compiledRegEx == NULL) {
        fprintf(stderr, "WARNING: Couldn't compile RegEx %s\n", condition);
      }

      /* Find the insertion index; start at the bottom of the list */
      insertIndex = gd.highlightsUsed - 1;

      /* Highest value priority is at the bottom of the list (highest index) */
      while (insertIndex > -1) {
        double diff =
            atof(priority) - atof(gd.highlights[insertIndex]->priority);

        if (diff >= 0) {
          insertIndex++; /* Same priority or higher; insert after */
          break;
        }
        insertIndex--; /* Our priority is less than insertIndex's priorty */
      }

      /* index must be 0 or higher */
      index = 0 > insertIndex ? 0 : insertIndex;

      gd.highlightsUsed++;

      /* Resize if full; make it twice as big */
      if (gd.highlightsUsed == gd.highlightsSize) {
        gd.highlightsSize *= 2;

        gd.highlights = (struct highlight **)realloc(
            gd.highlights, gd.highlightsSize * sizeof(struct highlight *));
      }

      memmove(&gd.highlights[index + 1], &gd.highlights[index],
              (gd.highlightsUsed - index) * sizeof(struct highlight *));

      gd.highlights[index] = highlight;
    }
  }
}

void delHighlight(char *condition) {
  int index;

  if (*condition == 0) {
    fprintf(stderr, "SYNTAX: UNHIGHLIGHT {CONDITION}\n");
  } else if ((index = findHighlightIndex(condition)) != -1) {
    if (gd.highlights[index]->compiledRegEx != NULL) {
      PCRE_FREE(gd.highlights[index]->compiledRegEx);
    }

    free(gd.highlights[index]);

    memmove(&gd.highlights[index], &gd.highlights[index + 1],
            (gd.highlightsUsed - index) * sizeof(struct highlight *));

    gd.highlightsUsed--;
  } else {
    fprintf(stderr, "ERROR: Highlight rule not found\n");
  }
}

int findHighlightIndex(char *condition) {
  int i;
  for (i = 0; i < gd.highlightsUsed; i++) {
    if (!strcmp(condition, gd.highlights[i]->condition)) {
      return i;
    }
  }
  return -1;
}

int getHighlightCodes(char *string, char *result) {
  int matched = FALSE;
  *result = 0;

  while (isspace((int)*string)) {
    string++;
  }

  if (string[0] == '<' && string[4] == '>') {
    substitute(string, result);
    return TRUE;
  }

  while (*string) {
    if (isalpha((int)*string)) {
      int cnt;
      for (cnt = 0; *colorTable[cnt].name; cnt++) {
        if (isAbbrev(colorTable[cnt].name, string)) {
          substitute(colorTable[cnt].code, result);

          matched = TRUE;
          result += strlen(result);
          break;
        }
      }

      if (*colorTable[cnt].name == 0) {
        return FALSE;
      }

      /* Skip until the next action (maybe there are multiple colors) */
      string += strlen(colorTable[cnt].name);
    } else {
      string++;
    }

    while (isspace((int)*string) || *string == ',') {
      string++;
    }
  }

  return matched;
}

void highlightString(char *string) {
  int i;

  /* Apply from the top since the bottom ones may not match after the action of
   * one of the top ones is applied */
  for (i = 0; i < gd.highlightsUsed && gd.highlights[i]->compiledRegEx; i++) {
    char *pti = string;
    struct regExRes res = regExCompare(gd.highlights[i]->compiledRegEx, pti);

    if (res.start == -1) { /* No match */
      continue;            /* Move to the next RegEx */
    }

    char output[INPUT_MAX * 2];
    *output = 0;

    do {
      if (!gd.collidingActions) { /* Colliding action disallowed */
        char oldChar = pti[res.end];
        struct regExRes lookbackRes;

        pti[res.end] = 0; /* Stop at the match */
        lookbackRes = regExCompare(colorLookback, pti);
        pti[res.end] = oldChar; /* Restore old char */

        if (lookbackRes.start != -1) {   /* We're in the middle of an action */
          strncat(output, pti, res.end); /* Add current match to output */
          pti += res.end;                /* Seek to end of current match */
          res = regExCompare(gd.highlights[i]->compiledRegEx, pti);
          continue;
        }
      }

      strncat(output, pti, res.start);                        /* Before */
      strcat(output, gd.highlights[i]->compiledAction);       /* Action */
      strncat(output, pti += res.start, res.end - res.start); /* Match */
      strcat(output, "\033[0m");                              /* Reset */

      pti += res.end - res.start; /* Move pto to after the match */

      res = regExCompare(gd.highlights[i]->compiledRegEx, pti);
    } while (res.start != -1);

    /* Add the remainder of the string and then copy it to*/
    strcat(output, pti);
    strcpy(string, output);
  }
}

/* copy *string into *result, but replace colors with terminal codes */
void substitute(char *string, char *result) {
  char *pti = string, *pto = result;

  while (*pti) {
    if (pti[0] == '<' && pti[4] == '>') {
      if (isdigit((int)pti[1]) && isdigit((int)pti[2]) &&
          isdigit((int)pti[3])) {
        if (pti[1] != '8' || pti[2] != '8' || pti[3] != '8') {
          *pto++ = '\e';
          *pto++ = '[';

          switch (pti[1]) {
          case '8':
            break;
          default:
            *pto++ = pti[1];
            *pto++ = ';';
          }
          switch (pti[2]) {
          case '8':
            break;
          default:
            *pto++ = '3';
            *pto++ = pti[2];
            *pto++ = ';';
            break;
          }
          switch (pti[3]) {
          case '8':
            break;
          default:
            *pto++ = '4';
            *pto++ = pti[3];
            *pto++ = ';';
            break;
          }
          pto--;
          *pto++ = 'm';
        }
        pti += 5;
      } else if ((pti[1] >= 'a' && pti[1] <= 'f' && pti[2] >= 'a' &&
                  pti[2] <= 'f' && pti[3] >= 'a' && pti[3] <= 'f') ||
                 (pti[1] >= 'A' && pti[1] <= 'F' && pti[2] >= 'A' &&
                  pti[2] <= 'F' && pti[3] >= 'A' && pti[3] <= 'F') ||
                 ((pti[1] == 'g' || pti[1] == 'G') && isdigit((int)pti[2]) &&
                  isdigit((int)pti[3]))) {
        int cnt, grayscale = (pti[1] == 'g' || pti[1] == 'G');
        char g = (pti[1] >= 'a' && pti[1] <= 'f') || pti[1] == 'g' ? '3' : '4';

        *pto++ = '\e';
        *pto++ = '[';
        *pto++ = g;
        *pto++ = '8';
        *pto++ = ';';
        *pto++ = '5';
        *pto++ = ';';

        if (grayscale) {
          cnt = 232 + (pti[2] - '0') * 10 + (pti[3] - '0');
        } else {
          g = g == '3' ? 'a' : 'A'; /* 3 means foreground */
          cnt = 16 + (pti[1] - g) * 36 + (pti[2] - g) * 6 + (pti[3] - g);
        }

        *pto++ = '0' + cnt / 100;
        *pto++ = '0' + cnt % 100 / 10;
        *pto++ = '0' + cnt % 10;
        *pto++ = 'm';

        pti += 5;
      } else {
        *pto++ = *pti++;
      }
    } else {
      *pto++ = *pti++;
    }
  }

  *pto = 0;
}
