'''chromaterm.platform.unix tests'''
import sys

import chromaterm.platform.unix


def test_get_stdin(monkeypatch):
    '''Ensure stdin is a file descriptor.'''
    monkeypatch.setattr(sys.stdin, 'fileno', lambda: 0)
    assert isinstance(chromaterm.platform.unix.get_stdin(), int)
