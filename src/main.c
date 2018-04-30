// This program is protected under the GNU GPL (See COPYING)

#include "defs.h"

struct session *gts;
struct global_data *gtd;

// main() - setup signals - init lists - readcoms - mainloop()
int main(int argc, char **argv) {
  signal(SIGTERM, abort_and_trap_handler);
  signal(SIGSEGV, abort_and_trap_handler);
  signal(SIGHUP, abort_and_trap_handler);
  signal(SIGABRT, abort_and_trap_handler);
  signal(SIGINT, abort_and_trap_handler);
  signal(SIGTSTP, suspend_handler);
  signal(SIGPIPE, pipe_handler);
  signal(SIGWINCH, winch_handler);

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
        script_driver(optarg);
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

void abort_and_trap_handler() { quitmsg("abort_and_trap_handler", 1); }

void pipe_handler() {
  restore_terminal();
  display_printf(TRUE, "broken_pipe");
}

void suspend_handler() { quitmsg("suspend_handler", 1); }

void winch_handler() { init_screen_size(); }
