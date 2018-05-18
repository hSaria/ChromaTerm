/* This program is protected under the GNU GPL (See COPYING) */

#include "defs.h"

static int process_already_running = FALSE;

DO_COMMAND(do_commands) {
  char buf[BUFFER_SIZE] = {0}, add[BUFFER_SIZE];
  int cmd;

  for (cmd = 0; *command_table[cmd].name != 0; cmd++) {
    if (*arg && !is_abbrev(arg, command_table[cmd].name)) {
      continue;
    }

    sprintf(add, "%-14s", command_table[cmd].name);
    strcat(buf, add);
  }
  if (buf[0]) {
    display_printf(buf);
  }
}

DO_COMMAND(do_configure) {
  char left[BUFFER_SIZE];

  arg = get_arg(arg, left);

  if (*left == 0) {
    display_printf("%-12s = %-3c    [%s]", "COMMAND CHAR", gtd.command_char,
                   "The character used for CT-- commands");
    display_printf("%-12s = %-3s    [%s]", "CONVERT META",
                   HAS_BIT(gtd.flags, SES_FLAG_CONVERTMETA) ? "ON" : "OFF",
                   "Convert meta and control characters");
    display_printf("%-12s = %-3s    [%s]", "HIGHLIGHT",
                   HAS_BIT(gtd.flags, SES_FLAG_HIGHLIGHT) ? "ON" : "OFF",
                   "Highlight according to rules");
  } else {
    if (is_abbrev(left, "COMMAND CHAR")) {
      if (*arg == 0) {
        display_printf("%cSYNTAX: %cCONFIG {COMMAND CHAR} {CHAR}",
                       gtd.command_char, gtd.command_char);
      } else if (!ispunct((int)arg[0])) {
        display_printf("%cERROR: Commad character must me a punctuation: "
                       "!@#$%%^&*-+=',.\"\\/:;?_`<>()[]{}|~",
                       gtd.command_char);
      } else {
        gtd.command_char = arg[0];
      }
    } else if (is_abbrev(left, "CONVERT META")) {
      if (!strcasecmp(arg, "ON")) {
        SET_BIT(gtd.flags, SES_FLAG_CONVERTMETA);
      } else if (!strcasecmp(arg, "OFF")) {
        DEL_BIT(gtd.flags, SES_FLAG_CONVERTMETA);
      } else {
        display_printf("%cSYNTAX: %cCONFIG {CONVERT META} {ON|OFF}",
                       gtd.command_char, gtd.command_char);
      }
    } else if (is_abbrev(left, "HIGHLIGHT")) {
      if (!strcasecmp(arg, "ON")) {
        SET_BIT(gtd.flags, SES_FLAG_HIGHLIGHT);
      } else if (!strcasecmp(arg, "OFF")) {
        DEL_BIT(gtd.flags, SES_FLAG_HIGHLIGHT);
      } else {
        display_printf("%cSYNTAX: %cCONFIG {HIGHLIGHT} {ON|OFF}",
                       gtd.command_char, gtd.command_char);
      }
    } else {
      display_printf("%cERROR: {%s} is not a valid option", gtd.command_char,
                     left);
    }
  }
}

DO_COMMAND(do_exit) {
  if (*arg) {
    quitmsg(arg, 0);
  }
  quitmsg(NULL, 0);
}

DO_COMMAND(do_help) {
  char left[BUFFER_SIZE], add[BUFFER_SIZE];
  int cnt;

  get_arg(arg, left);

  if (*left == 0) {
    for (cnt = add[0] = 0; *help_table[cnt].name != 0; cnt++) {
      cat_sprintf(add, "%-14s", help_table[cnt].name);
    }
    display_printf(add);

  } else {
    int found = FALSE;
    for (cnt = 0; *help_table[cnt].name != 0; cnt++) {
      if (is_abbrev(left, help_table[cnt].name) || is_abbrev(left, "all")) {
        char buf[BUFFER_SIZE];
        found = TRUE;

        substitute(help_table[cnt].text, buf);

        display_printf(buf);
      }
    }

    if (!found) {
      display_printf("%cHELP: No help found for topic '%s'", gtd.command_char,
                     left);
    }
  }
}

DO_COMMAND(do_run) {
  char temp[BUFFER_SIZE];
  int desc, pid;
  struct winsize size;

  /* Limit to a single process */
  if (process_already_running) {
    display_printf("%cRUN: Process is already running; see %chelp run ",
                   gtd.command_char, gtd.command_char);
    return;
  } else {
    process_already_running = TRUE;
  }

  /* If it's quiet, then that must mean we're reading this run command from a
   * file; do not launch a terminal after the read finishes */
  if (gtd.quiet) {
    gtd.run_overriden = TRUE;
  }

  char *argv[4] = {"sh", "-c", "", NULL};

  /* If no process is provided, use the SHELL environment variable */
  strcpy(temp, "exec ");
  if (arg == NULL || *arg == 0) {
    strcat(temp, getenv("SHELL") ? getenv("SHELL") : "");
    strcat(temp, " -l");
  } else {
    strcat(temp, arg);
    memset(arg, 0, strlen(arg));
  }

  size.ws_row = gtd.rows;
  size.ws_col = gtd.cols;

  pid = forkpty(&desc, NULL, &gtd.active_terminal, &size);

  switch (pid) {
  case -1:
    perror("forkpty");
    break;
  case 0:
    argv[2] = temp;
    execv("/bin/sh", argv);
    break;
  default:
    gtd.pid = pid;
    gtd.socket = desc;

    if (pthread_create(&output_thread, NULL, poll_session, NULL) != 0) {
      quitmsg("failed to create input thread", 1);
    }

    break;
  }
}

DO_COMMAND(do_showme) {
  char *pto = space_out(arg);

  check_all_highlights(pto);

  printline(pto, FALSE);
}
