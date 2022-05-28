'''platform tests'''
import atexit
import os
import sys
import threading
from unittest.mock import MagicMock

import pytest

import chromaterm.platform.windows as platform

# pylint: disable=invalid-name,no-member,protected-access


def patch_functions(monkeypatch):
    '''Replace the functions used by Windows with `MagicMock` objects.'''
    # Initialize to something as that wouldn't happen on a test (Unix) machine
    platform.K32 = None

    monkeypatch.setattr(platform, 'K32', MagicMock())
    monkeypatch.setattr(atexit, 'register', MagicMock())
    monkeypatch.setattr(threading, 'Thread', MagicMock())

    for function in ['create_socket_pipe', 'create_forwarder']:
        monkeypatch.setattr(platform, function, MagicMock())

    platform.create_socket_pipe.return_value = MagicMock(), MagicMock()


def test_create_forwarder():
    '''Validate forwarding and finalizing.'''
    read_out, read_in = os.pipe()
    write_out, write_in = os.pipe()

    worker = platform.create_forwarder(read=lambda: os.read(read_out, 100),
                                       write=lambda x: os.write(write_in, x),
                                       finalize=lambda: os.close(read_out))

    os.write(read_in, b'hello')
    assert os.read(write_out, 100) == b'hello'

    # The worker will receive empty data and close down
    os.close(read_in)
    worker.join()


def test_create_forwarder_error_handling():
    '''Validate errors are handled properly and the finalizer is ran.'''
    read_out, read_in = os.pipe()
    write_out, write_in = os.pipe()

    worker = platform.create_forwarder(read=lambda: os.read(read_out, 100),
                                       write=lambda x: os.write(write_in, x),
                                       finalize=lambda: os.close(read_out))

    # Trigger OSError by closing the write side
    os.close(write_out)
    os.write(read_in, b'bye')
    worker.join()

    # The finalizer should've closed the other end of the pipe
    with pytest.raises(BrokenPipeError):
        os.write(read_in, b'hello')


def test_create_socket_pipe():
    '''Ensure the two sockets are bidirectional and connected to each other.'''
    side_a, side_b = platform.create_socket_pipe()

    side_a.sendall(b'hello from a')
    assert side_b.recv(100) == b'hello from a'

    side_b.sendall(b'hello from b')
    assert side_a.recv(100) == b'hello from b'


def test_get_stdin_read(monkeypatch):
    '''Successfully completing a read operation and getting an empty string should
    emit CTRL-z (EOF on Windows).'''
    patch_functions(monkeypatch)
    monkeypatch.setattr(sys.stdin, 'isatty', lambda: True)

    def ReadFile(*args):
        args[1].value = b'hello'
        args[3]._obj.value = 4
        return True

    platform.K32.ReadFile = ReadFile
    platform.get_stdin()

    assert platform.create_forwarder.mock_calls[0][2]['read']() == b'hell'


def test_get_stdin_read_empty(monkeypatch):
    '''Successfully completing a read operation and getting an empty string should
    emit CTRL-z (EOF on Windows).'''
    patch_functions(monkeypatch)
    monkeypatch.setattr(sys.stdin, 'isatty', lambda: True)

    def ReadFile(*args):
        args[3]._obj.value = 0
        return True

    platform.K32.ReadFile = ReadFile
    platform.get_stdin()

    assert platform.create_forwarder.mock_calls[0][2]['read']() == b'\x1a'


def test_get_stdin_console_mode(monkeypatch):
    '''Verify the correct console mode is set and restored.'''
    patch_functions(monkeypatch)

    def GetConsoleMode(_, console_mode_ref):
        console_mode_ref._obj.value = 0x107

    platform.K32.GetConsoleMode = GetConsoleMode
    platform.get_stdin()

    # 0x7 is removed, 0x100 is unaffected, 0x200 is added
    assert platform.K32.SetConsoleMode.mock_calls[0][1][1] == 0x300
    assert atexit.register.mock_calls[0][1][2] == 0x107


def test_get_stdin_console_mode_stdout(monkeypatch):
    '''Verify the correct console mode is set and restored for stdin.'''
    patch_functions(monkeypatch)

    def GetConsoleMode(_, console_mode_ref):
        console_mode_ref._obj.value = 0x100

    platform.K32.GetConsoleMode = GetConsoleMode
    platform.get_stdin()

    # 0x100 is unaffected, 0x7 is added
    assert platform.K32.SetConsoleMode.mock_calls[1][1][1] == 0x107
    assert atexit.register.mock_calls[1][1][2] == 0x100


# Tests for run_program:
# * CloseHandle is called on the program side (extract from CreatePipe)
# * CreateProcessW command is escaped properly
# * `read` and `write` target the correct side of the pipe (extract from CreatePipe)
# * `close` waits on the process and then closes the console
# * Patch `get_terminal_size` for `resize`. set WINDOW_RESIZE_INTERVAL to a string to break out
