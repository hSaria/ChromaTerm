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

/* Will process only the lines that end of \n */
void process_input(int wait_for_new_line) {
  char *line, *next_line;

  gd.input_buffer[gd.input_buffer_length] = 0;

  /* separate into lines and print away */
  for (line = gd.input_buffer; line && *line; line = next_line) {
    char linebuf[INPUT_MAX];

    next_line = strchr(line, '\n');

    if (next_line) {
      *next_line = 0; /* Replace \n with a null-terminator */
      next_line++;    /* Move the pointer to just after that \n */
    } else {          /* Reached the last line */
      if (wait_for_new_line) {
        char temp[INPUT_MAX];

        strcpy(temp, line);
        strcpy(gd.input_buffer, temp);

        gd.input_buffer_length = (int)strlen(line);

        /* Leave and wait until called again without having to wait */
        return;
      }
    }

    /* Print the output after processing it */
    strcpy(linebuf, line);

    if (HAS_BIT(gd.flags, SES_FLAG_HIGHLIGHT)) {
      check_all_highlights(linebuf);
    }

    if (HAS_BIT(gd.flags, SES_FLAG_CONVERTMETA)) {
      char wrapped_str[BUFFER_SIZE * 2];

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

  /* If we reached this point, then there's no more output in the buffer; reset
   * the length */
  gd.input_buffer_length = 0;
}
