/* This program is protected under the GNU GPL (See COPYING) */

#include "defs.h"

struct global_data gd;

int main(int argc, char **argv) {
  fd_set readfds;
  int config_override = FALSE;
  char filename[4095];

  init_program();

  if (argc > 1) {
    int c;

    optind = 1;

    while ((c = getopt(argc, argv, "h c: t:")) != EOF) {
      switch (tolower(c)) {
      case 'c':
        config_override = TRUE;
        strcpy(filename, optarg);
        break;
      case 't':
        printf("\033]0;%s\007", optarg);
        break;
      default:
        help_menu(argv[0]);
        break;
      }
    }
  }

  /* Read configuration if not overridden by the launch arguments */
  if (!config_override) {
    if (access(".chromatermrc", R_OK) == 0) {
      strcpy(filename, ".chromatermrc");
    } else {
      if (getenv("HOME") != NULL) {
        char temp[PATH_MAX];
        sprintf(temp, "%s/%s", getenv("HOME"), ".chromatermrc");

        if (access(temp, R_OK) == 0) {
          strcpy(filename, temp);
        }
      }
    }
  }

  if (filename[0]) {
    do_read(filename);
  }

  FD_ZERO(&readfds); /* Initialise the file descriptor */
  FD_SET(STDIN_FILENO, &readfds);

  /* MAIN LOGIC OF THE PROGRAM STARTS HERE */
  while ((gd.input_buffer_length +=
          read(STDIN_FILENO, &gd.input_buffer[gd.input_buffer_length],
               INPUT_MAX - gd.input_buffer_length - 1)) > 0) {
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

void help_menu(char *proc_name) {
  printf("ChromaTerm-- v%s\n", VERSION);
  printf("Usage: [PROCESS] | %s [OPTIONS]\n", proc_name);
  printf("%6s %-18s Override configuration file\n", "-c", "{CONFIG_FILE}");
  printf("%6s %-18s Set title\n", "-t", "{TITLE}");

  quit_with_signal(2);
}

void init_program() {
  /* initial size is 8, but is dynamically resized as required */
  gd.highlights = (struct highlight **)calloc(8, sizeof(struct highlight *));
  gd.highlights_size = 8;

  gd.command_char = '%';
  SET_BIT(gd.flags, SES_FLAG_HIGHLIGHT);
}

void quit_with_signal(int exit_signal) {
  /* Free memory used by highlights */
  while (gd.highlights[0]) {
    do_unhighlight(gd.highlights[0]->condition);
  }

  free(gd.highlights);

  fflush(stdout);
  exit(exit_signal);
}
