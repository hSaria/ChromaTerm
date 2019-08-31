#!/usr/bin/env python3
"""Tests for the main program."""

import os
import re
import socket
import threading
import time

import chromaterm

CONFIG_RULE_SIMPLE = '''rules:
- description: simple
  regex: hello world
  color: f#fffaaa'''

CONFIG_RULE_GROUP = '''rules:
- description: group
  regex: hello (world)
  color: b#fffaaa
  group: 1'''

CONFIG_RULE_MULTIPLE_COLORS = '''rules:
- description: group
  regex: hello (world)
  color: b#fffaaa
  group: 1'''

CONFIG_RULE_ERROR = '''rules:
- description: simple'''

FILE_FAKE = '.test_chromaterm.yml'


def test_args_init():
    """Parse program arguments."""
    args = chromaterm.args_init(['--config', 'hello'])
    assert args.config == 'hello'


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
        assert chromaterm.get_color_code(color) == '\033[' + code


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
        assert chromaterm.get_color_code(color) == '\033[' + code


def test_parse_config_simple():
    """Parse a config file with a simple rule."""
    assert chromaterm.parse_config(CONFIG_RULE_SIMPLE)['rules']


def test_parse_config_group():
    """Parse a config file with a group-specific rule."""
    assert chromaterm.parse_config(CONFIG_RULE_GROUP)['rules']


def test_parse_config_multiple_colors():
    """Parse a config file with a multi-color rule."""
    assert chromaterm.parse_config(CONFIG_RULE_MULTIPLE_COLORS)['rules']


def test_parse_config_rule_format_error(capsys):
    """Parse a config file with a syntax problem."""
    chromaterm.parse_config(CONFIG_RULE_ERROR)
    assert 'Rule error on' in capsys.readouterr().err


def test_parse_config_yaml_format_error(capsys):
    """Parse an incorrect YAML file."""
    chromaterm.parse_config('-x-\nhi:')
    assert 'Parse error:' in capsys.readouterr().err


def test_parse_rule_regex_missing():
    """Parse a rule without a `regex` key."""
    msg = 'regex not found'
    rule = {'color': 'b#fffaaa'}
    assert chromaterm.parse_rule(rule, None) == msg


def test_parse_rule_regex_type_error():
    """Parse a rule with an incorrect `regex` value type."""
    msg = 'regex not a string or integer'
    rule = {'regex': ['hi'], 'color': 'b#fffaaa'}
    assert chromaterm.parse_rule(rule, None) == msg


def test_parse_rule_regex_invalid():
    """Parse a rule with an invalid `regex`."""
    rule = {'regex': '+', 'color': 'b#fffaaa'}
    assert 're.error: ' in chromaterm.parse_rule(rule, None)


def test_parse_rule_color_missing():
    """Parse a rule without a `color` key."""
    msg = 'color not found'
    rule = {'regex': 'x(y)z'}
    assert chromaterm.parse_rule(rule, None) == msg


def test_parse_rule_color_type_error():
    """Parse a rule with an incorrect `color` value type."""
    msg = 'color not a string'
    rule = {'regex': 'x(y)z', 'color': ['hi']}
    assert chromaterm.parse_rule(rule, None) == msg


def test_parse_rule_color_format_error():
    """Parse a rule with an incorrect `color` format."""
    msg = 'color not in the correct format'

    rule = {'regex': 'x(y)z', 'color': 'b#xyzxyz'}
    assert chromaterm.parse_rule(rule, None) == msg

    rule = {'regex': 'x(y)z', 'color': 'x#fffaaa'}
    assert chromaterm.parse_rule(rule, None) == msg

    rule = {'regex': 'x(y)z', 'color': 'b@fffaaa'}
    assert chromaterm.parse_rule(rule, None) == msg

    rule = {'regex': 'x(y)z', 'color': 'b#fffaaa-f#fffaaa'}
    assert chromaterm.parse_rule(rule, None) == msg


def test_parse_rule_group_type_error():
    """Parse a rule with an incorrect `group` value type."""
    msg = 'group not an integer'
    rule = {'regex': 'x(y)z', 'color': 'b#fffaaa', 'group': 'hi'}
    assert chromaterm.parse_rule(rule, None) == msg


def test_parse_rule_group_out_of_bounds():
    """Parse a rule with `group` number not in the regex."""
    msg = 'group ID over the number of groups in the regex'
    rule = {'regex': 'x(y)z', 'color': 'b#fffaaa', 'group': 2}
    assert chromaterm.parse_rule(rule, None) == msg


def test_process_buffer_empty(capsys):
    """Output processing of empty input."""
    config = {'rules': [], 'reset_string': '\033[0m'}
    chromaterm.process_buffer(config, '', False)
    assert capsys.readouterr().out == ''


def test_process_buffer_more(capsys):
    """Output processing of empty input."""
    config = {'rules': [], 'reset_string': '\033[0m'}
    chromaterm.process_buffer(config, '', False)
    assert capsys.readouterr().out == ''


def test_process_buffer_multiple_lines(capsys):
    """Output processing with multiple lines of input."""
    config = {'rules': [], 'reset_string': '\033[0m'}
    rule = {'regex': 'hello world', 'color': 'b#fffaaa'}

    config['rules'].append(chromaterm.parse_rule(rule, config))
    data = 'test hello world test\n'
    success = r'^test \033\[[34]8;5;[0-9]{1,3}mhello world\033\[0m test$'

    chromaterm.process_buffer(config, data * 2, False)
    captured = capsys.readouterr()

    for line in [x for x in captured.out.splitlines() if x]:
        assert re.search(success, line)


def test_process_buffer_rule_simple(capsys):
    """Output processing with a simple rule."""
    config = {'rules': [], 'reset_string': '\033[0m'}
    rule = {'regex': 'hello world', 'color': 'b#fffaaa'}

    config['rules'].append(chromaterm.parse_rule(rule, config))
    data = 'test hello world test'
    success = r'^test \033\[[34]8;5;[0-9]{1,3}mhello world\033\[0m test$'

    chromaterm.process_buffer(config, data, False)
    captured = capsys.readouterr()

    assert re.search(success, captured.out)


def test_process_buffer_rule_group(capsys):
    """Output processing with a group-specific rule."""
    config = {'rules': [], 'reset_string': '\033[0m'}
    rule = {'regex': 'hello (world)', 'color': 'b#fffaaa', 'group': 1}

    config['rules'].append(chromaterm.parse_rule(rule, config))
    data = 'test hello world test'
    success = r'^test hello \033\[[34]8;5;[0-9]{1,3}mworld\033\[0m test$'

    chromaterm.process_buffer(config, data, False)
    captured = capsys.readouterr()

    assert re.search(success, captured.out)


def test_process_buffer_rule_multiple_colors(capsys):
    """Output processing with a multi-color rule."""
    config = {'rules': [], 'reset_string': '\033[0m'}
    rule = {'regex': 'hello world', 'color': 'b#fffaaa f#aaafff'}

    config['rules'].append(chromaterm.parse_rule(rule, config))
    data = 'test hello world test'
    success = r'^test (\033\[[34]8;5;[0-9]{1,3}m){2}hello world\033\[0m test$'

    chromaterm.process_buffer(config, data, False)
    captured = capsys.readouterr()

    assert re.search(success, captured.out)


def test_read_file():
    """Read the default configuration."""
    assert chromaterm.read_file('.chromaterm.yml') is not None


def test_read_file_non_existent(capsys):
    """Read a non-existent file."""
    msg = 'Configuration file ' + FILE_FAKE + ' not found\n'
    chromaterm.read_file(FILE_FAKE)
    assert msg in capsys.readouterr().err


def test_read_file_no_permission(capsys):
    """Create a file with no permissions and attempt to read it. Delete the file
    once done with it."""
    msg = 'Cannot read configuration file ' + FILE_FAKE + ' (permission)\n'

    os.close(os.open(FILE_FAKE, os.O_CREAT | os.O_WRONLY, 0o0000))
    chromaterm.read_file(FILE_FAKE)
    os.remove(FILE_FAKE)

    assert msg in capsys.readouterr().err


def test_read_ready_input(monkeypatch):
    """Immediate ready when there is input buffered."""
    try:
        s_sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        c_sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        s_sock.bind(FILE_FAKE)
        s_sock.listen(2)
        c_sock.connect(FILE_FAKE)
        s_conn, _ = s_sock.accept()

        monkeypatch.setattr('sys.stdin', c_sock)

        s_conn.sendall(b'Hello world')
        assert chromaterm.read_ready()
    finally:
        s_conn.close()
        c_sock.close()
        s_sock.close()
        os.remove(FILE_FAKE)


def test_read_ready_timeout_empty(monkeypatch):
    """Wait for 1 second with no input."""
    try:
        s_sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        s_sock.bind(FILE_FAKE)
        s_sock.listen(2)
        c_sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        c_sock.connect(FILE_FAKE)
        s_conn, _ = s_sock.accept()

        monkeypatch.setattr('sys.stdin', c_sock)

        before = time.time()
        assert not chromaterm.read_ready(1)

        after = time.time()
        assert after - before >= 1
    finally:
        s_conn.close()
        c_sock.close()
        s_sock.close()
        os.remove(FILE_FAKE)


def test_read_ready_timeout_input(monkeypatch):
    """Immediate ready with timeout when there is input buffered."""
    try:
        s_sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        s_sock.bind(FILE_FAKE)
        s_sock.listen(2)
        c_sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        c_sock.connect(FILE_FAKE)
        s_conn, _ = s_sock.accept()

        monkeypatch.setattr('sys.stdin', c_sock)

        s_conn.sendall(b'Hello world')
        before = time.time()
        assert chromaterm.read_ready(1)

        after = time.time()
        assert after - before < 1
    finally:
        s_conn.close()
        c_sock.close()
        s_sock.close()
        os.remove(FILE_FAKE)


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
    """Test stdin processing"""
    try:
        # Will auto-shutdown once "stdin" is closed
        args = chromaterm.args_init([])
        main_thread = threading.Thread(target=chromaterm.main, args=(args, 10))

        s_sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        s_sock.bind(FILE_FAKE)
        s_sock.listen(2)
        c_sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        c_sock.connect(FILE_FAKE)
        s_conn, _ = s_sock.accept()

        monkeypatch.setattr('sys.stdin', c_sock)

        main_thread.start()
        time.sleep(0.5)  # Any start-up delay
        assert main_thread.is_alive()

        s_conn.sendall(b'Hello world\n')
        time.sleep(0.2)  # Any processing delay
        assert capsys.readouterr().out == 'Hello world\n'

        s_conn.sendall(b'Hey there')
        time.sleep(0.2 + chromaterm.WAIT_FOR_NEW_LINE)  # Include new-line wait
        assert capsys.readouterr().out == 'Hey there'

        s_conn.sendall(b'x' * (chromaterm.READ_SIZE + 1))
        time.sleep(0.2 + chromaterm.WAIT_FOR_NEW_LINE)  # Include new-line wait
        assert capsys.readouterr().out == 'x' * (chromaterm.READ_SIZE + 1)
    finally:
        s_conn.close()
        c_sock.close()
        s_sock.close()
        os.remove(FILE_FAKE)
        main_thread.join()
