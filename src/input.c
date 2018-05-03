/* This program is protected under the GNU GPL (See COPYING) */

#include "defs.h"

void convert_meta(char *input, char *output) {
  char *pti, *pto;

  DEL_BIT(gtd->flags, GLOBAL_FLAG_CONVERTMETACHAR);

  pti = input;
  pto = output;

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

void input_printf(char *format, ...) {
  char buf[BUFFER_SIZE * 2];
  va_list args;

  va_start(args, format);
  vsprintf(buf, format, args);
  va_end(args);

  printf("%s", buf);
}

/* The current output of the screen cannot be determined which means we can only
 * listen for commands after a new line. */
void read_key(void) {
  char buffer[BUFFER_SIZE];
  int len, cnt;

  if (gtd->input_buf[0] == gtd->command_char) {
    read_line();
    return;
  }
  len = (int)read(0, buffer, 1);

  buffer[len] = 0;

  if (HAS_BIT(gts->flags, SES_FLAG_CONVERTMETA) ||
      HAS_BIT(gtd->flags, GLOBAL_FLAG_CONVERTMETACHAR)) {
    convert_meta(buffer, &gtd->macro_buf[strlen(gtd->macro_buf)]);
  } else {
    strcat(gtd->macro_buf, buffer);
  }
  /* Handles normal input and transfering to CT */
  for (cnt = 0; gtd->macro_buf[cnt]; cnt++) {
    if (gtd->macro_buf[cnt] ==
        '\n') { /* Reset: \n is needed before a command */
      gtd->macro_buf[0] = 0;
      gtd->input_buf[0] = 0;
      gtd->input_len = 0;

      socket_printf(1, "%c", '\r');
    } else { /* Normal input */
      if (gtd->macro_buf[cnt] == gtd->command_char && gtd->input_buf[0] == 0) {
        /* Transfer to CT on next call of read_key */
        if (gtd->input_len != gtd->input_cur) {
          printf("\033[1@%c", gtd->macro_buf[cnt]);
        } else {
          printf("%c", gtd->macro_buf[cnt]);
        }
        gtd->input_buf[0] = gtd->command_char;
        gtd->input_buf[1] = 0;
        gtd->macro_buf[0] = 0;
        gtd->input_len = 1;
        gtd->input_cur = 1;
        gtd->input_pos = 1;
      } else { /* If not destined to CT, send to socket */
        socket_printf(1, "%c", gtd->macro_buf[cnt]);
        gtd->input_buf[0] = 127; /* != 0 means a reset (\n) is required */
        gtd->macro_buf[0] = 0;
        gtd->input_len = 0;
      }
    }
  }
}

void read_line() {
  char buffer[BUFFER_SIZE * 2];
  int len, cnt, match;

  gtd->input_buf[gtd->input_len] = 0;

  len = (int)read(0, buffer, 1);

  buffer[len] = 0;

  if (HAS_BIT(gts->flags, SES_FLAG_CONVERTMETA) ||
      HAS_BIT(gtd->flags, GLOBAL_FLAG_CONVERTMETACHAR)) {
    convert_meta(buffer, &gtd->macro_buf[strlen(gtd->macro_buf)]);
  } else {
    strcat(gtd->macro_buf, buffer);
  }

  if (!HAS_BIT(gts->flags, SES_FLAG_CONVERTMETA)) {
    match = 0;

    for (cnt = 0; *cursor_table[cnt].fun != NULL; cnt++) {
      if (!strcmp(gtd->macro_buf, cursor_table[cnt].code)) {
        cursor_table[cnt].fun();
        gtd->macro_buf[0] = 0;
        return;
      } else if (!strncmp(gtd->macro_buf, cursor_table[cnt].code,
                          strlen(gtd->macro_buf))) {
        match = 1;
      }
    }

    if (match) {
      return;
    }
  }

  if (gtd->macro_buf[0] == ESCAPE) {
    strcpy(buffer, gtd->macro_buf);
    convert_meta(buffer, gtd->macro_buf);
  }

  for (cnt = 0; gtd->macro_buf[cnt]; cnt++) {
    switch (gtd->macro_buf[cnt]) {
    case 10:
      cursor_enter();
      break;
    default:
      if (HAS_BIT(gtd->flags, GLOBAL_FLAG_INSERTINPUT) &&
          gtd->input_len != gtd->input_cur) {
        if (!HAS_BIT(gts->flags, SES_FLAG_UTF8) ||
            (gtd->macro_buf[cnt] & 192) != 128) {
          cursor_delete();
        }
      }

      ins_sprintf(&gtd->input_buf[gtd->input_cur], "%c", gtd->macro_buf[cnt]);

      gtd->input_len++;
      gtd->input_cur++;

      if (!HAS_BIT(gts->flags, SES_FLAG_UTF8) ||
          (gtd->macro_buf[cnt] & 192) != 128) {
        gtd->input_pos++;
      }

      if (gtd->input_len != gtd->input_cur) {
        if (HAS_BIT(gts->flags, SES_FLAG_UTF8) &&
            (gtd->macro_buf[cnt] & 192) == 128) {
          input_printf("%c", gtd->macro_buf[cnt]);
        } else {
          input_printf("\033[1@%c", gtd->macro_buf[cnt]);
        }
      } else {
        input_printf("%c", gtd->macro_buf[cnt]);
      }

      gtd->macro_buf[0] = 0;
      gtd->input_buf[gtd->input_len] = 0;

      cursor_check_line_modified();

      break;
    }
  }
}
