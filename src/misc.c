/* This program is protected under the GNU GPL (See COPYING) */

#include "defs.h"

DO_COMMAND(do_configure) {
  char left[BUFFER_SIZE], right[BUFFER_SIZE];

  strcpy(right, get_arg(arg, left));

  get_arg(right, right);

  if (*left != 0 && *right != 0) {
    if (is_abbrev(left, "CONVERT META")) {
      if (!strcasecmp(right, "ON")) {
        SET_BIT(gd.flags, SES_FLAG_CONVERTMETA);
      } else if (!strcasecmp(right, "OFF")) {
        DEL_BIT(gd.flags, SES_FLAG_CONVERTMETA);
      }
    } else if (is_abbrev(left, "HIGHLIGHT")) {
      if (!strcasecmp(right, "ON")) {
        SET_BIT(gd.flags, SES_FLAG_HIGHLIGHT);
      } else if (!strcasecmp(right, "OFF")) {
        DEL_BIT(gd.flags, SES_FLAG_HIGHLIGHT);
      }
    } else if (is_abbrev(left, "READ")) {
      do_read(right);
    } else if (is_abbrev(left, "WRITE")) {
      do_write(right);
    }
  }
}

DO_COMMAND(do_read) {
  FILE *fp;
  struct stat filedata;
  char *bufi, *bufo, filename[BUFFER_SIZE], *pti, *pto, last = 0;
  int lvl, com, lnc;
  wordexp_t p;

  get_arg(arg, filename);

  if (wordexp(filename, &p, 0) == 0) {
    strcpy(filename, *p.we_wordv);
    wordfree(&p);
  } else {
    display_printf("Failed while performing word expansion on {%s}", filename);
    return;
  }

  if ((fp = fopen(filename, "r")) == NULL) {
    display_printf("File {%s} not found", filename);
    return;
  }

  stat(filename, &filedata);

  if ((bufi = (char *)calloc(1, filedata.st_size + 2)) == NULL) {
    display_printf("Failed to allocate i_buffer memory to process file {%s}",
                   filename);
    fclose(fp);
    return;
  } else if ((bufo = (char *)calloc(1, filedata.st_size + 2)) == NULL) {
    display_printf("Failed to allocate o_buffer memory to process file {%s}",
                   filename);
    free(bufi);
    fclose(fp);
    return;
  }

  if (fread(bufi, 1, filedata.st_size, fp) == 0) {
    display_printf("File is empty", filename);
    free(bufi);
    free(bufo);
    fclose(fp);
    return;
  };

  pti = bufi;
  pto = bufo;

  lvl = com = lnc = 0;

  while (*pti) {
    if (com == 0) { /* Not in a comment */
      switch (*pti) {
      case DEFAULT_OPEN:
        *pto++ = *pti++;
        lvl++;
        last = DEFAULT_OPEN;
        break;
      case DEFAULT_CLOSE:
        *pto++ = *pti++;
        lvl--;
        last = DEFAULT_CLOSE;
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
            previous_line--;
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
        last = 0;
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

  gd.quiet++; /* Stop messages from printing */

  pti = bufo;
  pto = bufo;

  while (*pti) {
    if (*pti != '\n') { /* Seek until you reach \n */
      pti++;
      continue;
    }
    *pti = 0; /* replace \n with null-terminator */

    if (strlen(pto) >= BUFFER_SIZE) {
      gd.quiet--;
      /* Only output the first 20 characters of the overflowing command */
      *(pto + 20) = 0;
      display_printf("Command too long {%s}", pto);

      fclose(fp);
      free(bufi);
      free(bufo);
      return;
    }

    if (*pto) {
      int i;
      char args[BUFFER_SIZE], command[BUFFER_SIZE];

      strcpy(args, get_arg(pto, command));

      for (i = 0; *command_table[i].name != 0; i++) {
        if (is_abbrev(command, command_table[i].name)) {
          (*command_table[i].command)(args);
          break;
        }
      }
    }

    pti++; /* Move to the position after the null-terminator */
    pto = pti;
  }

  gd.quiet--; /* Resume messages */

  free(bufi);
  free(bufo);
  fclose(fp);
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
    display_printf("Failed while performing word expansion on {%s}", filename);
    return;
  }

  if ((file = fopen(filename, "w")) == NULL) {
    display_printf("ERROR: {%s} - Could not open to write", filename);
    return;
  }

  sprintf(result,
          "CONFIG {CONVERT META} {%s}\n"
          "CONFIG {HIGHLIGHT} {%s}\n\n",
          HAS_BIT(gd.flags, SES_FLAG_CONVERTMETA) ? "ON" : "OFF",
          HAS_BIT(gd.flags, SES_FLAG_HIGHLIGHT) ? "ON" : "OFF");
  fputs(result, file);

  for (i = 0; i < gd.highlights_used; i++) {
    sprintf(result, "HIGHLIGHT->ADD {%s} {%s} {%s}\n",
            gd.highlights[i]->condition, gd.highlights[i]->action,
            gd.highlights[i]->priority);
    fputs(result, file);
  }

  fclose(file);
}
