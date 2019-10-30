#!/usr/bin/env python3
"""Helper functions for initializing ChromaTerm's config."""

import os
import re
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

    if not isinstance(regex, str):
        return 'regex not a string'

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


def rgb_to_8bit(_r, _g, _b):
    """Downscale from 24-bit RGB to 8-bit ANSI."""
    def downscale(value, base=6):
        return int(value / 256 * base)

    if _r == _g == _b:  # Use the 24 shades of the grayscale.
        return 232 + downscale(_r, base=24)

    return 16 + (36 * downscale(_r)) + (6 * downscale(_g)) + downscale(_b)
