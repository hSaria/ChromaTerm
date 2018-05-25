/* This program is protected under the GNU GPL (See COPYING) */

#include "defs.h"

DO_COMMAND(do_highlight) {
  char arg1[BUFFER_SIZE], arg2[BUFFER_SIZE], arg3[BUFFER_SIZE];

  arg = get_arg(arg, arg1);
  arg = get_arg(arg, arg2);
  get_arg(arg, arg3);

  if (*arg3 == 0) {
    strcpy(arg3, "1000");
  }

  if (*arg1 == 0 || *arg2 == 0) {
    if (gd.highlights_used == 0) {
      display_printf("HIGHLIGHT: No rules configured");
    } else {
      int i;
      for (i = 0; i < gd.highlights_used; i++) {
        display_printf("HIGHLIGHT "
                       "\033[1;31m{\033[0m%s\033[1;31m}\033[1;36m "
                       "\033[1;31m{\033[0m%s\033[1;31m}\033[1;36m "
                       "\033[1;31m{\033[0m%s\033[1;31m}\033[0m",
                       gd.highlights[i]->condition, gd.highlights[i]->action,
                       gd.highlights[i]->priority);
      }
    }
  } else {
    char temp[BUFFER_SIZE];
    if (get_highlight_codes(arg2, temp) == FALSE) {
      display_printf("ERROR: Invalid color code; see %%HELP HIGHLIGHT");
    } else {
#ifdef HAVE_PCRE2_H
      PCRE2_SIZE error_pointer;
#else
#ifdef HAVE_PCRE_H
      const char *error_pointer;
#endif
#endif

      struct highlight *highlight;
      int bot, top, val, error_number, index = find_highlight_index(arg1);

      /* Remove if already exists */
      if (index != -1) {
        do_unhighlight(gd.highlights[index]->condition);
      }

      highlight = (struct highlight *)calloc(1, sizeof(struct highlight));

      strcpy(highlight->condition, arg1);
      strcpy(highlight->action, arg2);
      strcpy(highlight->priority, arg3);

      get_highlight_codes(arg2, highlight->compiled_action);

#ifdef HAVE_PCRE2_H
      highlight->compiled_regex =
          pcre2_compile((PCRE2_SPTR)arg1, PCRE2_ZERO_TERMINATED, 0,
                        &error_number, &error_pointer, NULL);
#else
#ifdef HAVE_PCRE_H
      highlight->compiled_regex =
          pcre_compile(arg1, 0, &error_pointer, &error_number, NULL);
#endif
#endif

      if (highlight->compiled_regex == NULL) {
        display_printf("WARNING: Couldn't compile regex at %i: %s",
                       error_number, error_pointer);
      } else {
#ifdef HAVE_PCRE2_H
        if (pcre2_jit_compile(highlight->compiled_regex, 0) == 0) {
          /* Accelerate pattern matching if JIT is supported on the platform */
          pcre2_jit_compile(highlight->compiled_regex, PCRE2_JIT_COMPLETE);
        }
#endif
      }

      /* Find the insertion index */
      bot = 0;
      top = gd.highlights_used - 1;
      val = top;

      while (bot <= top) {
        double srt = atof(arg3) - atof(gd.highlights[val]->priority);

        /* Same priority */
        if (srt == 0) {
          break;
        }

        if (srt < 0) {
          top = val - 1;
        } else {
          bot = val + 1;
        }

        val = bot + (top - bot) / 2;
      }

      index = 0 > val ? 0 : val;

      gd.highlights_used++;

      /* Expand if full; make it twice as big */
      if (gd.highlights_used == gd.highlights_size) {
        gd.highlights_size *= 2;

        gd.highlights = (struct highlight **)realloc(
            gd.highlights, gd.highlights_size * sizeof(struct highlight *));
      }

      memmove(&gd.highlights[index + 1], &gd.highlights[index],
              (gd.highlights_used - index) * sizeof(struct highlight *));

      gd.highlights[index] = highlight;
    }
  }
}

DO_COMMAND(do_unhighlight) {
  int index;

  get_arg(arg, arg);

  if (*arg == 0) {
    display_printf("SYNTAX: UNHIGHLIGHT {CONDITION}");

  } else if ((index = find_highlight_index(arg)) != -1) {
    struct highlight *highlight = gd.highlights[index];

    if (highlight->compiled_regex != NULL) {
#ifdef HAVE_PCRE2_H
      pcre2_code_free(highlight->compiled_regex);
#else
#ifdef HAVE_PCRE_H
      pcre_free(highlight->compiled_regex);
#endif
#endif
    }

    free(highlight);

    memmove(&gd.highlights[index], &gd.highlights[index + 1],
            (gd.highlights_used - index) * sizeof(struct highlight *));

    gd.highlights_used--;
  } else {
    display_printf("ERROR: Highlight rule not found");
  }
}

void check_all_highlights(char *original) {
  char stripped[BUFFER_SIZE];
  int i;

  strip_vt102_codes(original, stripped);

  /* Apply from the bottom since the top ones may overwrite them */
  for (i = gd.highlights_used - 1; i > -1; i--) {
    struct regex_result result =
        regex_compare(gd.highlights[i]->compiled_regex, stripped);

    if (result.start != -1) {
      char *pto, *pts, *ptm;
      char output[BUFFER_SIZE];

      *output = 0;

      pto = ptm = original;
      pts = stripped;

      do {
        int count_inc_skipped = 0; /* Skipped bytes until the match */
        int to_skip =
            (int)strlen(result.match); /* Number of chars to skip in match */
        char *ptt;

        /* Seek ptm (original with vt102 codes) until beginning of match */
        while (*ptm && result.start > 0) {
          while (skip_vt102_codes(ptm)) {
            count_inc_skipped += skip_vt102_codes(ptm);
            ptm += skip_vt102_codes(ptm);
          }

          if (*ptm && *pts) {
            ptm++;
            pts++;
            count_inc_skipped++;
          }

          result.start--;
        }

        ptt = ptm;

        while (*ptt && to_skip > 0) {
          while (skip_vt102_codes(ptt)) {
            ptt += skip_vt102_codes(ptt);
          }

          if (*ptt) {
            ptt++;
          }

          to_skip--;
        }

        cat_sprintf(output, "%.*s%s%s\033[0m", count_inc_skipped, pto,
                    gd.highlights[i]->compiled_action, result.match);

        /* Move pto to after the match, and skip any vt102 codes, too. */
        pto = ptt;
        ptm = pto; /* Sync: next iteration should simulate a fresh call */

        /* Move to the remaining of the stripped string */
        pts = strstr(pts, result.match);
        pts = pts + strlen(result.match);

        result = regex_compare(gd.highlights[i]->compiled_regex, pts);
      } while (result.start != -1);

      /* Add the remainder of the string and then copy it to*/
      strcat(output, pto);
      strcpy(original, output);
    }
  }
}

int find_highlight_index(char *condition) {
  int i;
  for (i = 0; i < gd.highlights_used; i++) {
    if (!strcmp(condition, gd.highlights[i]->condition)) {
      return i;
    }
  }
  return -1;
}

int get_highlight_codes(char *string, char *result) {
  *result = 0;

  if (*string == '<') {
    substitute(string, result);
    return TRUE;
  }

  while (*string) {
    if (isalpha((int)*string)) {
      int cnt;
      for (cnt = 0; *color_table[cnt].name; cnt++) {
        if (is_abbrev(color_table[cnt].name, string)) {
          substitute(color_table[cnt].code, result);

          result += strlen(result);
          break;
        }
      }

      if (*color_table[cnt].name == 0) {
        return FALSE;
      }

      /* Skip until the next action (maybe there are multiple colors) */
      string += strlen(color_table[cnt].name);
    }

    switch (*string) {
    case ' ':
    case ',':
      string++;
      break;
    case 0:
      return TRUE;
    default:
      return FALSE;
    }
  }
  return TRUE;
}

#ifdef HAVE_PCRE2_H
struct regex_result regex_compare(pcre2_code *compiled_regex, char *str) {
  PCRE2_SIZE *result_pos;
  struct regex_result result;

  pcre2_match_data *match =
      pcre2_match_data_create_from_pattern(compiled_regex, NULL);

  if (pcre2_match(compiled_regex, (PCRE2_SPTR)str, (int)strlen(str), 0, 0,
                  match, NULL) <= 0) {
    pcre2_match_data_free(match);
    result.start = -1;
    return result;
  }

  result_pos = pcre2_get_ovector_pointer(match);

  if (result_pos[0] > result_pos[1]) {
    result.start = -1;
    return result;
  }

  sprintf(result.match, "%.*s", (int)(result_pos[1] - result_pos[0]),
          &str[result_pos[0]]);
  result.start = (int)result_pos[0];

  pcre2_match_data_free(match);

  return result;
}
#else
#ifdef HAVE_PCRE_H
struct regex_result regex_compare(pcre *compiled_regex, char *str) {
  struct regex_result result;
  int match[2000];

  if (pcre_exec(compiled_regex, NULL, str, (int)strlen(str), 0, 0, match,
                2000) <= 0) {
    result.start = -1;
    return result;
  }

  sprintf(result.match, "%.*s", (int)(match[1] - match[0]), &str[match[0]]);
  result.start = (int)match[0];

  return result;
}
#endif
#endif

int skip_vt102_codes(char *str) {
  int skip;

  switch (str[0]) {
  case 5:   /* ENQ */
  case 7:   /* BEL */
  case 8:   /* BS  */
  case 11:  /* VT  */
  case 12:  /* FF  */
  case 13:  /* CR  */
  case 14:  /* SO  */
  case 15:  /* SI  */
  case 17:  /* DC1 */
  case 19:  /* DC3 */
  case 24:  /* CAN */
  case 26:  /* SUB */
  case 127: /* DEL */
    return 1;
  case 27: /* ESC */
    break;
  default:
    return 0;
  }

  switch (str[1]) {
  case '\0':
    return 1;
  case '%':
  case '#':
  case '(':
  case ')':
    return str[2] ? 3 : 2;
  case ']':
    switch (str[2]) {
    case 'P':
      for (skip = 3; skip < 10; skip++) {
        if (str[skip] == 0) {
          break;
        }
      }
      return skip;
    case 'R':
      return 3;
    }
    return 2;
  case '[':
    break;
  default:
    return 2;
  }

  for (skip = 2; str[skip] != 0; skip++) {
    if (isalpha((int)str[skip])) {
      return skip + 1;
    }

    switch (str[skip]) {
    case '@':
    case '`':
    case ']':
      return skip + 1;
    }
  }
  return skip;
}

/* If n is not null, then this function seeks n times, exluding vt102 codes */
void strip_vt102_codes(char *str, char *buf) {
  char *pti, *pto;

  pti = str;
  pto = buf;

  while (*pti) {
    int skip;
    while ((skip = skip_vt102_codes(pti))) {
      pti += skip;
    }

    if (*pti) {
      *pto++ = *pti++;
    }
  }

  *pto = 0;
}

/* copy *string into *result, but substitute the various colors with the
 * values they stand for */
void substitute(char *string, char *result) {
  char *pti = string, *pto = result;
  char old[6] = {0};

  while (TRUE) {
    switch (*pti) {
    case '\0':
      *pto = 0;
      return;
    case '<':
      if (isdigit((int)pti[1]) && isdigit((int)pti[2]) &&
          isdigit((int)pti[3]) && pti[4] == '>') {
        if (pti[1] != '8' || pti[2] != '8' || pti[3] != '8') {
          *pto++ = ESCAPE;
          *pto++ = '[';

          switch (pti[1]) {
          case '2':
            *pto++ = '2';
            *pto++ = '2';
            *pto++ = ';';
            break;
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
        pti += sprintf(old, "<%c%c%c>", pti[1], pti[2], pti[3]);
      } else if (((pti[1] >= 'a' && pti[1] <= 'f' && pti[2] >= 'a' &&
                   pti[2] <= 'f' && pti[3] >= 'a' && pti[3] <= 'f') ||
                  (pti[1] >= 'A' && pti[1] <= 'F' && pti[2] >= 'A' &&
                   pti[2] <= 'F' && pti[3] >= 'A' && pti[3] <= 'F') ||
                  ((pti[1] == 'g' || pti[1] == 'G') && isdigit((int)pti[2]) &&
                   isdigit((int)pti[3]))) &&
                 pti[4] == '>') {
        int cnt, grayscale = (pti[1] == 'g' || pti[1] == 'G');
        char g = (pti[1] >= 'a' && pti[1] <= 'f') || pti[1] == 'g' ? '3' : '4';

        *pto++ = ESCAPE;
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

        pti += sprintf(old, "<%c%c%c>", pti[1], pti[2], pti[3]);
      } else {
        *pto++ = *pti++;
      }
      break;
    default:
      *pto++ = *pti++;
      break;
    }
  }
}
