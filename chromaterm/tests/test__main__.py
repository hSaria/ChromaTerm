'''chromaterm.__main__ tests'''
import itertools
import os
import re
import select
import socket
import stat
import subprocess
import sys
import threading
import time

import psutil
import pytest

import chromaterm
import chromaterm.__main__

# pylint: disable=consider-using-with,too-many-lines

CLI = sys.executable + ' -m chromaterm'

CODE_ISATTY = '''import os, sys
stdin = os.isatty(sys.stdin.fileno())
stdout = os.isatty(sys.stdout.fileno())
print(f'stdin={stdin}, stdout={stdout}')'''

CODE_TTYNAME = '''import os, sys
print(os.ttyname(sys.stdin.fileno()) if os.isatty(sys.stdin.fileno()) else None)'''


def get_python_command(code):
    '''Returns the python shell command that runs `code`.'''
    return sys.executable + f''' -c "{'; '.join(code.splitlines())}"'''


def test_baseline_tty_test_code_no_pipe():
    '''Baseline the test code with no pipes on stdin or stdout.'''
    master, slave = os.openpty()
    subprocess.run(get_python_command(CODE_ISATTY),
                   check=True,
                   shell=True,
                   stdin=slave,
                   stdout=slave)

    assert 'stdin=True, stdout=True' in os.read(master, 100).decode()


def test_baseline_tty_test_code_in_pipe():
    '''Baseline the test code with a pipe on stdin.'''
    master, slave = os.openpty()
    subprocess.run(get_python_command(CODE_ISATTY),
                   check=True,
                   shell=True,
                   stdin=subprocess.PIPE,
                   stdout=slave)

    assert 'stdin=False, stdout=True' in os.read(master, 100).decode()


def test_baseline_tty_test_code_out_pipe():
    '''Baseline the test code with a pipe on stdout.'''
    _, slave = os.openpty()
    result = subprocess.run(get_python_command(CODE_ISATTY),
                            check=True,
                            shell=True,
                            stdin=slave,
                            stdout=subprocess.PIPE)

    assert 'stdin=True, stdout=False' in result.stdout.decode()


def test_baseline_tty_test_code_in_out_pipe():
    '''Baseline the test code with pipes on stdin and stdout.'''
    result = subprocess.run(get_python_command(CODE_ISATTY),
                            check=True,
                            shell=True,
                            stdin=subprocess.PIPE,
                            stdout=subprocess.PIPE)

    assert 'stdin=False, stdout=False' in result.stdout.decode()


def test_baseline_tty_test_code_ttyname_same():
    '''Baseline the ttyname code, ensuring it detects matching ttys.'''
    master, slave = os.openpty()

    subprocess.run(get_python_command(CODE_TTYNAME),
                   check=True,
                   shell=True,
                   stdin=slave,
                   stdout=slave)

    assert os.ttyname(slave) in os.read(master, 100).decode()


def test_baseline_tty_test_code_ttyname_different():
    '''Baseline the ttyname code, ensuring it detects different ttys.'''
    master, slave = os.openpty()
    _, another_slave = os.openpty()

    subprocess.run(get_python_command(CODE_TTYNAME),
                   check=True,
                   shell=True,
                   stdin=slave,
                   stdout=slave)

    assert os.ttyname(another_slave) not in os.read(master, 100).decode()


def test_eprint(capsys):
    '''Print a message to stderr.'''
    msg = 'Some random error message'
    chromaterm.__main__.eprint(msg)
    assert msg in capsys.readouterr().err


def test_get_default_config_location(monkeypatch):
    '''Assert that, if no file is found, the most-specific location is returned.'''
    monkeypatch.setattr(chromaterm.__main__, 'CONFIG_LOCATIONS', ['1', '2'])
    open('2.yml', 'a', encoding='utf-8').close()

    try:
        assert chromaterm.__main__.get_default_config_location() == '2.yml'
    finally:
        os.remove('2.yml')


def test_get_default_config_location_default(monkeypatch):
    '''Assert that, if no file is found, the most-specific location is returned.'''
    monkeypatch.setattr(chromaterm.__main__, 'CONFIG_LOCATIONS', ['1', '2'])
    assert chromaterm.__main__.get_default_config_location() == '1.yml'


def test_get_wait_duration():
    '''New lines in the buffer extend the wait duration.'''
    wait_duration_empty = chromaterm.__main__.get_wait_duration(b'')
    wait_duration_new_line = chromaterm.__main__.get_wait_duration(b'\n')
    assert wait_duration_empty < wait_duration_new_line


def test_load_config_simple():
    '''Parse config with a simple rule.'''
    # pylint: disable=protected-access
    config = chromaterm.__main__.Config()
    chromaterm.__main__.load_config(
        config, '''rules:
    - regex: hello world
      color: f#fffaaa''')

    assert len(config.rules) == 1
    assert config.rules[0].regex == 'hello world'
    assert config.rules[0]._regex_object.pattern == b'hello world'
    assert config.rules[0].color.color == 'f#fffaaa'


def test_load_config_exclusive_order():
    '''Ensure exclusive rules are sorted to top of the list.'''
    config = chromaterm.__main__.Config()
    chromaterm.__main__.load_config(
        config, '''rules:
    - regex: r1
      color: bold
    - regex: r2
      color: bold
      exclusive: True
    - regex: r3
      color: bold
    - regex: r4
      color: bold
      exclusive: True
    ''')

    assert len(config.rules) == 4
    assert config.rules[0].regex == 'r2'
    assert config.rules[1].regex == 'r4'
    assert config.rules[2].regex == 'r1'
    assert config.rules[3].regex == 'r3'


def test_load_config_group():
    '''Parse config with a group-specific rule.'''
    config = chromaterm.__main__.Config()
    chromaterm.__main__.load_config(
        config, '''rules:
    - description: group-specific
      regex: h(el)lo (world)
      color:
        0: bold
        1: b#fffaaa
        2: f#123123''')

    assert len(config.rules) == 1
    assert config.rules[0].description == 'group-specific'
    assert config.rules[0].regex == 'h(el)lo (world)'
    assert config.rules[0].colors[0].color == 'bold'
    assert config.rules[0].colors[1].color == 'b#fffaaa'
    assert config.rules[0].colors[2].color == 'f#123123'


def test_load_config_multiple_colors():
    '''Parse config with a multi-color rule.'''
    config = chromaterm.__main__.Config()
    chromaterm.__main__.load_config(
        config, '''rules:
    - regex: hello (world)
      color: b#fffaaa f#aaafff''')

    assert len(config.rules) == 1
    assert config.rules[0].color.color == 'b#fffaaa f#aaafff'


def test_load_config_missing_rules(capsys):
    '''Parse a config file with the `rules` list missing.'''
    config = chromaterm.__main__.Config()

    chromaterm.__main__.load_config(config, '')
    assert '"rules" list not found' in capsys.readouterr().err


def test_load_config_format_error(capsys):
    '''Parse a config file with format problems.'''
    config = chromaterm.__main__.Config()

    chromaterm.__main__.load_config(config, 'palette: 1')
    assert '"palette" is not a dictionary' in capsys.readouterr().err

    chromaterm.__main__.load_config(config, 'palette:\n  1: 1')
    assert 'Error on palette color' in capsys.readouterr().err

    chromaterm.__main__.load_config(config, 'rules: 1')
    assert '"rules" is not a list' in capsys.readouterr().err

    chromaterm.__main__.load_config(config, 'rules:\n- 1')
    assert 'Rule 1 not a dictionary' in capsys.readouterr().err


def test_load_config_yaml_format_error(capsys):
    '''Parse an incorrect YAML file.'''
    config = chromaterm.__main__.Config()

    chromaterm.__main__.load_config(config, '-x-\nhi:')
    assert 'Parse error:' in capsys.readouterr().err


def test_parse_palette():
    '''Parse a palette.'''
    palette = chromaterm.__main__.parse_palette({
        'red': '#ff0000',
        'blue': '#0000ff'
    })

    assert len(palette.colors) == 2
    assert palette.colors['red'] == '#ff0000'
    assert palette.colors['blue'] == '#0000ff'


def test_parse_palette_invalid_colors():
    '''Parse a palette with invalid colors.'''
    msg = 'Error on palette color 1: 1:'

    palette = {1: 1, 2: 2}
    assert msg in chromaterm.__main__.parse_palette(palette)


def test_parse_rule():
    '''Parse a simple rule.'''
    rule = chromaterm.__main__.parse_rule({
        'description': 'hello',
        'regex': 'hello world',
        'color': 'b#fffaaa',
        'exclusive': True
    })

    assert rule.description == 'hello'
    assert rule.regex == 'hello world'
    assert rule.color.color == 'b#fffaaa'
    assert rule.exclusive


def test_parse_rule_regex_missing():
    '''Parse a rule without a `regex` key.'''
    msg = 'regex must be a string'

    rule = {'color': 'b#fffaaa'}
    assert msg in chromaterm.__main__.parse_rule(rule)


def test_parse_rule_regex_type_error():
    '''Parse a rule with an incorrect `regex` value type.'''
    msg = 'regex must be a string'

    rule = {'regex': ['hi'], 'color': 'b#fffaaa'}
    assert msg in chromaterm.__main__.parse_rule(rule)

    rule = {'regex': 111, 'color': 'b#fffaaa'}
    assert msg in chromaterm.__main__.parse_rule(rule)


def test_parse_rule_regex_invalid():
    '''Parse a rule with an invalid `regex`.'''
    msg = 're.error: '

    rule = {'regex': '+', 'color': 'b#fffaaa'}
    assert msg in chromaterm.__main__.parse_rule(rule)


def test_parse_rule_color_missing():
    '''Parse a rule without a `color` key.'''
    msg = 'color must be a string'

    rule = {'regex': 'x(y)z'}
    assert msg in chromaterm.__main__.parse_rule(rule)


def test_parse_rule_color_type_error():
    '''Parse a rule with an incorrect `color` value type.'''
    msg = 'color must be a string'

    rule = {'regex': 'x(y)z', 'color': ['hi']}
    assert msg in chromaterm.__main__.parse_rule(rule)


def test_parse_rule_color_format_error():
    '''Parse a rule with an incorrect `color` format.'''
    msg = 'invalid color format'

    rule = {'regex': 'x(y)z', 'color': 'b#xyzxyz'}
    assert msg in chromaterm.__main__.parse_rule(rule)

    rule = {'regex': 'x(y)z', 'color': 'x#fffaaa'}
    assert msg in chromaterm.__main__.parse_rule(rule)

    rule = {'regex': 'x(y)z', 'color': 'b@fffaaa'}
    assert msg in chromaterm.__main__.parse_rule(rule)

    rule = {'regex': 'x(y)z', 'color': 'b#fffaaa-f#fffaaa'}
    assert msg in chromaterm.__main__.parse_rule(rule)


def test_parse_rule_exclusive_type_error():
    '''Parse a rule with an incorrect `exclusive` value type.'''
    msg = r'exclusive must be a boolean'

    rule = {'regex': 'x(y)z', 'color': 'bold', 'exclusive': 'hi'}
    assert msg in chromaterm.__main__.parse_rule(rule)


def test_parse_rule_group_index():
    '''Parse a rule that reference a group by its index.'''
    rule = chromaterm.__main__.parse_rule({
        'regex': r'(yo) (hello) (salutations)',
        'color': {
            2: 'b#fffaaa'
        }
    })

    assert rule.colors[2].color == 'b#fffaaa'


def test_parse_rule_group_named():
    '''Parse a rule that reference a group by its name.'''
    rule = chromaterm.__main__.parse_rule({
        'regex': r'(?P<sup>yo) (?P<hi>hello) (?P<greetings>salutations)',
        'color': {
            'hi': 'b#fffaaa'
        }
    })

    # Name is resolved to index
    assert rule.colors[2].color == 'b#fffaaa'


def test_parse_rule_group_type_error():
    '''Parse a rule with an incorrect `group` value type.'''
    msg = 'group must be an integer or a string'

    rule = {'regex': 'x(y)z', 'color': {1.1: 'b#fffaaa'}}
    assert msg in chromaterm.__main__.parse_rule(rule)


def test_parse_rule_group_out_of_bounds():
    '''Parse a rule with `group` number not in the regex.'''
    msg_re = r'regex has .* group\(s\); .* is invalid'

    rule = {'regex': 'x(y)z', 'color': {2: 'b#fffaaa'}}
    assert re.search(msg_re, chromaterm.__main__.parse_rule(rule))


def test_process_input_backspaces(capsys, pcre):
    '''Backspaces in the input should be accounted for when determining if typing
    is in progress.'''
    pipe_r, pipe_w = os.pipe()
    config = chromaterm.__main__.Config()

    rule = chromaterm.Rule('.', chromaterm.Color('bold'), pcre=pcre)
    config.rules.append(rule)

    os.write(pipe_w, b'\b\bxyz')
    chromaterm.__main__.process_input(config, pipe_r, max_wait=0)

    assert capsys.readouterr().out == '\b\bxyz'


def test_process_input_blocking_stdout():
    '''Ensure that `stdout` is put into a blocking state. Otherwise, it triggers
    a `BlockingIOError` if it is not ready to be written to. chromaterm#93.'''
    pipe_r, _ = os.pipe()
    config = chromaterm.__main__.Config()

    os.set_blocking(sys.stdout.fileno(), False)
    assert not os.get_blocking(sys.stdout.fileno())

    chromaterm.__main__.process_input(config, pipe_r, max_wait=0)
    assert os.get_blocking(sys.stdout.fileno())


def test_process_input_buffer_size(capsys, monkeypatch, pcre):
    '''Ensure a limit exists on the size of the buffer.'''
    rule = chromaterm.Rule('heyhey', chromaterm.Color('bold'), pcre=pcre)
    config = chromaterm.__main__.Config()
    config.rules.append(rule)

    patched_read_ready = lambda *_1, **_2: [pipe_r]
    monkeypatch.setattr(chromaterm.__main__, 'read_ready', patched_read_ready)

    pipe_r, pipe_w = os.pipe()
    os.write(pipe_w, b'heyhey')
    os.close(pipe_w)
    chromaterm.__main__.process_input(config, pipe_r)
    assert capsys.readouterr().out == '\x1b[1mheyhey\x1b[22m'

    # Patch READ_SIZE to force the buffer to hit its cap
    monkeypatch.setattr(chromaterm.__main__, 'READ_SIZE', 2)

    pipe_r, pipe_w = os.pipe()
    os.write(pipe_w, b'heyhey')
    os.close(pipe_w)
    chromaterm.__main__.process_input(config, pipe_r)
    assert capsys.readouterr().out == 'heyhey'


def test_process_input_empty(capsys):
    '''Input processing of empty input.'''
    pipe_r, _ = os.pipe()
    config = chromaterm.__main__.Config()

    chromaterm.__main__.process_input(config, pipe_r, max_wait=0)

    assert capsys.readouterr().out == ''


def test_process_input_forward_fd_check(capsys, monkeypatch):
    '''If input is received on `forward_fd` while waiting for data, stop waiting.'''
    config = chromaterm.__main__.Config()
    config.rules.append(chromaterm.Rule('hello', chromaterm.Color('bold')))

    pipe_r, pipe_w = os.pipe()
    trigger = threading.Event()
    call_log = []

    def patched_read_ready(*fds, timeout=None):
        # If timeout is used, then it's the call to check for more data
        if timeout:
            call_log.append(fds)
            return [666]

        # Set the trigger on the second loop
        if call_log:
            trigger.set()

        # The read operation will block while waiting for data
        return [pipe_r]

    monkeypatch.setattr(chromaterm.__main__, 'read_ready', patched_read_ready)

    worker = threading.Thread(target=chromaterm.__main__.process_input,
                              args=(config, pipe_r, 666))
    worker.start()

    try:
        os.write(pipe_w, b'hello')

        # Wait until the second loop
        trigger.wait()

        # Despite read_ready indicating more input becoming available, it is not
        # on the data_fd. Therefore, the buffer is fully processed
        assert capsys.readouterr().out == '\x1b[1mhello\x1b[22m'
        assert len(call_log) == 1
        assert call_log[0] == (pipe_r, 666)
    finally:
        os.close(pipe_w)
        worker.join()


def test_process_input_multibyte_character(capsys, monkeypatch):
    '''A multibyte character shouldn't be split when it falls at the boundary of
    the READ_SIZE.'''
    monkeypatch.setattr(chromaterm.__main__, 'READ_SIZE', 2)

    pipe_r, pipe_w = os.pipe()
    config = chromaterm.__main__.Config()

    os.write(pipe_w, '😀'.encode())
    chromaterm.__main__.process_input(config, pipe_r, max_wait=0)

    assert capsys.readouterr().out == '😀'


def test_process_input_multiline(capsys, pcre):
    '''Input processing with multiple lines of data.'''
    pipe_r, pipe_w = os.pipe()
    config = chromaterm.__main__.Config()

    rule = chromaterm.Rule('hello world', chromaterm.Color('bold'), pcre=pcre)
    config.rules.append(rule)

    os.write(pipe_w, b'\nt hello world t\n' * 2)
    chromaterm.__main__.process_input(config, pipe_r, max_wait=0)

    assert capsys.readouterr().out == '\nt \x1b[1mhello world\x1b[22m t\n' * 2


def test_process_input_partial_control_string(capsys, monkeypatch):
    '''An incomplete control string should not be printed.'''
    pipe_r, pipe_w = os.pipe()
    config = chromaterm.__main__.Config()
    event = threading.Event()

    def patched_read_ready(*_1, timeout=None):
        # If timeout is used, then it's the call to check for more data
        if timeout:
            return []

        event.set()
        return [pipe_r]

    monkeypatch.setattr(chromaterm.__main__, 'read_ready', patched_read_ready)

    worker = threading.Thread(target=chromaterm.__main__.process_input,
                              args=(config, pipe_r))
    worker.start()

    try:
        for end in ['\x07', '\x1b\x5c']:
            for code in ['\x50', '\x58', '\x5d', '\x5e', '\x5f']:
                start = '\x1b' + code

                # Data (printed), followed by the first part (not printed)
                event.clear()
                os.write(pipe_w, b'hello\n' + start.encode() + b'p1')
                event.wait()
                assert capsys.readouterr().out == 'hello\n'

                # Second part (not printed)
                event.clear()
                os.write(pipe_w, b'p2')
                event.wait()
                assert capsys.readouterr().out == ''

                # Final part (printed)
                event.clear()
                os.write(pipe_w, b'p3' + end.encode())
                event.wait()
                assert capsys.readouterr().out == start + 'p1p2p3' + end
    finally:
        os.close(pipe_w)
        worker.join()


def test_process_input_read_size(capsys, pcre):
    '''Input longer than READ_SIZE should not break highlighting.'''
    pipe_r, pipe_w = os.pipe()
    config = chromaterm.__main__.Config()
    write_size = chromaterm.__main__.READ_SIZE + 2

    rule = chromaterm.Rule('x' * write_size,
                           chromaterm.Color('bold'),
                           pcre=pcre)
    config.rules.append(rule)

    os.write(pipe_w, b'x' * write_size)
    chromaterm.__main__.process_input(config, pipe_r, max_wait=0)

    assert capsys.readouterr().out == '\x1b[1m' + 'x' * write_size + '\x1b[22m'


def test_process_input_single_character(capsys, monkeypatch, pcre):
    '''Input processing for a single character. Even with a rule that matches
    single character, the output should not be highlighted as it is typically
    just keyboard input.'''
    pipe_r, pipe_w = os.pipe()
    config = chromaterm.__main__.Config()
    run_once = threading.Event()

    rule = chromaterm.Rule('x', chromaterm.Color('bold'), pcre=pcre)
    config.rules.append(rule)

    os.write(pipe_w, b'x')
    chromaterm.__main__.process_input(config, pipe_r, max_wait=0)

    assert capsys.readouterr().out == 'x'

    # A single character in a trailing chunk is not considered keyboard input
    def patched_read_ready(*_, timeout=None):
        # If timeout is used, then it's the call to check for more data
        if timeout and not run_once.is_set():
            # Write a single character to attempt to trick it into thinking it's keyboard input
            os.write(pipe_w, b'x')
            os.close(pipe_w)
            run_once.set()

        return [pipe_r]

    monkeypatch.setattr(chromaterm.__main__, 'read_ready', patched_read_ready)
    os.write(pipe_w, b'hi')
    chromaterm.__main__.process_input(config, pipe_r, max_wait=0)

    assert capsys.readouterr().out == 'hi\x1b[1mx\x1b[22m'


def test_process_input_socket(capsys):
    '''Use sockets instead of a file descriptors.'''
    config = chromaterm.__main__.Config()
    config.rules.append(chromaterm.Rule('hello', chromaterm.Color('bold')))

    pipe_r = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    pipe_w = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    pipe_w.bind(('127.0.0.1', 0))
    pipe_w.listen(1)
    pipe_r.connect(pipe_w.getsockname())
    pipe_w, _ = pipe_w.accept()

    pipe_w.sendall(b'hello')
    pipe_w.close()
    select.select([pipe_r], [], [])
    chromaterm.__main__.process_input(config, pipe_r)

    assert capsys.readouterr().out == '\x1b[1mhello\x1b[22m'


def test_process_input_trailing_chunk(capsys, pcre):
    '''Ensure that a trailing chunk is joined with the next chunk if the latter
    arrives in time.'''
    pipe_r, pipe_w = os.pipe()
    config = chromaterm.__main__.Config()

    rule = chromaterm.Rule('hello world', chromaterm.Color('bold'), pcre=pcre)
    config.rules.append(rule)

    worker = threading.Thread(target=chromaterm.__main__.process_input,
                              args=(config, pipe_r))
    worker.start()

    # Write data, wait for it to be read, then write some more
    os.write(pipe_w, b'hello ')
    while select.select([pipe_r], [], [], 0)[0]:
        pass
    assert capsys.readouterr().out == ''

    os.write(pipe_w, b'world')

    os.close(pipe_w)
    worker.join()

    assert capsys.readouterr().out == '\x1b[1mhello world\x1b[22m'


def test_read_file():
    '''Read a file.'''
    with open(__name__ + '4', mode='a', encoding='utf-8') as file:
        file.write('hello world')

    try:
        assert chromaterm.__main__.read_file(__name__ + '4') == 'hello world'
    finally:
        os.remove(__name__ + '4')


def test_read_file_no_file(capsys):
    '''Read a non-existent file.'''
    msg = f'Configuration file {repr(__name__ + "1")} not found'
    chromaterm.__main__.read_file(__name__ + '1')
    assert msg in capsys.readouterr().err


def test_read_file_no_permission(capsys):
    '''Create a file with no permissions and attempt to read it. Delete the file
    once done with it.'''
    msg = f'Cannot read configuration file {repr(__name__ + "2")} (permission)'

    os.close(os.open(__name__ + '2', os.O_CREAT | os.O_WRONLY, 0o0000))
    chromaterm.__main__.read_file(__name__ + '2')
    os.chmod(__name__ + '2', stat.S_IWRITE)
    os.remove(__name__ + '2')

    assert msg in capsys.readouterr().err


def test_read_ready_input():
    '''Immediate ready when there is input buffered.'''
    pipe_r, pipe_w = os.pipe()

    os.write(pipe_w, b'Hello world')
    assert chromaterm.__main__.read_ready(pipe_r)


def test_read_ready_timeout_empty():
    '''Wait with no input.'''
    pipe_r, _ = os.pipe()

    before = time.time()
    assert not chromaterm.__main__.read_ready(pipe_r, timeout=0.1)

    after = time.time()
    assert after - before >= 0.1


def test_read_ready_timeout_input():
    '''Immediate ready with timeout when there is input buffered.'''
    pipe_r, pipe_w = os.pipe()

    os.write(pipe_w, b'Hello world')
    before = time.time()
    assert chromaterm.__main__.read_ready(pipe_r, timeout=0.1)

    after = time.time()
    assert after - before < 0.1


def test_split_buffer_printer_commands():
    '''Split based on new lines (\\r, \\n, \\r\\n), vertical space (\\v), and
    form feed (\\f).'''
    for printer_command in [b'\r', b'\n', b'\r\n', b'\v', b'\f']:
        data = b'Hello ' + printer_command + b'World'
        expected = ((b'Hello ', printer_command), (b'World', b''))

        assert chromaterm.__main__.split_buffer(data) == expected


def test_split_buffer_private_control_functions():
    '''Split based on the ECMA-035 private control functions.'''
    for char_id in range(int('30', 16), int('40', 16)):
        data = b'Hello \x1b%c World' % char_id
        expected = ((b'Hello ', b'\x1b%c' % char_id), (b' World', b''))

        assert chromaterm.__main__.split_buffer(data) == expected


def test_split_buffer_c1_set():
    '''Split based on the ECMA-048 C1 set, excluding CSI and control strings.'''
    c1_set = itertools.chain(range(int('40', 16), int('50', 16)),
                             range(int('51', 16), int('58', 16)),
                             range(int('59', 16), int('5b', 16)),
                             (int('5c', 16), ))

    for char_id in c1_set:
        data = b'Hello \x1b%c World' % char_id
        expected = ((b'Hello ', b'\x1b%c' % char_id), (b' World', b''))

        assert chromaterm.__main__.split_buffer(data) == expected


def test_split_buffer_independent_control_functions():
    '''Split based on the ECMA-048 independent control functions.'''
    for escape in (b'\x1b', b'\x1b\x23'):
        for char_id in range(int('60', 16), int('7f', 16)):
            code = escape + b'%c' % char_id
            data = b'Hello ' + code + b' World'
            expected = ((b'Hello ', code), (b' World', b''))

            assert chromaterm.__main__.split_buffer(data) == expected


def test_split_buffer_csi_exclude_sgr():
    '''Fail to split based on the ECMA-048 C1 CSI SGR. Added some intermediate
    characters to prevent matching other CSI codes; strictly checking empty SGR.'''
    data = b'Hello \x1b[!0World'
    expected = ((b'Hello \x1b[!0World', b''), )

    assert chromaterm.__main__.split_buffer(data) == expected


def test_split_buffer_csi_no_parameter_no_intermediate():
    '''Split based on CSI with no parameter or intermediate bytes.'''
    csi_up_to_sgr = range(int('40', 16), int('6d', 16))
    csi_above_sgr = range(int('6e', 16), int('7f', 16))

    for char_id in itertools.chain(csi_up_to_sgr, csi_above_sgr):
        data = b'Hello \x1b[%c World' % char_id
        expected = ((b'Hello ', b'\x1b[%c' % char_id), (b' World', b''))

        assert chromaterm.__main__.split_buffer(data) == expected


def test_split_buffer_csi_no_parameter_intermediate():
    '''Split based on CSI with intermediate bytes but no parameter bytes.'''
    csi_up_to_sgr = range(int('40', 16), int('6d', 16))
    csi_above_sgr = range(int('6e', 16), int('7f', 16))

    for char_id in itertools.chain(csi_up_to_sgr, csi_above_sgr):
        for intermediate in range(int('20', 16), int('30', 16)):
            for count in range(1, 4):
                code = (b'%c' % intermediate) * count + b'%c' % char_id
                data = b'Hello \x1b[' + code + b' World'
                expected = ((b'Hello ', b'\x1b[' + code), (b' World', b''))

                assert chromaterm.__main__.split_buffer(data) == expected


def test_split_buffer_csi_parameter_intermediate():
    '''Split based on CSI with parameter and intermediate bytes. Up to 3 bytes
    each.'''
    csi_up_to_sgr = range(int('40', 16), int('6d', 16))
    csi_above_sgr = range(int('6e', 16), int('7f', 16))

    for char_id in itertools.chain(csi_up_to_sgr, csi_above_sgr):
        for parameter in range(int('30', 16), int('40', 16)):
            for intermediate in range(int('20', 16), int('30', 16)):
                for count in range(1, 4):
                    code = ((b'%c' % parameter) * count +
                            (b'%c' % intermediate) * count + b'%c' % char_id)
                    data = b'Hello \x1b[' + code + b' World'
                    expected = ((b'Hello ', b'\x1b[' + code), (b' World', b''))

                    assert chromaterm.__main__.split_buffer(data) == expected


def test_split_buffer_csi_parameter_no_intermediate():
    '''Split based on CSI with parameters bytes but no intermediate bytes. Up to
    3 bytes.'''
    csi_up_to_sgr = range(int('40', 16), int('6d', 16))
    csi_above_sgr = range(int('6e', 16), int('7f', 16))

    for char_id in itertools.chain(csi_up_to_sgr, csi_above_sgr):
        for parameter in range(int('30', 16), int('40', 16)):
            for count in range(1, 4):
                code = (b'%c' % parameter) * count + b'%c' % char_id
                data = b'Hello \x1b[' + code + b' World'
                expected = ((b'Hello ', b'\x1b[' + code), (b' World', b''))

                assert chromaterm.__main__.split_buffer(data) == expected


def test_split_buffer_osc_title():
    '''Operating System Command (OSC) can supply arbitrary commands within the
    visible character set.'''
    for end in [b'\x07', b'\x1b\x5c']:
        # Includes a smiley-face
        osc = b'\x1b]Ignored\xf0\x9f\x98\x80' + end
        data = osc + b'Hello world'
        expected = ((b'', osc), (b'Hello world', b''))

        assert chromaterm.__main__.split_buffer(data) == expected


def test_split_buffer_scs():
    '''Select Character Set (SCS) is used to change the terminal character set.'''
    g_sets = [b'\x28', b'\x29', b'\x2a', b'\x2b', b'\x2d', b'\x2e', b'\x2f']
    char_sets = range(int('20', 16), int('7e', 16))

    for g_set in g_sets:
        for char_set in char_sets:
            code = b'\x1b%c%c' % (g_set, char_set)
            data = b'Hello ' + code + b' World'
            expected = ((b'Hello ', code), (b' World', b''))

            assert chromaterm.__main__.split_buffer(data) == expected


def test_main_benchmark():
    '''Ensure the benchmark table is printed to stderr.'''
    result = subprocess.run(CLI + ' --benchmark echo hello',
                            check=False,
                            shell=True,
                            stderr=subprocess.PIPE)

    assert b'Benchmark results' in result.stderr


def test_main_broken_pipe():
    '''Break a pipe while CT is trying to write to it. ChromaTerm should exit
    gracefully, even if the input data isn't complete.'''
    commands = [
        'yes "Hello" | ' + CLI + ' | head -3',
        CLI + ' yes "Hello" | head -3',
    ]

    for command in commands:
        result = subprocess.run(command,
                                check=False,
                                shell=True,
                                stdout=subprocess.PIPE,
                                stderr=subprocess.PIPE)

        assert b'Broken pipe' not in result.stderr
        assert len(result.stdout.splitlines()) == 3


def test_main_buffer_close_time():
    '''Confirm that the program exists as soon as stdin closes.'''
    before = time.time()
    subprocess.run('echo hi | ' + CLI, check=True, shell=True)
    after = time.time()

    assert after - before < 1


@pytest.mark.skipif(sys.platform != 'darwin',
                    reason='Not all CI environments support tracking')
def test_main_cwd_tracking():
    '''The `cwd` of ChromaTerm should match that of the spawned process.'''
    test_script = '''import os, time
    os.chdir('../')
    time.sleep(2)'''
    command = CLI + ' ' + get_python_command(test_script)

    with subprocess.Popen(command, shell=True) as process:
        time.sleep(1)
        assert psutil.Process(process.pid).cwd() != os.getcwd()


def test_main_default_config():
    '''Generate default config file in the home directory if it doesn't exist.'''
    env = os.environ.copy()
    env['HOME'] = os.path.dirname(os.path.abspath(__file__))

    assert not os.path.isfile(env['HOME'] + '/.chromaterm.yml')

    try:
        subprocess.run(CLI + ' echo', check=True, env=env, shell=True)

        assert os.path.isfile(env['HOME'] + '/.chromaterm.yml')
    finally:
        os.remove(env['HOME'] + '/.chromaterm.yml')


def test_main_exit_code():
    '''The exit code of the child process should be the exit code of ChromaTerm.'''
    command = CLI + ' ' + get_python_command('import sys; sys.exit(3)')

    assert subprocess.run(command, check=False, shell=True).returncode == 3


def test_main_reload_config(pcre):
    '''Reload the configuration while the program is running.'''
    try:
        # The initial configuration file
        with open(__name__ + '3', 'w', encoding='utf-8') as file:
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

        command = ' --rgb --config ' + __name__ + '3'
        process = subprocess.Popen(CLI + (' --pcre' if pcre else '') + command,
                                   shell=True,
                                   stdin=stdin_r,
                                   stdout=stdout_w)

        os.write(stdin_w, b'Hello world\n')
        assert select.select([stdout_r], [], [], 1)[0]
        assert repr(os.read(stdout_r, 100).decode()) == repr(''.join(expected))

        # Create file without the 'world' rule
        os.remove(__name__ + '3')
        with open(__name__ + '3', 'w', encoding='utf-8') as file:
            file.write('''rules:
            - regex: Hello
              color: f#123123''')

        expected = ['\x1b[38;2;18;49;35m', 'Hello', '\x1b[39m', ' world\n']

        # Reload config
        subprocess.run(CLI + ' --reload', check=False, shell=True)

        os.write(stdin_w, b'Hello world\n')
        assert select.select([stdout_r], [], [], 1)[0]
        assert repr(os.read(stdout_r, 100).decode()) == repr(''.join(expected))

        process.kill()
    finally:
        os.close(stdin_r)
        os.close(stdin_w)
        os.close(stdout_r)
        os.close(stdout_w)
        os.remove(__name__ + '3')


def test_main_reload_processes():
    '''Reload all other CT processes.'''
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
    '''Ensure that CT spawns the child in a pseudo-terminal.'''
    master, slave = os.openpty()
    subprocess.run(CLI + ' ' + get_python_command(CODE_TTYNAME),
                   check=True,
                   shell=True,
                   stdin=slave,
                   stdout=slave)

    assert os.ttyname(slave) not in os.read(master, 100).decode()


def test_main_run_no_file_found():
    '''Have CT run with an unavailable command.'''
    result = subprocess.run(CLI + ' plz-no-work',
                            check=False,
                            shell=True,
                            stdout=subprocess.PIPE)

    output = re.sub(br'\x1b\[[\d;]+?m', b'', result.stdout)
    assert b'no such file or directory: plz-no-work' in output


def test_main_run_no_pipe():
    '''Have CT run the tty test code with no pipes.'''
    master, slave = os.openpty()
    subprocess.run(CLI + ' ' + get_python_command(CODE_ISATTY),
                   check=True,
                   shell=True,
                   stdin=slave,
                   stdout=slave)

    assert 'stdin=True, stdout=True' in os.read(master, 100).decode()


def test_main_run_pipe_in():
    '''Have CT run the tty test code with a pipe on stdin.'''
    master, slave = os.openpty()
    subprocess.run(CLI + ' ' + get_python_command(CODE_ISATTY),
                   check=True,
                   shell=True,
                   stdin=subprocess.PIPE,
                   stdout=slave)

    assert 'stdin=True, stdout=True' in os.read(master, 100).decode()


def test_main_run_pipe_in_out():
    '''Have CT run the tty test code with pipes on stdin and stdout.'''
    result = subprocess.run(CLI + ' ' + get_python_command(CODE_ISATTY),
                            check=True,
                            shell=True,
                            stdin=subprocess.PIPE,
                            stdout=subprocess.PIPE)

    assert 'stdin=True, stdout=True' in result.stdout.decode()


def test_main_run_pipe_out():
    '''Have CT run the tty test code with a pipe on stdout.'''
    _, slave = os.openpty()
    result = subprocess.run(CLI + ' ' + get_python_command(CODE_ISATTY),
                            check=True,
                            shell=True,
                            stdin=slave,
                            stdout=subprocess.PIPE)

    assert 'stdin=True, stdout=True' in result.stdout.decode()
