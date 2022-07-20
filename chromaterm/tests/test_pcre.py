'''chromaterm.pcre tests'''
import re
from ctypes.util import find_library
from unittest.mock import MagicMock

import pytest

# pylint: disable=protected-access,wrong-import-position

if not find_library('pcre2-8'):  # pragma: no cover
    pytest.skip('skipping PCRE tests; lib not preset', allow_module_level=True)

import chromaterm.pcre


def test_finditer():
    '''Find all matches in data.'''
    regex = chromaterm.pcre.Pattern(b'hello (world)')
    match = list(regex.finditer(b'--- hello world ---'))[0]

    assert match.span(0) == (4, 15)
    assert match.span(1) == (10, 15)


def test_free_memory(monkeypatch):
    '''Free the PCRE `regex` and `match_data` on cleanup.'''
    mock = MagicMock()
    regex = chromaterm.pcre.Pattern(b'hello')
    _match_data = regex._match_data
    _regex = regex._regex

    monkeypatch.setattr(chromaterm.pcre.PCRE2, 'pcre2_match_data_free_8', mock)
    monkeypatch.setattr(chromaterm.pcre.PCRE2, 'pcre2_code_free_8', mock)

    del regex
    assert mock.mock_calls[0][1][0] is _match_data
    assert mock.mock_calls[1][1][0] is _regex


def test_groupindex():
    '''Lookup named groups.'''
    regex = chromaterm.pcre.Pattern(br'(?P<hi>hello)( )(?<place>world)')

    assert regex.groupindex['hi'] == 1
    assert regex.groupindex.get('place') == 3
    assert regex.groupindex.get('no') is None


def test_re_error():
    '''Invalid regular expression should raise `re.error`.'''
    with pytest.raises(re.error, match=r'position \d+: '):
        chromaterm.pcre.Pattern(b'hello (world')


def test_zero_length_matching():
    '''Ensure that a zero-length matches don't put PCRE into an infinite loop.'''
    regex = chromaterm.pcre.Pattern(br'(?<=hello)\b')
    assert len(list(regex.finditer(b'hello world'))) == 0
