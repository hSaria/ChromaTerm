/* This program is protected under the GNU GPL (See COPYING) */

#include "defs.h"

struct global_data gd;

int main(int argc, char **argv) {
  fd_set readfds;
  int config_override = FALSE;

  init_program();

  if (argc > 1) {
    int c;

    optind = 1;

    while ((c = getopt(argc, argv, "h c: t: p")) != EOF) {
      switch (c) {
      case 'c':
        if (access(optarg, R_OK) == 0) {
          do_read(optarg);
          config_override = TRUE;
        }
        break;
      case 't':
        printf("\033]0;%s\007", optarg);
        break;
      case 'p':
        DEL_BIT(gd.flags, SES_FLAG_INTERACTIVE);
        break;
      default:
        help_menu(argv[0]);
        break;
      }
    }
  }

  /* Read configuration if not overridden by the launch arguments */
  if (!config_override && FALSE) { // TEMP
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

  char *hello[10];
  int i = 0, sel = -1;
  for (i = 0; *command_table[i].name != 0; i++) {
    hello[i] = command_table[i].name;
  }

  sel = show_menu(hello, i);
  if (sel > -1) {
    printf("%s\n", command_table[sel].name);
  }

  sel = show_menu(hello, 3);
  if (sel > -1) {
    printf("%s\n", command_table[sel].name);
  }

  quit_with_msg(EXIT_SUCCESS); // DEBUG

  FD_ZERO(&readfds); /* Initialise the file descriptor */
  FD_SET(STDIN_FILENO, &readfds);

  /* MAIN LOGIC OF THE PROGRAM STARTS HERE */
  while ((gd.input_buffer_length +=
          read(STDIN_FILENO, &gd.input_buffer[gd.input_buffer_length],
               INPUT_MAX)) > 0) {
    /* Mandatoy wait before assuming no more output on the current line */
    struct timeval wait = {0, WAIT_FOR_NEW_LINE};

    /* Block for a small amount to see if there's more to read. If something
     * came up, stop waiting and move on. */
    int rv = select(STDIN_FILENO + 1, &readfds, NULL, NULL, &wait);

    if (rv > 0) { /* More data came up while waiting */
      /* Failsafe: if the buffer is full, process all of pending output.
       * Otherwise, process until the line that doesn't end with \n. */
      process_input(INPUT_MAX - gd.input_buffer_length <= 1 ? FALSE : TRUE);
    } else if (rv == 0) { /* timed-out while waiting for FD (no more output) */
      process_input(FALSE); /* Process all that's left */
    } else if (rv < 0) {    /* error */
      perror("select returned < 0");
      quit_with_msg(EXIT_FAILURE);
    }
  }

  quit_with_msg(EXIT_SUCCESS);
  return 0; /* Literally useless, but gotta make a warning shut up. */
}

void init_program() {
  /* initial size is 8, but is dynamically resized as required */
  gd.highlights = (struct highlight **)calloc(8, sizeof(struct highlight *));
  gd.highlights_size = 8;

  /* Default configuration values */
  gd.command_char = '%';
  SET_BIT(gd.flags, SES_FLAG_HIGHLIGHT);
  SET_BIT(gd.flags, SES_FLAG_INTERACTIVE);

#ifdef HAVE_CURSES_H
#ifdef HAVE_MENU_H
  initscr();            /* initialise default ncurses screen (stdscr) */
  cbreak();             /* Disable line buffering */
  noecho();             /* Don't print input to the CT menu back */
  keypad(stdscr, TRUE); /* Accept special characters (e.g. arrow keys) */
#endif
#endif

  if ((gd.fd_ct = open("/dev/tty", O_RDWR)) < 0) { /* Used for CT */
    perror("Couldn't open /dev/tty");
    quit_with_msg(EXIT_FAILURE);
  }
}

void help_menu(char *proc_name) {
  display_printf("ChromaTerm-- v%s", VERSION);
  display_printf("Usage: %s [OPTION]... [FILE]...", proc_name);
  display_printf("    -c {CONFIG_FILE}      Override configuration file");
  display_printf("    -p                    Passive CT-- (menu disabled)");
  display_printf("    -t {TITLE}            Set title");

  quit_with_msg(2);
}

void quit_with_msg(int exit_signal) {

  /* Free memory used by highlights */
  while (gd.highlights[0]) {
    do_unhighlight(gd.highlights[0]->condition);
  }

  free(gd.highlights);

  close(gd.fd_ct);

  fflush(stdout);
  exit(exit_signal);
}
