/* This program is protected under the GNU GPL (See COPYING) */

#include "defs.h"

/* read and execute a command file, supports multi lines */
DO_COMMAND(do_read) {
  FILE *fp;
  struct stat filedata;
  char *bufi, *bufo, filename[BUFFER_SIZE], temp[BUFFER_SIZE], *pti, *pto,
      last = 0;
  int lvl, cnt, com, lnc, fix, ok;
  wordexp_t p;

  get_arg(arg, filename);

  if (wordexp(filename, &p, 0) == 0) {
    if (*p.we_wordv != NULL) {
      strcpy(filename, *p.we_wordv);
      wordfree(&p);
    } else {
      display_printf("%1$cSYNTAX: %1$cREAD {FILE LOCATION}", gd.command_char);
      wordfree(&p);
      return;
    }
  } else {
    display_printf("%cREAD: {%s} - File not found", gd.command_char, filename);
    return;
  }

  if ((fp = fopen(filename, "r")) == NULL) {
    display_printf("%cREAD: {%s} - File not found", gd.command_char, filename);
    return;
  }

  temp[0] = getc(fp);

  if (!ispunct((int)temp[0])) {
    display_printf(
        "%cERROR: {%s} - Start of file is not a punctuation; see %chelp read",
        gd.command_char, filename, gd.command_char);
    fclose(fp);
    return;
  }

  ungetc(temp[0], fp);

  stat(filename, &filedata);

  if ((bufi = (char *)calloc(1, filedata.st_size + 2)) == NULL) {
    display_printf("%cERROR: {%s} - Failed to allocate first buffer memory to "
                   "process the file",
                   gd.command_char, filename);
    fclose(fp);
    return;
  } else if ((bufo = (char *)calloc(1, filedata.st_size + 2)) == NULL) {
    display_printf("%cERROR: {%s} - Failed to allocate second buffer memory to "
                   "process the file",
                   gd.command_char, filename);
    free(bufi);
    fclose(fp);
    return;
  }

  if (fread(bufi, 1, filedata.st_size, fp) == 0) {
    display_printf("%cERROR: {%s} - File is empty", gd.command_char, filename);
    fclose(fp);
    free(bufi);
    free(bufo);
    return;
  };

  pti = bufi;
  pto = bufo;

  lvl = com = lnc = fix = ok = 0;

  while (*pti) {
    if (com == 0) {
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
      case '/':
        if (lvl == 0 && pti[1] == '*') {
          pti += 2;
          com += 1;
        } else {
          *pto++ = *pti++;
        }
        break;
      case '\t':
        *pto++ = *pti++;
        break;
      case '\r':
        pti++;
        break;
      case '\n':
        lnc++;
        pto--;
        while (isspace((int)*pto)) {
          pto--;
        }
        pto++;
        if (fix == 0 && pti[1] == gd.command_char) {
          if (lvl == 0) {
            ok = lnc + 1;
          } else {
            fix = lnc;
          }
        }

        if (lvl) {
          pti++;

          while (isspace((int)*pti)) {
            if (*pti == '\n') {
              lnc++;

              if (fix == 0 && pti[1] == gd.command_char) {
                fix = lnc;
              }
            }
            pti++;
          }

          if (*pti != DEFAULT_CLOSE && last == 0) {
            *pto++ = ' ';
          }
        } else
          for (cnt = 1;; cnt++) {
            if (pti[cnt] == 0) {
              *pto++ = *pti++;
              break;
            }

            if (pti[cnt] == DEFAULT_OPEN) {
              pti++;
              while (isspace((int)*pti)) {
                pti++;
              }
              *pto++ = ' ';
              break;
            }

            if (!isspace((int)pti[cnt])) {
              *pto++ = *pti++;
              break;
            }
          }
        break;
      default:
        *pto++ = *pti++;
        last = 0;
        break;
      }
    } else {
      switch (*pti) {
      case '/':
        if (pti[1] == '*') {
          pti += 2;
          com += 1;
        } else {
          pti += 1;
        }
        break;
      case '*':
        if (pti[1] == '/') {
          pti += 2;
          com -= 1;
        } else {
          pti += 1;
        }
        break;
      case '\n':
        lnc++;
        pti++;
        break;
      default:
        pti++;
        break;
      }
    }
  }

  *pto++ = '\n';
  *pto = '\0';

  if (lvl) {
    display_printf("%cERROR: {%s} - Missing %d '%c' between line %d and %d",
                   gd.command_char, filename, abs(lvl),
                   lvl < 0 ? DEFAULT_OPEN : DEFAULT_CLOSE, fix == 0 ? 1 : ok,
                   fix == 0 ? lnc + 1 : fix);

    fclose(fp);
    free(bufi);
    free(bufo);
    return;
  }

  if (com) {
    display_printf("%cERROR: {%s} - Missing %d '%s'", gd.command_char, filename,
                   abs(com), com < 0 ? "/*" : "*/");

    fclose(fp);
    free(bufi);
    free(bufo);
    return;
  }

  gd.quiet++;

  /* Read the first character in the output buffer and configure that as the
   command char  */
  sprintf(temp, "{COMMAND CHAR} {%c}", bufo[0]);
  do_configure(temp);

  pti = bufo;
  pto = bufi;

  while (*pti) {
    if (*pti != '\n') {
      *pto++ = *pti++;
      continue;
    }
    *pto = 0;

    if (strlen(bufi) >= BUFFER_SIZE) {
      gd.quiet--;
      /* Only output the first 20 characters of the overflowing command */
      bufi[20] = 0;
      display_printf("%cERROR: {%s} - Buffer overflow at command: %s", filename,
                     bufi);

      fclose(fp);
      free(bufi);
      free(bufo);
      return;
    }

    if (bufi[0]) {
      script_driver(bufi);
    }

    pto = bufi;
    pti++;
  }

  gd.quiet--;

  fclose(fp);
  free(bufi);
  free(bufo);
}

DO_COMMAND(do_write) {
  FILE *file;
  char filename[BUFFER_SIZE], result[BUFFER_SIZE * 4];
  int i;
  wordexp_t p;

  get_arg(arg, filename);

  if (wordexp(filename, &p, 0) == 0) {
    if (*p.we_wordv != NULL) {
      strcpy(filename, *p.we_wordv);
      wordfree(&p);
    } else {
      display_printf("%1$cSYNTAX: %1$cWRITE {FILE LOCATION}", gd.command_char);
      wordfree(&p);
      return;
    }
  }

  if ((file = fopen(filename, "w")) == NULL) {
    display_printf("%cERROR: {%s} - Could not open to write", gd.command_char,
                   filename);
    return;
  }

  sprintf(result,
          "%1$cCONFIG     {COMMAND CHAR} {%1$c}\n"
          "%1$cCONFIG     {CONVERT META} {%2$s}\n"
          "%1$cCONFIG     {HIGHLIGHT} {%3$s}\n",
          gd.command_char,
          HAS_BIT(gd.flags, SES_FLAG_CONVERTMETA) ? "ON" : "OFF",
          HAS_BIT(gd.flags, SES_FLAG_HIGHLIGHT) ? "ON" : "OFF");
  fputs(result, file);

  for (i = 0; i < gd.highlights_used; i++) {
    sprintf(result, "%cHIGHLIGHT  {%s} {%s} {%s}\n", gd.command_char,
            gd.highlights[i]->condition, gd.highlights[i]->action,
            gd.highlights[i]->priority);

    fputs(result, file);
  }

  fclose(file);
}
