'''The command line utility for ChromaTerm'''
# pylint: disable=import-outside-toplevel
import argparse
import atexit
import io
import os
import re
import select
import signal
import sys
from ctypes.util import find_library

import yaml

from chromaterm import Color, Config, Palette, Rule, __version__

# Possible locations of the config file, without the extension. Most-specific
# locations lead in the list.
CONFIG_LOCATIONS = [
    '~/.chromaterm',
    os.getenv('XDG_CONFIG_HOME', '~/.config') + '/chromaterm/chromaterm',
    '/etc/chromaterm/chromaterm',
]

# Maximum chuck size per read
READ_SIZE = 8192

# Sequences upon which ct will split during processing (ECMA 035 and 048):
# * new lines, vertical spaces, form feeds;
# * private control functions, C1 set (excluding control strings);
# * independent control functions (\e#), SCS (G0 through G3 sets);
# * CSI (excluding SGR); and
# * control strings (DSC, SOS, OSC, PM, APC).
SPLIT_RE = re.compile(
    br'(\r\n?|[\n\v\f]|'
    br'\x1b[\x30-\x4f\x51-\x57\x59-\x5a\x5c\x60-\x7e]|'
    br'\x1b[\x23\x28-\x2b\x2d-\x2f][\x20-\x7e]|'
    br'\x1b\x5b[\x30-\x3f]*[\x20-\x2f]*[\x40-\x6c\x6e-\x7e]|'
    br'\x1b[\x50\x58\x5d\x5e\x5f][^\x07\x1b]*(?:\x07|\x1b\x5c)?)')

# Control strings that have arbitrary lengths
ANSI_CONTROL_STRINGS_START = (b'\x1b\x50', b'\x1b\x58', b'\x1b\x5d',
                              b'\x1b\x5e', b'\x1b\x5f')
ANSI_CONTROL_STRINGS_END = (b'\x07', b'\x1b\x5c')


def args_init(args=None):
    '''Returns the parsed arguments (an instance of argparse.Namespace).

    Args:
        args (list): A list of program arguments, Defaults to sys.argv.
    '''
    formatter = lambda prog: argparse.HelpFormatter(prog, max_help_position=30)
    parser = argparse.ArgumentParser(formatter_class=formatter)
    parser.epilog = 'For more info, go to https://github.com/hSaria/ChromaTerm.'

    parser.add_argument('program',
                        metavar='program ...',
                        nargs='?',
                        help='run a program with anything after it used as '
                        'arguments')

    parser.add_argument('arguments',
                        nargs=argparse.REMAINDER,
                        help=argparse.SUPPRESS)

    parser.add_argument('-b',
                        '--benchmark',
                        action='store_true',
                        help='at exit, print rule usage statistics')

    parser.add_argument('-c',
                        '--config',
                        metavar='file',
                        help='override config file location (default: ~/'
                        '.chromaterm.yml)')

    parser.add_argument('-r',
                        '--reload',
                        action='store_true',
                        help='reload the config of all CT instances')

    parser.add_argument('-R',
                        '--rgb',
                        action='store_true',
                        help='use RGB colors (default: detect support, '
                        'fallback to xterm-256)')

    parser.add_argument('-v',
                        '--version',
                        action='version',
                        version=f'%(prog)s {__version__}')

    parser.add_argument('--pcre',
                        action='store_true',
                        help="(advanced) use PCRE2 instead of Python's RE"
                        if find_library('pcre2-8') else argparse.SUPPRESS)

    args = parser.parse_args(args=args)

    # Detect rgb support if not specified
    args.rgb = args.rgb or os.getenv('COLORTERM') in ('truecolor', '24bit')

    return args


def eprint(*args, **kwargs):
    '''Prints a message to stderr.'''
    # Use \r\n to move to beginning of the new line in raw mode
    print('ct:', *args, end='\r\n', file=sys.stderr, **kwargs)


def get_default_config_location():
    '''Returns the first location in `CONFIG_LOCATIONS` that points to a file,
    defaulting to the first item in the list.'''
    resolve = lambda x: os.path.expanduser(os.path.expandvars(x))

    for location in CONFIG_LOCATIONS:
        for extension in ['.yml', '.yaml']:
            path = resolve(location + extension)

            if os.path.isfile(path):
                return path

    # No file found; default to the most-specific location
    return resolve(CONFIG_LOCATIONS[0] + '.yml')


def get_wait_duration(buffer, min_wait=1 / 256, max_wait=1 / 8):
    '''Returns the duration (float) to wait for more data before the last chunk
    of the received data can be processed independently.

    ChromaTerm cannot determine if it's processing data faster than input rate or
    if the input has finished. Therefore, ChromaTerm waits before processing the
    last chunk in the buffer. The waiting is cancelled if data becomes available.
    1/256 is smaller (shorter) than the fastest key repeat (1/255 second). 1/8th
    of a second is generally enough to account for the latency of command
    processing or a remote connection.

    Args:
        buffer (bytes): The incoming data that was processed.
        min_wait (float): Lower-bound on the returned duration.
        max_wait (float): Upper-bound on the returned duration.
    '''
    # New lines indicate long output; relax the wait duration
    if b'\n' in buffer:
        return max_wait

    return min_wait


def load_config(config, data, rgb=False, pcre=False):
    '''Reads configuration from a YAML-based string, formatted like so:
    palette:
      red: "#ff0000"
      blue: "#0000ff"

    rules:
    - description: My first rule
        regex: Hello
        color: b.red
    - description: My second rule is specfic to groups
        regex: W(or)ld
        color:
        0: f#321321
        1: bold

    Any errors are printed to stderr.

    Args:
        config (chromaterm.Config): The instance to which data is loaded.
        data (str): A string containg YAML data.
        rgb (bool): Whether the terminal is RGB-capable or not.
    '''
    try:
        data = yaml.safe_load(data) or {}
    except yaml.YAMLError as exception:
        eprint('Parse error:', exception)
        return

    palette = data.get('palette') if isinstance(data, dict) else None
    rules = data.get('rules') if isinstance(data, dict) else None

    if palette is not None:
        if not isinstance(palette, dict):
            eprint('Parse error: "palette" is not a dictionary')
            return

        palette = parse_palette(palette)

        if not isinstance(palette, Palette):
            eprint(palette)
            return

    if rules is None:
        eprint('Parse error: "rules" list not found in configuration')
        return

    if not isinstance(rules, list):
        eprint('Parse error: "rules" is not a list')
        return

    config.rules.clear()

    for rule in rules:
        rule = parse_rule(rule, palette=palette, rgb=rgb, pcre=pcre)

        if isinstance(rule, Rule):
            config.rules.append(rule)
        else:
            eprint(rule)

    # Put the non-overlapping (exclusive=True) rules at the top
    config.rules = sorted(config.rules, key=lambda x: not x.exclusive)


def parse_palette(data):
    '''Returns an instance of chromaterm.Palette if parsed correctly. Otherwise,
    a string with the error message is returned.

    Args:
        data (dict): A dictionary representing the color palette, formatted
            according to `chromaterm.__main__.load_config`.
    '''
    palette = Palette()

    try:
        for name, value in data.items():
            palette.add_color(name, value)
    except (TypeError, ValueError) as exception:
        return f'Error on palette color {repr(name)}: {repr(value)}: {exception}'

    return palette


def parse_rule(data, palette=None, rgb=False, pcre=False):
    '''Returns an instance of chromaterm.Rule if parsed correctly. Otherwise, a
    string with the error message is returned.

    Args:
        data (dict): A dictionary representing the rule, formatted according to
            `chromaterm.__main__.load_config`.
        palette (chromaterm.Palette): A palette to resolve colors.
        rgb (bool): Whether the terminal is RGB-capable or not.
    '''
    if not isinstance(data, dict):
        return f'Rule {repr(data)} not a dictionary'

    description = data.get('description')
    exclusive = data.get('exclusive', False)
    regex = data.get('regex')
    color = data.get('color')
    rule_repr = f'Rule(regex={repr(str(regex)[:30])})'

    if description:
        rule_repr = rule_repr[:-1] + f', description={repr(description)})'

    if not isinstance(color, dict):
        color = {0: color}

    try:
        for group, value in color.items():
            color[group] = Color(value, palette=palette, rgb=rgb)

        return Rule(regex, color, description, exclusive, pcre)
    except (TypeError, ValueError) as exception:
        return f'Error on {rule_repr}: {exception}'
    except re.error as exception:
        return f'Error on {rule_repr}: re.error: {exception}'


def process_input(config, data_fd, forward_fd=None, max_wait=None):
    '''Processes input by reading from data_fd, highlighting it using config,
    then printing it to sys.stdout. If forward_fd is not None, any data it has
    will be written (forwarded) into data_fd.

    Args:
        config (chromaterm.Config): Used for highlighting the data.
        data_fd (int): File descriptor or socket to be read and highlighted.
        forward_fd (int): File descriptor or socket to be forwarded into data_fd.
        max_wait (float): The maximum time to wait with no data on either of the
            file descriptors. None will block until at least one ready to be read.
    '''
    # pylint: disable=too-many-branches
    # #93: Avoid BlockingIOError when output to non-blocking stdout is too fast
    try:
        getattr(os, 'set_blocking', lambda *_: None)(sys.stdout.fileno(), True)
    except io.UnsupportedOperation:
        pass

    if isinstance(data_fd, int):
        forward = lambda: os.write(data_fd, os.read(forward_fd, READ_SIZE))
        read = lambda: os.read(data_fd, READ_SIZE)
    else:
        forward = lambda: data_fd.sendall(forward_fd.recv(READ_SIZE))
        read = lambda: data_fd.recv(READ_SIZE)

    fds = [data_fd] if forward_fd is None else [data_fd, forward_fd]
    buffer = b''

    ready_fds = read_ready(*fds, timeout=max_wait)

    while ready_fds:
        # There's some data to forward to the spawned program
        if forward_fd in ready_fds:
            try:
                forward()
            except OSError:
                # Spawned program or stdin closed; don't forward anymore
                fds.remove(forward_fd)

        # Data to be highlighted was received
        if data_fd in ready_fds:
            try:
                data_read = read()
            except OSError:
                data_read = b''

            buffer += data_read

            # Buffer was processed empty and data fd hit EOF
            if not buffer:
                break

            chunks = split_buffer(buffer)

            # Process chunks except for the last one as it might've been cut off
            for data, separator in chunks[:-1]:
                sys.stdout.buffer.write(config.highlight(data) + separator)

            # Flush as `read_ready` might delay the flush at the bottom
            sys.stdout.flush()

            # Wait is cancelled when the user is typing; relax the wait duration
            if forward_fd is not None:
                wait_duration = get_wait_duration(buffer, min_wait=1 / 64)
            else:
                wait_duration = get_wait_duration(buffer)

            data, separator = chunks[-1]

            # Separator is an incomplete control string; wait for the rest
            if data_read and separator.startswith(ANSI_CONTROL_STRINGS_START) \
                    and not separator.endswith(ANSI_CONTROL_STRINGS_END):
                buffer = data + separator
            # A single character indicates keyboard typing; don't highlight
            # Account for backspaces added by some shells, like zsh
            elif 0 < len(buffer) < 2 + data_read.count(b'\b') * 2:
                sys.stdout.buffer.write(data + separator)
                buffer = b''
            # There's more data; fetch it before highlighting, if buffer isn't full
            elif data_read and len(buffer) <= READ_SIZE \
                    and data_fd in read_ready(*fds, timeout=wait_duration):
                buffer = data + separator
            else:
                sys.stdout.buffer.write(config.highlight(data) + separator)
                buffer = b''

            # Flush as the last chunk might not end with a new line
            sys.stdout.flush()

        ready_fds = read_ready(*fds, timeout=max_wait)


def read_file(location):
    '''Returns the contents of a file or `None` on error. The error is printed
    to stderr.

    Args:
        location (str): The location of the file to be read.
    '''
    if not os.access(location, os.F_OK):
        eprint(f'Configuration file {repr(location)} not found')
        return None

    try:
        with open(location, 'r', encoding='utf-8') as file:
            return file.read()
    except PermissionError:
        eprint(f'Cannot read configuration file {repr(location)} (permission)')
        return None


def read_ready(*fds, timeout=None):
    '''Returns a list of file descriptors that are ready to be read.

    Args:
        *fds (int): Integers that refer to the file descriptors.
        timeout (float): Passed to `select.select`.
    '''
    return [] if not fds else select.select(fds, [], [], timeout)[0]


def signal_chromaterm_instances(sig):
    '''Sends `sig` signal to all other ChromaTerm CLI instances.

    Returns:
        The number of processes reloaded.
    '''
    import psutil

    count = 0
    current_process = psutil.Process()

    for process in psutil.process_iter():
        # Don't reload the current process
        if process.pid == current_process.pid:
            continue

        try:
            # Only compare the first two arguments (Python and script paths)
            if process.cmdline()[:2] == current_process.cmdline()[:2]:
                os.kill(process.pid, sig)
                count += 1
        except (psutil.AccessDenied, psutil.NoSuchProcess):
            pass

    return count


def split_buffer(buffer):
    '''Returns a tuple of tuples in the format of (data, separator). data should
    be highlighted while separator should be printed, unchanged, after data.

    Args:
        buffer (bytes): Data to split using `SPLIT_RE`.
    '''
    chunks = SPLIT_RE.split(buffer)

    # Append an empty separator when chunks end with data
    if chunks[-1]:
        chunks.append(b'')

    # Group all chunks into format of (data, separator)
    return tuple(zip(chunks[0::2], chunks[1::2]))


def main(args=None, max_wait=None, write_default=True):
    '''Command line utility entry point.

    Args:
        args (list): A list of program arguments. Defaults to sys.argv.
        max_wait (float): The maximum time to wait with no data. None will block
            until data is ready to be read.
        write_default (bool): Whether to write the default configuration or not.
            Only written if it doesn't exist already.

    Returns:
        A string indicating status/error. Otherwise, returns None.
    '''
    # pylint: disable=expression-not-assigned
    args = args_init(args)

    if args.reload:
        return f'Processes reloaded: {signal_chromaterm_instances(signal.SIGUSR1)}'

    # Config file wasn't overridden; use default file
    if not args.config:
        args.config = get_default_config_location()

        # Write default config if not there
        if write_default and not os.access(args.config, os.F_OK):
            import chromaterm.default_config
            chromaterm.default_config.write_default_config(args.config)

    config = Config(benchmark=args.benchmark)

    # Print benchmark after cleanup in `run_program`
    if args.benchmark:
        atexit.register(config.print_benchmark_results)

    import chromaterm.platform.unix as platform

    if args.program:
        # ChromaTerm is spawning the program in a pty; stdin is forwarded
        data_fd = platform.run_program([args.program] + args.arguments)
        forward_fd = platform.get_stdin()
    else:
        # Data is being piped into ChromaTerm's stdin; no forwarding needed
        data_fd = platform.get_stdin()
        forward_fd = None

    # Signal handler to trigger reloading the config
    def reload_config_handler(*_):
        config_data = read_file(args.config)

        if config_data:
            load_config(config, config_data, rgb=args.rgb, pcre=args.pcre)

    # Trigger the initial loading
    reload_config_handler()

    # Ignore SIGINT (CTRL+C) and attach reload handler
    signal.signal(signal.SIGINT, signal.SIG_IGN)
    signal.signal(signal.SIGUSR1, reload_config_handler)

    try:
        # Begin processing the data (blocking operation)
        process_input(config, data_fd, forward_fd, max_wait)
    except BrokenPipeError:
        # Surpress the implicit flush that Python runs on exit
        sys.stdout.flush = lambda: None
    finally:
        # Close data_fd to signal to the child process that we're done
        os.close(data_fd) if isinstance(data_fd, int) else data_fd.close()

    return os.wait()[1] >> 8 if args.program else 0


if __name__ == '__main__':
    sys.exit(main())
