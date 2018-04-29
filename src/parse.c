// This program is protected under the GNU GPL (See COPYING)

#include "defs.h"

struct session *parse_input(struct session *ses, char *input) {
  char *line;

  if (*input == 0) {
    write_mud(ses, input, SUB_EOL);

    return ses;
  }

  line = (char *)malloc(BUFFER_SIZE);

  strcpy(line, input);

  write_mud(ses, line, SUB_EOL);

  free(line);

  return ses;
}

// Deals with # stuff
struct session *parse_command(struct session *ses, char *input) {
  char line[BUFFER_SIZE];

  get_arg_stop_spaces(ses, input, line, 0);

  display_printf(ses, TRUE, "#ERROR: #UNKNOWN COMMAND '%s'", line);

  return ses;
}

// get all arguments - only check for unescaped command separators
char *get_arg_all(struct session *ses, char *string, char *result) {
  char *pto, *pti;
  int nest = 0;

  pti = string;
  pto = result;

  while (*pti) {
    if (*pti == '\\' && pti[1] == COMMAND_SEPARATOR) {
      *pto++ = *pti++;
    } else if (*pti == COMMAND_SEPARATOR && nest == 0) {
      break;
    } else if (*pti == DEFAULT_OPEN) {
      nest++;
    } else if (*pti == DEFAULT_CLOSE) {
      nest--;
    }
    *pto++ = *pti++;
  }
  *pto = '\0';

  return pti;
}

// Braces are stripped in braced arguments leaving all else as is.
char *get_arg_in_braces(struct session *ses, char *string, char *result,
                        int flag) {
  char *pti, *pto;
  int nest = 1;

  pti = space_out(string);
  pto = result;

  if (*pti != DEFAULT_OPEN) {
    if (!HAS_BIT(flag, GET_ALL)) {
      pti = get_arg_stop_spaces(ses, pti, result, flag);
    } else {
      pti = get_arg_with_spaces(ses, pti, result, flag);
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
    display_printf(NULL, FALSE, "#ERROR: UNMATCHED BRACKETS");
  } else {
    pti++;
  }
  *pto = '\0';

  return pti;
}

char *sub_arg_in_braces(struct session *ses, char *string, char *result,
                        int flag, int sub) {
  char buffer[BUFFER_SIZE];

  string = get_arg_in_braces(ses, string, buffer, flag);

  substitute(ses, buffer, result, sub);

  return string;
}

// get all arguments
char *get_arg_with_spaces(struct session *ses, char *string, char *result,
                          int flag) {
  char *pto, *pti;
  int nest = 0;

  pti = space_out(string);
  pto = result;

  while (*pti) {
    if (*pti == '\\' && pti[1] == COMMAND_SEPARATOR) {
      *pto++ = *pti++;
    } else if (*pti == COMMAND_SEPARATOR && nest == 0) {
      break;
    } else if (*pti == DEFAULT_OPEN) {
      nest++;
    } else if (*pti == DEFAULT_CLOSE) {
      nest--;
    }
    *pto++ = *pti++;
  }
  *pto = '\0';

  return pti;
}

// get one arg, stop at spaces
char *get_arg_stop_spaces(struct session *ses, char *string, char *result,
                          int flag) {
  char *pto, *pti;
  int nest = 0;

  pti = space_out(string);
  pto = result;

  while (*pti) {
    if (*pti == '\\' && pti[1] == COMMAND_SEPARATOR) {
      *pto++ = *pti++;
    } else if (*pti == COMMAND_SEPARATOR && nest == 0) {
      break;
    } else if (isspace((int)*pti) && nest == 0) {
      pti++;
      break;
    } else if (*pti == DEFAULT_OPEN) {
      nest++;
    } else if (*pti == '[' && HAS_BIT(flag, GET_NST)) {
      nest++;
    } else if (*pti == DEFAULT_CLOSE) {
      nest--;
    } else if (*pti == ']' && HAS_BIT(flag, GET_NST)) {
      nest--;
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
void write_mud(struct session *ses, char *command, int flags) {
  char output[BUFFER_SIZE];
  int size;

  size = substitute(ses, command, output, flags);

  write_line_socket(ses, output, size);
}

// do all of the functions to one line of buffer, VT102 codes and variables
// substituted beforehand.
void do_one_line(char *line, struct session *ses) {
  char strip[BUFFER_SIZE];

  strip_vt102_codes(line, strip);

  check_all_highlights(ses, line, strip);

  return;
}
