/* This program is protected under the GNU GPL (See COPYING) */

#include "defs.h"

struct global_data gtd;

pthread_t input_thread;
pthread_t output_thread;

static int quit_ran = FALSE;

int main(int argc, char **argv) {
  int config_override = FALSE, run_command = FALSE;
  char command[BUFFER_SIZE];
  struct sigaction trap, pipe, winch;

  trap.sa_handler = trap_handler;
  pipe.sa_handler = pipe_handler;
  winch.sa_handler = winch_handler;

  sigaction(SIGTERM, &trap, NULL);
  sigaction(SIGSEGV, &trap, NULL);
  sigaction(SIGHUP, &trap, NULL);
  sigaction(SIGABRT, &trap, NULL);
  sigaction(SIGPIPE, &pipe, NULL);
  sigaction(SIGWINCH, &winch, NULL);

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
        if (access(optarg, R_OK) == 0) {
          do_read(optarg);
          config_override = TRUE;
        }
        break;
      case 'e':
        run_command = TRUE;
        strcpy(command, optarg);
        break;
      case 'h':
        help_menu(FALSE, argv[0]);
        break;
      default:
        help_menu(TRUE, argv[0]);
        break;
      }
    }

    if (argv[optind] != NULL) {
      do_read(argv[optind]);
      config_override = TRUE;
    }
  } else {
    display_printf("%cHELP for more info", gtd.command_char);
  }

  if (!config_override) {
    if (access(".chromatermrc", R_OK) == 0) {
      do_read(".chromatermrc");
    } else {
      if (getenv("HOME") != NULL) {
        char filename[256];

        sprintf(filename, "%s/%s", getenv("HOME"), ".chromatermrc");

        if (access(filename, R_OK) == 0) {
          do_read(filename);
        }
      }
    }
  }

  if (run_command && !gtd.run_overriden) {
    gtd.run_overriden = TRUE;
    do_run(command);
  }

  if (!gtd.run_overriden) {
    do_run(NULL);
  }

  if (pthread_create(&input_thread, NULL, poll_input, NULL) != 0) {
    quitmsg("failed to create input thread", 1);
  }

  pthread_join(input_thread, NULL);

  quitmsg(NULL, 0);
  return 0;
}

void init_program() {
  struct termios io;

  /* initial size is 8, but is dynamically resized as required */
  gtd.highlights = (struct highlight **)calloc(8, sizeof(struct highlight *));
  gtd.highlights_size = 8;

  gtd.socket = 1;

  winch_handler(0);

  /* Default configuration values */
  gtd.command_char = '%';
  SET_BIT(gtd.flags, SES_FLAG_HIGHLIGHT);

  /* Save current terminal attributes and reset at exit */
  if (tcgetattr(STDIN_FILENO, &gtd.saved_terminal)) {
    quitmsg("tcgetattr", 1);
  }

  tcgetattr(STDIN_FILENO, &gtd.active_terminal);

  io = gtd.active_terminal;

  /*  Canonical mode off */
  DEL_BIT(io.c_lflag, ICANON);

  io.c_cc[VMIN] = 1;
  io.c_cc[VTIME] = 0;
  io.c_cc[VSTART] = 255;
  io.c_cc[VSTOP] = 255;

  DEL_BIT(io.c_lflag, ECHO | ECHONL | IEXTEN | ISIG);
  SET_BIT(io.c_cflag, CS8);

  if (tcsetattr(STDIN_FILENO, TCSANOW, &io)) {
    quitmsg("tcsetattr", 1);
  }

  atexit(quit_void);
}

void help_menu(int error, char *proc_name) {
  display_printf("ChromaTerm-- v%s", VERSION);
  display_printf("Usage: %s [OPTION]... [FILE]...", proc_name);
  display_printf("    -h                    This help section");
  display_printf("    -c {CONFIG_FILE}      Override configuration file");
  display_printf("    -e [EXECUTABLE]       Run executable");
  display_printf("    -t {TITLE}            Set title");

  quitmsg(NULL, error);
}

void quit_void(void) {
  if (!quit_ran) {
    quitmsg(NULL, 0);
  }
}

void quitmsg(char *message, int exit_signal) {
  quit_ran = TRUE;

  /* Restore original, saved terminal */
  tcsetattr(STDIN_FILENO, TCSANOW, &gtd.saved_terminal);

  if (kill(gtd.pid, 0) && gtd.socket) {
    close(gtd.socket);
    if (gtd.pid) {
      kill(gtd.pid, SIGKILL);
    }
  }

  if (input_thread) {
    pthread_kill(input_thread, 0);
  }

  if (output_thread) {
    pthread_kill(output_thread, 0);
  }

  while (gtd.highlights[0]) {
    do_unhighlight(gtd.highlights[0]->condition);
  }

  free(gtd.highlights);

  if (message) {
    printf("\n%s\n", message);
  }

  fflush(stdout);
  exit(exit_signal);
}

void pipe_handler(int sig) { display_printf("broken_pipe: %i", sig); }

void trap_handler(int sig) { quitmsg("trap_handler", sig); }

void winch_handler(int sig) {
  struct winsize screen;

  if (sig) { /* Just to make a compiler warning shut up */
  }

  if (ioctl(STDIN_FILENO, TIOCGWINSZ, &screen) == -1) {
    gtd.rows = SCREEN_HEIGHT;
    gtd.cols = SCREEN_WIDTH;
  } else {
    gtd.rows = screen.ws_row;
    gtd.cols = screen.ws_col;
  }
}
