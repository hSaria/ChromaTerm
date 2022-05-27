'''Unix/macOS functions'''
import atexit
import fcntl
import os
import pty
import signal
import sys
import termios
import threading
import time
import tty

# The frequency to check the child process' `cwd` and update our own
CWD_UPDATE_INTERVAL = 1 / 4


def get_stdin():
    '''Returns the file descriptor for stdin.'''
    return sys.stdin.fileno()


def run_program(program_args):
    '''Returns a file descriptor of the bidirectional pipe to the spawned program.

    Args:
        program_args (list): A list of program arguments. The first argument is
            the program name/location.
    '''
    try:
        # Set to raw as the pty will be handling any processing; restore at exit
        attributes = termios.tcgetattr(sys.stdin.fileno())
        atexit.register(termios.tcsetattr, sys.stdin.fileno(), termios.TCSANOW,
                        attributes)
        tty.setraw(sys.stdin.fileno(), termios.TCSANOW)
    except termios.error:
        attributes = None

    # openpty, login_tty, then fork
    pid, master_fd = pty.fork()

    # The fork process is replaced with exec or it exits if it hits an exception
    if pid == 0:
        # Update the slave's pty (now on standard fds) attributes
        if attributes:
            termios.tcsetattr(sys.stdin.fileno(), termios.TCSANOW, attributes)

        try:
            os.execvp(program_args[0], program_args)
        except OSError as exception:
            sys.exit(f'{exception.strerror.lower()}: {program_args[0]}')

    # Update the slave's window size as the master is capturing the signals
    def window_resize_handler(*_):
        size = fcntl.ioctl(sys.stdin.fileno(), termios.TIOCGWINSZ, '0000')
        fcntl.ioctl(master_fd, termios.TIOCSWINSZ, size)

    if sys.stdin.isatty():
        signal.signal(signal.SIGWINCH, window_resize_handler)
        window_resize_handler()

    # Some terminals update their titles based on `cwd` (see #94)
    def update_cwd():  # pragma: no cover
        # Covered by `test_main_cwd_tracking` but not detected by coverage
        # pylint: disable=import-outside-toplevel
        import psutil

        try:
            child_process = psutil.Process(pid)
        except (psutil.AccessDenied, psutil.NoSuchProcess):
            return

        while True:
            try:
                time.sleep(CWD_UPDATE_INTERVAL)
                os.chdir(child_process.cwd())
            except (OSError, psutil.AccessDenied, psutil.NoSuchProcess):
                pass

    # Daemonized to exit immediately with ChromaTerm
    threading.Thread(target=update_cwd, daemon=True).start()

    return master_fd
