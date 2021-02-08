"""chromaterm.cli tests"""
import itertools
import os
import re
import stat
import subprocess
import sys
import time

import chromaterm
import chromaterm.cli

# pylint: disable=too-many-lines

CLI = sys.executable + ' -m chromaterm.cli'

CODE_ISATTY = """import os, sys
stdin = os.isatty(sys.stdin.fileno())
stdout = os.isatty(sys.stdout.fileno())
print('stdin={}, stdout={}'.format(stdin, stdout))"""

CODE_TTYNAME = """import os, sys
print(os.ttyname(sys.stdin.fileno()) if os.isatty(sys.stdin.fileno()) else None)"""


def get_python_command(code):
    """Returns the python shell command that runs `code`."""
    return sys.executable + ' -c "{}"'.format('; '.join(code.splitlines()))


def test_baseline_tty_test_code_no_pipe():
    """Baseline the test code with no pipes on stdin or stdout."""
    master, slave = os.openpty()
    subprocess.run(get_python_command(CODE_ISATTY),
                   check=True,
                   shell=True,
                   stdin=slave,
                   stdout=slave)

    assert 'stdin=True, stdout=True' in os.read(master, 100).decode()


def test_baseline_tty_test_code_in_pipe():
    """Baseline the test code with a pipe on stdin."""
    master, slave = os.openpty()
    subprocess.run(get_python_command(CODE_ISATTY),
                   check=True,
                   shell=True,
                   stdin=subprocess.PIPE,
                   stdout=slave)

    assert 'stdin=False, stdout=True' in os.read(master, 100).decode()


def test_baseline_tty_test_code_out_pipe():
    """Baseline the test code with a pipe on stdout."""
    _, slave = os.openpty()
    result = subprocess.run(get_python_command(CODE_ISATTY),
                            check=True,
                            shell=True,
                            stdin=slave,
                            stdout=subprocess.PIPE)

    assert 'stdin=True, stdout=False' in result.stdout.decode()


def test_baseline_tty_test_code_in_out_pipe():
    """Baseline the test code with pipes on stdin and stdout."""
    result = subprocess.run(get_python_command(CODE_ISATTY),
                            check=True,
                            shell=True,
                            stdin=subprocess.PIPE,
                            stdout=subprocess.PIPE)

    assert 'stdin=False, stdout=False' in result.stdout.decode()


def test_baseline_tty_test_code_ttyname_same():
    """Baseline the ttyname code, ensuring it detects matching ttys."""
    master, slave = os.openpty()

    subprocess.run(get_python_command(CODE_TTYNAME),
                   check=True,
                   shell=True,
                   stdin=slave,
                   stdout=slave)

    assert os.ttyname(slave) in os.read(master, 100).decode()


def test_baseline_tty_test_code_ttyname_different():
    """Baseline the ttyname code, ensuring it detects different ttys."""
    master, slave = os.openpty()
    _, another_slave = os.openpty()

    subprocess.run(get_python_command(CODE_TTYNAME),
                   check=True,
                   shell=True,
                   stdin=slave,
                   stdout=slave)

    assert os.ttyname(another_slave) not in os.read(master, 100).decode()


def test_config_decode_sgr_bg():
    """Background colors and reset are being detected."""
    for code in ['\x1b[48;5;12m', '\x1b[48;2;1;1;1m', '\x1b[49m', '\x1b[101m']:
        colors = chromaterm.cli.Config.decode_sgr(code)
        assert len(colors) == 1

        for color_code, _, color_type in colors:
            assert repr(color_code) == repr(code)
            assert color_type == 'bg'


def test_config_decode_sgr_fg():
    """Foreground colors and reset are being detected."""
    for code in ['\x1b[38;5;12m', '\x1b[38;2;1;1;1m', '\x1b[39m', '\x1b[91m']:
        colors = chromaterm.cli.Config.decode_sgr(code)
        assert len(colors) == 1

        for color_code, _, color_type in colors:
            assert repr(color_code) == repr(code)
            assert color_type == 'fg'


def test_config_decode_sgr_styles_blink():
    """Blink and its reset are being detected."""
    for code in ['\x1b[5m', '\x1b[25m']:
        colors = chromaterm.cli.Config.decode_sgr(code)
        assert len(colors) == 1

        for color_code, _, color_type in colors:
            assert repr(color_code) == repr(code)
            assert color_type == 'blink'


def test_config_decode_sgr_styles_bold():
    """Bold and its reset are being detected."""
    for code in ['\x1b[1m', '\x1b[2m', '\x1b[22m']:
        colors = chromaterm.cli.Config.decode_sgr(code)
        assert len(colors) == 1

        for color_code, _, color_type in colors:
            assert repr(color_code) == repr(code)
            assert color_type == 'bold'


def test_config_decode_sgr_styles_italic():
    """Italic and its reset are being detected."""
    for code in ['\x1b[3m', '\x1b[23m']:
        colors = chromaterm.cli.Config.decode_sgr(code)
        assert len(colors) == 1

        for color_code, _, color_type in colors:
            assert repr(color_code) == repr(code)
            assert color_type == 'italic'


def test_config_decode_sgr_styles_strike():
    """Strike and its reset are being detected."""
    for code in ['\x1b[9m', '\x1b[29m']:
        colors = chromaterm.cli.Config.decode_sgr(code)
        assert len(colors) == 1

        for color_code, _, color_type in colors:
            assert repr(color_code) == repr(code)
            assert color_type == 'strike'


def test_config_decode_sgr_styles_underline():
    """Underline and its reset are being detected."""
    for code in ['\x1b[4m', '\x1b[24m']:
        colors = chromaterm.cli.Config.decode_sgr(code)
        assert len(colors) == 1

        for color_code, _, color_type in colors:
            assert repr(color_code) == repr(code)
            assert color_type == 'underline'


def test_config_decode_sgr_full_reset():
    """Full reset detection."""
    for code in ['\x1b[00m', '\x1b[0m', '\x1b[m']:
        colors = chromaterm.cli.Config.decode_sgr(code)
        assert len(colors) == 1

        for color_code, color_reset, color_type in colors:
            assert repr(color_code) == repr(code)
            assert color_reset is True
            assert color_type is None


def test_config_decode_sgr_malformed():
    """Malformed colors."""
    for code in ['\x1b[38;5m', '\x1b[38;2;1;1m', '\x1b[38;5;123;38;2;1;1m']:
        colors = chromaterm.cli.Config.decode_sgr(code)
        assert len(colors) == 1

        for color_code, color_reset, color_type in colors:
            assert repr(color_code) == repr(code)
            assert color_reset is False
            assert color_type is None


def test_config_decode_sgr_split_compound():
    """Split the a compound SGR into discrete SGR's."""
    colors = chromaterm.cli.Config.decode_sgr('\x1b[1;33;40m')
    codes = ['\x1b[1m', '\x1b[33m', '\x1b[40m']
    types = ['bold', 'fg', 'bg']

    for color, color_code, color_type in zip(colors, codes, types):
        assert repr(color_code) == repr(color[0])
        assert color[1] is False
        assert color_type == color[2]


def test_config_load_simple():
    """Parse config with a simple rule."""
    config = chromaterm.cli.Config()
    config.load('''rules:
    - regex: hello world
      color: f#fffaaa''')

    assert len(config.rules) == 1


def test_config_load_group():
    """Parse config with a group-specific rule."""
    config = chromaterm.cli.Config()
    config.load('''rules:
    - description: group-specific
      regex: h(el)lo (world)
      color:
        0: bold
        1: b#fffaaa
        2: f#123123''')

    assert len(config.rules) == 1
    assert config.rules[0].description == 'group-specific'
    assert config.rules[0].regex == re.compile(r'h(el)lo (world)')
    assert config.rules[0].colors[0].color == 'bold'
    assert config.rules[0].colors[1].color == 'b#fffaaa'
    assert config.rules[0].colors[2].color == 'f#123123'


def test_config_load_multiple_colors():
    """Parse config with a multi-color rule."""
    config = chromaterm.cli.Config()
    config.load('''rules:
    - regex: hello (world)
      color: b#fffaaa f#aaafff''')

    assert len(config.rules) == 1
    assert config.rules[0].color.color == 'b#fffaaa f#aaafff'


def test_config_load_rule_format_error(capsys):
    """Parse a config file with a syntax problem."""
    config = chromaterm.cli.Config()
    config.load('''rules:
    - 1''')

    assert 'Rule 1 not a dictionary' in capsys.readouterr().err


def test_config_load_yaml_format_error(capsys):
    """Parse an incorrect YAML file."""
    config = chromaterm.cli.Config()
    config.load('-x-\nhi:')

    assert 'Parse error:' in capsys.readouterr().err


def test_config_parse_rule_regex_missing():
    """Parse a rule without a `regex` key."""
    msg = 'regex must be a string'

    rule = {'color': 'b#fffaaa'}
    assert msg in chromaterm.cli.Config.parse_rule(rule)


def test_config_parse_rule_regex_type_error():
    """Parse a rule with an incorrect `regex` value type."""
    msg = 'regex must be a string'

    rule = {'regex': ['hi'], 'color': 'b#fffaaa'}
    assert msg in chromaterm.cli.Config.parse_rule(rule)

    rule = {'regex': 111, 'color': 'b#fffaaa'}
    assert msg in chromaterm.cli.Config.parse_rule(rule)


def test_config_parse_rule_regex_invalid():
    """Parse a rule with an invalid `regex`."""
    msg = 're.error: '

    rule = {'regex': '+', 'color': 'b#fffaaa'}
    assert msg in chromaterm.cli.Config.parse_rule(rule)


def test_config_parse_rule_color_missing():
    """Parse a rule without a `color` key."""
    msg_re = r'color .* is not a string'

    rule = {'regex': 'x(y)z'}
    assert re.search(msg_re, chromaterm.cli.Config.parse_rule(rule))


def test_config_parse_rule_color_type_error():
    """Parse a rule with an incorrect `color` value type."""
    msg_re = r'color .* is not a string'

    rule = {'regex': 'x(y)z', 'color': ['hi']}
    assert re.search(msg_re, chromaterm.cli.Config.parse_rule(rule))


def test_config_parse_rule_color_format_error():
    """Parse a rule with an incorrect `color` format."""
    msg = 'invalid color format'

    rule = {'regex': 'x(y)z', 'color': 'b#xyzxyz'}
    assert msg in chromaterm.cli.Config.parse_rule(rule)

    rule = {'regex': 'x(y)z', 'color': 'x#fffaaa'}
    assert msg in chromaterm.cli.Config.parse_rule(rule)

    rule = {'regex': 'x(y)z', 'color': 'b@fffaaa'}
    assert msg in chromaterm.cli.Config.parse_rule(rule)

    rule = {'regex': 'x(y)z', 'color': 'b#fffaaa-f#fffaaa'}
    assert msg in chromaterm.cli.Config.parse_rule(rule)


def test_config_parse_rule_group_type_error():
    """Parse a rule with an incorrect `group` value type."""
    msg = 'group must be an integer'

    rule = {'regex': 'x(y)z', 'color': {'1': 'b#fffaaa'}}
    assert msg in chromaterm.cli.Config.parse_rule(rule)


def test_config_parse_rule_group_out_of_bounds():
    """Parse a rule with `group` number not in the regex."""
    msg_re = r'regex only has .* group\(s\); .* is invalid'

    rule = {'regex': 'x(y)z', 'color': {2: 'b#fffaaa'}}
    assert re.search(msg_re, chromaterm.cli.Config.parse_rule(rule))


def test_config_highlight_tracking_common_beginning_type_different():
    """A rule with a match that has a color of a different type just before its
    start. The rule's color is closer to the match and the reset is unaffected by
    the existing color.
    1: x-------------"""
    config = chromaterm.cli.Config()

    rule = chromaterm.Rule('hello', color=chromaterm.Color('b#123123'))
    config.add_rule(rule)

    data = '\x1b[33mhello'
    expected = [
        '\x1b[33m', rule.color.color_code, 'hello', rule.color.color_reset
    ]

    assert repr(config.highlight(data)) == repr(''.join(expected))


def test_config_highlight_tracking_common_beginning_type_same():
    """A rule with a match that has a color of the same type just before its
    start. The rule's color is closer to the match and the reset used is the
    existing color.
    1: x-------------"""
    config = chromaterm.cli.Config()

    rule = chromaterm.Rule('hello', color=chromaterm.Color('f#321321'))
    config.add_rule(rule)

    data = '\x1b[33mhello'
    expected = ['\x1b[33m', rule.color.color_code, 'hello', '\x1b[33m']

    assert repr(config.highlight(data)) == repr(''.join(expected))


def test_config_highlight_tracking_common_end_type_different():
    """A rule with a match that has a color of a different type just after its
    end. The rule's reset is closer to the match and the reset is unaffected by
    the existing color.
    1: -------------x"""
    config = chromaterm.cli.Config()

    rule = chromaterm.Rule('hello', color=chromaterm.Color('b#123123'))
    config.add_rule(rule)

    data = 'hello\x1b[33m'
    expected = [
        rule.color.color_code, 'hello', rule.color.color_reset, '\x1b[33m'
    ]

    assert repr(config.highlight(data)) == repr(''.join(expected))


def test_config_highlight_tracking_common_end_type_same():
    """A rule with a match that has a color of the same type just after its end.
    The rule's reset is closer to the match and is unaffected by the existing
    color.
    1: -------------x"""
    config = chromaterm.cli.Config()

    rule = chromaterm.Rule('hello', color=chromaterm.Color('f#321321'))
    config.add_rule(rule)

    data = 'hello\x1b[33m'
    expected = [
        rule.color.color_code, 'hello', rule.color.color_reset, '\x1b[33m'
    ]

    assert repr(config.highlight(data)) == repr(''.join(expected))


def test_config_highlight_tracking_full_reset_beginning():
    """A rule with a match that has a full reset just before the start of the
    match. The rule's color is closer to the match and the reset used is the
    default for that color type.
    1: R-------------"""
    config = chromaterm.cli.Config()

    rule = chromaterm.Rule('hello', color=chromaterm.Color('f#321321'))
    config.add_rule(rule)

    data = '\x1b[mhello'
    expected = [
        '\x1b[m', rule.color.color_code, 'hello',
        chromaterm.COLOR_TYPES[rule.color.color_types[0][0]]['reset']
    ]

    assert repr(config.highlight(data)) == repr(''.join(expected))


def test_config_highlight_tracking_full_reset_end():
    """A rule with a match that has a full reset just after the end of the match.
    The rule's reset is closer to the match and the reset used is the default for
    that color type.
    1: -------------R"""
    config = chromaterm.cli.Config()

    rule = chromaterm.Rule('hello', color=chromaterm.Color('f#321321'))
    config.add_rule(rule)

    data = 'hello\x1b[m'
    expected = [
        rule.color.color_code, 'hello',
        chromaterm.COLOR_TYPES[rule.color.color_types[0][0]]['reset'], '\x1b[m'
    ]

    assert repr(config.highlight(data)) == repr(''.join(expected))


def test_config_highlight_tracking_full_reset_middle():
    """A rule with a match that has a full reset in the middle of it. The full
    reset is changed to the color code of the match and the reset of the match
    is changed to a full reset.
    1: ------R-------"""
    config = chromaterm.cli.Config()

    rule = chromaterm.Rule('hello', color=chromaterm.Color('f#321321'))
    config.add_rule(rule)

    data = 'hel\x1b[mlo'
    expected = [
        rule.color.color_code, 'hel', rule.color.color_code, 'lo', '\x1b[m'
    ]

    assert repr(config.highlight(data)) == repr(''.join(expected))


def test_config_highlight_tracking_malformed():
    """A rule with a match that has a malformed SGR in the middle. It should be
    ignored and inserted back into the match. Highlighting from the rule should
    still go through.
    1: x-------------"""
    config = chromaterm.cli.Config()

    rule = chromaterm.Rule('hello', color=chromaterm.Color('b#123123'))
    config.add_rule(rule)

    data = 'he\x1b[38;5mllo'
    expected = [
        rule.color.color_code, 'he', '\x1b[38;5m', 'llo',
        rule.color.color_reset
    ]

    assert repr(config.highlight(data)) == repr(''.join(expected))


def test_config_highlight_tracking_mixed_full_reset():
    """Track multiple color types and ensure a full reset only defaults the types
    that were not updated by other colors in the data."""
    config = chromaterm.cli.Config()

    rule1 = chromaterm.Rule('hello', color=chromaterm.Color('f#321321'))
    rule2 = chromaterm.Rule('world', color=chromaterm.Color('b#123123'))
    config.add_rule(rule1)
    config.add_rule(rule2)

    data = '\x1b[33mhello\x1b[m there \x1b[43mworld'
    expected = [
        '\x1b[33m', rule1.color.color_code, 'hello', '\x1b[33m', '\x1b[m',
        ' there ', '\x1b[43m', rule2.color.color_code, 'world', '\x1b[43m'
    ]

    assert repr(config.highlight(data)) == repr(''.join(expected))

    # The color of rule1 was reset to its default because a full reset came after
    # it, but the color of rule2 was already updated so it wasn't affected by the
    # full reset
    data = 'hello there world'
    expected = [
        rule1.color.color_code, 'hello', rule1.color.color_reset, ' there ',
        rule2.color.color_code, 'world', '\x1b[43m'
    ]

    assert repr(config.highlight(data)) == repr(''.join(expected))


def test_config_highlight_tracking_multiline_type_different():
    """Ensure that data with an existing color is tracked across highlights and
    does not affect the reset of a color of a different type."""
    config = chromaterm.cli.Config()

    rule = chromaterm.Rule('hello', color=chromaterm.Color('b#123123'))
    config.add_rule(rule)

    # Inject a foreground color to have it tracked
    assert repr(config.highlight('\x1b[33m')) == repr('\x1b[33m')

    data = 'hello'
    expected = [rule.color.color_code, 'hello', rule.color.color_reset]

    assert repr(config.highlight(data)) == repr(''.join(expected))


def test_config_highlight_tracking_multiline_type_same():
    """Ensure that data with an existing color is tracked across highlights and
    affects the reset of a color of the same type."""
    config = chromaterm.cli.Config()

    rule = chromaterm.Rule('hello', color=chromaterm.Color('f#321321'))
    config.add_rule(rule)

    # Inject a foreground color to have it tracked
    assert repr(config.highlight('\x1b[33m')) == repr('\x1b[33m')

    data = 'hello'
    expected = [rule.color.color_code, 'hello', '\x1b[33m']

    assert repr(config.highlight(data)) == repr(''.join(expected))


def test_config_highlight_no_rules():
    """Highlight with config that has no rules – nothing is changed."""
    config = chromaterm.cli.Config()
    assert repr(config.highlight('hello world')) == repr('hello world')


def test_eprint(capsys):
    """Print a message to stderr."""
    msg = 'Some random error message'
    chromaterm.cli.eprint(msg)
    assert msg in capsys.readouterr().err


def test_process_input_decode_error(capsys):
    """Attempt to decode a character that is not UTF-8."""
    pipe_r, pipe_w = os.pipe()
    config = chromaterm.cli.Config()

    os.write(pipe_w, b'\x80')
    chromaterm.cli.process_input(config, pipe_r, max_wait=0)

    assert capsys.readouterr().out == '�'


def test_process_input_empty(capsys):
    """Input processing of empty input."""
    pipe_r, _ = os.pipe()
    config = chromaterm.cli.Config()

    chromaterm.cli.process_input(config, pipe_r, max_wait=0)

    assert capsys.readouterr().out == ''


def test_process_input_multiline(capsys):
    """Input processing with multiple lines of data."""
    pipe_r, pipe_w = os.pipe()
    config = chromaterm.cli.Config()

    rule = chromaterm.Rule('hello world', color=chromaterm.Color('bold'))
    config.add_rule(rule)

    os.write(pipe_w, b'\nt hello world t\n' * 2)
    chromaterm.cli.process_input(config, pipe_r, max_wait=0)

    assert capsys.readouterr().out == '\nt \x1b[1mhello world\x1b[22m t\n' * 2


def test_process_input_single_character(capsys):
    """Input processing for a single character. Even with a rule that matches
    single character, the output should not be highlighted as it is typically
    just keyboard input."""
    pipe_r, pipe_w = os.pipe()
    config = chromaterm.cli.Config()

    rule = chromaterm.Rule('.', color=chromaterm.Color('bold'))
    config.add_rule(rule)

    os.write(pipe_w, b'x')
    chromaterm.cli.process_input(config, pipe_r, max_wait=0)

    assert capsys.readouterr().out == 'x'


def test_read_file():
    """Read the default configuration file."""
    file = os.path.join(os.path.expanduser('~'), '.chromaterm.yml')
    assert chromaterm.cli.read_file(file) is not None


def test_read_file_no_file(capsys):
    """Read a non-existent file."""
    msg = 'Configuration file ' + __name__ + '1' + ' not found\n'
    chromaterm.cli.read_file(__name__ + '1')
    assert msg in capsys.readouterr().err


def test_read_file_no_permission(capsys):
    """Create a file with no permissions and attempt to read it. Delete the file
    once done with it."""
    msg = 'Cannot read configuration file ' + __name__ + '2' + ' (permission)\n'

    os.close(os.open(__name__ + '2', os.O_CREAT | os.O_WRONLY, 0o0000))
    chromaterm.cli.read_file(__name__ + '2')
    os.chmod(__name__ + '2', stat.S_IWRITE)
    os.remove(__name__ + '2')

    assert msg in capsys.readouterr().err


def test_read_ready_input():
    """Immediate ready when there is input buffered."""
    pipe_r, pipe_w = os.pipe()

    os.write(pipe_w, b'Hello world')
    assert chromaterm.cli.read_ready(pipe_r)


def test_read_ready_timeout_empty():
    """Wait with no input."""
    pipe_r, _ = os.pipe()

    before = time.time()
    assert not chromaterm.cli.read_ready(pipe_r, timeout=0.1)

    after = time.time()
    assert after - before >= 0.1


def test_read_ready_timeout_input():
    """Immediate ready with timeout when there is input buffered."""
    pipe_r, pipe_w = os.pipe()

    os.write(pipe_w, b'Hello world')
    before = time.time()
    assert chromaterm.cli.read_ready(pipe_r, timeout=0.1)

    after = time.time()
    assert after - before < 0.1


def test_split_buffer_new_line_r():
    """Split based on \\r"""
    data = 'Hello \rWorld'
    expected = (('Hello ', '\r'), ('World', ''))

    assert repr(chromaterm.cli.split_buffer(data)) == repr(expected)


def test_split_buffer_new_line_r_n():
    """Split based on \\r\\n"""
    data = 'Hello \r\n World'
    expected = (('Hello ', '\r\n'), (' World', ''))

    assert repr(chromaterm.cli.split_buffer(data)) == repr(expected)


def test_split_buffer_new_line_n():
    """Split based on \\n"""
    data = 'Hello \n World'
    expected = (('Hello ', '\n'), (' World', ''))

    assert repr(chromaterm.cli.split_buffer(data)) == repr(expected)


def test_split_buffer_vertical_space():
    """Split based on \\v"""
    data = 'Hello \v World'
    expected = (('Hello ', '\v'), (' World', ''))

    assert repr(chromaterm.cli.split_buffer(data)) == repr(expected)


def test_split_buffer_form_feed():
    """Split based on \\f"""
    data = 'Hello \f World'
    expected = (('Hello ', '\f'), (' World', ''))

    assert repr(chromaterm.cli.split_buffer(data)) == repr(expected)


def test_split_buffer_c1_set():
    """Split based on the ECMA-048 C1 set, excluding CSI and OSC."""
    c1_except_csi_and_osc = itertools.chain(
        range(int('40', 16), int('5b', 16)),
        [int('5c', 16), int('5e', 16),
         int('5f', 16)],
    )

    for char_id in c1_except_csi_and_osc:
        data = 'Hello \x1b{} World'.format(chr(char_id))
        expected = (('Hello ', '\x1b' + chr(char_id)), (' World', ''))

        assert repr(chromaterm.cli.split_buffer(data)) == repr(expected)


def test_split_buffer_csi_exclude_sgr():
    """Fail to split based on the ECMA-048 C1 CSI SGR. Added some intermediate
    characters to prevent matching other CSI codes; strictly checking empty SGR."""
    data = 'Hello \x1b[!0World'
    expected = (('Hello \x1b[!0World', ''), )

    assert repr(chromaterm.cli.split_buffer(data)) == repr(expected)


def test_split_buffer_csi_no_parameter_no_intermediate():
    """Split based on CSI with no parameter or intermediate bytes."""
    csi_up_to_sgr = range(int('40', 16), int('6d', 16))
    csi_above_sgr = range(int('6e', 16), int('7f', 16))

    for char_id in itertools.chain(csi_up_to_sgr, csi_above_sgr):
        data = 'Hello \x1b[{} World'.format(chr(char_id))
        expected = (('Hello ', '\x1b[' + chr(char_id)), (' World', ''))

        assert repr(chromaterm.cli.split_buffer(data)) == repr(expected)


def test_split_buffer_csi_no_parameter_intermediate():
    """Split based on CSI with intermediate bytes but no parameter bytes."""
    csi_up_to_sgr = range(int('40', 16), int('6d', 16))
    csi_above_sgr = range(int('6e', 16), int('7f', 16))

    for char_id in itertools.chain(csi_up_to_sgr, csi_above_sgr):
        for intermediate in range(int('20', 16), int('30', 16)):
            for count in range(1, 4):
                code = chr(intermediate) * count + chr(char_id)
                data = 'Hello \x1b[{} World'.format(code)
                expected = (('Hello ', '\x1b[' + code), (' World', ''))

                assert repr(
                    chromaterm.cli.split_buffer(data)) == repr(expected)


def test_split_buffer_csi_parameter_intermediate():
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
                    expected = (('Hello ', '\x1b[' + code), (' World', ''))

                    assert repr(
                        chromaterm.cli.split_buffer(data)) == repr(expected)


def test_split_buffer_csi_parameter_no_intermediate():
    """Split based on CSI with parameters bytes but no intermediate bytes. Up to
    3 bytes."""
    csi_up_to_sgr = range(int('40', 16), int('6d', 16))
    csi_above_sgr = range(int('6e', 16), int('7f', 16))

    for char_id in itertools.chain(csi_up_to_sgr, csi_above_sgr):
        for parameter in range(int('30', 16), int('40', 16)):
            for count in range(1, 4):
                code = chr(parameter) * count + chr(char_id)
                data = 'Hello \x1b[{} World'.format(code)
                expected = (('Hello ', '\x1b[' + code), (' World', ''))

                assert repr(
                    chromaterm.cli.split_buffer(data)) == repr(expected)


def test_split_buffer_osc_title():
    """Operating System Command (OSC) can supply arbitrary commands within the
    visible character set."""
    for end in ['\x07', '\x1b\x5c']:
        osc = '\x1b]Ignored{}'.format(end)
        data = '{}Hello world'.format(osc)
        expected = (('', osc), ('Hello world', ''))

        assert repr(chromaterm.cli.split_buffer(data)) == repr(expected)


def test_split_buffer_scs():
    """Select Character Set (SCS) is used to change the terminal character set."""
    g_sets = ['\x28', '\x29', '\x2a', '\x2b', '\x2d', '\x2e', '\x2f']
    char_sets = map(chr, range(int('20', 16), int('7e', 16)))

    for g_set in g_sets:
        for char_set in char_sets:
            code = '\x1b{}{}'.format(g_set, char_set)
            data = 'Hello {} World'.format(code)
            expected = (('Hello ', code), (' World', ''))

            assert repr(chromaterm.cli.split_buffer(data)) == repr(expected)


def test_main_broken_pipe():
    """Break a pipe while CT is trying to write to it. The echo at the end will
    close before CT has had the chance to write to the pipe."""
    result = subprocess.run('echo | ' + CLI + ' | echo',
                            check=False,
                            shell=True,
                            stderr=subprocess.PIPE)

    assert b'Broken pipe' not in result.stderr


def test_main_buffer_close_time():
    """Confirm that the program exists as soon as stdin closes."""
    before = time.time()
    subprocess.run('echo hi | ' + CLI, check=True, shell=True)
    after = time.time()

    assert after - before < 1


def test_main_reload_config():
    """Reload the configuration while the program is running."""
    try:
        # The initial configuration file
        with open(__name__ + '3', 'w') as file:
            file.write('''rules:
            - regex: Hello
              color: f#123123
            - regex: world
              color: b#123123''')

        expected = [
            '\x1b[38;2;18;49;35m', 'Hello', '\x1b[39m', ' ',
            '\x1b[48;2;18;49;35m', 'world', '\x1b[49m', '\n'
        ]

        stdin_r, stdin_w = os.pipe()
        stdout_r, stdout_w = os.pipe()

        process = subprocess.Popen(CLI + ' --rgb --config ' + __name__ + '3',
                                   shell=True,
                                   stdin=stdin_r,
                                   stdout=stdout_w)

        os.write(stdin_w, b'Hello world\n')
        time.sleep(0.1)  # Any processing delay

        assert repr(os.read(stdout_r, 100).decode()) == repr(''.join(expected))

        # Create file without the 'world' rule
        os.remove(__name__ + '3')
        with open(__name__ + '3', 'w') as file:
            file.write('''rules:
            - regex: Hello
              color: f#123123''')

        expected = ['\x1b[38;2;18;49;35m', 'Hello', '\x1b[39m', ' world\n']

        # Reload config
        subprocess.run(CLI + ' --reload', check=False, shell=True)

        os.write(stdin_w, b'Hello world\n')
        time.sleep(0.1)  # Any processing delay

        assert repr(os.read(stdout_r, 100).decode()) == repr(''.join(expected))
    finally:
        os.close(stdin_r)
        os.close(stdin_w)
        os.close(stdout_r)
        os.close(stdout_w)
        os.remove(__name__ + '3')
        process.kill()


def test_main_reload_processes():
    """Reload all other CT processes."""
    processes = [
        subprocess.Popen('sleep 1 | ' + CLI, shell=True) for _ in range(3)
    ]

    result = subprocess.run(CLI + ' --reload',
                            check=False,
                            shell=True,
                            stderr=subprocess.PIPE)

    assert result.stderr == b'Processes reloaded: 3\n'

    for process in processes:
        process.wait()


def test_main_run_child_ttyname():
    """Ensure that CT spawns the child in a pseudo-terminal."""
    master, slave = os.openpty()
    subprocess.run(CLI + ' ' + get_python_command(CODE_TTYNAME),
                   check=True,
                   shell=True,
                   stdin=slave,
                   stdout=slave)

    assert os.ttyname(slave) not in os.read(master, 100).decode()


def test_main_run_no_file_found():
    """Have CT run with an unavailable command."""
    result = subprocess.run(CLI + ' plz-no-work',
                            check=False,
                            stdout=subprocess.PIPE,
                            shell=True)

    output = re.sub(br'\x1b\[[\d;]+?m', b'', result.stdout)
    assert b'plz-no-work: command not found' in output


def test_main_run_no_pipe():
    """Have CT run the tty test code with no pipes."""
    master, slave = os.openpty()
    subprocess.run(CLI + ' ' + get_python_command(CODE_ISATTY),
                   check=True,
                   shell=True,
                   stdin=slave,
                   stdout=slave)

    assert 'stdin=True, stdout=True' in os.read(master, 100).decode()


def test_main_run_pipe_in():
    """Have CT run the tty test code with a pipe on stdin."""
    master, slave = os.openpty()
    subprocess.run(CLI + ' ' + get_python_command(CODE_ISATTY),
                   check=True,
                   shell=True,
                   stdin=subprocess.PIPE,
                   stdout=slave)

    assert 'stdin=True, stdout=True' in os.read(master, 100).decode()


def test_main_run_pipe_in_out():
    """Have CT run the tty test code with pipes on stdin and stdout."""
    result = subprocess.run(CLI + ' ' + get_python_command(CODE_ISATTY),
                            check=True,
                            shell=True,
                            stdin=subprocess.PIPE,
                            stdout=subprocess.PIPE)

    assert 'stdin=True, stdout=True' in result.stdout.decode()


def test_main_run_pipe_out():
    """Have CT run the tty test code with a pipe on stdout."""
    _, slave = os.openpty()
    result = subprocess.run(CLI + ' ' + get_python_command(CODE_ISATTY),
                            check=True,
                            shell=True,
                            stdin=slave,
                            stdout=subprocess.PIPE)

    assert 'stdin=True, stdout=True' in result.stdout.decode()


def test_main_stdin_processing():
    """General stdin processing with relation to READ_SIZE and WAIT_FOR_SPLIT."""
    try:
        stdin_r, stdin_w = os.pipe()
        stdout_r, stdout_w = os.pipe()

        process = subprocess.Popen(CLI,
                                   shell=True,
                                   stdin=stdin_r,
                                   stdout=stdout_w)

        time.sleep(0.5)  # Any startup delay
        assert process.poll() is None

        os.write(stdin_w, b'Hello world\n')
        time.sleep(0.1)  # Any processing delay
        assert os.read(stdout_r, 100) == b'Hello world\n'

        os.write(stdin_w, b'Hey there')
        time.sleep(0.1 + chromaterm.cli.WAIT_FOR_SPLIT)  # Include split wait
        assert os.read(stdout_r, 100) == b'Hey there'

        write_size = chromaterm.cli.READ_SIZE + 1
        os.write(stdin_w, b'x' * write_size)
        time.sleep(0.1 + chromaterm.cli.WAIT_FOR_SPLIT)  # Include split wait
        assert os.read(stdout_r, write_size * 2) == b'x' * write_size
    finally:
        os.close(stdin_r)
        os.close(stdin_w)
        os.close(stdout_r)
        os.close(stdout_w)
        process.kill()
