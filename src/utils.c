/* This program is protected under the GNU GPL (See COPYING) */

#include "defs.h"

void display_printf(char *format, ...) {
  char buf[BUFFER_SIZE * 4];
  va_list args;

  va_start(args, format);
  vsprintf(buf, format, args);
  va_end(args);

  strcat(buf, "\n");

  write(STDERR_FILENO, buf, strlen(buf));
}

/* The outer-most brackets (if any) are stripped; all else left as is */
char *get_arg(char *string, char *result) {
  char *pti = string, *pto = result;

  while (isspace((int)*pti)) { /* advance to the next none-space character */
    pti++;
  }

  /* Not wrapped in brackets; use space as separator */
  if (*pti != DEFAULT_OPEN) {
    while (*pti) {
      if (isspace((int)*pti)) {
        pti++;
        break;
      }
      *pto++ = *pti++;
    }
  } else { /* Wrapped in brackets; use outer-most as separator */
    int nest = 1;

    pti++; /* Advance past the DEFAULT_OPEN (nest is 1 for this reason) */

    while (*pti) {
      if (*pti == DEFAULT_OPEN) {
        nest++;
      } else if (*pti == DEFAULT_CLOSE) {
        nest--;

        /* Stop once we've got the close bracket for the first open bracket */
        if (nest == 0) {
          break;
        }
      }
      *pto++ = *pti++;
    }

    if (*pti == 0) {
      display_printf("ERROR: Missing %i closing bracket(s)", nest);
    } else {
      pti++; /* Move over the closing bracket */
    }
  }

  *pto = '\0';
  return pti;
}

/* TRUE if s1 is an abbrevation of s2 (case-insensitive) */
int is_abbrev(char *s1, char *s2) {
  if (*s1 == 0) {
    return FALSE;
  }
  return !strncasecmp(s2, s1, strlen(s1));
}

/* if wait_for_new_line, will process all lines until the one without \n at the
 * end */
void process_input(int wait_for_new_line) {
  char *line, *next_line;

  gd.input_buffer[gd.input_buffer_length] = 0;

  /* separate into lines and process. Next interation = next line */
  for (line = gd.input_buffer; line && *line; line = next_line) {
    char linebuf[INPUT_MAX * 2];

    next_line = strchr(line, '\n');

    if (next_line) {
      *next_line = 0;               /* Replace \n with a null-terminator */
      next_line++;                  /* Move the pointer to just after that \n */
    } else if (wait_for_new_line) { /* Reached the last line */
      strcpy(linebuf, line);
      strcpy(gd.input_buffer, linebuf);
      gd.input_buffer_length = (int)strlen(linebuf);

      return; /* Leave and wait until called again without having to wait */
    }

    /* Print the output after processing it */
    strcpy(linebuf, line);
    check_all_highlights(linebuf);

    if (next_line) {
      strcat(linebuf, "\n");
    }

    printf("%s", linebuf);

    fflush(stdout);
  }

  /* If we reached this point, then there's no more output in the buffer */
  gd.input_buffer_length = 0;
}

void read_config(char *file) {
  FILE *fp;
  struct stat filedata;
  char *bufi, *bufo, filename[BUFFER_SIZE], *pti, *pto;
  int nest = 0, com = FALSE, line_number = 1;
  wordexp_t p;

  if (wordexp(file, &p, 0) == 0) {
    strcpy(filename, *p.we_wordv);
    wordfree(&p);
  } else {
    display_printf("ERROR: Failed while performing word expansion on {%s}",
                   file);
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
    display_printf("ERROR: File {%s} is empty", filename);
    free(bufi);
    free(bufo);
    fclose(fp);
    return;
  };

  fclose(fp); /* Done with FILE */

  pti = bufi;
  pto = bufo;

  while (*pti) {
    if (com == FALSE) { /* Not in a comment */
      switch (*pti) {
      case DEFAULT_OPEN:
        *pto++ = *pti++;
        nest++;
        break;
      case DEFAULT_CLOSE:
        *pto++ = *pti++;
        nest--;
        break;
      case '/': /* Check if comment */
        if (nest == 0 &&
            pti[1] == '*') { /* nest == 0 means not in an argument */
          pti += 2;
          com = TRUE;
        } else {
          *pto++ = *pti++;
        }
        break;
      case '\r': /* skip \r (we expect \n) */
        pti++;
        break;
      case '\n':
        if (nest) { /* Closing brackets missing; remove command */
          char *previous_line = strrchr(bufo, '\n');

          display_printf("ERROR: Missing %i closing bracket(s) at line %i",
                         nest, line_number);

          nest = 0; /* Reset the level to 0 (brackets are all matched) */

          if (previous_line) {   /* There's a previous line */
            pto = previous_line; /* Go back to the last sane line */
          } else { /* Go back to beginning of bufo (no previous line) */
            pto = bufo;
          }
        }

        line_number++;
        *pto++ = *pti++;
        break;
      default:
        *pto++ = *pti++;
        break;
      }
    } else { /* In a comment */
      switch (*pti) {
      case '*':
        if (pti[1] == '/') { /* Comment close */
          pti += 2;
          com = FALSE;
        } else {
          pti++;
        }
        break;
      case '\n':
        line_number++;
        pti++;
        break;
      default: /* Advance forward (we're in a comment) */
        pti++;
        break;
      }
    }
  }

  *pto++ = '\n'; /* Ensure there's a \n at the end of the last command */
  *pto = 0;

  pti = bufo;

  while (*pti) {
    char *args, command[BUFFER_SIZE];

    while (isspace((int)*pti)) {
      pti++;
    }

    if (*pti == 0) {
      break;
    }

    pto = pti;               /* Start of command */
    pti = strchr(pti, '\n'); /* End of command */
    *pti = 0;                /* Replace \n with null */
    pti++; /* Move to the position after the null-terminator */

    if (strlen(pto) > BUFFER_SIZE) {
      *(pto + 20) = 0; /* Only output the first 20 characters  */
      display_printf("ERROR: Command too long {%s}", pto);
      continue;
    }

    args = get_arg(pto, command);

    if (is_abbrev(command, "HIGHLIGHT")) {
      highlight(args);
    } else if (is_abbrev(command, "SHOWME")) {
      char buf[BUFFER_SIZE];

      strcpy(buf, pto);

      check_all_highlights(buf);
      display_printf(buf);
    } else if (is_abbrev(command, "UNHIGHLIGHT")) {
      unhighlight(args);
    } else {
      display_printf("ERROR: Unknown command {%s}", command);
    }
  }

  free(bufi);
  free(bufo);
}
