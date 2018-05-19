/* This program is protected under the GNU GPL (See COPYING) */

#include "defs.h"

struct global_data gd;

pthread_t input_thread;
pthread_t output_thread;

static int quit_ran = FALSE;

int main(int argc, char **argv) {
  int config_override = FALSE;
  char command[BUFFER_SIZE];
  struct sigaction trap, pipe, winch;

  trap.sa_handler = trap_handler;
  pipe.sa_handler = pipe_handler;
  winch.sa_handler = winch_handler;

  trap.sa_flags = pipe.sa_flags = winch.sa_flags = 0;

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

    /* Execute the hanging argument */
    if (argv[optind] != NULL) {
      strcpy(command, optarg);
    }
  }

  /* Read configuration if not overridden by the launch arguments */
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

  /* Only run command if it wasn't overridden by a configuration file */
  if (!gd.run_overriden) {
    gd.run_overriden = TRUE;
    do_run(command);
  }

  if (pthread_create(&input_thread, NULL, poll_input, NULL) != 0) {
    quit_with_msg("failed to create input thread", 1);
  }

  pthread_join(input_thread, NULL);

  quit_with_msg(NULL, 0);
  return 0; /* Literally useless, but gotta make a warning shut up. */
}

void init_program() {
  struct termios io;

  gd.run_overriden = FALSE;

  /* initial size is 8, but is dynamically resized as required */
  gd.highlights = (struct highlight **)calloc(8, sizeof(struct highlight *));
  gd.highlights_size = 8;

  /* Get the screen size */
  winch_handler(0);

  /* Default configuration values */
  gd.command_char = '%';
  SET_BIT(gd.flags, SES_FLAG_HIGHLIGHT);

  /* Save current terminal attributes and reset at exit */
  if (tcgetattr(STDIN_FILENO, &gd.saved_terminal)) {
    quit_with_msg("tcgetattr", 1);
  }

  tcgetattr(STDIN_FILENO, &gd.active_terminal);

  io = gd.active_terminal;

  /*  Canonical mode off */
  DEL_BIT(io.c_lflag, ICANON);

  io.c_cc[VMIN] = 1;
  io.c_cc[VTIME] = 0;
  io.c_cc[VSTART] = 255;
  io.c_cc[VSTOP] = 255;

  DEL_BIT(io.c_lflag, ECHO | ECHONL | IEXTEN | ISIG);
  SET_BIT(io.c_cflag, CS8);

  if (tcsetattr(STDIN_FILENO, TCSANOW, &io)) {
    quit_with_msg("tcsetattr", 1);
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

  quit_with_msg(NULL, error);
}

/* Unless there's an error, the quitmsg is ran before exitting */
void quit_void(void) {
  if (!quit_ran) {
    quit_with_msg(NULL, 1);
  }
}

void quit_with_msg(char *message, int exit_signal) {
  quit_ran = TRUE;

  /* Restore original, saved terminal */
  tcsetattr(STDIN_FILENO, TCSANOW, &gd.saved_terminal);

  if (kill(gd.pid, 0) && gd.socket) {
    close(gd.socket);
    if (gd.pid) {
      /* force kill */
      kill(gd.pid, SIGKILL);
    }
  }

  if (input_thread) {
    pthread_kill(input_thread, 0);
  }

  if (output_thread) {
    pthread_kill(output_thread, 0);
  }

  /* Free memory used by highlights */
  while (gd.highlights[0]) {
    do_unhighlight(gd.highlights[0]->condition);
  }

  free(gd.highlights);

  /* Print msg, if any */
  if (message) {
    printf("\n%s\n", message);
  }

  fflush(stdout);
  exit(exit_signal);
}

void pipe_handler(int sig) { display_printf("broken_pipe: %i", sig); }

void trap_handler(int sig) { quit_with_msg("trap_handler", sig); }

void winch_handler(int sig) {
  struct winsize screen;

  if (sig) { /* Just to make a compiler warning shut up */
  }

  if (ioctl(STDIN_FILENO, TIOCGWINSZ, &screen) == -1) {
    gd.rows = SCREEN_HEIGHT;
    gd.cols = SCREEN_WIDTH;
  } else {
    gd.rows = screen.ws_row;
    gd.cols = screen.ws_col;
  }
}
