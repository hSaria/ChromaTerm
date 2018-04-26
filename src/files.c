// This program is protected under the GNU GPL (See COPYING)

#include "defs.h"

#include <sys/stat.h>

// read and execute a command file, supports multi lines - Igor
DO_COMMAND(do_read) {
  FILE *fp;
  struct stat filedata;
  char *bufi, *bufo, filename[BUFFER_SIZE], temp[BUFFER_SIZE], *pti, *pto,
      last = 0;
  int lvl, cnt, com, lnc, fix, ok;
  int counter[LIST_MAX];

  get_arg_in_braces(ses, arg, filename, TRUE);
  substitute(ses, filename, filename, SUB_VAR | SUB_FUN);

  if ((fp = fopen(filename, "r")) == NULL) {
    display_printf(ses, "#READ {%s} - FILE NOT FOUND.", filename);
    return ses;
  }

  temp[0] = getc(fp);

  if (!isgraph((int)temp[0]) || isalpha((int)temp[0])) {
    display_printf(ses, "#ERROR: #READ {%s} - INVALID START OF FILE.",
                   filename);

    fclose(fp);

    return ses;
  }

  ungetc(temp[0], fp);

  for (cnt = 0; cnt < LIST_MAX; cnt++) {
    counter[cnt] = ses->list[cnt]->used;
  }

  stat(filename, &filedata);

  if ((bufi = (char *)calloc(1, filedata.st_size + 2)) == NULL ||
      (bufo = (char *)calloc(1, filedata.st_size + 2)) == NULL) {
    display_printf(ses, "#ERROR: #READ {%s} - FAILED TO ALLOCATE MEMORY.",
                   filename);

    fclose(fp);

    return ses;
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

      case COMMAND_SEPARATOR:
        *pto++ = *pti++;
        last = COMMAND_SEPARATOR;
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
    display_printf(
        ses, "#ERROR: #READ {%s} - MISSING %d '%c' BETWEEN LINE %d AND %d.",
        filename, abs(lvl), lvl < 0 ? DEFAULT_OPEN : DEFAULT_CLOSE,
        fix == 0 ? 1 : ok, fix == 0 ? lnc + 1 : fix);

    fclose(fp);

    free(bufi);
    free(bufo);

    return ses;
  }

  if (com) {
    display_printf(ses, "#ERROR: #READ {%s} - MISSING %d '%s'", filename,
                   abs(com), com < 0 ? "/*" : "*/");

    fclose(fp);

    free(bufi);
    free(bufo);

    return ses;
  }

  sprintf(temp, "{COMMAND CHAR} {%c}", bufo[0]);

  gtd->quiet++;

  do_configure(ses, temp);

  lvl = 0;
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

      bufi[20] = 0;

      display_printf(ses,
                     "#ERROR: #READ {%s} - BUFFER OVERFLOW AT COMMAND: %s.",
                     filename, bufi);

      fclose(fp);

      free(bufi);
      free(bufo);

      return ses;
    }

    if (bufi[0]) {
      ses = script_driver(ses, -1, bufi);
    }

    pto = bufi;
    pti++;
  }

  gtd->quiet--;

  for (cnt = 0; cnt < LIST_MAX; cnt++) {
    switch (ses->list[cnt]->used - counter[cnt]) {
    case 0:
      break;

    case 1:
      show_message(ses, LIST_MESSAGE, "#OK: %3d %s LOADED.",
                   ses->list[cnt]->used - counter[cnt], list_table[cnt].name);
      break;

    default:
      show_message(ses, LIST_MESSAGE, "#OK: %3d %s LOADED.",
                   ses->list[cnt]->used - counter[cnt],
                   list_table[cnt].name_multi);
      break;
    }
  }

  fclose(fp);

  free(bufi);
  free(bufo);

  return ses;
}

DO_COMMAND(do_write) {
  FILE *file;
  char filename[BUFFER_SIZE];
  struct listroot *root;
  int i, j, cnt = 0;

  get_arg_in_braces(ses, arg, filename, TRUE);

  if (*filename == 0 || (file = fopen(filename, "w")) == NULL) {
    display_printf(ses, "#ERROR: #WRITE: COULDN'T OPEN {%s} TO WRITE.",
                   filename);
    return ses;
  }

  for (i = 0; i < LIST_MAX; i++) {
    root = ses->list[i];
    for (j = 0; j < root->used; j++) {
      if (*root->list[j]->group == 0) {
        write_node(ses, i, root->list[j], file);

        cnt++;
      }
    }
  }

  fclose(file);

  show_message(ses, LIST_MESSAGE, "#WRITE: %d COMMANDS WRITTEN TO {%s}.", cnt,
               filename);

  return ses;
}

void write_node(struct session *ses, int list, struct listnode *node,
                FILE *file) {
  char result[STRING_SIZE];

  int llen = UMAX(20, strlen(node->left));
  int rlen = UMAX(25, strlen(node->right));

  switch (list) {
  default:
    switch (list_table[list].args) {
    case 0:
      result[0] = 0;
      break;
    case 1:
      sprintf(result, "%c%-16s {%s}\n", gtd->command_char,
              list_table[list].name, node->left);
      break;
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
    break;
  }
  fputs(result, file);

  return;
}
