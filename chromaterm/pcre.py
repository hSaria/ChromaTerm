'''A PCRE2 implementation. Similar to Python's `re`, albeit very minimal.'''
import collections
import re
from ctypes import (CDLL, POINTER, byref, c_char, c_size_t, c_uint32,
                    create_string_buffer)
from ctypes.util import find_library

PCRE2 = CDLL(find_library('pcre2-8'))
PCRE2.pcre2_compile_8.restype = POINTER(c_char)
PCRE2.pcre2_match_data_create_8.restype = POINTER(c_char)

# PCRE2_NOTEMPTY_ATSTART = 4
MATCH_OPTIONS = c_uint32(4)


class GroupIndex(collections.UserDict):
    '''Extracting the named groups table is too much work. This class patches
    `__getitem__` to extract a named group on-demand (much easier to do).'''

    def __init__(self, regex):
        super().__init__()
        self._regex = regex

    def __getitem__(self, name):
        group_id = PCRE2.pcre2_substring_number_from_name_8(
            self._regex,
            name.encode(),
        )

        if group_id < 1:
            raise KeyError(name)

        return group_id


class Pattern:
    '''PCRE2 Pattern. Like that of Python's, if it was stripped of everything.'''

    def __init__(self, pattern):
        '''Constructor.

        Args:
            pattern (bytes): The regex pattern.
        '''
        error_buffer = create_string_buffer(4096)
        error_code = c_uint32()
        error_offset = c_size_t()
        groups = c_uint32()

        self.pattern = pattern
        self._regex = PCRE2.pcre2_compile_8(
            self.pattern,
            c_size_t(len(self.pattern)),
            c_uint32(0),
            byref(error_code),
            byref(error_offset),
            None,
        )

        if error_code.value != 100:
            PCRE2.pcre2_get_error_message_8(
                error_code,
                error_buffer,
                byref(c_size_t(len(error_buffer) - 1)),
            )
            msg = f'position {error_offset.value}: {error_buffer.value.decode()}'

            raise re.error(msg)

        # PCRE2_JIT_COMPLETE = 1
        PCRE2.pcre2_jit_compile_8(self._regex, c_uint32(1))

        # PCRE2_INFO_CAPTURECOUNT = 4
        PCRE2.pcre2_pattern_info_8(self._regex, c_uint32(4), byref(groups))

        self.groups = groups.value
        self.groupindex = GroupIndex(self._regex)

        self._match_data = PCRE2.pcre2_match_data_create_8(
            c_uint32(self.groups + 1), None)

        PCRE2.pcre2_get_ovector_pointer_8.restype = POINTER(
            2 * (self.groups + 1) * c_size_t)

        # Emulate `span` method of `re.Match`
        match = PCRE2.pcre2_get_ovector_pointer_8(self._match_data).contents
        match.span = lambda gid: (match[2 * gid], match[2 * gid + 1])

        self._match = match
        self._data_len = c_size_t()

    def __del__(self):
        if PCRE2:
            if hasattr(self, '_match_data'):
                PCRE2.pcre2_match_data_free_8(self._match_data)
            if hasattr(self, '_regex'):
                PCRE2.pcre2_code_free_8(self._regex)

    def finditer(self, data):
        '''A generator for finding matches.

        Args:
            data (bytes): The subject of the search.
        '''
        # Reset the startoffset to 0
        self._match[1] = 0
        self._data_len.value = len(data)

        while PCRE2.pcre2_jit_match_8(
                self._regex,
                data,
                self._data_len,
                self._match[1],
                MATCH_OPTIONS,
                self._match_data,
                None,
        ) >= 0:
            yield self._match
