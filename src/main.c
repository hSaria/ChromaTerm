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
        config_override = TRUE;
        if (access(optarg, R_OK) == 0) {
          do_read(optarg);
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
      quit_with_signal(EXIT_FAILURE);
    }
  }

  quit_with_signal(EXIT_SUCCESS);
  return 0; /* Literally useless, but gotta make a warning shut up. */
}

void init_program() {
  /* initial size is 8, but is dynamically resized as required */
  gd.highlights = (struct highlight **)calloc(8, sizeof(struct highlight *));
  gd.highlights_size = 8;

  /* Default configuration values */
  SET_BIT(gd.flags, SES_FLAG_HIGHLIGHT);
  SET_BIT(gd.flags, SES_FLAG_INTERACTIVE);

#if (defined HAVE_CURSES_H && defined HAVE_MENU_H)
#ifdef IWPGWEPJGpowgpojwegjpowejpog
  while (TRUE) {
    main_menu();
  }

  quit_with_signal(EXIT_SUCCESS);

  if ((gd.fd_ct = open("/dev/tty", O_RDWR)) < 0) { /* Used for CT */
    perror("Couldn't open /dev/tty");
    quit_with_signal(EXIT_FAILURE);
  }
#endif
#endif
}

void help_menu(char *proc_name) {
  display_printf("ChromaTerm-- v%s", VERSION);
  display_printf("Usage: %s [OPTION]... [FILE]...", proc_name);
  display_printf("    -c {CONFIG_FILE}      Override configuration file");
  display_printf("    -p                    Passive CT-- (menu disabled)");
  display_printf("    -t {TITLE}            Set title");

  quit_with_signal(2);
}

void quit_with_signal(int exit_signal) {

  /* Free memory used by highlights */
  while (gd.highlights[0]) {
    do_unhighlight(gd.highlights[0]->condition);
  }

  free(gd.highlights);

  close(gd.fd_ct);

  fflush(stdout);
  exit(exit_signal);
}
