// This program is protected under the GNU GPL (See COPYING)

#include "defs.h"

// read and execute a command file, supports multi lines - Igor
DO_COMMAND(do_read) {
  FILE *fp;
  struct stat filedata;
  char *bufi, *bufo, filename[BUFFER_SIZE], temp[BUFFER_SIZE], *pti, *pto,
      last = 0;
  int lvl, cnt, com, lnc, fix, ok;
  int counter[LIST_MAX];
  wordexp_t p;

  get_arg_in_braces(arg, filename, GET_ALL);

  if (wordexp(filename, &p, 0) == 0) {
    if (*p.we_wordv != NULL) {
      strcpy(filename, *p.we_wordv);
      wordfree(&p);
    } else {
      display_printf("%cSYNTAX: %cREAD {FILE LOCATION}", gtd->command_char,
                     gtd->command_char);
      return;
    }
  } else {
    display_printf("%cREAD: {%s} - File not found", gtd->command_char,
                   filename);
    return;
  }

  if ((fp = fopen(filename, "r")) == NULL) {
    display_printf("%cREAD: {%s} - File not found", gtd->command_char,
                   filename);
    return;
  }

  temp[0] = getc(fp);

  if (!isgraph((int)temp[0]) || isalpha((int)temp[0])) {
    display_printf("%cERROR: {%s} - Invalid start of file", gtd->command_char,
                   filename);
    fclose(fp);
    return;
  }

  ungetc(temp[0], fp);

  for (cnt = 0; cnt < LIST_MAX; cnt++) {
    counter[cnt] = gts->list[cnt]->used;
  }

  stat(filename, &filedata);

  if ((bufi = (char *)calloc(1, filedata.st_size + 2)) == NULL ||
      (bufo = (char *)calloc(1, filedata.st_size + 2)) == NULL) {
    display_printf("%cERROR: {%s} - Failed to allocate memory",
                   gtd->command_char, filename);
    fclose(fp);
    return;
  }

  fread(bufi, 1, filedata.st_size, fp);

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
        if (fix == 0 && pti[1] == gtd->command_char) {
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

              if (fix == 0 && pti[1] == gtd->command_char) {
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
                   gtd->command_char, filename, abs(lvl),
                   lvl < 0 ? DEFAULT_OPEN : DEFAULT_CLOSE, fix == 0 ? 1 : ok,
                   fix == 0 ? lnc + 1 : fix);

    fclose(fp);
    free(bufi);
    free(bufo);
    return;
  }

  if (com) {
    display_printf("%cERROR: {%s} - Missing %d '%s'", gtd->command_char,
                   filename, abs(com), com < 0 ? "/*" : "*/");

    fclose(fp);
    free(bufi);
    free(bufo);
    return;
  }

  gtd->quiet++;

  // Read the first character in the output buffer and configure that as the
  // command char
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
      gtd->quiet--;

      // Only output the first 20 characters of the overflowing command
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

  gtd->quiet--;

  fclose(fp);
  free(bufi);
  free(bufo);
}

DO_COMMAND(do_write) {
  FILE *file;
  char filename[BUFFER_SIZE];
  struct listroot *root;
  int i, j, cnt = 0;
  wordexp_t p;

  get_arg_in_braces(arg, filename, GET_ALL);

  if (wordexp(filename, &p, 0) == 0) {
    if (*p.we_wordv != NULL) {
      strcpy(filename, *p.we_wordv);
      wordfree(&p);
    } else {
      display_printf("%cSYNTAX: %cREAD {FILE LOCATION}", gtd->command_char,
                     gtd->command_char);
      return;
    }
  }

  if (*filename == 0 || (file = fopen(filename, "w")) == NULL) {
    display_printf("%cERROR: {%s} - Could not open to write", gtd->command_char,
                   filename);
    return;
  }

  for (i = 0; i < LIST_MAX; i++) {
    root = gts->list[i];
    for (j = 0; j < root->used; j++) {
      write_node(i, root->list[j], file);

      cnt++;
    }
  }

  fclose(file);

  display_printf("%cWRITE: {%s} - %d commands written ", gtd->command_char,
                 filename, cnt);
}

void write_node(int list, struct listnode *node, FILE *file) {
  char result[BUFFER_SIZE * 2];

  int llen = UMAX(20, (int)strlen(node->left));
  int rlen = UMAX(25, (int)strlen(node->right));

  switch (list_table[list].args) {
  case 2:
    sprintf(result, "%c%-16s {%s} %*s {%s}\n", gtd->command_char,
            list_table[list].name, node->left, 20 - llen, "", node->right);
    break;
  case 3:
    sprintf(result, "%c%-16s {%s} %*s {%s} %*s {%s}\n", gtd->command_char,
            list_table[list].name, node->left, 20 - llen, "", node->right,
            25 - rlen, "", node->pr);
    break;
  }

  fputs(result, file);
}
