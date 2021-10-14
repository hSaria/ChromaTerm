'''Color your output to terminal'''
import os
import re
import sys
import time

# Color types, their color codes if it's style, their default reset codes, and
# RegEx's for detecting their color type.
COLOR_TYPES = {
    'fg': {
        'reset': b'\x1b[39m',
        're': re.compile(br'\x1b\[(?:3[0-79]|9[0-7]|38;[0-9;]+)m')
    },
    'bg': {
        'reset': b'\x1b[49m',
        're': re.compile(br'\x1b\[(?:4[0-79]|10[0-7]|48;[0-9;]+)m')
    },
    'blink': {
        'code': b'\x1b[5m',
        'reset': b'\x1b[25m',
        're': re.compile(br'\x1b\[2?5m')
    },
    'bold': {
        'code': b'\x1b[1m',
        'reset': b'\x1b[22m',  # Normal intensity
        're': re.compile(br'\x1b\[(?:1|2?2)m')  # Any intensity type
    },
    'invert': {
        'code': b'\x1b[7m',
        'reset': b'\x1b[27m',
        're': re.compile(br'\x1b\[2?7m')
    },
    'italic': {
        'code': b'\x1b[3m',
        'reset': b'\x1b[23m',
        're': re.compile(br'\x1b\[2?3m')
    },
    'strike': {
        'code': b'\x1b[9m',
        'reset': b'\x1b[29m',
        're': re.compile(br'\x1b\[2?9m')
    },
    'underline': {
        'code': b'\x1b[4m',
        'reset': b'\x1b[24m',
        're': re.compile(br'\x1b\[2?4m')
    }
}

# Detect rgb support
RGB_SUPPORTED = os.getenv('COLORTERM') in ('truecolor', '24bit')

# Select Graphic Rendition sequence (any type)
SGR_RE = re.compile(br'\x1b\[[0-9;]*m')


class Color:
    '''A color and its ANSI escape codes.'''
    def __init__(self, color, rgb=None):
        '''Constructor.

        Args:
            color (str): A string which must contain:
                * one foreground color (hex color prefixed with `f#`),
                * one background color (hex color prefixed with `b#`),
                * at least one style (blink, bold, invert, italic, strike,
                    underline), or
                * a combination of the above, seperated by spaces.

                Example: `"b#123123 bold"`
            rgb (bool): Whether the color is meant for RGB-enabled terminals or
                not. `False` will downscale the RGB colors to xterm-256. `None`
                will detect support for RGB and fallback to xterm-256.

        Raises:
            TypeError: If `color` is not a string. If `rgb` is not a boolean.
            ValueError: If the format of `color` is invalid.
        '''
        self.rgb = rgb
        self.color = color

    @property
    def color(self):
        '''String that represents the color.'''
        return self._color

    @color.setter
    def color(self, value):
        if not isinstance(value, str):
            raise TypeError('color must be a string')

        value = value.lower().strip()
        styles = tuple(k for k, v in COLOR_TYPES.items() if v.get('code'))
        color_re = r'^(((b|f)#[0-9a-f]{6}|' + '|'.join(styles) + r')(\s+|$))+$'
        color_code = color_reset = b''
        color_types = []

        if not re.search(color_re, value):
            raise ValueError(f'invalid color format {repr(value)}')

        # Colors
        for target, hex_code in re.findall(r'(b|f)#([0-9a-f]{6})', value):
            if target == 'f':
                target, color_type = b'\x1b[38;', 'fg'
            else:
                target, color_type = b'\x1b[48;', 'bg'

            if color_type in [x[0] for x in color_types]:
                raise ValueError('color accepts exactly one foreground and one'
                                 ' background colors')

            # Break down hex color to RGB integers
            rgb_int = [int(hex_code[i:i + 2], 16) for i in (0, 2, 4)]

            if self.rgb or (self.rgb is None and RGB_SUPPORTED):
                target += b'2;'
                color_id = b'%d;%d;%d' % tuple(rgb_int)
            else:
                target += b'5;'
                color_id = b'%d' % self.rgb_to_8bit(*rgb_int)

            color_code = color_code + target + color_id + b'm'
            color_reset = COLOR_TYPES[color_type]['reset'] + color_reset
            color_types.append((color_type, target + color_id + b'm'))

        # Styles
        for style in re.findall('|'.join(styles), value):
            if style in [x[0] for x in color_types]:
                raise ValueError('color does not accept duplicate styles')

            color_code = color_code + COLOR_TYPES[style]['code']
            color_reset = COLOR_TYPES[style]['reset'] + color_reset
            color_types.append((style, COLOR_TYPES[style]['code']))

        self._color = ' '.join(re.split(r'\s+', value.strip().lower()))
        self.color_code = color_code
        self.color_reset = color_reset
        self.color_types = color_types

    @property
    def rgb(self):
        '''Flag for RGB-support. When changed, updates `self.color`.'''
        return self._rgb

    @rgb.setter
    def rgb(self, value):
        if value is not None and not isinstance(value, bool):
            raise TypeError('rgb must be a boolean')

        self._rgb = value

        # Update the color if preset; it won't be during __init__
        if hasattr(self, '_color'):
            self.color = self._color

    @staticmethod
    def decode_sgr(source_color_code):
        '''Decodes an SGR, splitting it into a list of colors, each being a list
        containing color code (bytes), is reset (bool), and color type (bytes)
        which corresponds to `COLOR_TYPES`.

        Args:
            source_color_code (bytes): Bytes to be split into individual colors.
        '''
        def make_sgr(code_id):
            return b'\x1b[' + code_id + b'm'

        colors = []
        codes = source_color_code.lstrip(b'\x1b[').rstrip(b'm').split(b';')
        skip = 0

        for index, code in enumerate(codes):
            # Code processed by an index look-ahead; skip it
            if skip:
                skip -= 1
                continue

            # Full reset
            if code == b'' or int(code) == 0:
                colors.append([make_sgr(b'0'), True, None])
            # Multi-code SGR
            elif code in (b'38', b'48'):
                color_type = 'fg' if code == b'38' else 'bg'

                # xterm-256
                if len(codes) > index + 2 and codes[index + 1] == b'5':
                    skip = 2
                    code = b';'.join(codes[index:index + 3])
                # RGB
                elif len(codes) > index + 4 and codes[index + 1] == b'2':
                    skip = 4
                    code = b';'.join(codes[index:index + 5])
                # Does not conform to format; do not touch code
                else:
                    return [[source_color_code, False, None]]

                colors.append([make_sgr(code), False, color_type])
            # Single-code SGR
            else:
                color = [make_sgr(b'%d' % int(code)), False, None]

                for name, color_type in COLOR_TYPES.items():
                    if color_type['re'].search(color[0]):
                        color[1] = color[0] == color_type['reset']
                        color[2] = name

                        # Types don't overlap; only one can match
                        break

                colors.append(color)

        return colors

    @staticmethod
    def rgb_to_8bit(_r, _g, _b):
        '''Downscale from 24-bit RGB to 8-bit ANSI.'''
        def downscale(value, base=6):
            return int(value / 256 * base)

        # Use the 24 shades of the grayscale
        if _r == _g == _b:
            return 232 + downscale(_r, base=24)

        return 16 + (36 * downscale(_r)) + (6 * downscale(_g)) + downscale(_b)

    @staticmethod
    def strip_colors(data):
        '''Returns data after stripping the existing colors and a list of inserts
        containing the stripped colors. The format of the insert is that of
        `Config.get_inserts`.

        Args:
            data (bytes): Bytes from which the colors should be stripped.
        '''
        inserts = []
        match = SGR_RE.search(data)

        while match:
            start, end = match.span()

            for color in Color.decode_sgr(match.group()):
                color.insert(0, start)
                inserts.insert(0, color)

            # Next color's start index ignores length of this color
            data = data[:start] + data[end:]
            match = SGR_RE.search(data)

        return data, inserts


class Rule:
    '''A rule containing a regex and colors corresponding to the regex's groups.'''
    def __init__(self, regex, color=None, description=None, exclusive=False):
        '''Constructor.

        Args:
            regex (str): Regular expression for getting matches in data.
            color (chromaterm.Color): Color used to highlight the entire match.
            description (str): String to help identify the rule.
            exclusive (bool): Whether other rules should overlap with this one

        Raises:
            TypeError: If `color` is not an instance of `Color` or not `None`.
        '''
        if description is not None and not isinstance(description, str):
            raise TypeError('description must be a string')

        if not isinstance(regex, str):
            raise TypeError('regex must be a string')

        self.colors = {}
        self.description = description
        self.exclusive = bool(exclusive)
        self.regex = re.compile(regex.encode(encoding='utf-8'))

        self.set_color(color)

    @property
    def color(self):
        '''Color used for highlight the full match (group 0) of regex.'''
        return self.colors.get(0)

    @color.setter
    def color(self, value):
        self.set_color(value)

    def get_matches(self, data):
        '''Returns a list of tuples, each containing a start index, an end index,
        and the `Color` object for that match.

        Args:
            data (bytes): Bytes to match regex against.
        '''
        matches = []

        for match in self.regex.finditer(data):
            for group in self.colors:
                start, end = match.span(group)

                # Ignore zero-length matches, like unmatched optional groups
                if start != end:
                    matches.append((start, end, self.colors[group]))

        return matches

    def set_color(self, color, group=0):
        '''Sets a color to be used when highlighting. The group can be used to
        limit the parts of the match which are highlighted. Group 0 (the default)
        will highlight the entire match. If a color already exists for the group,
        it is overwritten. If `color` is None, the color of `group` is cleared.

        Args:
            color (chromaterm.Color): A color for highlighting the matched input.
            group (int): The regex group to be be highlighted with the color.

        Raises:
            TypeError: If `color` is not an instance of `Color` or None. If
                `group` is not an integer.
            ValueError: If `group` does not exist in the regular expression.
        '''
        if not isinstance(group, int):
            raise TypeError('group must be an integer')

        if color is None:
            self.colors.pop(group, None)
            return

        if not isinstance(color, Color):
            raise TypeError('color must be a chromaterm.Color')

        if group > self.regex.groups:
            raise ValueError(f'regex only has {self.regex.groups} group(s); '
                             f'{group} is invalid')

        self.colors[group] = color

        # Sort the colors according to the group number to ensure deterministic
        # highlighting
        self.colors = {k: self.colors[k] for k in sorted(self.colors)}


class Config:
    '''An aggregation of multiple rules which highlights by performing the regex
    matching of the rules before any colors are added.'''
    def __init__(self, benchmark=False):
        '''Constructor.'''
        self._reset_codes = {k: v['reset'] for k, v in COLOR_TYPES.items()}
        self.benchmark = benchmark
        self.benchmark_results = {}
        self.rules = []

    @staticmethod
    def get_insert_index(start, end, inserts):
        '''Returns a tuple containing the start and end indexes for where they
        should be inserted into the inserts list in order to maintain the
        position-based descending (reverse) order.

        Args:
            start (int): The start position of a match.
            end (int): The end position of a match.
            inserts (list): A list of inserts, where the first item of each insert
                is the position.
        '''
        start_index = end_index = None
        index = -1

        # Arrange the inserts in reverse order (index magic over data)
        for index, (position, _, _, _) in enumerate(inserts):
            if start_index is None and start >= position:
                start_index = index

            # In the case of overlapping matches, other colors exist between
            # the start and the end, so the end index needs to be located
            # independently
            if end_index is None and end > position:
                end_index = index

            if start_index is not None and end_index is not None:
                return start_index, end_index

        # If an index wasn't found, then it belongs at the end of the list
        if start_index is None:
            start_index = index + 1
        if end_index is None:
            end_index = index + 1

        return start_index, end_index

    def get_inserts(self, data, inserts):
        '''Returns a list containing the inserts for the color codes relative to
        data. An insert is a list containing a position (index relative to data),
        the code to be inserted, a boolean indicating if its a reset code or not,
        and The color type which corresponds to COLOR_TYPES or `None` if it's a
        full SGR reset.

        The list of inserts is ordered in descending order based on the position
        of each insert relative to the data. This makes them easy to insert into
        data without calculating index offsetes.

        Args:
            data (bytes): Bytes from which the inserts are gathered.
            inserts (list): Any pre-existing inserts to be added to it.
        '''
        # A lot of the code here is difficult to comprehend directly, because the
        # intent might not be immediately visible. You may find it easier to take
        # a test-driven approach by looking at the test_config_highlight_* tests
        for start, end, color in self.get_matches(data):
            start_index, end_index = self.get_insert_index(start, end, inserts)
            reset_codes = []

            # Each color type requires tracking of its respective type
            for color_type, color_code in color.color_types:
                start_insert = [start, color_code, False, color_type]
                end_insert = [
                    end, self._reset_codes[color_type], True, color_type
                ]

                # Find the last color before the end of this match (if any) and
                # use it as the reset code for this color
                for insert in inserts[end_index:]:
                    if insert[3] == color_type:
                        end_insert[1] = insert[1]
                        break

                    # No type (a full reset); use the default for this type
                    if insert[2] and insert[3] is None:
                        end_insert[1] = COLOR_TYPES[color_type]['reset']
                        break

                # Replace every color reset of the current color type with our
                # color code to prevent them from interrupting this color
                for insert in inserts[end_index:start_index]:
                    if insert[2] and insert[3] in (color_type, None):
                        # A full reset is moved forward to our reset (replaced)
                        if insert[3] is None:
                            end_insert[1:4] = insert[1:4]

                        insert[1:4] = color_code, False, color_type

                # After all of the start inserts are added, the end ones can
                # placed in reverse order (outward from the match).
                inserts.insert(start_index, start_insert)
                reset_codes.insert(0, end_insert)

            for reset_code in reset_codes:
                inserts.insert(end_index, reset_code)

        return inserts

    def get_matches(self, data):
        '''Returns a list of tuples, each of which containing a start index, an
        end index, and the `Color` object for that match. The tuples of the
        latter rules are towards the end of the list.

        Args:
            data (bytes): Bytes against which each rule is matched.
        '''
        matches = []

        for rule in self.rules:
            if self.benchmark:
                checkpoint = time.perf_counter()
                rule_matches = rule.get_matches(data)
                result = time.perf_counter() - checkpoint

                total = self.benchmark_results.get(rule, 0) + result
                self.benchmark_results[rule] = total
            else:
                rule_matches = rule.get_matches(data)

            # If overlap is not allowed, replace any matches with \n to prevent
            # overlap while maintaining correct indexes on other rules' matches
            if rule.exclusive:
                for start, end, _ in reversed(rule_matches):
                    data = data[:start] + b'\n' * (end - start) + data[end:]

            matches += rule_matches

        return matches

    def highlight(self, data):
        '''Returns a highlighted bytes of `data`. The matches from the rules
        are gathered prior to inserting any color codes, making it so the rules
        can match without the color codes interfering.

        Args:
            data (bytes): Bytes to highlight.
        '''
        data, inserts = Color.strip_colors(data)
        inserts = self.get_inserts(data, inserts)

        resets_to_update = list(self._reset_codes)

        for position, color_code, is_reset, color_type in inserts:
            data = data[:position] + color_code + data[position:]

            if resets_to_update:
                # A full reset; default the remaining resets
                if color_type is None and is_reset:
                    for key in resets_to_update:
                        self._reset_codes[key] = COLOR_TYPES[key]['reset']

                    resets_to_update = []
                elif color_type in resets_to_update:
                    self._reset_codes[color_type] = color_code
                    resets_to_update.remove(color_type)

        return data

    def print_benchmark_results(self, descending=True):
        """Prints the benchmark results to stderr, sorted by usage.

        Args:
            descending (bool): Ordering of the results.
        """
        total = sum(self.benchmark_results.values())

        if self.benchmark_results:
            print('Rule benchmark results:', file=sys.stderr)

        for rule, result in sorted(self.benchmark_results.items(),
                                   key=lambda x: x[1],
                                   reverse=descending):
            rule = rule.description or repr(rule.regex.pattern.decode())
            fraction = f'{result / total:^7.2%}'

            print(f'{fraction} {result:.3f}s  {rule}', file=sys.stderr)
