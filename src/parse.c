/* This program is protected under the GNU GPL (See COPYING) */

#include "defs.h"

/* get all arguments */
char *get_arg_all(char *string, char *result, int with_spaces) {
  char *pto, *pti;

  if (with_spaces) {
    pti = space_out(string);
  } else {
    pti = string;
  }

  pto = result;

  while (*pti) {
    *pto++ = *pti++;
  }
  *pto = '\0';

  return pti;
}

/* Braces are stripped in braced arguments leaving all else as is */
char *get_arg_in_braces(char *string, char *result, int flag) {
  char *pti, *pto;
  int nest = 1;

  pti = space_out(string);
  pto = result;

  if (*pti != DEFAULT_OPEN) {
    if (!HAS_BIT(flag, GET_ONE)) {
      pti = get_arg_stop_spaces(pti, result);
    } else {
      pti = get_arg_all(pti, result, TRUE);
    }
    return pti;
  }

  pti++;

  while (*pti) {
    if (*pti == DEFAULT_OPEN) {
      nest++;
    } else if (*pti == DEFAULT_CLOSE) {
      nest--;

      if (nest == 0) {
        break;
      }
    }
    *pto++ = *pti++;
  }

  if (*pti == 0) {
    display_printf("%cERROR: Unmatched brackets", gtd->command_char);
  } else {
    pti++;
  }
  *pto = '\0';

  return pti;
}

/* get one arg, stop at a space */
char *get_arg_stop_spaces(char *string, char *result) {
  char *pto, *pti;

  pti = space_out(string);
  pto = result;

  while (*pti) {
    if (isspace((int)*pti)) {
      pti++;
      break;
    }
    *pto++ = *pti++;
  }
  *pto = '\0';

  return pti;
}

/* advance ptr to next none-space */
char *space_out(char *string) {
  while (isspace((int)*string)) {
    string++;
  }
  return string;
}

/* do all of the VT102 code substitution beforehand */
void do_one_line(char *line) {
  char strip[BUFFER_SIZE];

  strip_vt102_codes(line, strip);
  check_all_highlights(line, strip);
}
