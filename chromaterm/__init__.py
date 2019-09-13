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
        're': re.compile(r'\033\[(?:0?|3(?:[0-79]|8;[0-9;]+))m')
    },
    'bg': {
        'default': '\033[49m',
        're': re.compile(r'\033\[(?:0?|4(?:[0-79]|8;[0-9;]+))m')
    },
    'blink': {
        'default': '\033[25m',
        're': re.compile(r'\033\[(?:0?|2?5)m')
    },
    'bold': {
        'default': '\033[21m',
        're': re.compile(r'\033\[(?:0?|2?1)m')
    },
    'italic': {
        'default': '\033[23m',
        're': re.compile(r'\033\[(?:0?|2?3)m')
    },
    'strike': {
        'default': '\033[29m',
        're': re.compile(r'\033\[(?:0?|2?9)m')
    },
    'underline': {
        'default': '\033[24m',
        're': re.compile(r'\033\[(?:0?|2?4)m')
    }
}

# Detect a complete reset of the SGR
COMPLETE_RESET_RE = re.compile(r'\033\[0?m')

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
# on performace as the wait is cancelled if stdin becomes ready to be read from.
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
                        help='Use RGB colors (default: attempt detection, '
                        'fall-back to xterm-256)')

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


def get_color_types(code):
    """Using RESET_TYPES, return a list of type names the match the code."""
    return [x for x in RESET_TYPES if RESET_TYPES[x]['re'].search(code)]


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
    """According to the rules in the `config`, return the highlighted 'data'."""
    # Empty data or no rules, don't bother doing anything
    if not data or not config['rules']:
        return data

    existing = []

    # Existing colors in the data
    for match in SGR_RE.finditer(data):
        types = get_color_types(match.group())
        if types:
            existing.append({
                'position': match.start(),
                'code': match.group(),
                'types': types
            })

    # Remove existing colors from the data for cleaner matching (added back later)
    shift = 0
    for insert in existing:
        insert['position'] -= shift
        index = insert['position']
        data = data[:index] + data[index + len(insert['code']):]
        shift += len(insert['code'])

    inserts = []  # The list of colors and their positions (inserts)

    # Get the list of the new colors
    for rule in config['rules']:
        inserts += get_rule_inserts(rule, data)

    # Process all of the inserts, returning the final list including existing
    inserts = process_inserts(inserts, existing, config)

    # Insert the colors into the data
    for insert in inserts:
        index = insert['position']
        data = data[:index] + insert['code'] + data[index:]

    updated_resets = []

    # Update the resets according to the last reset of each type
    for insert in inserts:
        left = [x for x in config['resets'] if x not in updated_resets]

        if not left:  # All resets updated
            break

        for name in left:
            if name in insert['types']:
                # Complete reset; go back to default reset of the type
                if COMPLETE_RESET_RE.search(insert['code']):
                    config['resets'][name] = RESET_TYPES[name]['default']
                else:
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

    # Parse the rules
    rules = load.get('rules', []) if isinstance(load, dict) else None
    rules = rules if isinstance(rules, list) else []

    for rule in rules:
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
    splits = split_buffer(buffer)

    if not splits:
        return ''

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
    returning a list of colors (dict containing position and code). The list of
    existing colors is used for recovery of colliding colors."""
    def get_last_color(colors, position, color_type):
        """Return the first color before the requested position and of the color
        type, or None."""
        for color in reversed(sorted(colors, key=lambda x: x['position'])):
            if color['position'] < position and color_type in color['types']:
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
                'types': [color['type']]
            })

            if not last_color:  # No last color; use current type reset
                code = config['resets'][color['type']]
            else:
                code = last_color['code']

                # Last color is in the middle of current color and is a reset;
                # set it to our color so that it doesn't reset it.
                if last_color['position'] > insert['start'] and last_color[
                        'code'] == config['resets'][color['type']]:
                    last_color['code'] = color['code']

            # Post-reverse, end of the current color comes after previous ends
            #finals.append({
            finals.insert(0, {
                'position': insert['end'],
                'code': code,
                'types': [color['type']]
            })

    # Sort from end to start (index magic). Must reverse after sorting, not during
    return list(reversed(sorted(finals, key=lambda x: x['position'])))


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
    block until the there's daata. If `timeout` is specified, the function will
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
    """Split the buffer based on movement sequences, returning an array with
    objects in the format of [data, separator]. `data` is the part that should
    be highlighted while the sperator is printed unchanged."""
    if not buffer:
        return buffer

    splits = MOVEMENT_RE.split(buffer)

    # Append an empty seperator in case of no splits or no seperator at the end
    splits.append('')

    # Group all splits into format of [data, separator]
    return [[x, y] for x, y in zip(splits[0::2], splits[1::2])]


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
