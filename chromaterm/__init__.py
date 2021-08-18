"""Color your output to terminal"""
import os
import re

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

# Select Graphic Rendition sequence (any type)
SGR_RE = re.compile(r'\x1b\[[0-9;]*m')


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

    @property
    def color(self):
        """String that represents the color."""
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
        Updated when `self.color` is changed."""
        return self._color_code

    @property
    def color_reset(self):
        """ANSI escape sequence that instructs a terminal to revert to the
        default color. Updated when `self.color` is changed."""
        return self._color_reset

    @property
    def color_types(self):
        """List of tuples for each color type in this instance and its value. The
        types correspond to `COLOR_TYPES`. Updated when `self.color` is changed."""
        return self._color_types.copy()

    @property
    def rgb(self):
        """Flag for RGB-support. When changed, updates `self.color`."""
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
        """Decodes an SGR, splitting it into discrete colors. Each color is a list
        that contains:
            * color code (str),
            * is reset (bool), and
            * color type (str) which corresponds to `chromaterm.COLOR_TYPES`.

        Args:
            source_color_code (str): The string to be split into individual codes.
        """
        def make_sgr(code_id):
            return '\x1b[' + str(code_id) + 'm'

        colors = []
        codes = source_color_code.lstrip('\x1b[').rstrip('m').split(';')
        skip = 0

        for index, code in enumerate(codes):
            # Code processed by an index look-ahead; skip it
            if skip:
                skip -= 1
                continue

            # Full reset
            if code == '' or int(code) == 0:
                colors.append([make_sgr(code), True, None])
            # Multi-code SGR
            elif code in ('38', '48'):
                color_type = 'fg' if code == '38' else 'bg'

                # xterm-256
                if len(codes) > index + 2 and codes[index + 1] == '5':
                    skip = 2
                    code = ';'.join([str(codes[index + x]) for x in range(3)])
                # RGB
                elif len(codes) > index + 4 and codes[index + 1] == '2':
                    skip = 4
                    code = ';'.join([str(codes[index + x]) for x in range(5)])
                # Does not conform to format; do not touch code
                else:
                    return [[source_color_code, False, None]]

                color_code = make_sgr(code)
                is_reset = color_code == COLOR_TYPES[color_type]['reset']

                colors.append([color_code, is_reset, color_type])
            # Single-code SGR
            else:
                color_code = make_sgr(int(code))
                recognized = False

                for name, color_type in COLOR_TYPES.items():
                    if color_type['re'].search(color_code):
                        is_reset = color_code == color_type['reset']
                        recognized = True

                        colors.append([color_code, is_reset, name])

                # An SGR that isn't known; add it as is
                if not recognized:
                    colors.append([color_code, False, None])

        return colors

    @staticmethod
    def rgb_to_8bit(_r, _g, _b):
        """Downscale from 24-bit RGB to 8-bit ANSI."""
        def downscale(value, base=6):
            return int(value / 256 * base)

        # Use the 24 shades of the grayscale
        if _r == _g == _b:
            return 232 + downscale(_r, base=24)

        return 16 + (36 * downscale(_r)) + (6 * downscale(_g)) + downscale(_b)

    @staticmethod
    def strip_colors(data):
        """Returns data after stripping the existing colors and a list of inserts
        containing the stripped colors. The format of the insert is that of
        `Config.get_inserts`.

        Args:
            data (str): The string from which the colors should be stripped.
        """
        inserts = []
        match = SGR_RE.search(data)

        while match:
            start, end = match.span()

            for color in Color.decode_sgr(match.group()):
                color.insert(0, start)
                inserts.insert(0, color)

            # Remove match from data; next match's start is in the clean data
            data = data[:start] + data[end:]
            match = SGR_RE.search(data)

        return data, inserts


class Rule:
    """A rule that highlights parts of strings which match a regular expression."""
    def __init__(self, regex, color=None, description=None):
        """Constructor.

        Args:
            regex (str): Regular expression for getting matches in data.
            color (chromaterm.Color): Color used to highlight the entire match.
            description (str): String to help identify the rule.

        Raises:
            TypeError: If `regex` is not a string. If `color` is not an instance
                of `Color` or not `None`. If `description` is not a string.
        """
        self._colors = {}
        self.regex = regex
        self.description = description

        self.set_color(color)

    @property
    def color(self):
        """Color used for highlight the full match (group 0) of regex."""
        return self.colors.get(0)

    @color.setter
    def color(self, value):
        self.set_color(value)

    @property
    def colors(self):
        """Colors of the rule. It is dictionary where the keys are integers
        corresponding to the groups in `self.regex` and the values are instances
        `Color` which are used for highlighting."""
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

        self._regex = re.compile(value)

    def get_matches(self, data):
        """Returns a list of tuples, each of which containing a start index, an
        end index, and the `Color` object for that match. Only regex
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

    def set_color(self, color, group=0):
        """Sets a color to be used when highlighting. The group can be used to
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
        """
        if not isinstance(group, int):
            raise TypeError('group must be an integer')

        if color is None:
            self._colors.pop(group, None)
            return

        if not isinstance(color, Color):
            raise TypeError('color must be a chromaterm.Color')

        if group > self.regex.groups:
            raise ValueError('regex only has {} group(s); {} is '
                             'invalid'.format(self.regex.groups, group))

        self._colors[group] = color

        # Sort the colors according to the group number to ensure deterministic
        # highlighting
        self._colors = {k: self._colors[k] for k in sorted(self._colors)}


class Config:
    """An aggregation of multiple rules which provides improved highlighting by
    performing the regular expression matching of the rules before any colors
    are added to the string."""
    def __init__(self):
        """Constructor."""
        self._reset_codes = {k: v['reset'] for k, v in COLOR_TYPES.items()}
        self._rules = []

    @property
    def rules(self):
        """List of `Rule objects used during highlighting."""
        # Return a copy of the list to prevent modification, like extending it.
        # Modifying the content of the items is fine as it doesn't change the
        # object type.
        return self._rules.copy()

    def add_rule(self, rule):
        """Adds `rule` to the `self.rules` list.

        Args:
            rule (chromaterm.Rule): The rule to be added to the list of rules.

        Raises:
            TypeError: If `rule` is not an instance of `Rule`.
        """
        if not isinstance(rule, Rule):
            raise TypeError('rule must be a chromaterm.Rule')

        self._rules.append(rule)

    def remove_rule(self, rule):
        """Removes rule from `self.rules`.

        Args:
            rule (chromaterm.Rule): The rule to be removed from the list of rules.

        Raises:
            TypeError: If `rule` is not an instance of `Rule`.
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
        inserts = [] if not isinstance(inserts, list) else inserts

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
        end index, and the `Color` object for that match. The tuples of the
        latter rules are towards the end of the list.

        Args:
            data (str): A string against which each rule is matched.
        """
        matches = []

        for rule in self.rules:
            matches += rule.get_matches(data)

        return matches

    def highlight(self, data):
        """Returns a highlighted string of `data`. The matches from the rules
        are gathered prior to inserting any color codes, making it so the rules
        can match without the color codes interfering.

        Args:
            data (str): A string to highlight. `__str__` of `data` is called.
        """
        if not self.rules:
            return str(data)

        data, inserts = Color.strip_colors(str(data))
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
