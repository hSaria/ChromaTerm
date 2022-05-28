'''Windows functions'''
# pylint: disable=invalid-name,missing-class-docstring,too-few-public-methods
import atexit
import ctypes
import shlex
import shutil
import socket
import threading
import time
from ctypes import byref, wintypes

# pytest will import this package during test collection but windll is Windows-only
if hasattr(ctypes, 'windll'):  # pragma: no cover
    K32 = ctypes.windll.kernel32
    K32.GetProcessHeap.restype = wintypes.HANDLE
    K32.HeapAlloc.argtypes = [wintypes.HANDLE, wintypes.DWORD, ctypes.c_size_t]
    K32.HeapAlloc.restype = wintypes.LPVOID

BUFSIZE = 8192
WINDOW_RESIZE_INTERVAL = 1 / 8


class COORD(ctypes.Structure):
    _fields_ = [('X', wintypes.SHORT), ('Y', wintypes.SHORT)]


class PROCESS_INFORMATION(ctypes.Structure):
    _fields_ = [
        ('hProcess', wintypes.HANDLE),
        ('hThread', wintypes.HANDLE),
        ('dwProcessId', wintypes.DWORD),
        ('dwThreadId', wintypes.DWORD),
    ]


class STARTUPINFO(ctypes.Structure):
    _fields_ = [
        ('cb', wintypes.DWORD),
        ('lpReserved', ctypes.c_void_p),
        ('lpDesktop', ctypes.c_void_p),
        ('lpTitle', ctypes.c_void_p),
        ('dwX', wintypes.DWORD),
        ('dwY', wintypes.DWORD),
        ('dwXSize', wintypes.DWORD),
        ('dwYSize', wintypes.DWORD),
        ('dwXCountChars', wintypes.DWORD),
        ('dwYCountChars', wintypes.DWORD),
        ('dwFillAttribute', wintypes.DWORD),
        ('dwFlags', wintypes.DWORD),
        ('wShowWindow', wintypes.WORD),
        ('cbReserved2', wintypes.WORD),
        ('lpReserved2', ctypes.c_char_p),
        ('hStdInput', wintypes.HANDLE),
        ('hStdOutput', wintypes.HANDLE),
        ('hStdError', wintypes.HANDLE),
    ]


class STARTUPINFOEX(ctypes.Structure):
    _fields_ = [('StartupInfo', STARTUPINFO),
                ('lpAttributeList', ctypes.POINTER(wintypes.LPVOID))]


def create_forwarder(read, write, finalize=lambda: None, break_on_empty=True):
    '''Spawns a thread that runs `write(read())`.'''

    def work():
        try:
            while True:
                data = read()

                if data:
                    write(data)
                elif break_on_empty:
                    break
        except OSError:
            pass
        finally:
            finalize()

    worker = threading.Thread(target=work, daemon=True)
    worker.start()

    return worker


def create_socket_pipe():
    '''Returns two sockets connected to each other.'''
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.bind(('127.0.0.1', 0))
    server.listen(1)

    client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client.connect(server.getsockname())
    server_connection, _ = server.accept()
    server.close()

    return server_connection, client


def get_stdin():
    '''Returns a socket for stdin.'''
    console_mode = wintypes.DWORD()
    stdin_handle = K32.GetStdHandle(-10)
    stdout_handle = K32.GetStdHandle(-11)

    # See https://docs.microsoft.com/en-us/windows/console/setconsolemode
    K32.GetConsoleMode(stdin_handle, byref(console_mode))
    K32.SetConsoleMode(stdin_handle, console_mode.value & ~0x7 | 0x200)
    atexit.register(K32.SetConsoleMode, stdin_handle, console_mode.value)
    K32.GetConsoleMode(stdout_handle, byref(console_mode))
    K32.SetConsoleMode(stdout_handle, console_mode.value | 0x7)
    atexit.register(K32.SetConsoleMode, stdout_handle, console_mode.value)

    # `select` on Windows only accepts sockets, so we use one for stdin
    stdin, source = create_socket_pipe()
    buffer = ctypes.create_string_buffer(BUFSIZE)
    count = wintypes.DWORD()

    def read():
        if K32.ReadFile(stdin_handle, buffer, BUFSIZE, byref(count), None):
            # CTRL-z (EOF on Windows) would return 0
            if not count.value:
                return b'\x1a'  # CTRL-z
        return buffer.raw[:count.value]

    create_forwarder(read=read, write=source.sendall, finalize=source.close)

    return stdin


def run_program(program_args):
    '''Returns a socket of a bidirectional pipe to the spawned program.

    Args:
        program_args (list): A list of program arguments. The first argument is
            the program name/location.
    '''
    # pylint: disable=attribute-defined-outside-init,too-many-locals
    # Create the pipes (the program's input and output)
    input_r, input_w = wintypes.HANDLE(), wintypes.HANDLE()
    output_r, output_w = wintypes.HANDLE(), wintypes.HANDLE()

    # Connect them (read, write)
    K32.CreatePipe(byref(input_r), byref(input_w), None, 0)
    K32.CreatePipe(byref(output_r), byref(output_w), None, 0)

    # Create the pty (connected to the pipes)
    console = wintypes.HANDLE()
    console_size = COORD(*shutil.get_terminal_size())
    K32.CreatePseudoConsole(console_size, input_r, output_w, 0, byref(console))

    # Close the handles on the program's side as the console has its own set now
    K32.CloseHandle(input_r)
    K32.CloseHandle(output_w)

    # Get the size of the thread attribute list to allocate the appropriate size
    lp_size = wintypes.LPVOID()
    K32.InitializeProcThreadAttributeList(None, 1, 0, byref(lp_size))

    # Initialize the attribute list
    startup_info_ex = STARTUPINFOEX()
    startup_info_ex.StartupInfo.cb = ctypes.sizeof(STARTUPINFOEX)
    startup_info_ex.lpAttributeList = ctypes.cast(
        K32.HeapAlloc(K32.GetProcessHeap(), 0, lp_size.value),
        ctypes.POINTER(ctypes.c_void_p))
    K32.InitializeProcThreadAttributeList(startup_info_ex.lpAttributeList, 1,
                                          0, byref(lp_size))

    # Set thread's pseudoconsole to the pty
    # PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE = 0x20016
    K32.UpdateProcThreadAttribute(startup_info_ex.lpAttributeList, 0, 0x20016,
                                  console, ctypes.sizeof(console), None, None)

    # EXTENDED_STARTUPINFO_PRESENT = 0x80000
    process_info = PROCESS_INFORMATION()
    K32.CreateProcessW(None, shlex.join(program_args), None, None, False,
                       0x80000, None, None, byref(startup_info_ex.StartupInfo),
                       byref(process_info))

    # Create pipe and necessary forwarder threads
    master, slave = create_socket_pipe()
    read_buffer = ctypes.create_string_buffer(BUFSIZE)
    write_buffer = ctypes.create_string_buffer(BUFSIZE)
    bytes_read = wintypes.DWORD()

    def read():
        K32.ReadFile(output_r, read_buffer, BUFSIZE, byref(bytes_read), None)
        return read_buffer.raw[:bytes_read.value]

    # Read from pty and write it over the socket pipe to master
    create_forwarder(read=read, write=slave.sendall, break_on_empty=False)

    def write(data):
        write_buffer.value = data
        K32.WriteFile(input_w, write_buffer, len(data), 0, None)

    # Read from the socket pipe from master and write it to pty
    create_forwarder(read=lambda: slave.recv(BUFSIZE), write=write)

    # Close the console after the child process ends
    def close():
        K32.WaitForSingleObject(process_info.hThread, -1)
        K32.ClosePseudoConsole(console)
        slave.close()

    threading.Thread(target=close, daemon=True).start()

    # Update the window size of the pty when the main window size changes
    # Using ENABLE_WINDOW_INPUT along with ReadConsoleInputW doesn't work because
    # the key events will be removed from the buffer. And peeking then flushing
    # doesn't work either as it flushes all events, not just the windows resize ones
    def resize():
        current_size = shutil.get_terminal_size()

        while True:
            new_size = shutil.get_terminal_size(current_size)

            # Avoid aggressive resizing as it scrolls the cursor into view
            if current_size != new_size:
                K32.ResizePseudoConsole(console, COORD(*new_size))
                current_size = new_size

            time.sleep(WINDOW_RESIZE_INTERVAL)

    threading.Thread(target=resize, daemon=True).start()

    return master
