'''pytest configuration'''
from ctypes.util import find_library


def pytest_generate_tests(metafunc):
    '''Run tests with the `pcre` fixture twice; Once with `pcre=False`, another
    time with `pcre=True` if the library is present.'''
    if find_library('pcre2-8') and 'pcre' in metafunc.fixturenames:
        metafunc.parametrize('pcre', [False, True])
