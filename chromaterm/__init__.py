"""Color your output to terminal"""
import os
import re
import sys

__all__ = ['Color', 'Rule', 'Config']

# Color types, their color codes if it's style, their default reset codes, and
# RegEx's for detecting their color type.
COLOR_TYPES = {
    'fg': {
        'reset': '\x1b[39m',
        're': re.compile(r'\x1b\[(?:3[0-79]|9[0-7]|38;[0-9;]+)m')
    },
    'bg': {
        'reset': '\x1b[49m',
        're': re.compile(r'\x1b\[(?:4[0-79]|10[0-7]|48;[0-9;]+)m')
    },
    'blink': {
        'code': '\x1b[5m',
        'reset': '\x1b[25m',
        're': re.compile(r'\x1b\[2?5m')
    },
    'bold': {
        'code': '\x1b[1m',
        'reset': '\x1b[22m',  # Normal intensity
        're': re.compile(r'\x1b\[(?:1|2?2)m')  # Any intensity type
    },
    'italic': {
        'code': '\x1b[3m',
        'reset': '\x1b[23m',
        're': re.compile(r'\x1b\[2?3m')
    },
    'strike': {
        'code': '\x1b[9m',
        'reset': '\x1b[29m',
        're': re.compile(r'\x1b\[2?9m')
    },
    'underline': {
        'code': '\x1b[4m',
        'reset': '\x1b[24m',
        're': re.compile(r'\x1b\[2?4m')
    }
}

# Detect rgb support
RGB_SUPPORTED = os.getenv('COLORTERM') in ('truecolor', '24bit')

# Enable VT100 processing on stdout (Windows 10.0.10586). At exit, revert back
if sys.platform.startswith('win32'):  # pragma: no cover
    import atexit
    import ctypes
    import ctypes.wintypes

    # https://docs.microsoft.com/en-us/windows/console/getstdhandle
    STDOUT = ctypes.windll.kernel32.GetStdHandle(-11)
    MODE = ctypes.wintypes.DWORD()

    # https://docs.microsoft.com/en-us/windows/console/getconsolemode
    ctypes.windll.kernel32.GetConsoleMode(STDOUT, ctypes.byref(MODE))
    ctypes.windll.kernel32.SetConsoleMode(STDOUT, MODE.value | 0x0004)

    # Restore the old console mode before exiting
    atexit.register(ctypes.windll.kernel32.SetConsoleMode, STDOUT, MODE.value)

    # ANSI RGB is supported even on CMD since Windows 10.0.10586
    RGB_SUPPORTED = True


class Color:
    """A color that highlights strings for terminals."""
    def __init__(self, color, rgb=None):
        """Constructor.

        Args:
            color (str): A string which must contain

                * one foreground color (hex color prefixed with `f#`),
                * one background color (hex color prefixed with `b#`),
                * at least one style (blink, bold, italic, strike, underline), or
                * a combination of the above, seperated by spaces.

                Example: `"b#123123 bold"`
            rgb (bool): Whether the color is meant for RGB-enabled terminals or
                not. `False` will downscale the RGB colors to xterm-256. `None`
                will detect support for RGB and fallback to xterm-256.

        Raises:
            TypeError: If `color` is not a string. If `rgb` is not a boolean.
            ValueError: If the format of `color` is invalid.
        """
        self.rgb = rgb
        self.color = color

    def __call__(self, function):
        def wrapped(*args, **kwargs):
            return self.highlight(function(*args, **kwargs))

        return wrapped

    def __repr__(self):
        args = [repr(self.color)]

        if self.rgb is not None:
            args.append('rgb=' + repr(self.rgb))

        return '{}({})'.format(self.__class__.__name__, ', '.join(args))

    def __str__(self):
        return 'Color: ' + self.color

    @property
    def color(self):
        """String that represents the color. When changed, updates
        [color_code][chromaterm.Color.color_code] and
        [color_reset][chromaterm.Color.color_reset]."""
        return self._color

    @color.setter
    def color(self, value):
        if not isinstance(value, str):
            raise TypeError('color must be a string')

        value = value.lower().strip()
        styles = tuple(k for k, v in COLOR_TYPES.items() if v.get('code'))
        color_re = r'^(((b|f)#[0-9a-f]{6}|' + '|'.join(styles) + r')(\s+|$))+$'
        color_code = color_reset = ''
        color_types = []

        if not re.search(color_re, value):
            raise ValueError('invalid color format {}'.format(repr(value)))

        # Colors
        for target, hex_code in re.findall(r'(b|f)#([0-9a-f]{6})', value):
            if target == 'f':
                target, color_type = '\x1b[38;', 'fg'
            else:
                target, color_type = '\x1b[48;', 'bg'

            if color_type in [x[0] for x in color_types]:
                raise ValueError(
                    'color accepts one foreground and one background colors')

            # Break down hex color to red, green, and blue integers
            rgb_int = [int(hex_code[i:i + 2], 16) for i in [0, 2, 4]]

            if self.rgb or (self.rgb is None and RGB_SUPPORTED):
                target += '2;'
                color_id = ';'.join([str(x) for x in rgb_int])
            else:
                target += '5;'
                color_id = str(self.rgb_to_8bit(*rgb_int))

            color_code = color_code + target + color_id + 'm'
            color_reset = COLOR_TYPES[color_type]['reset'] + color_reset
            color_types.append((color_type, target + color_id + 'm'))

        # Styles
        for style in re.findall('|'.join(styles), value):
            if style in [x[0] for x in color_types]:
                raise ValueError('color does not accept duplicate styles')

            color_code = color_code + COLOR_TYPES[style]['code']
            color_reset = COLOR_TYPES[style]['reset'] + color_reset
            color_types.append((style, COLOR_TYPES[style]['code']))

        self._color = ' '.join(re.split(r'\s+', value.strip().lower()))
        self._color_code = color_code
        self._color_reset = color_reset
        self._color_types = color_types

    @property
    def color_code(self):
        """ANSI escape sequence that instructs a terminal to color output.
        Updated when [color][chromaterm.Color.color] is changed."""
        return self._color_code

    @property
    def color_reset(self):
        """ANSI escape sequence that instructs a terminal to revert to the
        default color. Updated when [color][chromaterm.Color.color] is changed."""
        return self._color_reset

    @property
    def color_types(self):
        """List of tuples for each color type in this instance and its value. The
        types correspond to `chromaterm.COLOR_TYPES`. Updated when
        [color][chromaterm.Color.color] is changed."""
        return self._color_types.copy()

    @property
    def rgb(self):
        """Flag for RGB-support. When changed, updates
        [color][chromaterm.Color.color]."""
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
    def rgb_to_8bit(_r, _g, _b):
        """Downscale from 24-bit RGB to 8-bit ANSI."""
        def downscale(value, base=6):
            return int(value / 256 * base)

        # Use the 24 shades of the grayscale
        if _r == _g == _b:
            return 232 + downscale(_r, base=24)

        return 16 + (36 * downscale(_r)) + (6 * downscale(_g)) + downscale(_b)

    def highlight(self, data, force=None):
        """Returns a highlighted string of `data`.

        Args:
            data (str): A string to highlight. `__str__` of `data` is
                called.
            force (bool): If `True`, the color codes are used when highlighting.
                If `False`, the color codes will be omitted, there by disabling
                any highlighting and simply returning back `data`. If `None`,
                the value is determined with `isatty`.
        """
        if force is None:
            force = getattr(sys.stdout, 'isatty', lambda: False)()

        if force is False:
            return str(data)

        return self.color_code + str(data) + self.color_reset

    def print(self, *args, force=None, **kwargs):
        """A wrapper for the `print` function. It highlights before printing.

        Args:
            *args (...): Arguments to be printed. Highlighted before being
                passed to the `print` function.
            force (bool): Passed to [highlight][chromaterm.Color.highlight].
            **kwargs (x=y): Keyword arguments passed to the `print` function.
        """
        print(*[self.highlight(arg, force=force) for arg in args], **kwargs)


class Rule:
    """A rule that highlights parts of strings which match a regular expression.
    The regular expression engine used is Python's
    [re](https://docs.python.org/3/library/re.html)."""
    def __init__(self, regex, color=None, description=None):
        """Constructor.

        Args:
            regex (str): A regular expression used for matching the input when
                highlighting.
            color (chromaterm.Color): A color used to highlight the matched
                input. This will default to highlighting the entire match (also
                known as group 0 of the regular expression). This can left to
                `None` if you intend to use [add_color][chromaterm.Rule.add_color]
                to manually specify which group in the regular expression should
                be highlighted.
            description (str): A description to help identify the rule.

        Raises:
            TypeError: If `regex` is not a string. If `color` is not an instance
                of [chromaterm.Color](../color/). If `description` is not a
                string.
        """
        self._colors = {}
        self.regex = regex
        self.description = description

        if color:
            self.color = color

    def __call__(self, function):
        def wrapped(*args, **kwargs):
            return self.highlight(function(*args, **kwargs))

        return wrapped

    def __repr__(self):
        args = [repr(self.regex.pattern)]

        if self.color:
            args.append('color=' + repr(self.color))
        if self.description:
            args.append('description=' + repr(self.description))

        return '{}({})'.format(self.__class__.__name__, ', '.join(args))

    def __str__(self):
        if self.description:
            return 'Rule: ' + self.description
        return 'Rule: ' + repr(self.regex.pattern)

    @property
    def color(self):
        """Color used for highlight the full match (group 0) of regex."""
        return self.colors.get(0)

    @color.setter
    def color(self, value):
        self.add_color(value)

    @property
    def colors(self):
        """Colors of the rule. It is dictionary where the keys are integers
        corresponding to the groups in [regex][chromaterm.Rule.regex] and the
        values are instances of [chromaterm.Color](../color/) which are used for
        highlighting."""
        # Return a copy of the dictionary to prevent modification of shallow
        # values; modifying the content of the values, like colors[0].rgb = True
        # is fine as it doesn't change the object type.
        return self._colors.copy()

    @property
    def description(self):
        """Description for the rule."""
        return self._description

    @description.setter
    def description(self, value):
        if value is not None and not isinstance(value, str):
            raise TypeError('description must be a string')

        self._description = value

    @property
    def regex(self):
        """Regular expression used for matching the input when highlighting."""
        return self._regex

    @regex.setter
    def regex(self, value):
        if not isinstance(value, str):
            raise TypeError('regex must be a string')

        self._regex = re.compile(value) if isinstance(value, str) else value

    def add_color(self, color, group=0):
        """Adds a color to be used when highlighting. The group can be used to
        limit the parts of the match which are highlighted. Group 0 (the default)
        will highlight the entire match. If a color already exists for the group,
        it is overwritten.

        Args:
            color (chromaterm.Color): A color for highlighting the matched input.
            group (int): The regex group to be be highlighted with the color.

        Raises:
            TypeError: If `color` is not an instance of
                [chromaterm.Color](../color/). If `group` is not an integer.
            ValueError: If `group` does not exist in the regular expression.
        """
        if not isinstance(color, Color):
            raise TypeError('color must be a chromaterm.Color')

        if not isinstance(group, int):
            raise TypeError('group must be an integer')

        if group > self.regex.groups:
            raise ValueError('regex only has {} group(s); {} is '
                             'invalid'.format(self.regex.groups, group))

        self._colors[group] = color

        # Sort the colors according to the group number to ensure deterministic
        # highlighting
        self._colors = {k: self._colors[k] for k in sorted(self._colors)}

    def remove_color(self, group):
        """Removes a color from the rule's colors.

        Args:
            group (int): The regex group. It is a key in the colors dictionary.

        Raises:
            TypeError: If `group` is not an integer.
        """
        if not isinstance(group, int):
            raise TypeError('group must be an integer')

        self._colors.pop(group, None)

    def get_matches(self, data):
        """Returns a list of tuples, each of which containing a start index, an
        end index, and the [chromaterm.Color][] object for that match. Only regex
        groups associated with a color are included.

        Args:
            data (str): A string to match regex against.
        """
        if not self.colors:
            return []

        matches = []

        for match in self.regex.finditer(data):
            for group in self.colors:
                start, end = match.span(group)

                # Zero-length match or optional group not in the match
                if start == end:
                    continue

                matches.append((start, end, self.colors[group]))

        return matches

    def highlight(self, data, force=None):
        """Returns a highlighted string of `data`. The regex of the rule is used
        along with the colors to highlight the matching parts of the `data`.

        Args:
            data (str): A string to highlight. `__str__` of `data` is called.
            force (bool): If `True`, the color codes are used when highlighting.
                If `False`, the color codes will be omitted, there by disabling
                any highlighting and simply returning back `data`. If `None`,
                the value is determined with `isatty`.
        """
        if force is None:
            force = getattr(sys.stdout, 'isatty', lambda: False)()

        if force is False:
            return str(data)

        data = str(data)
        inserts = []

        for start, end, color in self.get_matches(data):
            insert_index = 0

            # Arrange the inserts in reverse order (index magic over data)
            for index, (position, _) in enumerate(inserts):
                # A rule will never create overlapping (because of re.finditer),
                # so the start and end insert indexes will always be adjacent
                if start >= position:
                    insert_index = index
                    break

            inserts.insert(insert_index, (start, color.color_code))
            inserts.insert(insert_index, (end, color.color_reset))

        for position, code in inserts:
            data = data[:position] + code + data[position:]

        return data

    def print(self, *args, force=None, **kwargs):
        """A wrapper for the `print` function. It highlights before printing.

        Args:
            *args (...): Arguments to be printed. Highlighted before being
                passed to the `print` function.
            force (bool): Passed to [highlight][chromaterm.Rule.highlight].
            **kwargs (x=y): Keyword arguments passed to the `print` function.
        """
        print(*[self.highlight(arg, force=force) for arg in args], **kwargs)


class Config:
    """An aggregation of multiple rules which provides improved highlighting by
    performing the regular expression matching of the rules before any colors
    are added to the string."""
    def __init__(self):
        """Constructor."""
        self._reset_codes = {k: COLOR_TYPES[k]['reset'] for k in COLOR_TYPES}
        self._rules = []

    def __call__(self, function):
        def wrapped(*args, **kwargs):
            return self.highlight(function(*args, **kwargs))

        return wrapped

    def __repr__(self):
        return '{}()'.format(self.__class__.__name__)

    def __str__(self):
        count = len(self.rules)
        return 'Config: {} rule{}'.format(count, '' if count == 1 else 's')

    @property
    def rules(self):
        """List of [chromaterm.Rule](../rule/) objects used during highlighting."""
        # Return a copy of the list to prevent modification, like extending it.
        # Modifying the content of the items is fine as it doesn't change the
        # object type.
        return self._rules.copy()

    def add_rule(self, rule):
        """Adds `rule` to the [rules][chromaterm.Config.rules] list.

        Args:
            rule (chromaterm.Rule): The rule to be added to the list of rules.

        Raises:
            TypeError: If `rule` is not an instance of [chromaterm.Rule](../rule/).
        """
        if not isinstance(rule, Rule):
            raise TypeError('rule must be a chromaterm.Rule')

        self._rules.append(rule)

    def remove_rule(self, rule):
        """Removes rules from the [rules][chromaterm.Config.rules] list.

        Args:
            rule (chromaterm.Rule): The rule to be removed from the list of rules.

        Raises:
            TypeError: If `rule` is not an instance of [chromaterm.Rule](../rule/).
        """
        if not isinstance(rule, Rule):
            raise TypeError('rule must be a chromaterm.Rule')

        return self._rules.remove(rule)

    @staticmethod
    def get_insert_index(start, end, inserts):
        """Returns a tuple containing the start and end indexes for where they
        should be inserted into the inserts list in order to maintain the
        position-based descending (reverse) order.

        Args:
            start (int): The start position of a match.
            end (int): The end position of a match.
            inserts (list): A list of inserts, where the first item of each insert
                is the position.
        """
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

    def get_inserts(self, data, inserts=None):
        """Returns a list containing the inserts for the color codes relative to
        data. An insert is a list containing:

            * A position (index relative to data),
            * The code to be inserted,
            * A boolean indicating if its a reset code or not, and
            * The color type which corresponds to COLOR_TYPES, or None if it's a
                full SGR reset.

        The list of inserts is ordered in descending order based on the position
        of each insert relative to the data. This makes them easy to insert into
        data without calculating any index offset.

        Args:
            data (str): The string for which the matches are gathered.
            inserts (list): If this list is provided, the inserts are added to it.
                Any existing inserts are respected during processing, but ensure
                that they are sorted in descending order based on their position.
        """
        if not isinstance(inserts, list):
            inserts = []

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
        """Returns a list of tuples, each of which containing a start index, an
        end index, and the [chromaterm.Color](../color/) object for that match.
        The tuples of the latter rules are towards the end of the list.

        Args:
            data (str): A string against which each rule is matched.
        """
        matches = []

        for rule in self.rules:
            matches += rule.get_matches(data)

        return matches

    def highlight(self, data, force=None):
        """Returns a highlighted string of `data`. The matches from the rules
        are gathered prior to inserting any color codes, making it so the rules
        can match without the color codes interfering.

        Args:
            data (str): A string to highlight. `__str__` of `data` is called.
            force (bool): If `True`, the color codes are used when highlighting.
                If `False`, the color codes will be omitted, there by disabling
                any highlighting and simply returning back `data`. If `None`,
                the value is determined with `isatty`.
        """
        if force is None:
            force = getattr(sys.stdout, 'isatty', lambda: False)()

        if force is False or not self.rules:
            return str(data)

        data = str(data)

        for position, color_code, _, _ in self.get_inserts(data):
            data = data[:position] + color_code + data[position:]

        return data

    def print(self, *args, force=None, **kwargs):
        """A wrapper for the `print` function. It highlights before printing.

        Args:
            *args (...): Arguments to be printed. Highlighted before being
                passed to the `print` function.
            force (bool): Passed to [highlight][chromaterm.Config.highlight].
            **kwargs (x=y): Keyword arguments passed to the `print` function.
        """
        print(*[self.highlight(arg, force=force) for arg in args], **kwargs)
