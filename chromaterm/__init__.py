#!/usr/bin/env python3
"""Colorize your output using RegEx"""

import argparse
import os
import re
import select
import signal
import sys

import yaml

COLOR_RE = re.compile(r'\033\[[0-9;]*m')
MOVEMENT_RE = re.compile(r'(\033\[[0-9]*[A-GJKST]|\033\[[0-9;]*[Hf]|\033\[\?'
                         r'1049[hl]|\r|\r\n|\n|\v|\f)')

# Maximum chuck size per read
READ_SIZE = 65536  # 64 KiB

STYLES = {
    'blink': '5',
    'bold': '1',
    'italic': '3',
    'strike': '9',
    'underline': '4',
}

# CT cannot determine if it is processing input faster than the piping process
# is outputting or if the input has finished. To work around this, CT will wait
# a bit prior to assuming there's no more data in the buffer. There's no impact
# on performace as the wait is cancelled if stdin becomes ready to be read from.
WAIT_FOR_NEW_LINE = 0.0005

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

    def update_config_handler(_1=None, _2=None):
        parse_config(read_file(args.config) or '', config, rgb)

    # Ignore SIGINT, reload config with SIGUSR1
    signal.signal(signal.SIGINT, signal.SIG_IGN)
    signal.signal(signal.SIGUSR1, update_config_handler)

    return config


def eprint(*args, **kwargs):
    """Error print."""
    print(sys.argv[0] + ':', *args, file=sys.stderr, **kwargs)


def get_color_code(color, rgb=False):
    """Return the ANSI code to be used when highlighting with `color` or None if
    the `color` is invalid. The `color` is a string in the format of b#abcdef
    for background or f#abcdef for foreground. Can be multiple colors if
    separated by a space."""
    color = color.lower().strip()
    words = '|'.join([x for x in STYLES])
    color_re = r'(?i)^(((b|f)#([0-9a-fA-F]{6})|' + words + r')(\s+|$))+$'

    if not re.search(color_re, color):
        return None

    codes = []

    # Colors
    for match in re.findall(r'(b|f)#([0-9a-fA-F]{6})', color):
        target = '38;' if match[0] == 'f' else '48;'
        rgb_int = [int(match[1][i:i + 2], 16) for i in [0, 2, 4]]

        for code in codes:
            if code.startswith(target):  # Duplicate color target
                return None

        if rgb:
            target += '2;'
            color_id = ';'.join([str(x) for x in rgb_int])
        else:
            target += '5;'
            color_id = rgb_to_8bit(*rgb_int)

        codes.append(target + str(color_id))

    # Styles
    for match in re.findall(words, color.lower().strip()):
        if STYLES[match] in codes:  # Duplicate style
            return None

        codes.append(STYLES[match])

    return '\033[' + ';'.join(codes) + 'm' if codes else None


def get_default_config():
    """Return a dict with the default configuration."""
    return {'rules': [], 'reset_code': '\033[m'}


def get_rule_inserts(rule, data):
    """Return a list of dicts, with each dict containing a start position, end
    position, and color of a match."""
    inserts = []

    for match in rule['regex'].finditer(data):
        for group in rule['color']:
            if match.group(group) is None:  # Group not part of the match
                continue

            inserts.append({
                'start': match.start(group),
                'end': match.end(group),
                'color': rule['color'][group]
            })

    return inserts


def highlight(config, data):
    """According to the rules in the `config`, return the highlighted 'data'."""
    if not data:  # Empty data, don't bother doing anything
        return data

    inserts = []
    existing = []

    # Find existing colors
    for match in COLOR_RE.finditer(data):
        existing.append({'position': match.start(0), 'code': match.group(0)})

    # Get all inserts from the rules
    for rule in config['rules']:
        inserts += get_rule_inserts(rule, data)

    # Process all of the inserts, returning the final list.
    inserts = process_inserts(inserts, existing, config['reset_code'])

    # Sort the inserts according to the position, from end to beginning
    inserts = sorted(inserts, key=lambda x: x['position'], reverse=True)

    # Insert the colors into the data
    for insert in inserts:
        index = insert['position']
        data = data[:index] + insert['code'] + data[index:]

    # Use the last code as the reset for any next colors. The last reset is
    # whatever the color originally was.
    if inserts + existing:
        last_code = sorted(inserts + existing, key=lambda x: x['position'])[-1]
        config['reset_code'] = last_code['code']

    return data


def parse_config(data, config=None, rgb=False):
    """Parse `data` (a YAML string), modifying/returning the `config` dict."""
    if config is None:
        config = get_default_config()

    config['rules'] = []

    try:  # Load the YAML configuration file
        load = yaml.safe_load(data) or {}
    except yaml.YAMLError as exception:
        eprint('Parse error:', exception)
        return config

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
    """Return a dict containing the description, regex (compiled), color (code),
    and group."""
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
        'color': color,
        'group': group
    }


def process_buffer(config, buffer, more):
    """Process the `buffer` using the `config`, returning any left-over data.
    If there's `more` data coming up, only process up to the last movement
    split. Otherwise, process all of the buffer."""
    lines = split_buffer(buffer)

    if not lines:
        return ''

    for line in lines[:-1]:  # Process all lines except for the last
        print(highlight(config, line[0]) + line[1], end='')

    # Indicated more data to possibly come and stdin confirmed it
    if more and read_ready(WAIT_FOR_NEW_LINE):
        # Return last line as the left-over data
        return lines[-1][0] + lines[-1][1]

    # No more data; print last line and flush as it doesn't have a new line
    print(highlight(config, lines[-1][0]) + lines[-1][1], end='', flush=True)

    return ''  # All of the buffer was processed; return an empty buffer


def process_inserts(inserts, existing, reset_code):
    """Process a list of rule inserts, removing any unnecessary colors, and
    returning a list of colors (dict containing position and code). The list of
    existing colors is used for recovery of colliding colors."""
    def get_last_color(colors, position):
        """Return the last color before the requested position, or None if no
        previous color."""
        for color in sorted(colors, key=lambda x: x['position'], reverse=True):
            if color['position'] < position:
                return color
        return None

    final_inserts = []

    # Remove any resets in the middle of another color
    for insert in inserts:
        current_positions = [x['position'] for x in final_inserts]
        overlapping_start = insert['start'] in current_positions
        overlapping_end = insert['end'] in current_positions

        # Already matched by a different rule; don't bother adding the insert
        if overlapping_start and overlapping_end:
            continue

        # Get the last color prior to adding this insert into the final list
        last_color = get_last_color(final_inserts + existing, insert['end'])

        final_inserts.append({
            'position': insert['start'],
            'code': insert['color']
        })

        # There's already a color at the end position
        if overlapping_end:
            continue

        if not last_color:  # No last color; default reset
            code = reset_code
        else:
            code = last_color['code']

            # Last color is a reset and in the middle of current color; update it
            if last_color['position'] > insert['start'] and last_color[
                    'code'] == reset_code:
                last_color['code'] = insert['color']

        final_inserts.append({'position': insert['end'], 'code': code})

    return final_inserts


def read_file(location):
    """Read a file at `location`, returning its contents, or None if there's a
    problem."""
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
    this function will block until the True condition is met. If `timeout` is
    specified, the function will return False if expired or True as soon as the
    condition is met."""
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
    be highlighted while the sperator remains untouched."""
    lines = MOVEMENT_RE.split(buffer)

    if lines == ['']:
        return ''

    # If no splits or did not end with a separator, append a empty seperator
    if len(lines) == 1 or lines[-1] != '':
        lines.append('')

    # Group all lines into format of [data, separator]
    lines = [[x, y] for x, y in zip(lines[0::2], lines[1::2])]

    return lines


def main(config, max_wait=None):
    """Main entry point that uses `config` from config_init to process stdin.
    `max_wait` is the longest period to wait without input before quiting."""
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
