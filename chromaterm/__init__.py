#!/usr/bin/env python3
"""Colorize your output using RegEx"""

import argparse
import os
import re
import select
import signal
import sys

import yaml

# Named SGR codes
STYLES = {
    'blink': '\033[5m',
    'bold': '\033[1m',
    'italic': '\033[3m',
    'strike': '\033[9m',
    'underline': '\033[4m',
}

# Reset types, their default reset codes, and RegEx's for detecting color type
RESET_TYPES = {
    'fg': {
        'default': '\033[39m',
        're': re.compile(r'\033\[3(?:[0-79]|8;[0-9;]+)m')
    },
    'bg': {
        'default': '\033[49m',
        're': re.compile(r'\033\[4(?:[0-79]|8;[0-9;]+)m')
    },
    'blink': {
        'default': '\033[25m',
        're': re.compile(r'\033\[2?5m')
    },
    'bold': {
        'default': '\033[21m',
        're': re.compile(r'\033\[2?1m')
    },
    'italic': {
        'default': '\033[23m',
        're': re.compile(r'\033\[2?3m')
    },
    'strike': {
        'default': '\033[29m',
        're': re.compile(r'\033\[2?9m')
    },
    'underline': {
        'default': '\033[24m',
        're': re.compile(r'\033\[2?4m')
    }
}

# Sequences that change the screen's layout or cursor's position
MOVEMENT_RE = re.compile(r'(\033\[[0-9]*[A-GJKST]|\033\[[0-9;]*[Hf]|\033\[\?'
                         r'1049[hl]|\r|\r\n|\n|\v|\f)')

# Select Graphic Rendition sequence (all types)
SGR_RE = re.compile(r'\033\[[0-9;]*m')

# Maximum chuck size per read
READ_SIZE = 4096  # 4 KiB

# CT cannot determine if it is processing input faster than the piping process
# is outputting or if the input has finished. To work around this, CT will wait
# a bit prior to assuming there's no more data in the buffer. There's no impact
# on performance as the wait is cancelled if stdin becomes ready to be read from.
WAIT_FOR_SPLIT = 0.0005

# Print-once for deprecation messages
# pylint: disable=global-statement
DEPRECATE_MSG_GROUP = False


def config_init(args=None):
    """Return the parsed configuration according to the program arguments. if
    there is an error, a string with the message is returned."""
    parser = argparse.ArgumentParser(description='Colorize your output using'
                                     'RegEx.')

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
        import psutil  # Imported here to reduce normal startup delay
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

    signal.signal(signal.SIGINT, signal.SIG_IGN)  # Ignore SIGINT
    signal.signal(signal.SIGUSR1, update_config_handler)  # Reload handler

    return config


def decode_sgr(source_code):
    """Decode an SGR, splitting it into discrete colors."""
    def make_sgr(code_id):
        return '\033[' + str(code_id) + 'm'

    colors = []
    codes = source_code.lstrip('\033[').rstrip('m').split(';')
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


def get_color_code(color, rgb=False):
    """Return the ANSI codes, one for each color, to be used when highlighting
    with `color` or None if the `color` is invalid."""
    color = color.lower().strip()
    words = '|'.join([x for x in STYLES])
    color_re = r'(?i)^(((b|f)#([0-9a-fA-F]{6})|' + words + r')(\s+|$))+$'

    if not re.search(color_re, color):
        return None

    codes = []

    # Colors
    for match in re.findall(r'(b|f)#([0-9a-fA-F]{6})', color):
        if match[0] == 'f':
            target, name = '\033[38;', 'fg'
        else:
            target, name = '\033[48;', 'bg'

        for code in [x['code'] for x in codes]:
            if target in code:  # Duplicate color target
                return None

        rgb_int = [int(match[1][i:i + 2], 16) for i in [0, 2, 4]]

        if rgb:
            target += '2;'
            color_id = ';'.join([str(x) for x in rgb_int])
        else:
            target += '5;'
            color_id = str(rgb_to_8bit(*rgb_int))

        codes.append({'code': target + color_id + 'm', 'type': name})

    # Styles
    for name in re.findall(words, color.lower().strip()):
        if STYLES[name] in [x['code'] for x in codes]:  # Duplicate style
            return None

        codes.append({'code': STYLES[name], 'type': name})

    return codes or None


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
        global DEPRECATE_MSG_GROUP
        group = rule.get('group')

        if not DEPRECATE_MSG_GROUP and group is not None:
            DEPRECATE_MSG_GROUP = True
            eprint('group key is deprecated; use color dictionary instead')

        color = {group or 0: color}
    elif not isinstance(color, dict):
        return 'color not a string or dictionary'

    for group in color:
        if not isinstance(group, int):
            return 'group "{}" not an integer'.format(group)

        if group > regex_compiled.groups:
            return 'group {} not in the regex'.format(group)

        color_code = get_color_code(color[group], rgb=rgb)

        if not color_code:
            return 'color "{}" not in the correct format'.format(color)

        color[group] = color_code

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

    # Indicated more data to possibly come and stdin confirmed it
    if more and read_ready(WAIT_FOR_SPLIT):
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


def read_ready(timeout=None):
    """Return True if sys.stdin has data or is closed. If `timeout` is None,
    block until there's data. If `timeout` is specified, the function will
    return False if expired or True as soon as the condition is met."""
    return sys.stdin in select.select([sys.stdin], [], [], timeout)[0]


def rgb_to_8bit(_r, _g, _b):
    """Downscale from 24-bit RGB to 8-bit ANSI."""
    def downscale(value, base=6):
        return int(value / 256 * base)

    if _r == _g == _b:  # Use the 24 shades of the grayscale.
        return 232 + downscale(_r, base=24)

    return 16 + (36 * downscale(_r)) + (6 * downscale(_g)) + downscale(_b)


def split_buffer(buffer):
    """Split the buffer based on movement sequences, returning a list of lists
    in the format of [data, separator]. `data` is the part that should be
    highlighted while the sperator is printed unchanged."""
    splits = MOVEMENT_RE.split(buffer)

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


def main(config, max_wait=None):
    """Main entry point that uses `config` from config_init to process stdin.
    `max_wait` is the longest period to wait without input before returning."""
    if isinstance(config, str):  # An error message
        return config

    buffer = ''

    while read_ready(max_wait):
        data = os.read(sys.stdin.fileno(), READ_SIZE)
        buffer += data.decode()

        if not buffer:  # Entire buffer was processed empty and fd is closed
            break

        # Process the buffer, updating it with any left-over data
        buffer = process_buffer(config, buffer, bool(data))
