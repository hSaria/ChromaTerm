/* This program is protected under the GNU GPL (See COPYING) */

#include "defs.h"

DO_COMMAND(do_commands) {
  char add[BUFFER_SIZE], left[BUFFER_SIZE];
  int cmd;

  get_arg(arg, left);

  for (cmd = add[0] = 0; *command_table[cmd].name != 0; cmd++) {
    if (*left && !is_abbrev(left, command_table[cmd].name)) {
      continue;
    }

    sprintf(strchr(add, '\0'), "%-14s", command_table[cmd].name);
  }

  if (add[0]) {
    display_printf(add);
  }
}

DO_COMMAND(do_configure) {
  char left[BUFFER_SIZE], right[BUFFER_SIZE];

  arg = get_arg(arg, left);
  get_arg(arg, right);

  if (*left != 0) {
    if (is_abbrev(left, "COMMAND CHAR")) {
      if (*right != 0) {
        if (!ispunct((int)right[0])) {
          display_printf("ERROR: Commad character must me a punctuation: "
                         "!@#$%%^&*-+=',.\"\\/:;?_`<>()[]{}|~");
        } else {
          gd.command_char = right[0];
        }
      } else {
        display_printf("SYNTAX: CONFIG {COMMAND CHAR} {CHAR}");
      }
    } else if (is_abbrev(left, "CONVERT META")) {
      if (!strcasecmp(right, "ON")) {
        SET_BIT(gd.flags, SES_FLAG_CONVERTMETA);
      } else if (!strcasecmp(right, "OFF")) {
        DEL_BIT(gd.flags, SES_FLAG_CONVERTMETA);
      } else {
        display_printf("SYNTAX: CONFIG {CONVERT META} {ON|OFF}");
      }
    } else if (is_abbrev(left, "HIGHLIGHT")) {
      if (!strcasecmp(right, "ON")) {
        SET_BIT(gd.flags, SES_FLAG_HIGHLIGHT);
      } else if (!strcasecmp(right, "OFF")) {
        DEL_BIT(gd.flags, SES_FLAG_HIGHLIGHT);
      } else {
        display_printf("SYNTAX: CONFIG {HIGHLIGHT} {ON|OFF}");
      }
    } else if (is_abbrev(left, "READ")) {
      if (*right != 0) {
        do_read(right);
      } else {
        display_printf("SYNTAX: CONFIG {READ} {PATH/TO/FILE}");
      }
    } else if (is_abbrev(left, "WRITE")) {
      if (*right != 0) {
        do_write(right);
      } else {
        display_printf("SYNTAX: CONFIG {WRITE} {PATH/TO/FILE}");
      }
    } else {
      display_printf("ERROR: Unknown option '%s'", left);
    }
  } else {
    display_printf("%-12s = %-6c [%s]", "COMMAND CHAR", gd.command_char,
                   "The character used for CT-- commands");
    display_printf("%-12s = %-6s [%s]", "CONVERT META",
                   HAS_BIT(gd.flags, SES_FLAG_CONVERTMETA) ? "ON" : "OFF",
                   "Convert meta and control characters");
    display_printf("%-12s = %-6s [%s]", "HIGHLIGHT",
                   HAS_BIT(gd.flags, SES_FLAG_HIGHLIGHT) ? "ON" : "OFF",
                   "Highlight according to rules");
    display_printf("%-21s [%s]", "READ", "Read configuration from a file");
    display_printf("%-21s [%s]", "WRITE", "Write configuration to a file");
  }
}

DO_COMMAND(do_help) {
  char left[BUFFER_SIZE];
  int cnt;

  get_arg(arg, left);

  if (*left == 0) {
    char add[BUFFER_SIZE];
    for (cnt = add[0] = 0; *help_table[cnt].name != 0; cnt++) {
      sprintf(strchr(add, '\0'), "%-14s", help_table[cnt].name);
    }
    if (add[0]) {
      display_printf(add);
    }
  } else {
    int found = FALSE;
    for (cnt = 0; *help_table[cnt].name != 0; cnt++) {
      if (is_abbrev(left, help_table[cnt].name) || is_abbrev(left, "all")) {
        char buf[BUFFER_SIZE];
        found = TRUE;

        substitute(help_table[cnt].text, buf);
        display_printf(buf);
      }
    }

    if (!found) {
      display_printf("ERROR: No help found for topic '%s'", left);
    }
  }
}

DO_COMMAND(do_highlight) {
  char condition[BUFFER_SIZE], action[BUFFER_SIZE], priority[BUFFER_SIZE];

  arg = get_arg(arg, condition);
  arg = get_arg(arg, action);
  get_arg(arg, priority);

  if (*priority == 0) {
    strcpy(priority, "1000");
  }

  if (*condition == 0 || *action == 0) {
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
    if (get_highlight_codes(action, temp) == FALSE) {
      display_printf("ERROR: Invalid color code; see %%HELP HIGHLIGHT");
    } else {
#ifdef HAVE_PCRE2_H
      PCRE2_SIZE error_pointer;
#else
      const char *error_pointer;
#endif

      struct highlight *highlight;
      int error_number, index, insert_index;

      /* Remove if already exists */
      if ((index = find_highlight_index(condition)) != -1) {
        do_unhighlight(gd.highlights[index]->condition);
      }

      highlight = (struct highlight *)calloc(1, sizeof(struct highlight));

      strcpy(highlight->condition, condition);
      strcpy(highlight->action, action);
      strcpy(highlight->priority, priority);

      get_highlight_codes(action, highlight->compiled_action);

#ifdef HAVE_PCRE2_H
      highlight->compiled_regex =
          pcre2_compile((PCRE2_SPTR)condition, PCRE2_ZERO_TERMINATED, 0,
                        &error_number, &error_pointer, NULL);
#else
      highlight->compiled_regex =
          pcre_compile(condition, 0, &error_pointer, &error_number, NULL);
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
      insert_index = gd.highlights_used - 1;

      /* Highest value priority is at the bottom of the list (highest index) */
      while (insert_index > -1) {
        double diff =
            atof(priority) - atof(gd.highlights[insert_index]->priority);

        if (diff >= 0) {
          insert_index++; /* Same priority or higher; insert after */
          break;
        }
        insert_index--; /* Our priority is less than insert_index's priorty */
      }

      index = 0 > insert_index ? 0 : insert_index;

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

DO_COMMAND(do_read) {
  FILE *fp;
  struct stat filedata;
  char *bufi, *bufo, filename[BUFFER_SIZE], *pti, *pto;
  int lvl, com, lnc;
  wordexp_t p;

  get_arg(arg, filename);

  if (wordexp(filename, &p, 0) == 0) {
    strcpy(filename, *p.we_wordv);
    wordfree(&p);
  } else {
    display_printf("ERROR: Failed while performing word expansion on {%s}",
                   filename);
    return;
  }

  if ((fp = fopen(filename, "r")) == NULL) {
    display_printf("ERROR: File {%s} not found", filename);
    return;
  }

  stat(filename, &filedata);

  if ((bufi = (char *)calloc(1, filedata.st_size + 2)) == NULL) {
    display_printf(
        "ERROR: Failed to allocate i_buffer memory to process file {%s}",
        filename);
    fclose(fp);
    return;
  } else if ((bufo = (char *)calloc(1, filedata.st_size + 2)) == NULL) {
    display_printf(
        "ERROR: Failed to allocate o_buffer memory to process file {%s}",
        filename);
    free(bufi);
    fclose(fp);
    return;
  }

  if (fread(bufi, 1, filedata.st_size, fp) == 0) {
    display_printf("ERROR: File is empty", filename);
    free(bufi);
    free(bufo);
    fclose(fp);
    return;
  };

  fclose(fp); /* Done with FILE */

  pti = bufi;
  pto = bufo;

  lvl = com = lnc = 0;

  while (*pti) {
    if (com == 0) { /* Not in a comment */
      switch (*pti) {
      case DEFAULT_OPEN:
        *pto++ = *pti++;
        lvl++;
        break;
      case DEFAULT_CLOSE:
        *pto++ = *pti++;
        lvl--;
        break;
      case ' ':
        *pto++ = *pti++;
        break;
      case '/':                          /* Check if comment */
        if (lvl == 0 && pti[1] == '*') { /* lvl == 0 means not in an argument */
          pti += 2;
          com += 1;
        } else {
          *pto++ = *pti++;
        }
        break;
      case '\r': /* skip \r (we expect \n) */
        pti++;
        break;
      case '\n':
        if (lvl) { /* Closing brackets missing; remove command */
          char *previous_line = strrchr(pto, '\n');
          if (previous_line) { /* There's a previous line */
            previous_line--;   /* Go back one character before \n */
            pto = previous_line;
          } else { /* Go back to beginning of bufo (no previous line) */
            pto = bufo;
          }
        }

        *pto++ = *pti++;
        lnc++;
        break;
      default:
        *pto++ = *pti++;
        break;
      }
    } else { /* In a comment */
      switch (*pti) {
      case '/':
        if (pti[1] == '*') { /* Comment in a comment */
          pti += 2;
          com += 1;
        } else {
          pti++;
        }
        break;
      case '*':
        if (pti[1] == '/') { /* Comment close */
          pti += 2;
          com -= 1;
        } else {
          pti++;
        }
        break;
      default: /* Advance forward (we're in a comment) */
        pti++;
        break;
      }
    }
  }

  *pto++ = '\n';
  *pto = 0;

  pti = bufo;
  pto = bufo;

  while (*pti) {
    if (*pti != '\n') { /* Seek until you reach \n */
      pti++;
      continue;
    }
    *pti = 0; /* replace \n with null-terminator */

    if (strlen(pto) >= BUFFER_SIZE) {
      /* Only output the first 20 characters of the overflowing command */
      *(pto + 20) = 0;
      display_printf("ERROR: Command too long {%s}", pto);

      free(bufi);
      free(bufo);
      return;
    }

    if (*pto) {
      script_driver(pto);
    }

    pti++; /* Move to the position after the null-terminator */
    pto = pti;
  }

  free(bufi);
  free(bufo);
}

DO_COMMAND(do_showme) {
  char *pto = arg, buf[BUFFER_SIZE];

  while (isspace((int)*pto)) {
    pto++;
  }

  strcpy(buf, pto);

  check_all_highlights(buf);
  display_printf(buf);
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
      pcre_free(highlight->compiled_regex);
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

DO_COMMAND(do_write) {
  FILE *file;
  char filename[BUFFER_SIZE], result[BUFFER_SIZE * 4];
  int i;
  wordexp_t p;

  get_arg(arg, filename);

  if (wordexp(filename, &p, 0) == 0) {
    strcpy(filename, *p.we_wordv);
    wordfree(&p);
  } else {
    display_printf("ERROR: Failed while performing word expansion on {%s}",
                   filename);
    return;
  }

  if ((file = fopen(filename, "w")) == NULL) {
    display_printf("ERROR: {%s} - Failed while attempting to write", filename);
    return;
  }

  sprintf(result,
          "CONFIG {CONVERT META} {%s}\n"
          "CONFIG {HIGHLIGHT} {%s}\n\n",
          HAS_BIT(gd.flags, SES_FLAG_CONVERTMETA) ? "ON" : "OFF",
          HAS_BIT(gd.flags, SES_FLAG_HIGHLIGHT) ? "ON" : "OFF");
  fputs(result, file);

  for (i = 0; i < gd.highlights_used; i++) {
    sprintf(result, "HIGHLIGHT {%s} {%s} {%s}\n", gd.highlights[i]->condition,
            gd.highlights[i]->action, gd.highlights[i]->priority);
    fputs(result, file);
  }

  fclose(file);
}
