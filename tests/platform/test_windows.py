'''platform tests'''
import atexit
import ctypes
import os
import shutil
import threading
from unittest.mock import MagicMock, call

import pytest

import chromaterm.platform.windows as platform

# pylint: disable=invalid-name,no-member,protected-access


def patch_functions(monkeypatch):
    '''Replace the functions used by Windows with `MagicMock` objects.'''
    # Initialize to something as that wouldn't happen on a test (Unix) machine
    platform.K32 = platform.wintypes = None

    monkeypatch.setattr(platform, 'K32', MagicMock())
    monkeypatch.setattr(platform, 'wintypes', MagicMock())
    monkeypatch.setattr(atexit, 'register', MagicMock())
    monkeypatch.setattr(threading, 'Thread', MagicMock())

    for wintype in ['SHORT', 'WORD', 'DWORD', 'HANDLE']:
        setattr(platform.wintypes, wintype, ctypes.c_int)

    platform.wintypes.LPVOID = ctypes.c_void_p

    for function in ['create_socket_pipe', 'create_forwarder']:
        monkeypatch.setattr(platform, function, MagicMock())

    platform.K32.HeapAlloc.return_value = 1
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

    def ReadFile(*args):
        args[1].value = b'hello'
        args[3]._obj.value = 4
        return True

    patch_functions(monkeypatch)
    platform.K32.ReadFile = ReadFile
    platform.get_stdin()

    assert platform.create_forwarder.mock_calls[0][2]['read']() == b'hell'


def test_get_stdin_read_empty(monkeypatch):
    '''Successfully completing a read operation and getting an empty string should
    emit CTRL-z (EOF on Windows).'''

    def ReadFile(*args):
        args[3]._obj.value = 0
        return True

    patch_functions(monkeypatch)
    platform.K32.ReadFile = ReadFile
    platform.get_stdin()

    assert platform.create_forwarder.mock_calls[0][2]['read']() == b'\x1a'


def test_get_stdin_console_mode(monkeypatch):
    '''Verify the correct console mode is set and restored.'''

    def GetConsoleMode(_, console_mode_ref):
        console_mode_ref._obj.value = 0x107

    patch_functions(monkeypatch)
    platform.K32.GetConsoleMode = GetConsoleMode
    platform.get_stdin()

    # 0x7 is removed, 0x100 is unaffected, 0x200 is added
    assert platform.K32.SetConsoleMode.mock_calls[0][1][1] == 0x300
    assert atexit.register.mock_calls[0][1][2] == 0x107


def test_get_stdin_console_mode_stdout(monkeypatch):
    '''Verify the correct console mode is set and restored for stdin.'''

    def GetConsoleMode(_, console_mode_ref):
        console_mode_ref._obj.value = 0x100

    patch_functions(monkeypatch)
    platform.K32.GetConsoleMode = GetConsoleMode
    platform.get_stdin()

    # 0x100 is unaffected, 0x7 is added
    assert platform.K32.SetConsoleMode.mock_calls[1][1][1] == 0x107
    assert atexit.register.mock_calls[1][1][2] == 0x100


def test_run_program_close_program_handles(monkeypatch):
    '''Ensure the program's side of the pipes/handles is closed in ChromaTerm.'''
    patch_functions(monkeypatch)
    platform.run_program([])

    # The program's side reads input (input_r) and writes output (output_w)
    input_r = platform.K32.CreatePipe.mock_calls[0][1][0]._obj
    output_w = platform.K32.CreatePipe.mock_calls[1][1][1]._obj

    platform.K32.CloseHandle.assert_has_calls([call(input_r), call(output_w)])


def test_run_program_escape_program_args(monkeypatch):
    '''The program arguments should be escaped properly.'''
    patch_functions(monkeypatch)
    platform.run_program(['foo bar', '1', '2'])
    assert platform.K32.CreateProcessW.mock_calls[0][1][1] == "'foo bar' 1 2"


def test_run_program_read(monkeypatch):
    '''Confirm ChromaTerm _reads_ the program's _output_ (output_r).'''
    patch_functions(monkeypatch)
    platform.run_program([])
    platform.create_forwarder.mock_calls[0][2]['read']()

    output_r = platform.K32.CreatePipe.mock_calls[1][1][0]._obj
    assert platform.K32.ReadFile.mock_calls[0][1][0] == output_r


def test_run_program_write(monkeypatch):
    '''Confirm ChromaTerm _writes_ to the program's _input_ (input_w).'''
    patch_functions(monkeypatch)
    platform.run_program([])
    platform.create_forwarder.mock_calls[1][2]['write'](b'hello')

    input_w = platform.K32.CreatePipe.mock_calls[0][1][1]._obj
    assert platform.K32.WriteFile.mock_calls[0][1][0] == input_w
    assert platform.K32.WriteFile.mock_calls[0][1][1].value == b'hello'


def test_run_program_close(monkeypatch):
    '''When the program exits, the console and slave socket should be closed.'''
    patch_functions(monkeypatch)
    platform.run_program([])
    threading.Thread.mock_calls[0][2]['target']()

    platform.K32.WaitForSingleObject.assert_called()
    platform.K32.ClosePseudoConsole.assert_called()
    platform.create_socket_pipe.return_value[1].close.assert_called()


def test_run_program_resize(monkeypatch):
    '''When the program exits, the console and slave socket should be closed.'''
    patch_functions(monkeypatch)
    monkeypatch.setattr(shutil, 'get_terminal_size', MagicMock())
    shutil.get_terminal_size.side_effect = [(1, 2), (3, 4), (5, 6)]

    try:
        platform.run_program([])
        # The `.start()` accounts for one mock call
        threading.Thread.mock_calls[2][2]['target']()
    except StopIteration:
        pass

    platform.K32.ResizePseudoConsole.assert_called()
