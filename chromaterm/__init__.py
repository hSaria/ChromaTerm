#!/usr/bin/env python3
"""Colorize your output using RegEx."""

# A couple of sections of the program are used _rarely_ and I don't want to
# _always_ spend time importing them.
# pylint: disable=import-outside-toplevel

import argparse
import os
import re
import select
import signal
import sys

from chromaterm.config import RESET_TYPES, eprint, parse_config, read_file

# Sequences upon which ct will split during processing. This includes new lines,
# vertical spaces, form feeds, C1 set (ECMA-048), and CSI (excluding SGR).
SPLIT_RE = re.compile(r'(\r\n?|\n|\v|\f|\x1b[\x40-\x5a\x5c-\x5f]|'
                      r'\x1b\[[\x30-\x3f]*[\x20-\x2f]*[\x40-\x6c\x6e-\x7e])')

# Select Graphic Rendition sequence (all types)
SGR_RE = re.compile(r'\x1b\[[0-9;]*m')

# Maximum chuck size per read
READ_SIZE = 4096  # 4 KiB

# CT cannot determine if it is processing input faster than the piping process
# is outputting or if the input has finished. To work around this, CT will wait
# a bit prior to assuming there's no more data in the buffer. There's no impact
# on performance as the wait is cancelled if read_fd becomes ready.
WAIT_FOR_SPLIT = 0.0005


def args_init(args=None):
    """Return the parsed configuration according to the program arguments. if
    there is an error, a string with the message is returned."""
    epilog = """ChromaTerm reads from standard input; just pipe data to it. As
    this exposes the existance of a pipe to the piping process, ChromaTerm
    can run your program in order to hide the pipe. This is normally only
    needed for programs that want to be on a controlling terminal, like `less`."""

    parser = argparse.ArgumentParser(epilog=epilog)

    parser.add_argument('program',
                        type=str,
                        metavar='program ...',
                        nargs='?',
                        help='run a program with anything after it used as '
                        'arguments')
    parser.add_argument('arguments',
                        type=str,
                        nargs=argparse.REMAINDER,
                        help=argparse.SUPPRESS,
                        default=[])
    parser.add_argument('--config',
                        metavar='FILE',
                        type=str,
                        help='location of config file (default: %(default)s)',
                        default='$HOME/.chromaterm.yml')
    parser.add_argument('--reload',
                        action='store_true',
                        help='Reload the config of all CT instances')
    parser.add_argument('--rgb',
                        action='store_true',
                        help='Use RGB colors (default: detect support, '
                        'fallback to xterm-256)')

    args = parser.parse_args(args)

    if args.reload:
        import psutil
        count = 0

        for process in [x.as_dict() for x in psutil.process_iter()]:
            if process['pid'] == os.getpid():  # Skip the current process
                continue

            if process['cmdline'] and sys.argv[0] in process['cmdline']:
                os.kill(process['pid'], signal.SIGUSR1)
                count += 1

        return 'Processes reloaded: ' + str(count)

    rgb = os.getenv('COLORTERM') == 'truecolor' or args.rgb
    config = parse_config(read_file(args.config) or '', rgb=rgb)

    def update_config_handler(_1, _2):
        parse_config(read_file(args.config) or '', config, rgb)

    if args.program:
        run_program(config, [args.program] + args.arguments)

    signal.signal(signal.SIGPIPE, signal.SIG_DFL)  # Default for broken pipe
    signal.signal(signal.SIGINT, signal.SIG_IGN)  # Ignore SIGINT
    signal.signal(signal.SIGUSR1, update_config_handler)  # Reload handler

    return config


def decode_sgr(source_code):
    """Decode an SGR, splitting it into discrete colors."""
    def make_sgr(code_id):
        return '\x1b[' + str(code_id) + 'm'

    colors = []
    codes = source_code.lstrip('\x1b[').rstrip('m').split(';')
    skip = 0

    for index, code in enumerate(codes):
        if skip:  # Code processed by an index look-ahead; skip it
            skip -= 1
            continue

        if code == '' or int(code) == 0:  # Complete reset
            colors.append({'code': make_sgr(code), 'type': 'complete_reset'})
        elif code in ['38', '48']:  # Multi-code SGR
            color_type = 'fg' if code == '38' else 'bg'

            if len(codes) > index + 2 and codes[index + 1] == '5':  # xterm-256
                skip = 2
                code = ';'.join([str(codes[index + x]) for x in range(3)])
            elif len(codes) > index + 4 and codes[index + 1] == '2':  # RGB
                skip = 4
                code = ';'.join([str(codes[index + x]) for x in range(5)])
            else:  # Does not conform to format; do not touch code
                return [{'code': source_code, 'type': None}]

            colors.append({'code': make_sgr(code), 'type': color_type})
        else:  # Single-code SGR
            for name in RESET_TYPES:
                if RESET_TYPES[name]['re'].search(make_sgr(int(code))):
                    colors.append({'code': make_sgr(code), 'type': name})

    return colors


def get_rule_inserts(rule, data):
    """Return a list of dicts, with each dict containing the start position, end
    position, and color of a match."""
    inserts = []

    for match in rule['regex'].finditer(data):
        for group in rule['colors']:
            # Group is zero-length or is not in the match (start = end = -1)
            if match.start(group) == match.end(group):
                continue

            inserts.append({
                'start': match.start(group),
                'end': match.end(group),
                'colors': rule['colors'][group]
            })

    return inserts


def highlight(config, data):
    """According to the rules in the `config`, return the highlighted 'data'.
    The resets of each color type in the config are updated accordingly."""
    if not data:  # Empty data, don't bother doing anything
        return data

    existing, data = strip_colors(data)

    inserts = []  # The list of colors and their positions (inserts)

    for rule in config['rules']:  # Get the list of the new colors
        inserts += get_rule_inserts(rule, data)

    # Process all of the inserts, returning the final list, including existing
    inserts = process_inserts(inserts, existing, config)
    updated_resets = []

    for insert in inserts:  # Insert the colors into the data
        index = insert['position']
        data = data[:index] + insert['code'] + data[index:]

        # Update the resets according to the last reset of each type
        for name in [x for x in config['resets'] if x not in updated_resets]:
            if insert['type'] == 'complete_reset':
                # Set to type's default reset on a complete reset
                config['resets'][name] = RESET_TYPES[name]['default']
                updated_resets.append(name)
            elif name == insert['type']:
                config['resets'][name] = insert['code']
                updated_resets.append(name)

    return data


def process_buffer(config, buffer, more):
    """Process the `buffer` using the `config`, returning any left-over data. If
    there's `more`, only process up to the last split. Otherwise, process all of
    the buffer."""
    if not buffer or not config['rules']:  # Nothing to do
        print(buffer, end='')
        return ''

    splits = split_buffer(buffer)

    for split in splits[:-1]:  # Process all splits except for the last
        print(highlight(config, split[0]) + split[1], end='')

    # Indicated more data to possibly come and read_fd confirmed it
    if more and read_ready(config.get('read_fds'), WAIT_FOR_SPLIT):
        # Return last split as the left-over data
        return splits[-1][0] + splits[-1][1]

    # No more data; print last split and flush as it doesn't have a new line
    print(highlight(config, splits[-1][0]) + splits[-1][1], end='', flush=True)

    return ''  # All of the buffer was processed; return an empty buffer


def process_inserts(inserts, existing, config):
    """Process a list of rule inserts, removing any unnecessary colors, and
    returning an iterator of colors. The list of existing colors is used for
    recovery of colliding colors. Existing colors are included in the iterator."""
    def get_last_color(colors, position, color_type):
        """Return the first color before the requested position and of the color
        type, or None."""
        color_types = [color_type, 'complete_reset']

        for color in reversed(sorted(colors, key=lambda x: x['position'])):
            if color['position'] < position and color['type'] in color_types:
                return color
        return None

    finals = existing

    for insert in inserts:
        for color in insert['colors']:
            # Get the last color prior to adding current color to the final list
            last_color = get_last_color(finals, insert['end'], color['type'])

            finals.append({
                'position': insert['start'],
                'code': color['code'],
                'type': color['type']
            })

            if not last_color:  # No last color; use current type reset
                reset = config['resets'][color['type']]
                color_type = color['type']
            else:
                reset = last_color['code']

                # Last color is in the middle of current color.
                if last_color['position'] > insert['start']:
                    # Last color is a (complete )?reset
                    if last_color['type'] == 'complete_reset' or last_color[
                            'code'] == config['resets'][color['type']]:
                        # Set it to our color so that it doesn't reset it.
                        last_color['code'] = color['code']
                        last_color['type'] = color['type']
                        reset = config['resets'][color['type']]

                color_type = last_color['type']

            # Post-reverse, end of the current color comes after previous ends
            finals.insert(0, {
                'position': insert['end'],
                'code': reset,
                'type': color_type
            })

    # Sort from end to start (index magic). Must reverse after sorting, not during
    return reversed(sorted(finals, key=lambda x: x['position']))


def read_ready(read_fds, timeout=None):
    """Return the list of fds that have  data or have hit EOF. If `timeout` is
    None, block until data/EOF. If `timeout` is specified, assume not ready on
    timeout (i.e. won't be added to the list)."""
    if read_fds is None:
        return []
    return select.select(read_fds, [], [], timeout)[0]


def run_program(config, program_args):
    """Fork a program with its stdout set to use an os.opentty. Once the program
    closes, it will write a dummy byte to the close pipe. config['read_fds'] is
    populated with the program FD followed by the close FD, in that order."""
    import fcntl
    import termios
    import shutil
    import struct

    # Create the tty and close_signal file decriptors
    tty_r, tty_w = os.openpty()
    close_r, close_w = os.pipe()

    config['read_fds'] = [tty_r, close_r]

    # Update terminal size on the program's TTY (starts uninitialized)
    window_size = shutil.get_terminal_size()
    window_size = struct.pack('2H', window_size.lines, window_size.columns)
    fcntl.ioctl(tty_w, termios.TIOCSWINSZ, window_size)

    if os.fork() == 0:  # Program
        try:
            import subprocess
            subprocess.run(program_args, check=False, stdout=tty_w)
        except FileNotFoundError:
            eprint(program_args[0] + ': command not found')
        except KeyboardInterrupt:  # pragma: no cover  # Limitation when forking
            pass  # Program gets the signal; CT shouldn't freak out
        finally:
            os.write(close_w, b'\x00')
        sys.exit()


def split_buffer(buffer):
    """Split the buffer based on split sequences, returning a list of lists in
    the format of [data, separator]. `data` should be highlighted while the
    `sperator` is printed unchanged, after the `data`."""
    splits = SPLIT_RE.split(buffer)

    # Append an empty separator in case of no splits or no separator at the end
    splits.append('')

    # Group all splits into format of [data, separator]
    return [[x, y] for x, y in zip(splits[0::2], splits[1::2])]


def strip_colors(data):
    """Remove the colors from the data, returning a list of colors as well as
    the clean data. The color positions are relative to the "clean" data."""
    colors = []

    while True:
        match = SGR_RE.search(data)  # Get the first match

        if not match:  # Stop if there aren't any SGR's in the data
            break

        for color in decode_sgr(match.group()):  # Split compound colors
            color['position'] = match.start()
            colors.append(color)

        # Remove match from data; next match's start is in the clean data
        data = data[:match.start()] + data[match.end():]

    return colors, data


def main(config, max_wait=None, read_fd=None):
    """Main entry point that uses `config` from args_init to process data.
    `max_wait` is the longest period to wait without input before returning.
    read_fd will utilize stdin if not specified. If config['read_fds'] is set,
    the read_fd keyword is ignored."""
    if isinstance(config, str):  # An error message
        return config

    if read_fd is None:
        read_fd = sys.stdin.fileno()

    buffer = ''
    config['read_fds'] = config.get('read_fds', [read_fd])

    while True:
        ready_fds = read_ready(config['read_fds'], max_wait)

        if config['read_fds'][0] in ready_fds:  # Data FD
            data = os.read(config['read_fds'][0], READ_SIZE)
            buffer += data.decode(encoding='utf-8', errors='replace')

        if not buffer:  # Buffer was processed empty and data fd hit EOF
            break

        # Process the buffer, updating it with any left-over data
        buffer = process_buffer(config, buffer, bool(data))
