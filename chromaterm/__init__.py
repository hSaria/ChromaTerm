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

import yaml

# Named SGR codes
STYLES = {
    'blink': '\x1b[5m',
    'bold': '\x1b[1m',
    'italic': '\x1b[3m',
    'strike': '\x1b[9m',
    'underline': '\x1b[4m',
}

# Reset types, their default reset codes, and RegEx's for detecting color type
RESET_TYPES = {
    'fg': {
        'default': '\x1b[39m',
        're': re.compile(r'\x1b\[3(?:[0-79]|8;[0-9;]+)m')
    },
    'bg': {
        'default': '\x1b[49m',
        're': re.compile(r'\x1b\[4(?:[0-79]|8;[0-9;]+)m')
    },
    'blink': {
        'default': '\x1b[25m',
        're': re.compile(r'\x1b\[2?5m')
    },
    'bold': {
        'default': '\x1b[22m',  # Normal intensity
        're': re.compile(r'\x1b\[(?:1|2?2)m')  # Any intensity type
    },
    'italic': {
        'default': '\x1b[23m',
        're': re.compile(r'\x1b\[2?3m')
    },
    'strike': {
        'default': '\x1b[29m',
        're': re.compile(r'\x1b\[2?9m')
    },
    'underline': {
        'default': '\x1b[24m',
        're': re.compile(r'\x1b\[2?4m')
    }
}

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


def config_init(args=None):
    """Return the parsed configuration according to the program arguments. if
    there is an error, a string with the message is returned."""
    epilog = """ChromaTerm reads from standard input; just pipe data to it. As
    this exposes the existance of a pipe to the piping process, ChromaTerm
    can run your program in order to hide the pipe. This is normally only
    needed for programs that want to be on a controlling terminal, like `less`."""

    parser = argparse.ArgumentParser(epilog=epilog)

    parser.add_argument('program',
                        type=str,
                        nargs=argparse.REMAINDER,
                        help='run a program with anything after it used as '
                        'arguments')
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
        run_program(config, args.program)

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


def eprint(*args, **kwargs):
    """Error print."""
    print(sys.argv[0] + ':', *args, file=sys.stderr, **kwargs)


def get_color_codes(color_str, rgb=False):
    """Return the ANSI codes, one for each color, to be used when highlighting
    with `color_str` or None if the `color_str` is invalid."""
    color_str = color_str.lower().strip()
    color_re = r'^(((b|f)#[0-9a-f]{6}|' + '|'.join(STYLES) + r')(\s+|$))+$'

    if not re.search(color_re, color_str):
        return None

    colors = []

    # Colors
    for match in re.findall(r'(b|f)#([0-9a-f]{6})', color_str):
        if match[0] == 'f':
            target, name = '\x1b[38;', 'fg'
        else:
            target, name = '\x1b[48;', 'bg'

        if name in [x['type'] for x in colors]:  # Duplicate color target
            return None

        rgb_int = [int(match[1][i:i + 2], 16) for i in [0, 2, 4]]

        if rgb:
            target += '2;'
            color_id = ';'.join([str(x) for x in rgb_int])
        else:
            target += '5;'
            color_id = str(rgb_to_8bit(*rgb_int))

        colors.append({'code': target + color_id + 'm', 'type': name})

    # Styles
    for name in re.findall('|'.join(STYLES), color_str):
        if name in [x['type'] for x in colors]:  # Duplicate style
            return None

        colors.append({'code': STYLES[name], 'type': name})

    return colors or None


def get_default_config():
    """Return a dict with the default configuration."""
    config = {'rules': []}
    config['resets'] = {x: RESET_TYPES[x]['default'] for x in RESET_TYPES}

    return config


def get_rule_inserts(rule, data):
    """Return a list of dicts, with each dict containing the start position, end
    position, and color of a match."""
    inserts = []

    for match in rule['regex'].finditer(data):
        for group in rule['colors']:
            if match.group(group) is None:  # Group not part of the match
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


def parse_config(data, config=None, rgb=False):
    """Parse `data` (a YAML string), modifying/returning the `config` dict."""
    if config is None:
        config = get_default_config()

    try:  # Load the YAML configuration file
        load = yaml.safe_load(data) or {}
    except yaml.YAMLError as exception:
        eprint('Parse error:', exception)
        return config

    config['rules'] = []  # Reset the rules list
    rules = load.get('rules', []) if isinstance(load, dict) else None

    # Parse the rules
    for rule in rules if isinstance(rules, list) else []:
        parsed_rule = parse_rule(rule, rgb=rgb)
        if isinstance(parsed_rule, dict):
            config['rules'].append(parsed_rule)
        else:
            eprint('Rule error on {}: {}'.format(rule, parsed_rule))

    return config


def parse_rule(rule, rgb=False):
    """Return a dict of the description, regex (compiled), and color (code)."""
    # pylint: disable=too-many-return-statements

    description = rule.get('description', '')

    regex = rule.get('regex')
    if not regex:
        return 'regex not found'

    if not isinstance(regex, str) and not isinstance(regex, int):
        return 'regex not a string or integer'

    try:
        regex_compiled = re.compile(regex)
    except re.error as exception:
        return 're.error: ' + str(exception)

    color = rule.get('color')
    if not color:
        return 'color not found'

    if isinstance(color, str):
        color = {0: color}
    elif not isinstance(color, dict):
        return 'color not a string or dictionary'

    for group in color:
        if not isinstance(group, int):
            return 'group "{}" not an integer'.format(group)

        if group > regex_compiled.groups:
            return 'group {} not in the regex'.format(group)

        color_codes = get_color_codes(color[group], rgb=rgb)

        if not color_codes:
            return 'color "{}" not in the correct format'.format(color)

        color[group] = color_codes

    return {
        'description': description,
        'regex': regex_compiled,
        'colors': color
    }


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


def read_file(location):
    """Read a file at `location`, returning its contents, or None on error."""
    location = os.path.expandvars(location)

    if not os.access(location, os.F_OK):
        eprint('Configuration file', location, 'not found')
        return None

    if not os.access(location, os.R_OK):
        eprint('Cannot read configuration file', location, '(permission)')
        return None

    with open(location, 'r') as file:
        return file.read()


def read_ready(read_fds, timeout=None):
    """Return the list of fds that have  data or have hit EOF. If `timeout` is
    None, block until data/EOF. If `timeout` is specified, assume not ready on
    timeout (i.e. won't be added to the list)."""
    if read_fds is None:
        return []
    return select.select(read_fds, [], [], timeout)[0]


def rgb_to_8bit(_r, _g, _b):
    """Downscale from 24-bit RGB to 8-bit ANSI."""
    def downscale(value, base=6):
        return int(value / 256 * base)

    if _r == _g == _b:  # Use the 24 shades of the grayscale.
        return 232 + downscale(_r, base=24)

    return 16 + (36 * downscale(_r)) + (6 * downscale(_g)) + downscale(_b)


def run_program(config, program_args):
    """Fork a program with its stdout set to use an os.opentty. config['fork' is
    updated with the tty and close pipes Once the program closes, it will write
    a dummy byte to the close pipe."""
    import fcntl
    import termios
    import shutil
    import struct

    # Create the tty and close_signal file decriptors
    tty_r, tty_w = os.openpty()
    close_r, close_w = os.pipe()

    config['read_fds'] = [tty_r, close_r]
    config['fork'] = {
        'tty': {
            'read': tty_r,
            'write': tty_w
        },
        'close': {
            'read': close_r,
            'write': close_w
        }
    }

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
    """Main entry point that uses `config` from config_init to process data.
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
            buffer += data.decode()

        if not buffer:  # Buffer was processed empty and data fd hit EOF
            break

        # Process the buffer, updating it with any left-over data
        buffer = process_buffer(config, buffer, bool(data))
