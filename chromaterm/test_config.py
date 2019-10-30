#!/usr/bin/env python3
"""Config-related tests."""

import os
import re

import chromaterm.config

TEMP_FILE = '.test_chromaterm_config.yml'


def test_eprint(capsys):
    """Print a message to stderr."""
    msg = 'Some random error message'
    chromaterm.config.eprint(msg)
    assert msg in capsys.readouterr().err


def test_get_color_codes():
    """Random known-good color codes."""
    colors = [
        'b#0973d8', 'b#8b6dd3', 'b#2867c7', 'b#18923a', 'b#636836', 'b#a0da5e',
        'b#b99153', 'b#fafb19', 'f#6cb0d7', 'f#6f5e3a', 'f#7d6256', 'f#15c93d',
        'f#45f2d7', 'f#50a910', 'f#1589b3', 'f#b8df23', 'f#d5a3bf', 'f#d7e764',
        'f#e002d7', 'f#f56726'
    ]
    codes = [
        '48;5;33m', '48;5;140m', '48;5;32m', '48;5;35m', '48;5;101m',
        '48;5;156m', '48;5;179m', '48;5;226m', '38;5;117m', '38;5;101m',
        '38;5;102m', '38;5;41m', '38;5;87m', '38;5;70m', '38;5;38m',
        '38;5;190m', '38;5;182m', '38;5;228m', '38;5;201m', '38;5;208m'
    ]

    for color, code in zip(colors, codes):
        color_code = chromaterm.config.get_color_codes(color)[0]['code']
        assert color_code == '\x1b[' + code


def test_get_color_codes_grayscale():
    """Random known-good grayscale codes."""
    colors = [
        'b#000000', 'b#5d5d5d', 'b#373737', 'b#c8c8c8', 'b#cecece', 'b#d7d7d7',
        'b#d8d8d8', 'b#fcfcfc', 'f#0b0b0b', 'f#000000', 'f#2b2b2b', 'f#2f2f2f',
        'f#4c4c4c', 'f#4d4d4d', 'f#9d9d9d', 'f#808080'
    ]
    codes = [
        '48;5;232m', '48;5;240m', '48;5;237m', '48;5;250m', '48;5;251m',
        '48;5;252m', '48;5;252m', '48;5;255m', '38;5;233m', '38;5;232m',
        '38;5;236m', '38;5;236m', '38;5;239m', '38;5;239m', '38;5;246m',
        '38;5;244m'
    ]

    for color, code in zip(colors, codes):
        color_code = chromaterm.config.get_color_codes(color)[0]['code']
        assert color_code == '\x1b[' + code


def test_get_color_codes_rgb():
    """RGB color-codes."""
    colors = ['b#010101', 'f#020202']
    codes = ['48;2;1;1;1m', '38;2;2;2;2m']

    for color, code in zip(colors, codes):
        color_code = chromaterm.config.get_color_codes(color, True)[0]['code']
        assert color_code == '\x1b[' + code


def test_get_color_codes_style():
    """Terminal styles."""
    colors = ['blink', 'BOLD', 'iTaLiC', 'strike', 'underline']
    codes = ['5m', '1m', '3m', '9m', '4m']

    for color, code in zip(colors, codes):
        color_code = chromaterm.config.get_color_codes(color, True)[0]['code']
        assert color_code == '\x1b[' + code


def test_get_color_codes_compound():
    """All sorts of color codes."""
    colors = 'bold b#0973d8 underline f#45f2d7'
    # Styles are always added last
    codes = ['\x1b[48;5;33m', '\x1b[38;5;87m', '\x1b[1m', '\x1b[4m']

    for color, code in zip(chromaterm.config.get_color_codes(colors), codes):
        assert color['code'] == code


def test_get_color_codes_mixed_case():
    """Color is mixed case."""
    colors = 'b#abcABC bOlD'

    assert len(chromaterm.config.get_color_codes(colors)) == 2


def test_get_color_codes_excessive_colors():
    """Too many colors (more than 2)."""
    colors = 'b#010101 f#020202 f#020202'

    assert chromaterm.config.get_color_codes(colors) is None


def test_get_color_codes_duplicate_target():
    """Duplicate targets (e.g. two foreground colors)."""
    colors = ['f#020202 f#030303', 'bold bold']

    for color in colors:
        assert chromaterm.config.get_color_codes(color) is None


def test_parse_config_simple():
    """Parse a config file with a simple rule."""
    config_data = '''rules:
    - description: simple
      regex: hello world
      color: f#fffaaa'''

    assert chromaterm.config.parse_config(config_data)['rules']


def test_parse_config_group():
    """Parse a config file with a group-specific rule."""
    config_data = '''rules:
    - regex: hello (world)! It's (me).
      color:
        1: b#fffaaa
        2: f#123123'''

    assert chromaterm.config.parse_config(config_data)['rules']


def test_parse_config_group_legacy():
    """Parse a config file with a legacy group-specific rule."""
    config_data = '''rules:
    - regex: hello (world)
      color: b#fffaaa
      group: 1'''

    assert chromaterm.config.parse_config(config_data)['rules']


def test_parse_config_multiple_colors():
    """Parse a config file with a multi-color rule."""
    config_data = '''rules:
    - description: group
      regex: hello (world)
      color: b#fffaaa f#aaafff'''

    assert chromaterm.config.parse_config(config_data)['rules']


def test_parse_config_rule_format_error(capsys):
    """Parse a config file with a syntax problem."""
    config_data = '''rules:
    - description: simple'''
    chromaterm.config.parse_config(config_data)

    assert 'Rule error on' in capsys.readouterr().err


def test_parse_config_yaml_format_error(capsys):
    """Parse an incorrect YAML file."""
    chromaterm.config.parse_config('-x-\nhi:')
    assert 'Parse error:' in capsys.readouterr().err


def test_parse_rule_regex_missing():
    """Parse a rule without a `regex` key."""
    msg = 'regex not found'
    rule = {'color': 'b#fffaaa'}
    assert chromaterm.config.parse_rule(rule) == msg


def test_parse_rule_regex_type_error():
    """Parse a rule with an incorrect `regex` value type."""
    msg = 'regex not a string'
    rule = {'regex': ['hi'], 'color': 'b#fffaaa'}
    assert chromaterm.config.parse_rule(rule) == msg

    rule = {'regex': 111, 'color': 'b#fffaaa'}
    assert chromaterm.config.parse_rule(rule) == msg


def test_parse_rule_regex_invalid():
    """Parse a rule with an invalid `regex`."""
    rule = {'regex': '+', 'color': 'b#fffaaa'}
    assert 're.error: ' in chromaterm.config.parse_rule(rule)


def test_parse_rule_color_missing():
    """Parse a rule without a `color` key."""
    msg = 'color not found'
    rule = {'regex': 'x(y)z'}
    assert chromaterm.config.parse_rule(rule) == msg


def test_parse_rule_color_type_error():
    """Parse a rule with an incorrect `color` value type."""
    msg = 'color not a string or dictionary'
    rule = {'regex': 'x(y)z', 'color': ['hi']}
    assert chromaterm.config.parse_rule(rule) == msg


def test_parse_rule_color_format_error():
    """Parse a rule with an incorrect `color` format."""
    msg_re = 'color ".+" not in the correct format'

    rule = {'regex': 'x(y)z', 'color': 'b#xyzxyz'}
    assert re.search(msg_re, chromaterm.config.parse_rule(rule))

    rule = {'regex': 'x(y)z', 'color': 'x#fffaaa'}
    assert re.search(msg_re, chromaterm.config.parse_rule(rule))

    rule = {'regex': 'x(y)z', 'color': 'b@fffaaa'}
    assert re.search(msg_re, chromaterm.config.parse_rule(rule))

    rule = {'regex': 'x(y)z', 'color': 'b#fffaaa-f#fffaaa'}
    assert re.search(msg_re, chromaterm.config.parse_rule(rule))


def test_parse_rule_group_type_error():
    """Parse a rule with an incorrect `group` value type."""
    msg_re = 'group .+ not an integer'
    rule = {'regex': 'x(y)z', 'color': {'1': 'b#fffaaa'}}
    assert re.search(msg_re, chromaterm.config.parse_rule(rule))


def test_parse_rule_group_out_of_bounds():
    """Parse a rule with `group` number not in the regex."""
    msg_re = 'group .+ not in the regex'
    rule = {'regex': 'x(y)z', 'color': {2: 'b#fffaaa'}}
    assert re.search(msg_re, chromaterm.config.parse_rule(rule))


def test_read_file():
    """Read the default configuration."""
    assert chromaterm.read_file('$HOME/.chromaterm.yml') is not None


def test_read_file_no_permission(capsys):
    """Create a file with no permissions and attempt to read it. Delete the file
    once done with it."""
    msg = 'Cannot read configuration file ' + TEMP_FILE + '1' + ' (permission)\n'

    os.close(os.open(TEMP_FILE + '1', os.O_CREAT | os.O_WRONLY, 0o0000))
    chromaterm.read_file(TEMP_FILE + '1')
    os.remove(TEMP_FILE + '1')

    assert msg in capsys.readouterr().err


def test_read_file_non_existent(capsys):
    """Read a non-existent file."""
    msg = 'Configuration file ' + TEMP_FILE + '2' + ' not found\n'
    chromaterm.read_file(TEMP_FILE + '2')
    assert msg in capsys.readouterr().err


def test_rgb_to_8bit():
    """20 random known-good translations."""
    assert chromaterm.config.rgb_to_8bit(13, 176, 1) == 40
    assert chromaterm.config.rgb_to_8bit(22, 32, 13) == 16
    assert chromaterm.config.rgb_to_8bit(29, 233, 205) == 50
    assert chromaterm.config.rgb_to_8bit(43, 48, 138) == 61
    assert chromaterm.config.rgb_to_8bit(44, 4, 5) == 52
    assert chromaterm.config.rgb_to_8bit(45, 245, 37) == 82
    assert chromaterm.config.rgb_to_8bit(75, 75, 194) == 62
    assert chromaterm.config.rgb_to_8bit(88, 121, 30) == 100
    assert chromaterm.config.rgb_to_8bit(119, 223, 223) == 123
    assert chromaterm.config.rgb_to_8bit(139, 87, 30) == 136
    assert chromaterm.config.rgb_to_8bit(146, 83, 47) == 131
    assert chromaterm.config.rgb_to_8bit(149, 67, 58) == 131
    assert chromaterm.config.rgb_to_8bit(149, 230, 209) == 158
    assert chromaterm.config.rgb_to_8bit(151, 153, 27) == 142
    assert chromaterm.config.rgb_to_8bit(163, 25, 80) == 125
    assert chromaterm.config.rgb_to_8bit(165, 186, 53) == 149
    assert chromaterm.config.rgb_to_8bit(171, 186, 6) == 184
    assert chromaterm.config.rgb_to_8bit(178, 249, 57) == 191
    assert chromaterm.config.rgb_to_8bit(229, 112, 100) == 210
    assert chromaterm.config.rgb_to_8bit(246, 240, 108) == 228
