#!/usr/bin/env python3
"""Tests for the main program."""

# pylint: disable=too-many-lines
## You can never have too many tests, and I don't want to defy logic by moving
## code to a different file, like test_highlight.py when it lives in __init__.py

import os
import re
import signal
import socket
import subprocess
import time
from threading import Thread

import chromaterm

TEMP_FILE = '.test_chromaterm.yml'
TEMP_SOCKET = '.test_chromaterm.socket'


def test_decode_sgr_bg():
    """Background colors and reset are being detected."""
    assert chromaterm.decode_sgr('\033[48;5;123m')[0]['type'] == 'bg'
    assert chromaterm.decode_sgr('\033[48;2;1;1;1m')[0]['type'] == 'bg'
    assert chromaterm.decode_sgr('\033[49m')[0]['type'] == 'bg'


def test_decode_sgr_fg():
    """Foreground colors and reset are being detected."""
    assert chromaterm.decode_sgr('\033[38;5;123m')[0]['type'] == 'fg'
    assert chromaterm.decode_sgr('\033[38;2;1;1;1m')[0]['type'] == 'fg'
    assert chromaterm.decode_sgr('\033[39m')[0]['type'] == 'fg'


def test_decode_sgr_styles_blink():
    """Blink and its reset are being detected."""
    assert chromaterm.decode_sgr('\033[5m')[0]['type'] == 'blink'
    assert chromaterm.decode_sgr('\033[25m')[0]['type'] == 'blink'


def test_decode_sgr_styles_bold():
    """Bold and its reset are being detected."""
    assert chromaterm.decode_sgr('\033[1m')[0]['type'] == 'bold'
    assert chromaterm.decode_sgr('\033[21m')[0]['type'] == 'bold'


def test_decode_sgr_styles_italic():
    """Italic and its reset are being detected."""
    assert chromaterm.decode_sgr('\033[3m')[0]['type'] == 'italic'
    assert chromaterm.decode_sgr('\033[23m')[0]['type'] == 'italic'


def test_decode_sgr_styles_strike():
    """Strike and its reset are being detected."""
    assert chromaterm.decode_sgr('\033[9m')[0]['type'] == 'strike'
    assert chromaterm.decode_sgr('\033[29m')[0]['type'] == 'strike'


def test_decode_sgr_styles_underline():
    """Underline and its reset are being detected."""
    assert chromaterm.decode_sgr('\033[4m')[0]['type'] == 'underline'
    assert chromaterm.decode_sgr('\033[24m')[0]['type'] == 'underline'


def test_decode_sgr_complete_reset():
    """Complete reset detection."""
    assert chromaterm.decode_sgr('\033[00m')[0]['type'] == 'complete_reset'
    assert chromaterm.decode_sgr('\033[0m')[0]['type'] == 'complete_reset'
    assert chromaterm.decode_sgr('\033[m')[0]['type'] == 'complete_reset'


def test_decode_sgr_malformed():
    """A malformed color."""
    assert chromaterm.decode_sgr('\033[38;5m')[0]['type'] is None


def test_decode_sgr_split_compound():
    """Split the a compound SGR into discrete SGR's."""
    colors = chromaterm.decode_sgr('\033[1;33;40m')
    codes = ['\033[1m', '\033[33m', '\033[40m']
    types = ['bold', 'fg', 'bg']

    for color, code, name in zip(colors, codes, types):
        assert color['type'] == name
        assert repr(color['code']) == repr(code)


def test_eprint(capsys):
    """Print a message to stderr."""
    msg = 'Some random error message'
    chromaterm.eprint(msg)
    assert msg in capsys.readouterr().err


def test_get_color_code():
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
        assert chromaterm.get_color_code(color)[0]['code'] == '\033[' + code


def test_get_color_code_grayscale():
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
        assert chromaterm.get_color_code(color)[0]['code'] == '\033[' + code


def test_get_color_code_rgb():
    """RGB color-codes."""
    colors = ['b#010101', 'f#020202']
    codes = ['\033[48;2;1;1;1m', '\033[38;2;2;2;2m']

    for color, code in zip(colors, codes):
        assert chromaterm.get_color_code(color, rgb=True)[0]['code'] == code


def test_get_color_code_style():
    """Terminal styles."""
    colors = ['blink', 'BOLD', 'iTaLiC', 'strike', 'underline']
    codes = ['5m', '1m', '3m', '9m', '4m']

    for color, code in zip(colors, codes):
        assert chromaterm.get_color_code(color)[0]['code'] == '\033[' + code


def test_get_color_code_compound():
    """All sorts of color codes."""
    colors = 'bold b#0973d8 underline f#45f2d7'
    # Styles are always added last
    codes = ['\033[48;5;33m', '\033[38;5;87m', '\033[1m', '\033[4m']

    for color, code in zip(chromaterm.get_color_code(colors), codes):
        assert color['code'] == code


def test_get_color_code_excessive_colors():
    """Too many colors (more than 2)."""
    colors = 'b#010101 f#020202 f#020202'

    assert chromaterm.get_color_code(colors) is None


def test_get_color_code_duplicate_target():
    """Duplicate targets (e.g. two foreground colors)."""
    colors = ['f#020202 f#030303', 'bold bold']

    for color in colors:
        assert chromaterm.get_color_code(color) is None


def test_highlight_enscapsulated_same_type():
    """Two rules of the same target type (e.g. both foreground) and one rule
    encapsulating the other. Also tested in reverse order.
    x: --------------
    y:    ------"""
    config_data = '''rules:
    - description: first
      regex: Hello there, World
      color: f#aaafff
    - description: second
      regex: there
      color: f#fffaaa'''
    config = chromaterm.parse_config(config_data)

    data = 'Hello there, World'
    expected = [
        '\033[38;5;153m', 'Hello ', '\033[38;5;229m', 'there',
        '\033[38;5;153m', ', World', '\033[39m'
    ]

    assert repr(chromaterm.highlight(config, data)) == repr(''.join(expected))
    config['rules'] = list(reversed(config['rules']))
    assert repr(chromaterm.highlight(config, data)) == repr(''.join(expected))


def test_highlight_enscapsulated_different_type():
    """Two rules of a different target type (e.g. foreground and background)
    and one rule encapsulating the other. Also tested in reverse order.
    x: --------------
    y:    ------"""
    config_data = '''rules:
    - description: first
      regex: Hello there, World
      color: f#aaafff
    - description: second
      regex: there
      color: b#fffaaa'''
    config = chromaterm.parse_config(config_data)

    data = 'Hello there, World'
    expected = [
        '\033[38;5;153m', 'Hello ', '\033[48;5;229m', 'there', '\033[49m',
        ', World', '\033[39m'
    ]

    assert repr(chromaterm.highlight(config, data)) == repr(''.join(expected))
    config['rules'] = list(reversed(config['rules']))
    assert repr(chromaterm.highlight(config, data)) == repr(''.join(expected))


def test_highlight_partial_overlap_same_type():
    """Two rules of the same target type (e.g. both foreground) and one
    overlapping the other. Also tested in reverse order.
    x: ------
    y:   ------"""
    config_data = '''rules:
    - description: first
      regex: Hello there
      color: f#aaafff
    - description: second
      regex: there, World
      color: f#fffaaa'''
    config = chromaterm.parse_config(config_data)

    data = 'Hello there, World'
    expected = [
        '\033[38;5;153m', 'Hello ', '\033[38;5;229m', 'there',
        '\033[38;5;229m', ', World', '\033[39m'
    ]

    assert repr(chromaterm.highlight(config, data)) == repr(''.join(expected))
    config['rules'] = list(reversed(config['rules']))
    assert repr(chromaterm.highlight(config, data)) == repr(''.join(expected))


def test_highlight_partial_overlap_different_type():
    """Two rules of a different target type (e.g. foreground and background)
    and one overlapping the other. Also tested in reverse order.
    x: ------
    y:   ------"""
    config_data = '''rules:
    - description: first
      regex: Hello there
      color: b#aaafff
    - description: second
      regex: there, World
      color: f#fffaaa'''
    config = chromaterm.parse_config(config_data)

    data = 'Hello there, World'
    expected = [
        '\033[48;5;153m', 'Hello ', '\033[38;5;229m', 'there', '\033[49m',
        ', World', '\033[39m'
    ]

    assert repr(chromaterm.highlight(config, data)) == repr(''.join(expected))
    config['rules'] = list(reversed(config['rules']))
    assert repr(chromaterm.highlight(config, data)) == repr(''.join(expected))


def test_highlight_full_overlap_same_type():
    """Two rules of the same target type (e.g. both foreground) fully overlapping
    each other. Both are applied.
    x: --------------
    y: --------------"""
    config_data = '''rules:
    - description: first
      regex: Hello there, World
      color: f#aaafff
    - description: second
      regex: Hello there, World
      color: f#fffaaa'''
    config = chromaterm.parse_config(config_data)

    data = 'Hello there, World'
    expected = [
        '\033[38;5;153m', '\033[38;5;229m', 'Hello there, World',
        '\033[38;5;153m', '\033[39m'
    ]

    assert repr(chromaterm.highlight(config, data)) == repr(''.join(expected))


def test_highlight_full_overlap_different_type():
    """Two rules of a different target type (e.g. foreground and background)
    fully overlapping each other. Both are applied, both ends are kept.
    x: --------------
    y: --------------"""
    config_data = '''rules:
    - description: first
      regex: Hello there, World
      color: b#aaafff
    - description: second
      regex: Hello there, World
      color: f#fffaaa'''
    config = chromaterm.parse_config(config_data)

    data = 'Hello there, World'
    expected = [
        '\033[48;5;153m', '\033[38;5;229m', 'Hello there, World', '\033[39m',
        '\033[49m'
    ]

    assert repr(chromaterm.highlight(config, data)) == repr(''.join(expected))


def test_highlight_start_overlap_same_type():
    """Two rules of the same target type (e.g. both foreground) overlapping at
    the start. Both are applied. Also tested in reverse order. The last rule's
    color is applied closest to the match.
    x: -------
    y: --------------"""
    config_data = '''rules:
    - description: first
      regex: Hello there
      color: f#aaafff
    - description: second
      regex: Hello there, World
      color: f#fffaaa'''
    config = chromaterm.parse_config(config_data)

    data = 'Hello there, World'
    expected = [
        '\033[38;5;153m', '\033[38;5;229m', 'Hello there', '\033[38;5;229m',
        ', World', '\033[39m'
    ]

    assert repr(chromaterm.highlight(config, data)) == repr(''.join(expected))
    config['rules'] = list(reversed(config['rules']))
    expected[0], expected[1] = expected[1], expected[0]  # Flip start color
    assert repr(chromaterm.highlight(config, data)) == repr(''.join(expected))


def test_highlight_start_overlap_different_type():
    """Two rules of a different target type (e.g. foreground and background)
    overlapping at the start. Both are applied. Also tested in reverse order.
    The last rule's color is applied closest to the match.
    x: -------
    y: --------------"""
    config_data = '''rules:
    - description: first
      regex: Hello there
      color: b#aaafff
    - description: second
      regex: Hello there, World
      color: f#fffaaa'''
    config = chromaterm.parse_config(config_data)

    data = 'Hello there, World'
    expected = [
        '\033[48;5;153m', '\033[38;5;229m', 'Hello there', '\033[49m',
        ', World', '\033[39m'
    ]

    assert repr(chromaterm.highlight(config, data)) == repr(''.join(expected))
    config['rules'] = list(reversed(config['rules']))
    expected[0], expected[1] = expected[1], expected[0]  # Flip start color
    assert repr(chromaterm.highlight(config, data)) == repr(''.join(expected))


def test_highlight_end_overlap_same_type():
    """Two rules of the same target type (e.g. both foreground) overlapping at
    the end. Both are applied. Also tested in reverse order. Only the first
    rule's end color is applied.
    x:        -------
    y: --------------"""
    config_data = '''rules:
    - description: first
      regex: World
      color: f#aaafff
    - description: second
      regex: Hello there, World
      color: f#fffaaa'''
    config = chromaterm.parse_config(config_data)

    data = 'Hello there, World'
    expected = [
        '\033[38;5;229m', 'Hello there, ', '\033[38;5;153m', 'World',
        '\033[38;5;153m', '\033[39m'
    ]

    assert repr(chromaterm.highlight(config, data)) == repr(''.join(expected))
    config['rules'] = list(reversed(config['rules']))
    expected[4] = '\033[38;5;229m'  # Adjust mid-color shift
    assert repr(chromaterm.highlight(config, data)) == repr(''.join(expected))


def test_highlight_end_overlap_different_type():
    """Two rules of a different target type (e.g. foreground and background)
    overlapping at the end. Both are applied. Also tested in reverse order. Both
    rule's end colors are applied.
    x:        -------
    y: --------------"""
    config_data = '''rules:
    - description: first
      regex: World
      color: b#aaafff
    - description: second
      regex: Hello there, World
      color: f#fffaaa'''
    config = chromaterm.parse_config(config_data)

    data = 'Hello there, World'
    expected = [
        '\033[38;5;229m', 'Hello there, ', '\033[48;5;153m', 'World',
        '\033[39m', '\033[49m'
    ]

    assert repr(chromaterm.highlight(config, data)) == repr(''.join(expected))
    config['rules'] = list(reversed(config['rules']))
    expected[-1], expected[-2] = expected[-2], expected[-1]  # Flip end color
    assert repr(chromaterm.highlight(config, data)) == repr(''.join(expected))


def test_highlight_end_start_overlap_same_type():
    """Two rules of the same target type (e.g. both foreground) overlapping the
    end of one with the start of the other. Both are applied. Also tested in
    reverse order.
    x: -------
    y:        -------"""
    config_data = '''rules:
    - description: first
      regex: Hello Wo
      color: f#aaafff
    - description: second
      regex: rld
      color: f#fffaaa'''
    config = chromaterm.parse_config(config_data)

    data = 'Hello World'
    expected = [
        '\033[38;5;153m', 'Hello Wo', '\033[39m', '\033[38;5;229m', 'rld',
        '\033[39m'
    ]

    assert repr(chromaterm.highlight(config, data)) == repr(''.join(expected))
    config['rules'] = list(reversed(config['rules']))
    assert repr(chromaterm.highlight(config, data)) == repr(''.join(expected))


def test_highlight_end_start_overlap_different_type():
    """Two rules of a different target type (e.g. foreground and background)
    overlapping the end of one with the start of the other. Both are applied.
    Also tested in reverse order.
    x: -------
    y:        -------"""
    config_data = '''rules:
    - description: first
      regex: Hello Wo
      color: b#aaafff
    - description: second
      regex: rld
      color: f#fffaaa'''
    config = chromaterm.parse_config(config_data)

    data = 'Hello World'
    expected = [
        '\033[48;5;153m', 'Hello Wo', '\033[49m', '\033[38;5;229m', 'rld',
        '\033[39m'
    ]

    assert repr(chromaterm.highlight(config, data)) == repr(''.join(expected))
    config['rules'] = list(reversed(config['rules']))
    assert repr(chromaterm.highlight(config, data)) == repr(''.join(expected))


def test_highlight_existing_start_same_type():
    """Highlight with an existing, same-type, color at the start of the data."""
    config_data = '''rules:
    - description: first
      regex: Hello World
      color: f#aaafff'''
    config = chromaterm.parse_config(config_data)

    data = '\033[33mHello World'
    expected = ['\033[33m', '\033[38;5;153m', 'Hello World', '\033[33m']

    assert repr(chromaterm.highlight(config, data)) == repr(''.join(expected))


def test_highlight_existing_start_different_type():
    """Highlight with an existing, different-type, color at the start of the data."""
    config_data = '''rules:
    - description: first
      regex: Hello World
      color: b#aaafff'''
    config = chromaterm.parse_config(config_data)

    data = '\033[33mHello World'
    expected = ['\033[33m', '\033[48;5;153m', 'Hello World', '\033[49m']

    assert repr(chromaterm.highlight(config, data)) == repr(''.join(expected))


def test_highlight_existing_end_same_type():
    """Highlight with an existing, same-type, color at the end of the data."""
    config_data = '''rules:
    - description: first
      regex: Hello World
      color: f#aaafff'''
    config = chromaterm.parse_config(config_data)

    data = 'Hello World\033[33m'
    expected = ['\033[38;5;153m', 'Hello World', '\033[39m', '\033[33m']

    assert repr(chromaterm.highlight(config, data)) == repr(''.join(expected))


def test_highlight_existing_end_different_type():
    """Highlight with an existing, different-type, color at the end of the data."""
    config_data = '''rules:
    - description: first
      regex: Hello World
      color: b#aaafff'''
    config = chromaterm.parse_config(config_data)

    data = 'Hello World\033[33m'
    expected = ['\033[48;5;153m', 'Hello World', '\033[49m', '\033[33m']

    assert repr(chromaterm.highlight(config, data)) == repr(''.join(expected))


def test_highlight_existing_multiline():
    """Highlight with an existing color in the multi-line data."""
    config_data = '''rules:
    - description: first
      regex: Hello World
      color: f#aaafff'''
    config = chromaterm.parse_config(config_data)

    data = ['\033[33mHi there', 'Hello World']
    expected = [['\033[33m', 'Hi there'],
                ['\033[38;5;153m', 'Hello World', '\033[33m']]

    for line_data, line_expected in zip(data, expected):
        assert repr(chromaterm.highlight(config, line_data)) == repr(
            ''.join(line_expected))


def test_highlight_existing_orphaned():
    """Highlight with two existing colors in the data, the first of which was
    not closed."""
    config_data = '''rules:
    - description: first
      regex: World
      color: f#aaafff'''
    config = chromaterm.parse_config(config_data)

    data = '\033[33mHello \033[34mthere\033[m, World'
    expected = [
        '\033[33m', 'Hello ', '\033[34m', 'there', '\033[m', ', ',
        '\033[38;5;153m', 'World', '\033[m'
    ]

    assert repr(chromaterm.highlight(config, data)) == repr(''.join(expected))


def test_highlight_existing_complete_reset():
    """Highlight with an existing complete reset in the data. It should be set
    to our color."""
    config_data = '''rules:
    - description: first
      regex: Hello there, World
      color: f#aaafff'''
    config = chromaterm.parse_config(config_data)

    data = 'Hello\033[m there, World'
    expected = [
        '\033[38;5;153m', 'Hello', '\033[38;5;153m', ' there, World',
        '\033[39m'
    ]

    assert repr(chromaterm.highlight(config, data)) == repr(''.join(expected))


def test_highlight_existing_complete_reset_same_type():
    """Highlight with an existing color and complete reset in the data. The
    color is before the reset. The first rule's match covers both. The second
    rule's match covers the reset only. The rules are of the same type."""
    config_data = '''rules:
    - description: first
      regex: Hello there
      color: f#aaafff
    - description: first
      regex: there, World
      color: f#fffaaa'''
    config = chromaterm.parse_config(config_data)

    data = '\033[33mHello the\033[mre, World'
    expected = [
        '\033[33m', '\033[38;5;153m', 'Hello ', '\033[38;5;229m', 'the',
        '\033[38;5;153m', 're', '\033[38;5;229m', ', World', '\033[39m'
    ]

    assert repr(chromaterm.highlight(config, data)) == repr(''.join(expected))


def test_highlight_existing_complete_reset_different_type():
    """Highlight with an existing color and complete reset in the data. The
    color is before the reset. The first rule's match covers both. The second
    rule's match covers the reset only. The rules are of different types."""
    config_data = '''rules:
    - description: first
      regex: Hello there
      color: f#aaafff
    - description: first
      regex: there, World
      color: b#fffaaa'''
    config = chromaterm.parse_config(config_data)

    data = '\033[44mHello the\033[mre, World'
    expected = [
        '\033[44m', '\033[38;5;153m', 'Hello ', '\033[48;5;229m', 'the',
        '\033[38;5;153m', 're', '\033[39m', ', World', '\033[44m'
    ]

    assert repr(chromaterm.highlight(config, data)) == repr(''.join(expected))


def test_highlight_existing_compound():
    """Highlight with an existing compound color (multiple SGR's) in the data."""
    config_data = '''rules:
    - regex: World|me
      color: f#aaafff'''
    config = chromaterm.parse_config(config_data)

    data = '\033[1;38;5;123;38;2;1;1;1;4mHello World\033[35;5m It\'s \033[32;24mme'
    expected = [
        '\033[1m\033[38;5;123m\033[38;2;1;1;1m\033[4m', 'Hello ',
        '\033[38;5;153m', 'World', '\033[38;2;1;1;1m', '\033[35m\033[5m',
        ' It\'s ', '\033[32m\033[24m', '\033[38;5;153m', 'me', '\033[32m'
    ]

    assert repr(chromaterm.highlight(config, data)) == repr(''.join(expected))


def test_highlight_partial_overlap_existing_multiline():
    """Three partially-overlapping rules with an existing color over multiple
    lines. Also tested in reverse order.
    x: ------
    y:   ------
    z:     ------"""
    config_data = '''rules:
    - description: first
      regex: Hello World
      color: f#aaafff
    - description: second
      regex: World! It's
      color: f#fffaaa
    - description: third
      regex: It's me
      color: f#0973d8'''
    config = chromaterm.parse_config(config_data)

    data = ['\033[33mSup', 'Hello World! It\'s me']
    expected = [['\033[33m', 'Sup'],
                [
                    '\033[38;5;153m', 'Hello ', '\033[38;5;229m', 'World',
                    '\033[38;5;229m', '! ', '\033[38;5;33m', 'It\'s',
                    '\033[38;5;33m', ' me', '\033[33m'
                ]]

    for line_data, line_expected in zip(data, expected):
        assert repr(chromaterm.highlight(config, line_data)) == repr(
            ''.join(line_expected))

    # Reset color tracking and reverse the rule order
    config = chromaterm.parse_config(config_data)
    config['rules'] = list(reversed(config['rules']))

    for line_data, line_expected in zip(data, expected):
        assert repr(chromaterm.highlight(config, line_data)) == repr(
            ''.join(line_expected))


def test_highlight_optional_multi_group():
    """Multiple group-specific colors."""
    config_data = '''rules:
    - regex: Hello (World)! It's (me)
      color:
        0: bold
        1: f#f7e08b
        2: f#aaafff'''
    config = chromaterm.parse_config(config_data)

    data = 'Hello World! It\'s me'
    expected = [
        '\033[1m', 'Hello ', '\033[38;5;229m', 'World', '\033[39m', '! It\'s ',
        '\033[38;5;153m', 'me', '\033[39m', '\033[21m'
    ]

    assert repr(chromaterm.highlight(config, data)) == repr(''.join(expected))


def test_highlight_optional_group_not_matched():
    """RegEx matched but the group specified is optional and did not match."""
    config_data = '''rules:
    - regex: Hello (World)?
      color:
        1: f#f7e08b'''
    config = chromaterm.parse_config(config_data)

    data = 'Hello there! Hello World'
    expected = ['Hello there! Hello ', '\033[38;5;229m', 'World', '\033[39m']

    assert repr(chromaterm.highlight(config, data)) == repr(''.join(expected))


def test_highlight_update_type_reset():
    """Feed a string to chromaterm that updates the config with new resets to
    confirm they are not colliding."""
    config_data = '''rules:
    - regex: Hello
      color: f#f7e08b'''
    config = chromaterm.parse_config(config_data)

    resets = {
        'fg': '\033[33m',
        'bg': '\033[45m',
        'blink': '\033[5m',
        'bold': '\033[1m',
        'italic': '\033[3m',
        'strike': '\033[9m',
        'underline': '\033[4m'
    }

    # Feed colors
    for data in resets.values():
        chromaterm.highlight(config, data)

    # Check the resets
    for name in resets:
        assert repr(config['resets'][name]) == repr(resets[name])


def test_highlight_complete_reset_defaulting_type_resets():
    """A complete SGR resets sets the type resets back to their defaults as
    defined in RESET_TYPES[x]['default']."""
    config_data = '''rules:
    - description: first
      regex: there
      color: f#aaafff'''
    config = chromaterm.parse_config(config_data)
    default = chromaterm.RESET_TYPES['fg']['default']

    # Feed a color to update the type's reset
    assert chromaterm.highlight(config, '\033[33mHello there, \033[43mWorld')
    assert repr(config['resets']['fg']) == repr('\033[33m')

    # Feed the complete reset to send back to default
    assert chromaterm.highlight(config, '\033[mHello there, World')
    assert repr(config['resets']['fg']) == repr(default)


def test_parse_config_simple():
    """Parse a config file with a simple rule."""
    config_data = '''rules:
    - description: simple
      regex: hello world
      color: f#fffaaa'''

    assert chromaterm.parse_config(config_data)['rules']


def test_parse_config_group():
    """Parse a config file with a group-specific rule."""
    config_data = '''rules:
    - regex: hello (world)! It's (me).
      color:
        1: b#fffaaa
        2: f#123123'''

    assert chromaterm.parse_config(config_data)['rules']


def test_parse_config_group_legacy():
    """Parse a config file with a legacy group-specific rule."""
    config_data = '''rules:
    - regex: hello (world)
      color: b#fffaaa
      group: 1'''

    assert chromaterm.parse_config(config_data)['rules']


def test_parse_config_multiple_colors():
    """Parse a config file with a multi-color rule."""
    config_data = '''rules:
    - description: group
      regex: hello (world)
      color: b#fffaaa f#aaafff'''

    assert chromaterm.parse_config(config_data)['rules']


def test_parse_config_rule_format_error(capsys):
    """Parse a config file with a syntax problem."""
    config_data = '''rules:
    - description: simple'''
    chromaterm.parse_config(config_data)

    assert 'Rule error on' in capsys.readouterr().err


def test_parse_config_yaml_format_error(capsys):
    """Parse an incorrect YAML file."""
    chromaterm.parse_config('-x-\nhi:')
    assert 'Parse error:' in capsys.readouterr().err


def test_parse_rule_regex_missing():
    """Parse a rule without a `regex` key."""
    msg = 'regex not found'
    rule = {'color': 'b#fffaaa'}
    assert chromaterm.parse_rule(rule) == msg


def test_parse_rule_regex_type_error():
    """Parse a rule with an incorrect `regex` value type."""
    msg = 'regex not a string or integer'
    rule = {'regex': ['hi'], 'color': 'b#fffaaa'}
    assert chromaterm.parse_rule(rule) == msg


def test_parse_rule_regex_invalid():
    """Parse a rule with an invalid `regex`."""
    rule = {'regex': '+', 'color': 'b#fffaaa'}
    assert 're.error: ' in chromaterm.parse_rule(rule)


def test_parse_rule_color_missing():
    """Parse a rule without a `color` key."""
    msg = 'color not found'
    rule = {'regex': 'x(y)z'}
    assert chromaterm.parse_rule(rule) == msg


def test_parse_rule_color_type_error():
    """Parse a rule with an incorrect `color` value type."""
    msg = 'color not a string or dictionary'
    rule = {'regex': 'x(y)z', 'color': ['hi']}
    assert chromaterm.parse_rule(rule) == msg


def test_parse_rule_color_format_error():
    """Parse a rule with an incorrect `color` format."""
    msg_re = 'color ".+" not in the correct format'

    rule = {'regex': 'x(y)z', 'color': 'b#xyzxyz'}
    assert re.search(msg_re, chromaterm.parse_rule(rule))

    rule = {'regex': 'x(y)z', 'color': 'x#fffaaa'}
    assert re.search(msg_re, chromaterm.parse_rule(rule))

    rule = {'regex': 'x(y)z', 'color': 'b@fffaaa'}
    assert re.search(msg_re, chromaterm.parse_rule(rule))

    rule = {'regex': 'x(y)z', 'color': 'b#fffaaa-f#fffaaa'}
    assert re.search(msg_re, chromaterm.parse_rule(rule))


def test_parse_rule_group_type_error():
    """Parse a rule with an incorrect `group` value type."""
    msg_re = 'group .+ not an integer'
    rule = {'regex': 'x(y)z', 'color': {'1': 'b#fffaaa'}}
    assert re.search(msg_re, chromaterm.parse_rule(rule))


def test_parse_rule_group_out_of_bounds():
    """Parse a rule with `group` number not in the regex."""
    msg_re = 'group .+ not in the regex'
    rule = {'regex': 'x(y)z', 'color': {2: 'b#fffaaa'}}
    assert re.search(msg_re, chromaterm.parse_rule(rule))


def test_process_buffer_empty(capsys):
    """Output processing of empty input."""
    config = chromaterm.get_default_config()
    chromaterm.process_buffer(config, '', False)
    assert capsys.readouterr().out == ''


def test_process_buffer_more(capsys):
    """Output processing of empty input."""
    config = chromaterm.get_default_config()
    chromaterm.process_buffer(config, '', False)
    assert capsys.readouterr().out == ''


def test_process_buffer_multiline(capsys):
    """Output processing with multiple lines of input."""
    config = chromaterm.get_default_config()
    rule = {'regex': 'hello world', 'color': 'b#fffaaa'}

    config['rules'].append(chromaterm.parse_rule(rule))
    data = '\ntest hello world test\n'
    success = r'^test \033\[[34]8;5;[0-9]{1,3}mhello world\033\[49m test$'

    chromaterm.process_buffer(config, data * 2, False)
    captured = capsys.readouterr()

    for line in [x for x in captured.out.splitlines() if x]:
        assert re.search(success, line)


def test_process_buffer_rule_simple(capsys):
    """Output processing with a simple rule."""
    config = chromaterm.get_default_config()
    rule = {'regex': 'hello world', 'color': 'b#fffaaa'}

    config['rules'].append(chromaterm.parse_rule(rule))
    data = 'test hello world test'
    success = r'^test \033\[48;5;[0-9]{1,3}mhello world\033\[49m test$'

    chromaterm.process_buffer(config, data, False)
    captured = capsys.readouterr()

    assert re.search(success, captured.out)


def test_process_buffer_rule_group(capsys):
    """Output processing with a group-specific rule."""
    config = chromaterm.get_default_config()
    rule = {'regex': 'hello (world)', 'color': {1: 'b#fffaaa'}}

    config['rules'].append(chromaterm.parse_rule(rule))
    data = 'test hello world test'
    success = r'^test hello \033\[48;5;[0-9]{1,3}mworld\033\[49m test$'

    chromaterm.process_buffer(config, data, False)
    captured = capsys.readouterr()

    assert re.search(success, captured.out)


def test_process_buffer_rule_multiple_colors(capsys):
    """Output processing with a multi-color rule."""
    config = chromaterm.get_default_config()
    rule = {'regex': 'hello', 'color': 'b#fffaaa f#aaafff'}

    config['rules'].append(chromaterm.parse_rule(rule))
    data = 'hello world'
    success = r'^(\033\[[34]8;5;[0-9]{1,3}m){2}hello(\033\[[34]9m){2} world$'

    chromaterm.process_buffer(config, data, False)
    captured = capsys.readouterr()

    assert re.search(success, captured.out)


def test_process_buffer_movement_sequences(capsys):
    """Input data includes movement sequences; the data must be split on it,
    thus not matching the rule's regex."""
    config_data = '''rules:
    - description: first
      regex: Hello.+World
      color: f#aaafff'''
    config = chromaterm.parse_config(config_data)

    data_fmt = 'Hello{} World'
    movements = [
        'A', '1A', '123A', 'B', 'C', 'D', 'E', 'F', 'G', 'J', 'K', 'S', 'T',
        'H', '1;H', ';1H', '1;1H', 'f', '?1049h', '?1049l'
    ]
    splits = ['\r', '\n', '\r\n', '\v', '\f']

    for prefix, items in zip(['\033[', ''], [movements, splits]):
        for item in items:
            data = data_fmt.format(prefix + item)
            chromaterm.process_buffer(config, data, False)
            assert repr(data) == repr(capsys.readouterr().out)


def test_process_buffer_false_movement_sequences(capsys):
    """Input data includes sequences that appear to be movements, but are not;
    the data must not be split and thus the rule's regex is matched."""
    config_data = '''rules:
    - description: first
      regex: Hello.+World
      color: f#aaafff'''
    config = chromaterm.parse_config(config_data)

    data_fmt = 'Hello{} World'
    movements = [
        'A', '1A', '123A', 'B', 'C', 'D', 'E', 'F', 'G', 'J', 'K', 'S', 'T',
        'H', '1;H', ';1H', '1;1H', 'f', '?1049h', '?1049l'
    ]
    splits = ['r', 'n', 'rn', 'v', 'f']

    for prefix, items in zip(['[', ''], [movements, splits]):
        for item in items:
            data = data_fmt.format(prefix + item)
            chromaterm.process_buffer(config, data, False)
            assert repr(data) != repr(capsys.readouterr().out)


def test_read_file():
    """Read the default configuration."""
    assert chromaterm.read_file('$HOME/.chromaterm.yml') is not None


def test_read_file_non_existent(capsys):
    """Read a non-existent file."""
    msg = 'Configuration file ' + TEMP_FILE + '1' + ' not found\n'
    chromaterm.read_file(TEMP_FILE + '1')
    assert msg in capsys.readouterr().err


def test_read_file_no_permission(capsys):
    """Create a file with no permissions and attempt to read it. Delete the file
    once done with it."""
    msg = 'Cannot read configuration file ' + TEMP_FILE + '2' + ' (permission)\n'

    os.close(os.open(TEMP_FILE + '2', os.O_CREAT | os.O_WRONLY, 0o0000))
    chromaterm.read_file(TEMP_FILE + '2')
    os.remove(TEMP_FILE + '2')

    assert msg in capsys.readouterr().err


def test_read_ready_input(monkeypatch):
    """Immediate ready when there is input buffered."""
    try:
        s_sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        c_sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        s_sock.bind(TEMP_SOCKET + '1')
        s_sock.listen(2)
        c_sock.connect(TEMP_SOCKET + '1')
        s_conn, _ = s_sock.accept()

        monkeypatch.setattr('sys.stdin', c_sock)

        s_conn.sendall(b'Hello world')
        assert chromaterm.read_ready()
    finally:
        s_conn.close()
        c_sock.close()
        s_sock.close()
        os.remove(TEMP_SOCKET + '1')


def test_read_ready_timeout_empty(monkeypatch):
    """Wait with no input."""
    try:
        s_sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        s_sock.bind(TEMP_SOCKET + '2')
        s_sock.listen(2)
        c_sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        c_sock.connect(TEMP_SOCKET + '2')
        s_conn, _ = s_sock.accept()

        monkeypatch.setattr('sys.stdin', c_sock)

        before = time.time()
        assert not chromaterm.read_ready(0.5)

        after = time.time()
        assert after - before >= 0.5
    finally:
        s_conn.close()
        c_sock.close()
        s_sock.close()
        os.remove(TEMP_SOCKET + '2')


def test_read_ready_timeout_input(monkeypatch):
    """Immediate ready with timeout when there is input buffered."""
    try:
        s_sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        s_sock.bind(TEMP_SOCKET + '3')
        s_sock.listen(2)
        c_sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        c_sock.connect(TEMP_SOCKET + '3')
        s_conn, _ = s_sock.accept()

        monkeypatch.setattr('sys.stdin', c_sock)

        s_conn.sendall(b'Hello world')
        before = time.time()
        assert chromaterm.read_ready(0.5)

        after = time.time()
        assert after - before < 0.5
    finally:
        s_conn.close()
        c_sock.close()
        s_sock.close()
        os.remove(TEMP_SOCKET + '3')


def test_rgb_to_8bit():
    """20 random known-good translations."""
    assert chromaterm.rgb_to_8bit(13, 176, 1) == 40
    assert chromaterm.rgb_to_8bit(22, 32, 13) == 16
    assert chromaterm.rgb_to_8bit(29, 233, 205) == 50
    assert chromaterm.rgb_to_8bit(43, 48, 138) == 61
    assert chromaterm.rgb_to_8bit(44, 4, 5) == 52
    assert chromaterm.rgb_to_8bit(45, 245, 37) == 82
    assert chromaterm.rgb_to_8bit(75, 75, 194) == 62
    assert chromaterm.rgb_to_8bit(88, 121, 30) == 100
    assert chromaterm.rgb_to_8bit(119, 223, 223) == 123
    assert chromaterm.rgb_to_8bit(139, 87, 30) == 136
    assert chromaterm.rgb_to_8bit(146, 83, 47) == 131
    assert chromaterm.rgb_to_8bit(149, 67, 58) == 131
    assert chromaterm.rgb_to_8bit(149, 230, 209) == 158
    assert chromaterm.rgb_to_8bit(151, 153, 27) == 142
    assert chromaterm.rgb_to_8bit(163, 25, 80) == 125
    assert chromaterm.rgb_to_8bit(165, 186, 53) == 149
    assert chromaterm.rgb_to_8bit(171, 186, 6) == 184
    assert chromaterm.rgb_to_8bit(178, 249, 57) == 191
    assert chromaterm.rgb_to_8bit(229, 112, 100) == 210
    assert chromaterm.rgb_to_8bit(246, 240, 108) == 228


def test_main(capsys, monkeypatch):
    """General stdin processing."""
    try:
        config = chromaterm.config_init([])
        main_thread = Thread(target=chromaterm.main, args=(config, 1))

        s_sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        s_sock.bind(TEMP_SOCKET + '4')
        s_sock.listen(2)
        c_sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        c_sock.connect(TEMP_SOCKET + '4')
        s_conn, _ = s_sock.accept()

        monkeypatch.setattr('sys.stdin', c_sock)

        main_thread.start()
        time.sleep(0.2)  # Any start-up delay
        assert main_thread.is_alive()

        s_conn.sendall(b'Hello world\n')
        time.sleep(0.1)  # Any processing delay
        assert capsys.readouterr().out == 'Hello world\n'

        s_conn.sendall(b'Hey there')
        time.sleep(0.1 + chromaterm.WAIT_FOR_SPLIT)  # Include split wait
        assert capsys.readouterr().out == 'Hey there'

        s_conn.sendall(b'x' * (chromaterm.READ_SIZE + 1))
        time.sleep(0.1 + chromaterm.WAIT_FOR_SPLIT)  # Include split wait
        assert capsys.readouterr().out == 'x' * (chromaterm.READ_SIZE + 1)
    finally:
        s_conn.close()
        c_sock.close()
        s_sock.close()
        os.remove(TEMP_SOCKET + '4')
        main_thread.join()


def test_main_reload_config(capsys, monkeypatch):
    """Reload the configuration while the program is running."""
    try:
        with open(TEMP_FILE + '3', 'w') as file:
            file.write('''rules:
            - regex: Hello
              color: f#123123
            - regex: world
              color: b#321321''')

        config = chromaterm.config_init(['--config', TEMP_FILE + '3'])
        main_thread = Thread(target=chromaterm.main, args=(config, 1))

        s_sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        s_sock.bind(TEMP_SOCKET + '5')
        s_sock.listen(2)
        c_sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        c_sock.connect(TEMP_SOCKET + '5')
        s_conn, _ = s_sock.accept()

        monkeypatch.setattr('sys.stdin', c_sock)

        main_thread.start()
        time.sleep(0.2)  # Any start-up delay
        assert main_thread.is_alive()

        s_conn.sendall(b'Hello world')
        expected = [
            '\033[38;5;22m', 'Hello', '\033[39m', ' ', '\033[48;5;52m',
            'world', '\033[49m'
        ]
        time.sleep(0.1)  # Any processing delay
        assert repr(capsys.readouterr().out) == repr(''.join(expected))

        # Create file without the 'world' rule
        os.remove(TEMP_FILE + '3')
        with open(TEMP_FILE + '3', 'w') as file:
            file.write('''rules:
            - regex: Hello
              color: f#123123''')

        # Reload config
        os.kill(os.getpid(), signal.SIGUSR1)

        s_conn.sendall(b'Hello world')
        expected = ['\033[38;5;22m', 'Hello', '\033[39m', ' world']
        time.sleep(0.1)  # Any processing delay
        assert repr(capsys.readouterr().out) == repr(''.join(expected))
    finally:
        s_conn.close()
        c_sock.close()
        s_sock.close()
        os.remove(TEMP_FILE + '3')
        os.remove(TEMP_SOCKET + '5')
        main_thread.join()


def test_main_reload_processes():
    """Reload all other CT processes."""
    for _ in range(3):  # Spawn processes
        subprocess.Popen("sleep 1 | ./ct", shell=True)

    result = subprocess.run(['./ct', '--reload'], stderr=subprocess.PIPE)
    assert result.stderr == b'Processes reloaded: 3\n'


def test_main_buffer_close_time():
    """Confirm that the program exists as soon as stdin closes."""
    before = time.time()
    subprocess.run("echo hi | ./ct", shell=True)
    after = time.time()

    assert after - before < 1
