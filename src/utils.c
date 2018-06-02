/* This program is protected under the GNU GPL (See COPYING) */

#include "defs.h"

void convert_meta(char *input, char *output) {
  char *pti = input, *pto = output;

  while (*pti) {
    switch (*pti) {
    case ESCAPE:
      *pto++ = '\\';
      *pto++ = 'e';
      pti++;
      break;
    case 127:
      *pto++ = '\\';
      *pto++ = 'b';
      pti++;
      break;
    case '\a':
      *pto++ = '\\';
      *pto++ = 'a';
      pti++;
      break;
    case '\b':
      *pto++ = '\\';
      *pto++ = 'b';
      pti++;
      break;
    case '\t':
      *pto++ = '\\';
      *pto++ = 't';
      pti++;
      break;
    case '\r':
      *pto++ = '\\';
      *pto++ = 'r';
      pti++;
      break;
    case '\n':
      *pto++ = *pti++;
      break;
    default:
      if (*pti > 0 && *pti < 32) {
        *pto++ = '\\';
        *pto++ = 'c';
        if (*pti <= 26) {
          *pto++ = 'a' + *pti - 1;
        } else {
          *pto++ = 'A' + *pti - 1;
        }
        pti++;
        break;
      } else {
        *pto++ = *pti++;
      }
      break;
    }
  }
  *pto = 0;
}

void display_printf(char *format, ...) {
  char buf[BUFFER_SIZE * 4];
  va_list args;

  va_start(args, format);
  vsprintf(buf, format, args);
  va_end(args);

  strcat(buf, "\n");

  write(STDERR_FILENO, buf, strlen(buf));
}

/* The outer-most braces (if any) are stripped; all else left as is */
char *get_arg(char *string, char *result) {
  char *pti, *pto, output[BUFFER_SIZE];

  /* advance to the next none-space character */
  pti = string;
  pto = output;

  while (isspace((int)*pti)) {
    pti++;
  }

  /* Use a space as the separator if not wrapped with braces */
  if (*pti != DEFAULT_OPEN) {
    while (*pti) {
      if (isspace((int)*pti)) {
        pti++;
        break;
      }
      *pto++ = *pti++;
    }
  } else {
    int nest = 1;

    pti++; /* Advance past the DEFAULT_OPEN (nest is 1 for this reason) */

    while (*pti) {
      if (*pti == DEFAULT_OPEN) {
        nest++;
      } else if (*pti == DEFAULT_CLOSE) {
        nest--;

        /* Stop once we've met the closing backet for the openning we advanced
         * past before this loop */
        if (nest == 0) {
          break;
        }
      }
      *pto++ = *pti++;
    }

    if (*pti == 0) {
      display_printf("ERROR: Missing %i closing bracket(s)", nest);
    } else {
      pti++;
    }
  }

  *pto = '\0';

  strcpy(result, output);
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

  /* separate into lines and print away */
  for (line = gd.input_buffer; line && *line; line = next_line) {
    char linebuf[INPUT_MAX + (INPUT_MAX / 10)];

    next_line = strchr(line, '\n');

    if (next_line) {
      *next_line = 0;               /* Replace \n with a null-terminator */
      next_line++;                  /* Move the pointer to just after that \n */
    } else if (wait_for_new_line) { /* Reached the last line */
      strcpy(gd.input_buffer, line);
      gd.input_buffer_length = (int)strlen(line);

      return; /* Leave and wait until called again without having to wait */
    }

    /* Print the output after processing it */
    strcpy(linebuf, line);

    check_all_highlights(linebuf);

    if (gd.debug) {
      char wrapped_str[INPUT_MAX + (INPUT_MAX / 10)];

      convert_meta(linebuf, wrapped_str);
      printf("%s", wrapped_str);
    } else {
      printf("%s", linebuf);
    }

    if (next_line) {
      printf("\n");
    }

    fflush(stdout);
  }

  /* If we reached this point, then there's no more output in the buffer */
  gd.input_buffer_length = 0;
}

void read_config(char *arg) {
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
    display_printf("ERROR: File {%s} is empty", filename);
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
    if (com == FALSE) { /* Not in a comment */
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
          com = TRUE;
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
      case '*':
        if (pti[1] == '/') { /* Comment close */
          pti += 2;
          com = FALSE;
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
      char *args, command[BUFFER_SIZE];

      args = get_arg(pto, command);

      if (is_abbrev(command, "HIGHLIGHT")) {
        highlight_add(args);
      } else if (is_abbrev(command, "SHOWME")) {
        char *tmp = pto, buf[BUFFER_SIZE];

        while (isspace((int)*tmp)) {
          tmp++;
        }

        strcpy(buf, tmp);

        check_all_highlights(buf);
        display_printf(buf);
      } else if (is_abbrev(command, "UNHIGHLIGHT")) {
        highlight_remove(args);
      } else {
        display_printf("ERROR: Unknown command {%s}", command);
      }
    }

    pti++; /* Move to the position after the null-terminator */
    pto = pti;
  }

  free(bufi);
  free(bufo);
}
