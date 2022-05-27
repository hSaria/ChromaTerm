'''chromaterm.platform.unix tests'''
import sys

import chromaterm.platform.unix


def test_get_stdin():
    '''Ensure stdin is the correct file descriptor.'''
    assert chromaterm.platform.unix.get_stdin() == sys.stdin.fileno()
