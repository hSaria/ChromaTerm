// This program is protected under the GNU GPL (See COPYING)

#include "defs.h"

void parse_input(char *input) {
  char line[BUFFER_SIZE];

  if (*input == 0) {
    write_mud(input, SUB_EOL);
    return;
  }

  strcpy(line, input);
  write_mud(line, SUB_EOL);
}

// get all arguments
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

// Braces are stripped in braced arguments leaving all else as is.
char *get_arg_in_braces(char *string, char *result, int flag) {
  char *pti, *pto;
  int nest = 1;

  pti = space_out(string);
  pto = result;

  if (*pti != DEFAULT_OPEN) {
    if (!HAS_BIT(flag, GET_ALL)) {
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
    display_printf(FALSE, "#ERROR: UNMATCHED BRACKETS");
  } else {
    pti++;
  }
  *pto = '\0';

  return pti;
}

char *sub_arg_in_braces(char *string, char *result, int flag, int sub) {
  char buffer[BUFFER_SIZE];

  string = get_arg_in_braces(string, buffer, flag);

  substitute(buffer, result, sub);

  return string;
}

// get one arg, stop at a space
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

// advance ptr to next none-space
char *space_out(char *string) {
  while (isspace((int)*string)) {
    string++;
  }
  return string;
}

// send command to the socket
void write_mud(char *command, int flags) {
  char output[BUFFER_SIZE];
  int size;

  size = substitute(command, output, flags);

  write_line_socket(output, size);
}

// do all of the functions to one line of buffer, VT102 codes and variables
// substituted beforehand.
void do_one_line(char *line) {
  char strip[BUFFER_SIZE];

  strip_vt102_codes(line, strip);
  check_all_highlights(line, strip);
}
