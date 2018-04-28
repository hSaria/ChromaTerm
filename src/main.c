// This program is protected under the GNU GPL (See COPYING)

#include "defs.h"

#include <signal.h>
#include <sys/socket.h>

struct session *gts;
struct global_data *gtd;

void pipe_handler(int signal) {
  restore_terminal();

  display_printf(NULL, "broken_pipe");
}

void winch_handler(int signal) { init_screen_size(gts); }

void abort_and_trap_handler(int signal) {
  restore_terminal();

  fflush(NULL);

  exit(-1);
}

void suspend_handler(int signal) {
  printf("\033[r\033[%d;%dH", gtd->ses->rows, 1);

  fflush(stdout);

  restore_terminal();

  kill(0, SIGSTOP);

  init_terminal();

  display_puts(NULL, "#WELCOME BACK");
}

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
          gtd->ses = do_read(gtd->ses, optarg);
        }
        break;
      case 'e':
        gtd->ses = script_driver(gtd->ses, optarg);
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
      gtd->ses = do_read(gtd->ses, argv[optind]);
    }
  } else {
    display_printf(NULL, "#HELP for more info");
  }

  if (getenv("HOME") != NULL) {
    char filename[256];
    sprintf(filename, "%s", ".chromatermrc");

    if (access(filename, R_OK) != -1) {
      gtd->ses = do_read(gtd->ses, filename);
    } else {
      sprintf(filename, "%s/%s", getenv("HOME"), ".chromatermrc");

      if (access(filename, R_OK) != -1) {
        gtd->ses = do_read(gtd->ses, filename);
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
    gts->list[index] = init_list(gts, index, 32);
  }

  gts->socket = 1;

  gtd->ses = gts;
  gtd->mud_output_max = 16384;
  gtd->mud_output_buf = (char *)calloc(1, gtd->mud_output_max);
  gtd->input_off = 1;

  for (index = 0; index < 100; index++) {
    gtd->vars[index] = strdup("");
    gtd->cmds[index] = strdup("");
  }

  init_screen_size(gts);

  gts->input_level++;
  do_configure(gts, "{CHARSET}         {UTF-8}");
  do_configure(gts, "{COMMAND CHAR}        {#}");
  do_configure(gts, "{CONVERT META}      {OFF}");
  gts->input_level--;

  gts->check_output = (long long)0;

  init_terminal();
}

void help_menu(int error, char c, char *proc_name) {
  if (error) {
    display_printf(NULL, "Unknown option '%c'", c);
  }

  display_printf(NULL, "Usage: %s [OPTION]... [FILE]...", proc_name);
  display_printf(NULL, "  -e  Execute function");
  display_printf(NULL, "  -h  This help section");
  display_printf(NULL, "  -c  Specify configuration file");
  display_printf(NULL, "  -t  Set title");

  restore_terminal();
  if (error) {
    exit(1);
  } else {
    quitmsg(NULL);
  }
}

void quitmsg(char *message) {
  cleanup_session(gts);
  restore_terminal();

  if (message) {
    printf("\n%s\n", message);
  }

  fflush(NULL);
  exit(0);
}
