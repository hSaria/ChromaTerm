#!/usr/bin/env python3
"""Tests for the main program."""

# You can never have too many tests, and I don't want to defy logic by moving
# code to a different file, like test_highlight.py when it lives in __init__.py.
# pylint: disable=too-many-lines

import itertools
import os
import re
import signal
import subprocess
import time
from threading import Thread

import chromaterm

TEMP_FILE = '.test_chromaterm.yml'

TTY_TEST_CODE = """import os, sys
t_stdin = os.isatty(sys.stdin.fileno())
t_stdout = os.isatty(sys.stdout.fileno())
print('stdin={}, stdout={}'.format(t_stdin, t_stdout))"""
TTY_TEST_PROGRAM = 'python3 -c "{}"'.format('; '.join(
    TTY_TEST_CODE.splitlines()))


def test_decode_sgr_bg():
    """Background colors and reset are being detected."""
    for code in ['\x1b[48;5;123m', '\x1b[48;2;1;1;1m', '\x1b[49m']:
        colors = chromaterm.decode_sgr(code)
        assert len(colors) == 1

        for color in colors:
            assert color['type'] == 'bg'
            assert color['code'] == code


def test_decode_sgr_fg():
    """Foreground colors and reset are being detected."""
    for code in ['\x1b[38;5;123m', '\x1b[38;2;1;1;1m', '\x1b[39m']:
        colors = chromaterm.decode_sgr(code)
        assert len(colors) == 1

        for color in colors:
            assert color['type'] == 'fg'
            assert color['code'] == code


def test_decode_sgr_styles_blink():
    """Blink and its reset are being detected."""
    for code in ['\x1b[5m', '\x1b[25m']:
        colors = chromaterm.decode_sgr(code)
        assert len(colors) == 1

        for color in colors:
            assert color['type'] == 'blink'
            assert color['code'] == code


def test_decode_sgr_styles_bold():
    """Bold and its reset are being detected."""
    for code in ['\x1b[1m', '\x1b[2m', '\x1b[22m']:
        colors = chromaterm.decode_sgr(code)
        assert len(colors) == 1

        for color in colors:
            assert color['type'] == 'bold'
            assert color['code'] == code


def test_decode_sgr_styles_italic():
    """Italic and its reset are being detected."""
    for code in ['\x1b[3m', '\x1b[23m']:
        colors = chromaterm.decode_sgr(code)
        assert len(colors) == 1

        for color in colors:
            assert color['type'] == 'italic'
            assert color['code'] == code


def test_decode_sgr_styles_strike():
    """Strike and its reset are being detected."""
    for code in ['\x1b[9m', '\x1b[29m']:
        colors = chromaterm.decode_sgr(code)
        assert len(colors) == 1

        for color in colors:
            assert color['type'] == 'strike'
            assert color['code'] == code


def test_decode_sgr_styles_underline():
    """Underline and its reset are being detected."""
    for code in ['\x1b[4m', '\x1b[24m']:
        colors = chromaterm.decode_sgr(code)
        assert len(colors) == 1

        for color in colors:
            assert color['type'] == 'underline'
            assert color['code'] == code


def test_decode_sgr_complete_reset():
    """Complete reset detection."""
    for code in ['\x1b[00m', '\x1b[0m', '\x1b[m']:
        colors = chromaterm.decode_sgr(code)
        assert len(colors) == 1

        for color in colors:
            assert color['type'] == 'complete_reset'
            assert color['code'] == code


def test_decode_sgr_malformed():
    """Malformed colors."""
    for code in ['\x1b[38;5m', '\x1b[38;2;1;1m', '\x1b[38;5;123;38;2;1;1m']:
        colors = chromaterm.decode_sgr(code)
        assert len(colors) == 1

        for color in colors:
            assert color['type'] is None
            assert color['code'] == code


def test_decode_sgr_split_compound():
    """Split the a compound SGR into discrete SGR's."""
    colors = chromaterm.decode_sgr('\x1b[1;33;40m')
    codes = ['\x1b[1m', '\x1b[33m', '\x1b[40m']
    types = ['bold', 'fg', 'bg']

    for color, code, name in zip(colors, codes, types):
        assert color['type'] == name
        assert repr(color['code']) == repr(code)


def test_eprint(capsys):
    """Print a message to stderr."""
    msg = 'Some random error message'
    chromaterm.eprint(msg)
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
        assert chromaterm.get_color_codes(color)[0]['code'] == '\x1b[' + code


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
        assert chromaterm.get_color_codes(color)[0]['code'] == '\x1b[' + code


def test_get_color_codes_rgb():
    """RGB color-codes."""
    colors = ['b#010101', 'f#020202']
    codes = ['\x1b[48;2;1;1;1m', '\x1b[38;2;2;2;2m']

    for color, code in zip(colors, codes):
        assert chromaterm.get_color_codes(color, rgb=True)[0]['code'] == code


def test_get_color_codes_style():
    """Terminal styles."""
    colors = ['blink', 'BOLD', 'iTaLiC', 'strike', 'underline']
    codes = ['5m', '1m', '3m', '9m', '4m']

    for color, code in zip(colors, codes):
        assert chromaterm.get_color_codes(color)[0]['code'] == '\x1b[' + code


def test_get_color_codes_compound():
    """All sorts of color codes."""
    colors = 'bold b#0973d8 underline f#45f2d7'
    # Styles are always added last
    codes = ['\x1b[48;5;33m', '\x1b[38;5;87m', '\x1b[1m', '\x1b[4m']

    for color, code in zip(chromaterm.get_color_codes(colors), codes):
        assert color['code'] == code


def test_get_color_codes_mixed_case():
    """Color is mixed case."""
    colors = 'b#abcABC bOlD'

    assert len(chromaterm.get_color_codes(colors)) == 2


def test_get_color_codes_excessive_colors():
    """Too many colors (more than 2)."""
    colors = 'b#010101 f#020202 f#020202'

    assert chromaterm.get_color_codes(colors) is None


def test_get_color_codes_duplicate_target():
    """Duplicate targets (e.g. two foreground colors)."""
    colors = ['f#020202 f#030303', 'bold bold']

    for color in colors:
        assert chromaterm.get_color_codes(color) is None


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
        '\x1b[38;5;153m', 'Hello ', '\x1b[38;5;229m', 'there',
        '\x1b[38;5;153m', ', World', '\x1b[39m'
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
        '\x1b[38;5;153m', 'Hello ', '\x1b[48;5;229m', 'there', '\x1b[49m',
        ', World', '\x1b[39m'
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
        '\x1b[38;5;153m', 'Hello ', '\x1b[38;5;229m', 'there',
        '\x1b[38;5;229m', ', World', '\x1b[39m'
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
        '\x1b[48;5;153m', 'Hello ', '\x1b[38;5;229m', 'there', '\x1b[49m',
        ', World', '\x1b[39m'
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
        '\x1b[38;5;153m', '\x1b[38;5;229m', 'Hello there, World',
        '\x1b[38;5;153m', '\x1b[39m'
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
        '\x1b[48;5;153m', '\x1b[38;5;229m', 'Hello there, World', '\x1b[39m',
        '\x1b[49m'
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
        '\x1b[38;5;153m', '\x1b[38;5;229m', 'Hello there', '\x1b[38;5;229m',
        ', World', '\x1b[39m'
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
        '\x1b[48;5;153m', '\x1b[38;5;229m', 'Hello there', '\x1b[49m',
        ', World', '\x1b[39m'
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
        '\x1b[38;5;229m', 'Hello there, ', '\x1b[38;5;153m', 'World',
        '\x1b[38;5;153m', '\x1b[39m'
    ]

    assert repr(chromaterm.highlight(config, data)) == repr(''.join(expected))
    config['rules'] = list(reversed(config['rules']))
    expected[4] = '\x1b[38;5;229m'  # Adjust mid-color shift
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
        '\x1b[38;5;229m', 'Hello there, ', '\x1b[48;5;153m', 'World',
        '\x1b[39m', '\x1b[49m'
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
        '\x1b[38;5;153m', 'Hello Wo', '\x1b[39m', '\x1b[38;5;229m', 'rld',
        '\x1b[39m'
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
        '\x1b[48;5;153m', 'Hello Wo', '\x1b[49m', '\x1b[38;5;229m', 'rld',
        '\x1b[39m'
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

    data = '\x1b[33mHello World'
    expected = ['\x1b[33m', '\x1b[38;5;153m', 'Hello World', '\x1b[33m']

    assert repr(chromaterm.highlight(config, data)) == repr(''.join(expected))


def test_highlight_existing_start_different_type():
    """Highlight with an existing, different-type, color at the start of the data."""
    config_data = '''rules:
    - description: first
      regex: Hello World
      color: b#aaafff'''
    config = chromaterm.parse_config(config_data)

    data = '\x1b[33mHello World'
    expected = ['\x1b[33m', '\x1b[48;5;153m', 'Hello World', '\x1b[49m']

    assert repr(chromaterm.highlight(config, data)) == repr(''.join(expected))


def test_highlight_existing_end_same_type():
    """Highlight with an existing, same-type, color at the end of the data."""
    config_data = '''rules:
    - description: first
      regex: Hello World
      color: f#aaafff'''
    config = chromaterm.parse_config(config_data)

    data = 'Hello World\x1b[33m'
    expected = ['\x1b[38;5;153m', 'Hello World', '\x1b[39m', '\x1b[33m']

    assert repr(chromaterm.highlight(config, data)) == repr(''.join(expected))


def test_highlight_existing_end_different_type():
    """Highlight with an existing, different-type, color at the end of the data."""
    config_data = '''rules:
    - description: first
      regex: Hello World
      color: b#aaafff'''
    config = chromaterm.parse_config(config_data)

    data = 'Hello World\x1b[33m'
    expected = ['\x1b[48;5;153m', 'Hello World', '\x1b[49m', '\x1b[33m']

    assert repr(chromaterm.highlight(config, data)) == repr(''.join(expected))


def test_highlight_existing_multiline():
    """Highlight with an existing color in the multi-line data."""
    config_data = '''rules:
    - description: first
      regex: Hello World
      color: f#aaafff'''
    config = chromaterm.parse_config(config_data)

    data = ['\x1b[33mHi there', 'Hello World']
    expected = [['\x1b[33m', 'Hi there'],
                ['\x1b[38;5;153m', 'Hello World', '\x1b[33m']]

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

    data = '\x1b[33mHello \x1b[34mthere\x1b[m, World'
    expected = [
        '\x1b[33m', 'Hello ', '\x1b[34m', 'there', '\x1b[m', ', ',
        '\x1b[38;5;153m', 'World', '\x1b[m'
    ]

    assert repr(chromaterm.highlight(config, data)) == repr(''.join(expected))


def test_highlight_existing_intensity_orphaned():
    """Highlight with an existing intensity (i.e. bold or faint) in the data."""
    config_data = '''rules:
    - description: first
      regex: World
      color: bold'''
    config = chromaterm.parse_config(config_data)

    data = ['\x1b[2mHello World', '\x1b[1mHello World']
    expected = [['\x1b[2m', 'Hello ', '\x1b[1m', 'World', '\x1b[2m'],
                ['\x1b[1m', 'Hello ', '\x1b[1m', 'World', '\x1b[1m']]

    for line_data, line_expected in zip(data, expected):
        assert repr(chromaterm.highlight(config, line_data)) == repr(
            ''.join(line_expected))


def test_highlight_existing_complete_reset():
    """Highlight with an existing complete reset in the data. It should be set
    to our color."""
    config_data = '''rules:
    - description: first
      regex: Hello there, World
      color: f#aaafff'''
    config = chromaterm.parse_config(config_data)

    data = 'Hello\x1b[m there, World'
    expected = [
        '\x1b[38;5;153m', 'Hello', '\x1b[38;5;153m', ' there, World',
        '\x1b[39m'
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

    data = '\x1b[33mHello the\x1b[mre, World'
    expected = [
        '\x1b[33m', '\x1b[38;5;153m', 'Hello ', '\x1b[38;5;229m', 'the',
        '\x1b[38;5;153m', 're', '\x1b[38;5;229m', ', World', '\x1b[39m'
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

    data = '\x1b[44mHello the\x1b[mre, World'
    expected = [
        '\x1b[44m', '\x1b[38;5;153m', 'Hello ', '\x1b[48;5;229m', 'the',
        '\x1b[38;5;153m', 're', '\x1b[39m', ', World', '\x1b[44m'
    ]

    assert repr(chromaterm.highlight(config, data)) == repr(''.join(expected))


def test_highlight_existing_compound():
    """Highlight with an existing compound color (multiple SGR's) in the data."""
    config_data = '''rules:
    - regex: World|me
      color: f#aaafff'''
    config = chromaterm.parse_config(config_data)

    data = '\x1b[1;38;5;123;38;2;1;1;1;4mHello World\x1b[35;5m It\'s \x1b[32;24mme'
    expected = [
        '\x1b[1m\x1b[38;5;123m\x1b[38;2;1;1;1m\x1b[4m', 'Hello ',
        '\x1b[38;5;153m', 'World', '\x1b[38;2;1;1;1m', '\x1b[35m\x1b[5m',
        ' It\'s ', '\x1b[32m\x1b[24m', '\x1b[38;5;153m', 'me', '\x1b[32m'
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

    data = ['\x1b[33mSup', 'Hello World! It\'s me']
    expected = [['\x1b[33m', 'Sup'],
                [
                    '\x1b[38;5;153m', 'Hello ', '\x1b[38;5;229m', 'World',
                    '\x1b[38;5;229m', '! ', '\x1b[38;5;33m', 'It\'s',
                    '\x1b[38;5;33m', ' me', '\x1b[33m'
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
        '\x1b[1m', 'Hello ', '\x1b[38;5;229m', 'World', '\x1b[39m', '! It\'s ',
        '\x1b[38;5;153m', 'me', '\x1b[39m', '\x1b[22m'
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
    expected = ['Hello there! Hello ', '\x1b[38;5;229m', 'World', '\x1b[39m']

    assert repr(chromaterm.highlight(config, data)) == repr(''.join(expected))


def test_highlight_update_type_reset():
    """Feed a string to chromaterm that updates the config with new resets to
    confirm they are not colliding."""
    config_data = '''rules:
    - regex: Hello
      color: f#f7e08b'''
    config = chromaterm.parse_config(config_data)

    resets = {
        'fg': '\x1b[33m',
        'bg': '\x1b[45m',
        'blink': '\x1b[5m',
        'bold': '\x1b[1m',
        'italic': '\x1b[3m',
        'strike': '\x1b[9m',
        'underline': '\x1b[4m'
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
    assert chromaterm.highlight(config, '\x1b[33mHello there, \x1b[43mWorld')
    assert repr(config['resets']['fg']) == repr('\x1b[33m')

    # Feed the complete reset to send back to default
    assert chromaterm.highlight(config, '\x1b[mHello there, World')
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
    success = r'^test \x1b\[[34]8;5;[0-9]{1,3}mhello world\x1b\[49m test$'

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
    success = r'^test \x1b\[48;5;[0-9]{1,3}mhello world\x1b\[49m test$'

    chromaterm.process_buffer(config, data, False)
    captured = capsys.readouterr()

    assert re.search(success, captured.out)


def test_process_buffer_rule_group(capsys):
    """Output processing with a group-specific rule."""
    config = chromaterm.get_default_config()
    rule = {'regex': 'hello (world)', 'color': {1: 'b#fffaaa'}}

    config['rules'].append(chromaterm.parse_rule(rule))
    data = 'test hello world test'
    success = r'^test hello \x1b\[48;5;[0-9]{1,3}mworld\x1b\[49m test$'

    chromaterm.process_buffer(config, data, False)
    captured = capsys.readouterr()

    assert re.search(success, captured.out)


def test_process_buffer_rule_multiple_colors(capsys):
    """Output processing with a multi-color rule."""
    config = chromaterm.get_default_config()
    rule = {'regex': 'hello', 'color': 'b#fffaaa f#aaafff'}

    config['rules'].append(chromaterm.parse_rule(rule))
    data = 'hello world'
    success = r'^(\x1b\[[34]8;5;[0-9]{1,3}m){2}hello(\x1b\[[34]9m){2} world$'

    chromaterm.process_buffer(config, data, False)
    captured = capsys.readouterr()

    assert re.search(success, captured.out)


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


def test_read_ready_input():
    """Immediate ready when there is input buffered."""
    try:
        pipe_r, pipe_w = os.pipe()

        os.write(pipe_w, b'Hello world')
        assert chromaterm.read_ready([pipe_r])
    finally:
        os.close(pipe_r)
        os.close(pipe_w)


def test_read_ready_no_read_fd():
    """read_ready with None as read_fd must return False (no data to read)."""
    assert not chromaterm.read_ready(None)


def test_read_ready_timeout_empty():
    """Wait with no input."""
    try:
        pipe_r, pipe_w = os.pipe()

        before = time.time()
        assert not chromaterm.read_ready([pipe_r], 0.5)

        after = time.time()
        assert after - before >= 0.5
    finally:
        os.close(pipe_r)
        os.close(pipe_w)


def test_read_ready_timeout_input():
    """Immediate ready with timeout when there is input buffered."""
    try:
        pipe_r, pipe_w = os.pipe()

        os.write(pipe_w, b'Hello world')
        before = time.time()
        assert chromaterm.read_ready([pipe_r], 0.5)

        after = time.time()
        assert after - before < 0.5
    finally:
        os.close(pipe_r)
        os.close(pipe_w)


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


def test_split_buffer_new_line_r():
    """Split based on \\r"""
    data = 'Hello \rWorld'
    expected = [['Hello ', '\r'], ['World', '']]

    assert repr(chromaterm.split_buffer(data)) == repr(expected)


def test_split_buffer_new_line_r_n():
    """Split based on \\r\\n"""
    data = 'Hello \r\n World'
    expected = [['Hello ', '\r\n'], [' World', '']]

    assert repr(chromaterm.split_buffer(data)) == repr(expected)


def test_split_buffer_new_line_n():
    """Split based on \\n"""
    data = 'Hello \n World'
    expected = [['Hello ', '\n'], [' World', '']]

    assert repr(chromaterm.split_buffer(data)) == repr(expected)


def test_split_buffer_vertical_space():
    """Split based on \\v"""
    data = 'Hello \v World'
    expected = [['Hello ', '\v'], [' World', '']]

    assert repr(chromaterm.split_buffer(data)) == repr(expected)


def test_split_buffer_form_feed():
    """Split based on \\f"""
    data = 'Hello \f World'
    expected = [['Hello ', '\f'], [' World', '']]

    assert repr(chromaterm.split_buffer(data)) == repr(expected)


def test_split_buffer_ecma_048_c1_set():
    """Split based on the ECMA-048 C1 set, excluding CSI."""
    c1_set_up_to_csi = range(int('40', 16), int('5b', 16))
    c1_set_above_csi = range(int('5c', 16), int('60', 16))

    for char_id in itertools.chain(c1_set_up_to_csi, c1_set_above_csi):
        data = 'Hello \x1b{} World'.format(chr(char_id))
        expected = [['Hello ', '\x1b' + chr(char_id)], [' World', '']]

        assert repr(chromaterm.split_buffer(data)) == repr(expected)


def test_split_buffer_ecma_048_exclude_csi_sgr():
    """Fail to split based on the ECMA-048 C1 CSI SGR. Added some intermediate
    characters to prevent matching other CSI codes; strictly checking empty SGR."""
    data = 'Hello \x1b[!0World'
    expected = [['Hello \x1b[!0World', '']]

    assert repr(chromaterm.split_buffer(data)) == repr(expected)


def test_split_buffer_ecma_048_csi_no_parameter_no_intermediate():
    """Split based on CSI with no parameter or intermediate bytes."""
    csi_up_to_sgr = range(int('40', 16), int('6d', 16))
    csi_above_sgr = range(int('6e', 16), int('7f', 16))

    for char_id in itertools.chain(csi_up_to_sgr, csi_above_sgr):
        data = 'Hello \x1b[{} World'.format(chr(char_id))
        expected = [['Hello ', '\x1b[' + chr(char_id)], [' World', '']]

        assert repr(chromaterm.split_buffer(data)) == repr(expected)


def test_split_buffer_ecma_048_csi_parameter_no_intermediate():
    """Split based on CSI with parameters bytes but no intermediate bytes. Up to
    3 bytes."""
    csi_up_to_sgr = range(int('40', 16), int('6d', 16))
    csi_above_sgr = range(int('6e', 16), int('7f', 16))

    for char_id in itertools.chain(csi_up_to_sgr, csi_above_sgr):
        for parameter in range(int('30', 16), int('40', 16)):
            for count in range(1, 4):
                code = chr(parameter) * count + chr(char_id)
                data = 'Hello \x1b[{} World'.format(code)
                expected = [['Hello ', '\x1b[' + code], [' World', '']]

                assert repr(chromaterm.split_buffer(data)) == repr(expected)


def test_split_buffer_ecma_048_csi_intermediate_no_parameter():
    """Split based on CSI with intermediate bytes but no parameter bytes."""
    csi_up_to_sgr = range(int('40', 16), int('6d', 16))
    csi_above_sgr = range(int('6e', 16), int('7f', 16))

    for char_id in itertools.chain(csi_up_to_sgr, csi_above_sgr):
        for intermediate in range(int('20', 16), int('30', 16)):
            for count in range(1, 4):
                code = chr(intermediate) * count + chr(char_id)
                data = 'Hello \x1b[{} World'.format(code)
                expected = [['Hello ', '\x1b[' + code], [' World', '']]

                assert repr(chromaterm.split_buffer(data)) == repr(expected)


def test_split_buffer_ecma_048_csi_parameter_intermediate():
    """Split based on CSI with parameter and intermediate bytes. Up to 3 bytes
    each."""
    csi_up_to_sgr = range(int('40', 16), int('6d', 16))
    csi_above_sgr = range(int('6e', 16), int('7f', 16))

    for char_id in itertools.chain(csi_up_to_sgr, csi_above_sgr):
        for parameter in range(int('30', 16), int('40', 16)):
            for intermediate in range(int('20', 16), int('30', 16)):
                for count in range(1, 4):
                    code = chr(parameter) * count + chr(
                        intermediate) * count + chr(char_id)
                    data = 'Hello \x1b[{} World'.format(code)
                    expected = [['Hello ', '\x1b[' + code], [' World', '']]

                    assert repr(
                        chromaterm.split_buffer(data)) == repr(expected)


def test_tty_test_code_no_pipe():
    """Baseline the test code with no pipes on stdin or stdout."""
    master, slave = os.openpty()
    subprocess.run(TTY_TEST_PROGRAM,
                   check=True,
                   shell=True,
                   stdin=master,
                   stdout=slave)
    assert 'stdin=True, stdout=True' in os.read(master, 1024).decode()


def test_tty_test_code_in_pipe():
    """Baseline the test code with a pipe on stdin."""
    master, slave = os.openpty()
    subprocess.run(TTY_TEST_PROGRAM,
                   check=True,
                   shell=True,
                   stdin=subprocess.PIPE,
                   stdout=slave)
    assert 'stdin=False, stdout=True' in os.read(master, 1024).decode()


def test_tty_test_code_out_pipe():
    """Baseline the test code with a pipe on stdout."""
    master, _ = os.openpty()
    result = subprocess.run(TTY_TEST_PROGRAM,
                            check=True,
                            shell=True,
                            stdin=master,
                            stdout=subprocess.PIPE)
    assert 'stdin=True, stdout=False' in result.stdout.decode()


def test_tty_test_code_in_out_pipe():
    """Baseline the test code with pipes on stdin and stdout."""
    result = subprocess.run(TTY_TEST_PROGRAM,
                            check=True,
                            shell=True,
                            stdin=subprocess.PIPE,
                            stdout=subprocess.PIPE)
    assert 'stdin=False, stdout=False' in result.stdout.decode()


def test_main(capsys):
    """General stdin processing."""
    try:
        pipe_r, pipe_w = os.pipe()
        config = chromaterm.config_init([])
        main_thread = Thread(target=chromaterm.main, args=(config, 1, pipe_r))

        main_thread.start()
        time.sleep(0.1)  # Any start-up delay
        assert main_thread.is_alive()

        os.write(pipe_w, b'Hello world\n')
        time.sleep(0.1)  # Any processing delay
        assert capsys.readouterr().out == 'Hello world\n'

        os.write(pipe_w, b'Hey there')
        time.sleep(0.1 + chromaterm.WAIT_FOR_SPLIT)  # Include split wait
        assert capsys.readouterr().out == 'Hey there'

        os.write(pipe_w, b'x' * (chromaterm.READ_SIZE + 1))
        time.sleep(0.1 + chromaterm.WAIT_FOR_SPLIT)  # Include split wait
        assert capsys.readouterr().out == 'x' * (chromaterm.READ_SIZE + 1)
    finally:
        os.close(pipe_r)
        os.close(pipe_w)
        main_thread.join()


def test_main_buffer_close_time():
    """Confirm that the program exists as soon as stdin closes."""
    before = time.time()
    subprocess.run('echo hi | ./ct', check=True, shell=True)
    after = time.time()

    assert after - before < 1


def test_main_reload_config(capsys):
    """Reload the configuration while the program is running."""
    try:
        with open(TEMP_FILE + '3', 'w') as file:
            file.write('''rules:
            - regex: Hello
              color: f#123123
            - regex: world
              color: b#321321''')

        pipe_r, pipe_w = os.pipe()
        config = chromaterm.config_init(['--config', TEMP_FILE + '3'])
        main_thread = Thread(target=chromaterm.main, args=(config, 1, pipe_r))

        main_thread.start()
        time.sleep(0.1)  # Any start-up delay
        assert main_thread.is_alive()

        os.write(pipe_w, b'Hello world')
        expected = [
            '\x1b[38;5;22m', 'Hello', '\x1b[39m', ' ', '\x1b[48;5;52m',
            'world', '\x1b[49m'
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

        os.write(pipe_w, b'Hello world')
        expected = ['\x1b[38;5;22m', 'Hello', '\x1b[39m', ' world']
        time.sleep(0.1)  # Any processing delay
        assert repr(capsys.readouterr().out) == repr(''.join(expected))
    finally:
        os.close(pipe_r)
        os.close(pipe_w)
        os.remove(TEMP_FILE + '3')
        main_thread.join()


def test_main_reload_processes():
    """Reload all other CT processes."""
    processes = [  # Spawn dummy ct processes
        subprocess.Popen('sleep 1 | ./ct', shell=True) for _ in range(3)
    ]

    program = ['./ct', '--reload']
    result = subprocess.run(program, check=False, stderr=subprocess.PIPE)
    assert result.stderr == b'Processes reloaded: 3\n'

    for process in processes:
        process.wait()


def test_main_run_no_file_found():
    """Have CT run with an unavailable command."""
    program = ['./ct', 'plz-no-work']
    result = subprocess.run(program, check=False, stderr=subprocess.PIPE)
    assert result.stderr == b'./ct: plz-no-work: command not found\n'


def test_main_run_no_pipe():
    """Have CT run the tty test code with no pipes."""
    master, slave = os.openpty()
    subprocess.run('./ct ' + TTY_TEST_PROGRAM,
                   check=True,
                   shell=True,
                   stdin=master,
                   stdout=slave)
    assert 'stdin=True, stdout=True' in os.read(master, 1024).decode()


def test_main_run_in_pipe():
    """Have CT run the tty test code with a pipe on stdin."""
    master, slave = os.openpty()
    subprocess.run('./ct ' + TTY_TEST_PROGRAM,
                   check=True,
                   shell=True,
                   stdin=subprocess.PIPE,
                   stdout=slave)
    assert 'stdin=False, stdout=True' in os.read(master, 1024).decode()


def test_main_run_out_pipe():
    """Have CT run the tty test code with a pipe on stdout."""
    master, _ = os.openpty()
    result = subprocess.run('./ct ' + TTY_TEST_PROGRAM,
                            check=True,
                            shell=True,
                            stdin=master,
                            stdout=subprocess.PIPE)
    assert 'stdin=True, stdout=True' in result.stdout.decode()


def test_main_run_in_out_pipe():
    """Have CT run the tty test code with pipes on stdin and stdout."""
    result = subprocess.run('./ct ' + TTY_TEST_PROGRAM,
                            check=True,
                            shell=True,
                            stdin=subprocess.PIPE,
                            stdout=subprocess.PIPE)
    assert 'stdin=False, stdout=True' in result.stdout.decode()
