"""chromaterm.__main__ tests"""
import itertools
import os
import re
import select
import stat
import subprocess
import sys
import threading
import time

import chromaterm
import chromaterm.__main__

# pylint: disable=too-many-lines, consider-using-with

CLI = sys.executable + ' -m chromaterm'

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


def test_eprint(capsys):
    """Print a message to stderr."""
    msg = 'Some random error message'
    chromaterm.__main__.eprint(msg)
    assert msg in capsys.readouterr().err


def test_get_default_config_location(monkeypatch):
    """Assert that, if no file is found, the most-specific location is returned."""
    monkeypatch.setattr(chromaterm.__main__, 'CONFIG_LOCATIONS', ['1', '2'])
    open('2.yml', 'a', encoding='utf-8').close()

    try:
        assert chromaterm.__main__.get_default_config_location() == '2.yml'
    finally:
        os.remove('2.yml')


def test_get_default_config_location_default(monkeypatch):
    """Assert that, if no file is found, the most-specific location is returned."""
    monkeypatch.setattr(chromaterm.__main__, 'CONFIG_LOCATIONS', ['1', '2'])
    assert chromaterm.__main__.get_default_config_location() == '1.yml'


def test_get_wait_duration():
    """The delay is minimum when on normal input."""
    wait_duration = chromaterm.__main__.get_wait_duration('')
    assert wait_duration == chromaterm.__main__.INPUT_WAIT_MIN


def test_get_wait_duration_bounds(monkeypatch):
    """Ensure the wait duration cannot exceed the upper bound."""
    monkeypatch.setattr(chromaterm.__main__, 'INPUT_WAIT_MIN', 10)

    wait_duration = chromaterm.__main__.get_wait_duration('')
    assert wait_duration == chromaterm.__main__.INPUT_WAIT_MAX


def test_get_wait_duration_buffer_new_lines():
    """New lines in the buffer extend the wait duration."""
    wait_duration_0 = chromaterm.__main__.get_wait_duration('\n' * 0)
    wait_duration_1 = chromaterm.__main__.get_wait_duration('\n' * 1)
    wait_duration_2 = chromaterm.__main__.get_wait_duration('\n' * 2)
    assert wait_duration_0 < wait_duration_1 < wait_duration_2


def test_load_rules_simple():
    """Parse config with a simple rule."""
    config = chromaterm.__main__.Config()
    chromaterm.__main__.load_rules(
        config, '''rules:
    - regex: hello world
      color: f#fffaaa''')

    assert len(config.rules) == 1


def test_load_rules_group():
    """Parse config with a group-specific rule."""
    config = chromaterm.__main__.Config()
    chromaterm.__main__.load_rules(
        config, '''rules:
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


def test_load_rules_multiple_colors():
    """Parse config with a multi-color rule."""
    config = chromaterm.__main__.Config()
    chromaterm.__main__.load_rules(
        config, '''rules:
    - regex: hello (world)
      color: b#fffaaa f#aaafff''')

    assert len(config.rules) == 1
    assert config.rules[0].color.color == 'b#fffaaa f#aaafff'


def test_load_rules_missing(capsys):
    """Parse a config file with the `rules` list missing."""
    config = chromaterm.__main__.Config()
    chromaterm.__main__.load_rules(config, '')

    assert '"rules" list not found' in capsys.readouterr().err


def test_load_rules_format_error(capsys):
    """Parse a config file with a syntax problem."""
    config = chromaterm.__main__.Config()
    chromaterm.__main__.load_rules(config, '''rules:
    - 1''')

    assert 'Rule 1 not a dictionary' in capsys.readouterr().err

    chromaterm.__main__.load_rules(config, 'rules: 1')

    assert '"rules" is not a list' in capsys.readouterr().err


def test_load_rules_yaml_format_error(capsys):
    """Parse an incorrect YAML file."""
    config = chromaterm.__main__.Config()
    chromaterm.__main__.load_rules(config, '-x-\nhi:')

    assert 'Parse error:' in capsys.readouterr().err


def test_parse_rule_regex_missing():
    """Parse a rule without a `regex` key."""
    msg = 'regex must be a string'

    rule = {'color': 'b#fffaaa'}
    assert msg in chromaterm.__main__.parse_rule(rule)


def test_parse_rule_regex_type_error():
    """Parse a rule with an incorrect `regex` value type."""
    msg = 'regex must be a string'

    rule = {'regex': ['hi'], 'color': 'b#fffaaa'}
    assert msg in chromaterm.__main__.parse_rule(rule)

    rule = {'regex': 111, 'color': 'b#fffaaa'}
    assert msg in chromaterm.__main__.parse_rule(rule)


def test_parse_rule_regex_invalid():
    """Parse a rule with an invalid `regex`."""
    msg = 're.error: '

    rule = {'regex': '+', 'color': 'b#fffaaa'}
    assert msg in chromaterm.__main__.parse_rule(rule)


def test_parse_rule_color_missing():
    """Parse a rule without a `color` key."""
    msg_re = r'color .* is not a string'

    rule = {'regex': 'x(y)z'}
    assert re.search(msg_re, chromaterm.__main__.parse_rule(rule))


def test_parse_rule_color_type_error():
    """Parse a rule with an incorrect `color` value type."""
    msg_re = r'color .* is not a string'

    rule = {'regex': 'x(y)z', 'color': ['hi']}
    assert re.search(msg_re, chromaterm.__main__.parse_rule(rule))


def test_parse_rule_color_format_error():
    """Parse a rule with an incorrect `color` format."""
    msg = 'invalid color format'

    rule = {'regex': 'x(y)z', 'color': 'b#xyzxyz'}
    assert msg in chromaterm.__main__.parse_rule(rule)

    rule = {'regex': 'x(y)z', 'color': 'x#fffaaa'}
    assert msg in chromaterm.__main__.parse_rule(rule)

    rule = {'regex': 'x(y)z', 'color': 'b@fffaaa'}
    assert msg in chromaterm.__main__.parse_rule(rule)

    rule = {'regex': 'x(y)z', 'color': 'b#fffaaa-f#fffaaa'}
    assert msg in chromaterm.__main__.parse_rule(rule)


def test_parse_rule_group_type_error():
    """Parse a rule with an incorrect `group` value type."""
    msg = 'group must be an integer'

    rule = {'regex': 'x(y)z', 'color': {'1': 'b#fffaaa'}}
    assert msg in chromaterm.__main__.parse_rule(rule)


def test_parse_rule_group_out_of_bounds():
    """Parse a rule with `group` number not in the regex."""
    msg_re = r'regex only has .* group\(s\); .* is invalid'

    rule = {'regex': 'x(y)z', 'color': {2: 'b#fffaaa'}}
    assert re.search(msg_re, chromaterm.__main__.parse_rule(rule))


def test_process_input_blocking_stdout():
    """Ensure that `stdout` is put into a blocking state. Otherwise, it triggers
    a `BlockingIOError` if it is not ready to be written to. chromaterm#93."""
    pipe_r, _ = os.pipe()
    config = chromaterm.__main__.Config()

    os.set_blocking(sys.stdout.fileno(), False)
    assert not os.get_blocking(sys.stdout.fileno())

    chromaterm.__main__.process_input(config, pipe_r, max_wait=0)
    assert os.get_blocking(sys.stdout.fileno())


def test_process_input_decode_error(capsys):
    """Attempt to decode a character that is not UTF-8."""
    pipe_r, pipe_w = os.pipe()
    config = chromaterm.__main__.Config()

    os.write(pipe_w, b'\x80')
    chromaterm.__main__.process_input(config, pipe_r, max_wait=0)

    assert capsys.readouterr().out == '�'


def test_process_input_empty(capsys):
    """Input processing of empty input."""
    pipe_r, _ = os.pipe()
    config = chromaterm.__main__.Config()

    chromaterm.__main__.process_input(config, pipe_r, max_wait=0)

    assert capsys.readouterr().out == ''


def test_process_input_multiline(capsys):
    """Input processing with multiple lines of data."""
    pipe_r, pipe_w = os.pipe()
    config = chromaterm.__main__.Config()

    rule = chromaterm.Rule('hello world', color=chromaterm.Color('bold'))
    config.add_rule(rule)

    os.write(pipe_w, b'\nt hello world t\n' * 2)
    chromaterm.__main__.process_input(config, pipe_r, max_wait=0)

    assert capsys.readouterr().out == '\nt \x1b[1mhello world\x1b[22m t\n' * 2


def test_process_input_read_size(capsys):
    """Input longer than READ_SIZE should not break highlighting."""
    pipe_r, pipe_w = os.pipe()
    config = chromaterm.__main__.Config()
    write_size = chromaterm.__main__.READ_SIZE + 1

    rule = chromaterm.Rule('x' * write_size, color=chromaterm.Color('bold'))
    config.add_rule(rule)

    os.write(pipe_w, b'x' * write_size)
    chromaterm.__main__.process_input(config, pipe_r, max_wait=0)

    assert capsys.readouterr().out == '\x1b[1m' + 'x' * write_size + '\x1b[22m'


def test_process_input_single_character(capsys):
    """Input processing for a single character. Even with a rule that matches
    single character, the output should not be highlighted as it is typically
    just keyboard input."""
    pipe_r, pipe_w = os.pipe()
    config = chromaterm.__main__.Config()

    rule = chromaterm.Rule('.', color=chromaterm.Color('bold'))
    config.add_rule(rule)

    os.write(pipe_w, b'x')
    chromaterm.__main__.process_input(config, pipe_r, max_wait=0)

    assert capsys.readouterr().out == 'x'


def test_process_input_trailing_chunk(capsys):
    """Ensure that a trailing chunk is joined with the next chunk if the latter
    arrives in time."""
    pipe_r, pipe_w = os.pipe()
    config = chromaterm.__main__.Config()

    rule = chromaterm.Rule('hello world', color=chromaterm.Color('bold'))
    config.add_rule(rule)

    worker = threading.Thread(target=chromaterm.__main__.process_input,
                              args=(config, pipe_r))
    worker.start()

    # Write data, wait for it to be read, then write some more
    os.write(pipe_w, b'hello ')
    while select.select([pipe_r], [], [], 0)[0]:
        pass
    os.write(pipe_w, b'world')

    os.close(pipe_w)
    worker.join()

    assert capsys.readouterr().out == '\x1b[1mhello world\x1b[22m'


def test_read_file():
    """Read the default configuration file."""
    file = os.path.join(os.path.expanduser('~'), '.chromaterm.yml')
    assert chromaterm.__main__.read_file(file) is not None


def test_read_file_no_file(capsys):
    """Read a non-existent file."""
    msg = 'Configuration file ' + __name__ + '1' + ' not found\n'
    chromaterm.__main__.read_file(__name__ + '1')
    assert msg in capsys.readouterr().err


def test_read_file_no_permission(capsys):
    """Create a file with no permissions and attempt to read it. Delete the file
    once done with it."""
    msg = 'Cannot read configuration file ' + __name__ + '2' + ' (permission)\n'

    os.close(os.open(__name__ + '2', os.O_CREAT | os.O_WRONLY, 0o0000))
    chromaterm.__main__.read_file(__name__ + '2')
    os.chmod(__name__ + '2', stat.S_IWRITE)
    os.remove(__name__ + '2')

    assert msg in capsys.readouterr().err


def test_read_ready_input():
    """Immediate ready when there is input buffered."""
    pipe_r, pipe_w = os.pipe()

    os.write(pipe_w, b'Hello world')
    assert chromaterm.__main__.read_ready(pipe_r)


def test_read_ready_timeout_empty():
    """Wait with no input."""
    pipe_r, _ = os.pipe()

    before = time.time()
    assert not chromaterm.__main__.read_ready(pipe_r, timeout=0.1)

    after = time.time()
    assert after - before >= 0.1


def test_read_ready_timeout_input():
    """Immediate ready with timeout when there is input buffered."""
    pipe_r, pipe_w = os.pipe()

    os.write(pipe_w, b'Hello world')
    before = time.time()
    assert chromaterm.__main__.read_ready(pipe_r, timeout=0.1)

    after = time.time()
    assert after - before < 0.1


def test_split_buffer_new_line_r():
    """Split based on \\r"""
    data = 'Hello \rWorld'
    expected = (('Hello ', '\r'), ('World', ''))

    assert repr(chromaterm.__main__.split_buffer(data)) == repr(expected)


def test_split_buffer_new_line_r_n():
    """Split based on \\r\\n"""
    data = 'Hello \r\n World'
    expected = (('Hello ', '\r\n'), (' World', ''))

    assert repr(chromaterm.__main__.split_buffer(data)) == repr(expected)


def test_split_buffer_new_line_n():
    """Split based on \\n"""
    data = 'Hello \n World'
    expected = (('Hello ', '\n'), (' World', ''))

    assert repr(chromaterm.__main__.split_buffer(data)) == repr(expected)


def test_split_buffer_vertical_space():
    """Split based on \\v"""
    data = 'Hello \v World'
    expected = (('Hello ', '\v'), (' World', ''))

    assert repr(chromaterm.__main__.split_buffer(data)) == repr(expected)


def test_split_buffer_form_feed():
    """Split based on \\f"""
    data = 'Hello \f World'
    expected = (('Hello ', '\f'), (' World', ''))

    assert repr(chromaterm.__main__.split_buffer(data)) == repr(expected)


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

        assert repr(chromaterm.__main__.split_buffer(data)) == repr(expected)


def test_split_buffer_csi_exclude_sgr():
    """Fail to split based on the ECMA-048 C1 CSI SGR. Added some intermediate
    characters to prevent matching other CSI codes; strictly checking empty SGR."""
    data = 'Hello \x1b[!0World'
    expected = (('Hello \x1b[!0World', ''), )

    assert repr(chromaterm.__main__.split_buffer(data)) == repr(expected)


def test_split_buffer_csi_no_parameter_no_intermediate():
    """Split based on CSI with no parameter or intermediate bytes."""
    csi_up_to_sgr = range(int('40', 16), int('6d', 16))
    csi_above_sgr = range(int('6e', 16), int('7f', 16))

    for char_id in itertools.chain(csi_up_to_sgr, csi_above_sgr):
        data = 'Hello \x1b[{} World'.format(chr(char_id))
        expected = (('Hello ', '\x1b[' + chr(char_id)), (' World', ''))

        assert repr(chromaterm.__main__.split_buffer(data)) == repr(expected)


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
                    chromaterm.__main__.split_buffer(data)) == repr(expected)


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

                    assert repr(chromaterm.__main__.split_buffer(
                        data)) == repr(expected)


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
                    chromaterm.__main__.split_buffer(data)) == repr(expected)


def test_split_buffer_osc_title():
    """Operating System Command (OSC) can supply arbitrary commands within the
    visible character set."""
    for end in ['\x07', '\x1b\x5c']:
        osc = '\x1b]Ignored{}'.format(end)
        data = '{}Hello world'.format(osc)
        expected = (('', osc), ('Hello world', ''))

        assert repr(chromaterm.__main__.split_buffer(data)) == repr(expected)


def test_split_buffer_scs():
    """Select Character Set (SCS) is used to change the terminal character set."""
    g_sets = ['\x28', '\x29', '\x2a', '\x2b', '\x2d', '\x2e', '\x2f']
    char_sets = map(chr, range(int('20', 16), int('7e', 16)))

    for g_set in g_sets:
        for char_set in char_sets:
            code = '\x1b{}{}'.format(g_set, char_set)
            data = 'Hello {} World'.format(code)
            expected = (('Hello ', code), (' World', ''))

            assert repr(
                chromaterm.__main__.split_buffer(data)) == repr(expected)


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

        process = subprocess.Popen(CLI + ' --rgb --config ' + __name__ + '3',
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
