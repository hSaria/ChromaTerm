// This program is protected under the GNU GPL (See COPYING)

#include "defs.h"

#include <signal.h>
#include <sys/socket.h>

struct session *gts;
struct global_data *gtd;

// main() - setup signals - init lists - readcoms - mainloop()
int main(int argc, char **argv) {
#ifdef SOCKS
  SOCKSinit(argv[0]);
#endif

  if (signal(SIGTERM, abort_and_trap_handler) == BADSIG) {
    syserr("signal SIGTERM");
  }

  if (signal(SIGSEGV, abort_and_trap_handler) == BADSIG) {
    syserr("signal SIGSEGV");
  }

  if (signal(SIGHUP, abort_and_trap_handler) == BADSIG) {
    syserr("signal SIGHUP");
  }

  if (signal(SIGABRT, abort_and_trap_handler) == BADSIG) {
    syserr("signal SIGTERM");
  }

  if (signal(SIGINT, abort_and_trap_handler) == BADSIG) {
    syserr("signal SIGINT");
  }

  if (signal(SIGTSTP, suspend_handler) == BADSIG) {
    syserr("signal SIGSTOP");
  }

  if (signal(SIGPIPE, pipe_handler) == BADSIG) {
    syserr("signal SIGPIPE");
  }

  if (signal(SIGWINCH, winch_handler) == BADSIG) {
    syserr("signal SIGWINCH");
  }

  init_program();

  if (argc > 1) {
    int c;

    optind = 1;

    while ((c = getopt(argc, argv, "e: h c: t:")) != EOF) {
      switch (c) {
      case 't':
        printf("\033]0;%s\007", optarg);
        break;
      case 'c':
        if (access(optarg, R_OK) != -1) {
          do_read(optarg);
        }
        break;
      case 'e':
        gts = script_driver(optarg);
        break;
      case 'h':
        help_menu(FALSE, c, argv[0]);
        break;

      default:
        help_menu(TRUE, c, argv[0]);
        break;
      }
    }

    if (argv[optind] != NULL) {
      do_read(argv[optind]);
    }
  } else {
    display_printf(TRUE, "#HELP for more info");
  }

  if (getenv("HOME") != NULL) {
    char filename[256];
    sprintf(filename, "%s", ".chromatermrc");

    if (access(filename, R_OK) != -1) {
      do_read(filename);
    } else {
      sprintf(filename, "%s/%s", getenv("HOME"), ".chromatermrc");

      if (access(filename, R_OK) != -1) {
        do_read(filename);
      }
    }
  }

  mainloop();

  return 0;
}

void init_program() {
  int index;

  gts = (struct session *)calloc(1, sizeof(struct session));
  gtd = (struct global_data *)calloc(1, sizeof(struct global_data));

  for (index = 0; index < LIST_MAX; index++) {
    gts->list[index] = init_list(index, 32);
  }

  gts->socket = 1;

  gtd->mud_output_max = 16384;
  gtd->mud_output_buf = (char *)calloc(1, gtd->mud_output_max);
  gtd->input_off = 1;

  for (index = 0; index < 100; index++) {
    gtd->vars[index] = strdup("");
    gtd->cmds[index] = strdup("");
  }

  init_screen_size();

  gts->input_level++;
  do_configure("{CHARSET}         {UTF-8}");
  do_configure("{COMMAND CHAR}        {#}");
  do_configure("{CONVERT META}      {OFF}");
  gts->input_level--;

  init_terminal();
}

void help_menu(int error, char c, char *proc_name) {
  if (error) {
    display_printf(TRUE, "Unknown option '%c'", c);
  }

  display_printf(TRUE, "Usage: %s [OPTION]... [FILE]...", proc_name);
  display_printf(TRUE, "  -e  Execute function");
  display_printf(TRUE, "  -h  This help section");
  display_printf(TRUE, "  -c  Specify configuration file");
  display_printf(TRUE, "  -t  Set title");

  restore_terminal();
  if (error) {
    exit(64);
  } else {
    quitmsg(NULL, 0);
  }
}

void quitmsg(char *message, int exit_signal) {
  cleanup_session();
  restore_terminal();

  if (message) {
    printf("\n%s\n", message);
  }

  fflush(stdout);
  exit(exit_signal);
}

void abort_and_trap_handler(int sig) {
  char temp[BUFFER_SIZE];
  sprintf(temp, "abort_and_trap_handler: %i", sig);
  quitmsg(temp, 1);
}

void pipe_handler(int sig) {
  restore_terminal();
  display_printf(TRUE, "broken_pipe: %i", sig);
}

void suspend_handler(int sig) {
  char temp[BUFFER_SIZE];
  sprintf(temp, "suspend_handler: %i", sig);
  quitmsg(temp, 1);
}

void winch_handler(int sig) { init_screen_size(); }
