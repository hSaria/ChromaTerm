"""chromaterm tests"""
import re

import pytest

import chromaterm

# pylint: disable=too-many-lines

# Aliases to two reset codes used during tests
RESET_BOLD = chromaterm.COLOR_TYPES['bold']['reset']
RESET_ITALIC = chromaterm.COLOR_TYPES['italic']['reset']


def test_color_known_colors():
    """Random known-good color codes."""
    colors = [
        'b#0973d8', 'b#8b6dd3', 'b#2867c7', 'b#18923a', 'b#636836', 'b#a0da5e',
        'b#b99153', 'b#fafb19', 'f#6cb0d7', 'f#6f5e3a', 'f#7d6256', 'f#15c93d',
        'f#45f2d7', 'f#50a910', 'f#1589b3', 'f#b8df23', 'f#d5a3bf', 'f#d7e764',
        'f#e002d7', 'f#f56726'
    ]
    codes = [
        '48;5;33m', '48;5;140m', '48;5;32m', '48;5;35m', '48;5;101m',
        '48;5;156m', '48;5;179m', '48;5;226m', '38;5;117m', '38;5;101m',
        '38;5;102m', '38;5;41m', '38;5;87m', '38;5;70m', '38;5;38m',
        '38;5;190m', '38;5;182m', '38;5;228m', '38;5;201m', '38;5;208m'
    ]

    for color_str, color_code in zip(colors, codes):
        assert chromaterm.Color(color_str).color_code == '\x1b[' + color_code


def test_color_known_grayscale():
    """Random known-good grayscale codes."""
    colors = [
        'b#000000', 'b#5d5d5d', 'b#373737', 'b#c8c8c8', 'b#cecece', 'b#d7d7d7',
        'b#d8d8d8', 'b#fcfcfc', 'f#0b0b0b', 'f#000000', 'f#2b2b2b', 'f#2f2f2f',
        'f#4c4c4c', 'f#4d4d4d', 'f#9d9d9d', 'f#808080'
    ]
    codes = [
        '48;5;232m', '48;5;240m', '48;5;237m', '48;5;250m', '48;5;251m',
        '48;5;252m', '48;5;252m', '48;5;255m', '38;5;233m', '38;5;232m',
        '38;5;236m', '38;5;236m', '38;5;239m', '38;5;239m', '38;5;246m',
        '38;5;244m'
    ]

    for color_str, color_code in zip(colors, codes):
        assert chromaterm.Color(color_str).color_code == '\x1b[' + color_code


def test_color_highlight():
    """Highlight some text"""
    color = chromaterm.Color('b#123123')

    data = 'hello'
    expected = [color.color_code, 'hello', color.color_reset]

    assert repr(color.highlight(data)) == repr(''.join(expected))


def test_color_change_color():
    """Confirm that the color_code is updated when the color or rgb is changed."""
    color = chromaterm.Color('b#123123')

    # Change from background to foreground
    old_color_code = color.color_code
    color.color = 'f#123123'
    assert color.color_code != old_color_code

    # Toggle rgb
    old_color_code = color.color_code
    color.rgb = not color.rgb
    assert color.color_code != old_color_code


def test_color_format_color_background():
    """Background color."""
    color = chromaterm.Color('b#123123')

    assert color.color == 'b#123123'
    assert color.color_code == '\x1b[48;5;22m'
    assert color.color_reset == '\x1b[49m'
    assert color.color_types == [('bg', '\x1b[48;5;22m')]
    assert color.rgb is False


def test_color_format_color_foreground():
    """Foreground color."""
    color = chromaterm.Color('f#123123')

    assert color.color == 'f#123123'
    assert color.color_code == '\x1b[38;5;22m'
    assert color.color_reset == '\x1b[39m'
    assert color.color_types == [('fg', '\x1b[38;5;22m')]
    assert color.rgb is False


def test_color_format_color_multiple():
    """Multiple different color types. Also tests case insensitivity"""
    color = chromaterm.Color('f#123123 b#aBCDef bOLd')

    assert color.color == 'f#123123 b#abcdef bold'
    assert color.color_code == '\x1b[38;5;22m\x1b[48;5;189m\x1b[1m'
    assert color.color_reset == '\x1b[22m\x1b[49m\x1b[39m'
    assert color.color_types == [
        ('fg', '\x1b[38;5;22m'),
        ('bg', '\x1b[48;5;189m'),
        ('bold', '\x1b[1m'),
    ]
    assert color.rgb is False


def test_color_format_color_rgb():
    """RGB color."""
    color = chromaterm.Color('b#123123', rgb=True)

    assert color.color == 'b#123123'
    assert color.color_code == '\x1b[48;2;18;49;35m'
    assert color.color_reset == '\x1b[49m'
    assert color.color_types == [('bg', '\x1b[48;2;18;49;35m')]
    assert color.rgb is True


def test_color_format_color_style():
    """Style color."""
    styles = {k: v for k, v in chromaterm.COLOR_TYPES.items() if v.get('code')}

    for style in styles:
        color = chromaterm.Color(style)

        assert color.color == style
        assert color.color_code == styles[style]['code']
        assert color.color_reset == styles[style]['reset']
        assert color.color_types == [(style, styles[style]['code'])]
        assert color.rgb is False


def test_color_invalid_type_color():
    """Color with an int for color."""
    with pytest.raises(TypeError, match='color must be a string'):
        chromaterm.Color(123)


def test_color_invalid_type_rgb():
    """Color with an int for rgb."""
    with pytest.raises(TypeError, match='rgb must be a boolean'):
        chromaterm.Color('bold', rgb=1)


def test_color_invalid_value_background():
    """Color with an incorrect background color."""
    with pytest.raises(ValueError, match='invalid color format'):
        chromaterm.Color('b#12312')

    with pytest.raises(ValueError, match='invalid color format'):
        chromaterm.Color('b#12312x')


def test_color_invalid_value_foreground():
    """Color with an incorrect foreground color."""
    with pytest.raises(ValueError, match='invalid color format'):
        chromaterm.Color('f#12312')

    with pytest.raises(ValueError, match='invalid color format'):
        chromaterm.Color('f#12312x')


def test_color_invalid_value_style():
    """Color with an incorrect style."""
    with pytest.raises(ValueError, match='invalid color format'):
        chromaterm.Color('something')


def test_color_invalid_value_duplicate_background():
    """Color with multiple background colors."""
    with pytest.raises(ValueError, match='one foreground and one background'):
        chromaterm.Color('b#123123 b#321321')


def test_color_invalid_value_duplicate_foreground():
    """Color with multiple background colors."""
    with pytest.raises(ValueError, match='one foreground and one background'):
        chromaterm.Color('f#123123 f#321321')


def test_color_invalid_value_duplicate_style():
    """Color with multiple background colors."""
    styles = (k for k, v in chromaterm.COLOR_TYPES.items() if v.get('code'))

    for style in styles:
        with pytest.raises(ValueError, match='duplicate styles'):
            chromaterm.Color('{0} {0}'.format(style))


def test_color_read_only_color_code():
    """Color's color_code attribute is read only."""
    color = chromaterm.Color('b#123123')

    with pytest.raises(AttributeError, match='can.t set attribute'):
        color.color_code = {}


def test_color_read_only_color_reset():
    """Color's color_reset attribute is read only."""
    color = chromaterm.Color('b#123123')

    with pytest.raises(AttributeError, match='can.t set attribute'):
        color.color_reset = {}


def test_color_read_only_color_types():
    """Color's color_types attribute is read only."""
    color = chromaterm.Color('b#123123')

    with pytest.raises(AttributeError, match='can.t set attribute'):
        color.color_types = {}


def test_color_shallow_copied_color_types():
    """Color's color_types attribute should be shallow copied on read to prevent
    changes to the list's items."""
    color = chromaterm.Color('b#123123')
    assert color.color_types is not color.color_types


def test_color___call__():
    """Confim Color's decorator (__call__)."""
    color = chromaterm.Color('b#123123')

    @color
    def echo(*args):
        return ', '.join(args)

    assert repr(color.highlight('hello world')) == repr(echo('hello world'))


def test_color___repr__():
    """Confim Color's __repr__ format."""
    color = chromaterm.Color('f#123123   bold   b#123123  ')
    assert repr(color) == "Color('f#123123 bold b#123123')"

    color = chromaterm.Color('bold b#123123', rgb=True)
    assert repr(color) == "Color('bold b#123123', rgb=True)"


def test_color___str__():
    """Confim Color's __str__ format."""
    color = chromaterm.Color('b#123123')
    assert str(color) == 'Color: b#123123'


def test_rule_add_color_default_group():
    """Add to a rule a color to the default regex group (0)."""
    rule = chromaterm.Rule('hello')
    assert rule.color is None

    color = chromaterm.Color('bold')
    rule.add_color(color)
    assert rule.color is color
    assert rule.colors.get(0) is color


def test_rule_add_color_specific_group():
    """Add to a rule a color to a specific regex group. Additionally, ensure
    that the dict keys are ordered according to the group number."""
    rule = chromaterm.Rule('he(llo)')
    assert rule.color is None

    color = chromaterm.Color('bold')
    rule.add_color(color, group=1)
    assert rule.color is None
    assert rule.colors.get(1) is color

    rule.add_color(color)
    assert list(rule.colors) == [0, 1]


def test_rule_add_color_invalid_type_color():
    """Add to a rule a color with invalid group type."""
    rule = chromaterm.Rule('hello')

    with pytest.raises(TypeError, match='color must be a chromaterm.Color'):
        rule.add_color(True)


def test_rule_add_color_invalid_type_group():
    """Add to a rule a color with invalid color type."""
    rule = chromaterm.Rule('hello')

    with pytest.raises(TypeError, match='group must be an integer'):
        rule.add_color(chromaterm.Color('bold'), group='3')


def test_rule_add_color_invalid_value_group():
    """Add to a rule a color with invalid group value."""
    rule = chromaterm.Rule('hello')

    with pytest.raises(ValueError, match='regex only has 0 group'):
        rule.add_color(chromaterm.Color('bold'), group=2)


def test_rule_remove_color_default_group():
    """Remove from a rule the color of the default regex group (0)."""
    rule = chromaterm.Rule('hello')
    rule.add_color(chromaterm.Color('bold'))
    assert rule.colors.get(0) is not None

    rule.remove_color(0)
    assert rule.colors.get(0) is None


def test_rule_remove_color_specific_group():
    """Remove from a rule the color of a specific regex group."""
    rule = chromaterm.Rule('he(llo)')
    rule.add_color(chromaterm.Color('bold'), group=1)
    assert rule.colors.get(1) is not None

    rule.remove_color(1)
    assert rule.colors.get(1) is None


def test_rule_remove_color_invalid_type_group():
    """Add to a rule a color with invalid group value."""
    rule = chromaterm.Rule('hello')

    with pytest.raises(TypeError, match='group must be an integer'):
        rule.remove_color('2')


def test_rule_highlight():
    """Highlight with rule that only colors the default regex group."""
    color = chromaterm.Color('bold')
    rule = chromaterm.Rule('hello|world', color=color)

    data = 'hello world'
    expected = [
        color.color_code, 'hello', color.color_reset, ' ', color.color_code,
        'world', color.color_reset
    ]

    assert repr(rule.highlight(data)) == repr(''.join(expected))


def test_rule_highlight_groups():
    """Highlight with rule that colors default and specific regex groups."""
    color1 = chromaterm.Color('bold')
    color2 = chromaterm.Color('b#123123')
    color3 = chromaterm.Color('f#321321')
    rule = chromaterm.Rule('(hello) (world)', color=color1)
    rule.add_color(color2, group=1)
    rule.add_color(color3, group=2)

    data = 'hello world'
    expected = [
        color1.color_code, color2.color_code, 'hello', color2.color_reset, ' ',
        color3.color_code, 'world', color3.color_reset, color1.color_reset
    ]

    assert repr(rule.highlight(data)) == repr(''.join(expected))


def test_rule_highlight_groups_optional_not_matches():
    """Highlight with rule that has a specific regex group that is optional. When
    the optional group is not in the match, the color should not be inserted."""
    color = chromaterm.Color('bold')
    rule = chromaterm.Rule('hello(,)? world')
    rule.add_color(color, group=1)

    data = 'hello, world'
    expected = ['hello', color.color_code, ',', color.color_reset, ' world']

    assert repr(rule.highlight(data)) == repr(''.join(expected))

    data = 'hello world'
    expected = ['hello world']

    assert repr(rule.highlight(data)) == repr(''.join(expected))


def test_rule_highlight_no_colors():
    """Highlight with rule that has no colors – nothing is changed."""
    rule = chromaterm.Rule('hello')
    assert repr(rule.highlight('hello world')) == repr('hello world')


def test_rule_highlight_zero_length_match():
    """Highlight with rule that has a regex that matches zero-length data."""
    color = chromaterm.Color('bold')
    rule = chromaterm.Rule('hello() world')
    rule.add_color(color, group=1)

    assert repr(rule.highlight('hello world')) == repr('hello world')


def test_rule_change_color():
    """Confirm that a color change overrides the old one."""
    rule = chromaterm.Rule('hello', color=chromaterm.Color('bold'))

    old_color = rule.color
    rule.color = chromaterm.Color('b#123123')
    assert old_color is not rule.color


def test_rule_change_regex():
    """Confirm that a regex change is correctly compiled into a pattern."""
    rule = chromaterm.Rule('hello')

    rule.regex = 'hey'
    assert isinstance(rule.regex, re.Pattern)
    assert rule.regex.pattern == 'hey'

    rule.regex = re.compile('hi')
    assert isinstance(rule.regex, re.Pattern)
    assert rule.regex.pattern == 'hi'


def test_rule_invalid_type_color():
    """Rule with an invalid color type."""
    with pytest.raises(TypeError, match='color must be a chromaterm.Color'):
        chromaterm.Rule('hello', color=True)


def test_rule_invalid_type_description():
    """Rule with an invalid description type."""
    with pytest.raises(TypeError, match='description must be a string'):
        chromaterm.Rule('hello', description=True)


def test_rule_invalid_type_regex():
    """Rule with an invalid regex type."""
    with pytest.raises(TypeError, match='regex must be a string or re.Patter'):
        chromaterm.Rule(True)


def test_rule_read_only_colors():
    """Rule's colors attribute is read only."""
    rule = chromaterm.Rule('hello')

    with pytest.raises(AttributeError, match='can.t set attribute'):
        rule.colors = {}


def test_rule_shallow_copied_colors():
    """Rule's colors attribute should be shallow copied on read to prevent
    changes to the keys and values. Modifying the content of the values, like
    colors[0].rgb = True is fine as it doesn't change the object type."""
    rule = chromaterm.Rule('hello')
    assert rule.colors is not rule.colors


def test_rule___call__():
    """Confim Rule's decorator (__call__)."""
    rule = chromaterm.Rule('hello', color=chromaterm.Color('bold'))

    @rule
    def echo(*args):
        return ', '.join(args)

    assert repr(rule.highlight('hello world')) == repr(echo('hello world'))


def test_rule___repr__():
    """Confim Rule's __repr__ format."""
    rule = chromaterm.Rule('hello')
    assert repr(rule) == "Rule('hello')"

    rule = chromaterm.Rule('hello', color=chromaterm.Color('bold'))
    assert repr(rule) == "Rule('hello', color=Color('bold'))"

    rule = chromaterm.Rule('hello', description='Yo')
    assert repr(rule) == "Rule('hello', description='Yo')"

    rule = chromaterm.Rule('hello',
                           color=chromaterm.Color('bold'),
                           description='Yo')
    assert repr(rule) == "Rule('hello', color=Color('bold'), description='Yo')"


def test_rule___str__():
    """Confim Rule's __str__ format."""
    rule = chromaterm.Rule('hello')
    assert str(rule) == "Rule: 'hello'"

    rule = chromaterm.Rule('hello', description='Yo')
    assert str(rule) == 'Rule: Yo'


def test_config_add_rule():
    """Add a rule to config."""
    config = chromaterm.Config()
    rule = chromaterm.Rule('hello')

    config.add_rule(rule)
    assert rule in config.rules


def test_config_add_rule_invalid_type_rule():
    """Add a rule to config with invalid rule type."""
    config = chromaterm.Config()

    with pytest.raises(TypeError, match='rule must be a chromaterm.Rule'):
        config.add_rule(True)


def test_config_remove_rule():
    """Remove a rule from config."""
    config = chromaterm.Config()
    rule1 = chromaterm.Rule('hello')
    rule2 = chromaterm.Rule('hello')

    config.add_rule(rule1)
    config.add_rule(rule2)
    assert rule1 in config.rules
    assert rule2 in config.rules

    config.remove_rule(rule2)
    assert rule1 in config.rules
    assert rule2 not in config.rules


def test_config_remove_rule_invalid_type_rule():
    """Remove a rule from config with invalid rule type."""
    config = chromaterm.Config()

    with pytest.raises(TypeError, match='rule must be a chromaterm.Rule'):
        config.remove_rule(True)


def test_config_highlight():
    """Highlight with one rule."""
    config = chromaterm.Config()

    rule = chromaterm.Rule('hello', color=chromaterm.Color('b#123123'))
    config.add_rule(rule)

    data = 'hello world'
    expected = [
        rule.color.color_code, 'hello', rule.color.color_reset, ' world'
    ]

    assert repr(config.highlight(data)) == repr(''.join(expected))


def test_config_highlight_adjoin_type_different():
    """Two rules with different color types, and one ending where the other
    starts. Both are applied without any overlap in the codes, independent of the
    order.
    1: -------
    2:        -------"""
    config = chromaterm.Config()

    rule1 = chromaterm.Rule('he', color=chromaterm.Color('b#123123'))
    rule2 = chromaterm.Rule('llo', color=chromaterm.Color('f#321321'))
    config.add_rule(rule1)
    config.add_rule(rule2)

    data = 'hello'
    expected = [
        rule1.color.color_code, 'he', rule1.color.color_reset,
        rule2.color.color_code, 'llo', rule2.color.color_reset
    ]

    assert repr(config.highlight(data)) == repr(''.join(expected))

    config.remove_rule(rule1)
    config.add_rule(rule1)

    assert repr(config.highlight(data)) == repr(''.join(expected))


def test_config_highlight_adjoin_type_mixed():
    """Two rules with mixed color types, with one ending where the other starts.
    Both are applied without any overlap in the codes, independent of the
    order.
    1: -------
    2:        -------"""
    config = chromaterm.Config()

    rule1 = chromaterm.Rule('he', color=chromaterm.Color('b#123123 italic'))
    rule2 = chromaterm.Rule('llo', color=chromaterm.Color('b#321321 bold'))
    config.add_rule(rule1)
    config.add_rule(rule2)

    data = 'hello'
    expected = [
        rule1.color.color_code, 'he', rule1.color.color_reset,
        rule2.color.color_code, 'llo', rule2.color.color_reset
    ]

    assert repr(config.highlight(data)) == repr(''.join(expected))

    config.remove_rule(rule1)
    config.add_rule(rule1)

    assert repr(config.highlight(data)) == repr(''.join(expected))


def test_config_highlight_adjoin_type_same():
    """Two rules with same color type, with one ending where the other starts.
    Both are applied without any overlap in the codes, independent of the order.
    1: -------
    2:        -------"""
    config = chromaterm.Config()

    rule1 = chromaterm.Rule('he', color=chromaterm.Color('b#123123'))
    rule2 = chromaterm.Rule('llo', color=chromaterm.Color('b#321321'))
    config.add_rule(rule1)
    config.add_rule(rule2)

    data = 'hello'
    expected = [
        rule1.color.color_code, 'he', rule1.color.color_reset,
        rule2.color.color_code, 'llo', rule2.color.color_reset
    ]

    assert repr(config.highlight(data)) == repr(''.join(expected))

    config.remove_rule(rule1)
    config.add_rule(rule1)

    assert repr(config.highlight(data)) == repr(''.join(expected))


def test_config_highlight_common_beginning_type_different():
    """Two rules with different color types, and both sharing the same start of
    a match. The most recent rule will be closer to the match's start.
    1: --------------
    2: -------"""
    config = chromaterm.Config()

    rule1 = chromaterm.Rule('hello', color=chromaterm.Color('b#123123'))
    rule2 = chromaterm.Rule('he', color=chromaterm.Color('f#321321'))
    config.add_rule(rule1)
    config.add_rule(rule2)

    data = 'hello'
    expected = [
        rule1.color.color_code, rule2.color.color_code, 'he',
        rule2.color.color_reset, 'llo', rule1.color.color_reset
    ]

    assert repr(config.highlight(data)) == repr(''.join(expected))

    config.remove_rule(rule1)
    config.add_rule(rule1)

    # Flip start color
    expected[0], expected[1] = expected[1], expected[0]

    assert repr(config.highlight(data)) == repr(''.join(expected))


def test_config_highlight_common_beginning_type_mixed():
    """Two rules with different color types, and both sharing the same start of
    a match. The most recent rule will be closer to the match's start.
    1: --------------
    2: -------"""
    config = chromaterm.Config()

    rule1 = chromaterm.Rule('hello', color=chromaterm.Color('b#123123 italic'))
    rule2 = chromaterm.Rule('he', color=chromaterm.Color('b#321321 bold'))
    config.add_rule(rule1)
    config.add_rule(rule2)

    data = 'hello'
    expected = [
        rule1.color.color_code, rule2.color.color_code, 'he', RESET_BOLD,
        rule1.color.color_types[0][1], 'llo', rule1.color.color_reset
    ]

    assert repr(config.highlight(data)) == repr(''.join(expected))


def test_config_highlight_common_beginning_type_same():
    """Two rules with same color type, and both sharing the same start of a
    match. The most recent rule will be closer to the match's start.
    1: --------------
    2: -------"""
    config = chromaterm.Config()

    rule1 = chromaterm.Rule('hello', color=chromaterm.Color('b#123123'))
    rule2 = chromaterm.Rule('he', color=chromaterm.Color('b#321321'))
    config.add_rule(rule1)
    config.add_rule(rule2)

    data = 'hello'
    expected = [
        rule1.color.color_code, rule2.color.color_code, 'he',
        rule1.color.color_code, 'llo', rule1.color.color_reset
    ]

    assert repr(config.highlight(data)) == repr(''.join(expected))


def test_config_highlight_common_end_type_different():
    """Two rules with different color types, and both sharing the same end of a
    match. The most recent rule will be closer to the match's end.
    1: --------------
    2:        -------"""
    config = chromaterm.Config()

    rule1 = chromaterm.Rule('hello', color=chromaterm.Color('b#123123'))
    rule2 = chromaterm.Rule('llo', color=chromaterm.Color('f#321321'))
    config.add_rule(rule1)
    config.add_rule(rule2)

    data = 'hello'
    expected = [
        rule1.color.color_code, 'he', rule2.color.color_code, 'llo',
        rule2.color.color_reset, rule1.color.color_reset
    ]

    assert repr(config.highlight(data)) == repr(''.join(expected))

    config.remove_rule(rule1)
    config.add_rule(rule1)

    # Flip end color
    expected[-1], expected[-2] = expected[-2], expected[-1]

    assert repr(config.highlight(data)) == repr(''.join(expected))


def test_config_highlight_common_end_type_mixed():
    """Two rules with different color types, and both sharing the same end of a
    match. The most recent rule will be closer to the match's end.
    1: --------------
    2:        -------"""
    config = chromaterm.Config()

    rule1 = chromaterm.Rule('hello', color=chromaterm.Color('b#123123 italic'))
    rule2 = chromaterm.Rule('llo', color=chromaterm.Color('b#321321 bold'))
    config.add_rule(rule1)
    config.add_rule(rule2)

    data = 'hello'
    expected = [
        rule1.color.color_code, 'he', rule2.color.color_code, 'llo',
        RESET_BOLD, rule1.color.color_types[0][1], rule1.color.color_reset
    ]

    assert repr(config.highlight(data)) == repr(''.join(expected))


def test_config_highlight_common_end_type_same():
    """Two rules with same color type, and both sharing the same end of a
    match. The most recent rule will be closer to the match's end.
    1: --------------
    2:        -------"""
    config = chromaterm.Config()

    rule1 = chromaterm.Rule('hello', color=chromaterm.Color('b#123123'))
    rule2 = chromaterm.Rule('llo', color=chromaterm.Color('b#321321'))
    config.add_rule(rule1)
    config.add_rule(rule2)

    data = 'hello'
    expected = [
        rule1.color.color_code, 'he', rule2.color.color_code, 'llo',
        rule1.color.color_code, rule1.color.color_reset
    ]

    assert repr(config.highlight(data)) == repr(''.join(expected))


def test_config_highlight_encapsulate_type_different():
    """Three rules, with matches that encapsulate each other, different color
    types. None of them should affect the rest as they are of different types. The
    order in which the rules are applied should not matter.
    1:     --
    2:   ------
    3: ----------"""
    config = chromaterm.Config()

    rule1 = chromaterm.Rule('lo wo', color=chromaterm.Color('b#123123'))
    rule2 = chromaterm.Rule('llo wor', color=chromaterm.Color('f#321321'))
    rule3 = chromaterm.Rule('hello world', color=chromaterm.Color('bold'))
    config.add_rule(rule1)
    config.add_rule(rule2)
    config.add_rule(rule3)

    data = 'hello world'
    expected = [
        rule3.color.color_code, 'he', rule2.color.color_code, 'l',
        rule1.color.color_code, 'lo wo', rule1.color.color_reset, 'r',
        rule2.color.color_reset, 'ld', rule3.color.color_reset
    ]

    assert repr(config.highlight(data)) == repr(''.join(expected))

    config.remove_rule(rule1)
    config.remove_rule(rule2)
    config.add_rule(rule2)
    config.add_rule(rule1)

    assert repr(config.highlight(data)) == repr(''.join(expected))


def test_config_highlight_encapsulate_type_mixed():
    """Three rules, with matches that encapsulate each other, mixed color types.
    The colors that are different should not affect each other, but those that
    are the same should correctly track each others colors and update their
    resets appropriately.
    1:     --
    2:   ------
    3: ----------"""
    config = chromaterm.Config()

    rule1 = chromaterm.Rule('lo wo', color=chromaterm.Color('b#123123 italic'))
    rule2 = chromaterm.Rule('llo wor', color=chromaterm.Color('b#321321 bold'))
    rule3 = chromaterm.Rule('hello world', color=chromaterm.Color('bold'))
    config.add_rule(rule1)
    config.add_rule(rule2)
    config.add_rule(rule3)

    data = 'hello world'
    expected = [
        rule3.color.color_code, 'he', rule2.color.color_code, 'l',
        rule1.color.color_code, 'lo wo', RESET_ITALIC,
        rule2.color.color_types[0][1], 'r', rule2.color.color_types[1][1],
        chromaterm.COLOR_TYPES[rule2.color.color_types[0][0]]['reset'], 'ld',
        rule3.color.color_reset
    ]

    assert repr(config.highlight(data)) == repr(''.join(expected))

    config.remove_rule(rule1)
    config.remove_rule(rule2)
    config.add_rule(rule2)
    config.add_rule(rule1)

    assert repr(config.highlight(data)) == repr(''.join(expected))


def test_config_highlight_encapsulate_type_same():
    """Three rules, with matches that encapsulate each other, same color type.
    The reset of a match would be updated to the color of the encapsulating one.
    Once the second match updates the reset of the first one to a color, it is no
    longer considered a reset, and therefore the third match would not update it
    to its own color. The order in which the rules are applied should not matter.
    1:     --
    2:   ------
    3: ----------"""
    config = chromaterm.Config()

    rule1 = chromaterm.Rule('lo wo', color=chromaterm.Color('b#123123'))
    rule2 = chromaterm.Rule('llo wor', color=chromaterm.Color('b#321321'))
    rule3 = chromaterm.Rule('hello world', color=chromaterm.Color('b#abcabc'))
    config.add_rule(rule1)
    config.add_rule(rule2)
    config.add_rule(rule3)

    data = 'hello world'
    expected = [
        rule3.color.color_code, 'he', rule2.color.color_code, 'l',
        rule1.color.color_code, 'lo wo', rule2.color.color_code, 'r',
        rule3.color.color_code, 'ld', rule3.color.color_reset
    ]

    assert repr(config.highlight(data)) == repr(''.join(expected))

    config.remove_rule(rule1)
    config.remove_rule(rule2)
    config.add_rule(rule2)
    config.add_rule(rule1)

    assert repr(config.highlight(data)) == repr(''.join(expected))


def test_config_highlight_overlap_full_type_different():
    """Two rules, fully overlapping matches, different color types. Should not
    affect each other as they have different types. Most recent rule should be
    closest to the match.
    1: ----------
    2: ----------"""
    config = chromaterm.Config()

    rule1 = chromaterm.Rule('hello', color=chromaterm.Color('b#123123'))
    rule2 = chromaterm.Rule('hello', color=chromaterm.Color('f#321321'))
    config.add_rule(rule1)
    config.add_rule(rule2)

    data = 'hello'
    expected = [
        rule1.color.color_code, rule2.color.color_code, 'hello',
        rule2.color.color_reset, rule1.color.color_reset
    ]

    assert repr(config.highlight(data)) == repr(''.join(expected))


def test_config_highlight_overlap_full_type_mixed():
    """Two rules, fully overlapping matches, mixed color types. The different
    color types should not affect each other, but those that are the same should
    update the oldest reset with the most recent rule's color code.
    1: ----------
    2: ----------"""
    config = chromaterm.Config()

    rule1 = chromaterm.Rule('hello', color=chromaterm.Color('b#123123 bold'))
    rule2 = chromaterm.Rule('hello', color=chromaterm.Color('b#321321 italic'))
    config.add_rule(rule1)
    config.add_rule(rule2)

    data = 'hello'

    expected = [
        rule1.color.color_code, rule2.color.color_code, 'hello', RESET_ITALIC,
        rule1.color.color_types[0][1], rule1.color.color_reset
    ]

    assert repr(config.highlight(data)) == repr(''.join(expected))


def test_config_highlight_overlap_full_type_same():
    """Two rules, fully overlapping matches, same color type. The reset of the
    first rule should be replaced with the color code of the second (most recent)
    rule to prevent the reset from interrupting the color.
    1: ----------
    2: ----------"""
    config = chromaterm.Config()

    rule1 = chromaterm.Rule('hello', color=chromaterm.Color('b#123123'))
    rule2 = chromaterm.Rule('hello', color=chromaterm.Color('b#321321'))
    config.add_rule(rule1)
    config.add_rule(rule2)

    data = 'hello'
    expected = [
        rule1.color.color_code, rule2.color.color_code, 'hello',
        rule1.color.color_code, rule1.color.color_reset
    ]

    assert repr(config.highlight(data)) == repr(''.join(expected))


def test_config_highlight_overlap_partial_type_different():
    """Two rules, partially overlapping matches, different color types. The rules
    should not affect each other. Tested in reverse order, too.
    1: ----------
    2:     ----------"""
    config = chromaterm.Config()

    rule1 = chromaterm.Rule('hell', color=chromaterm.Color('b#123123'))
    rule2 = chromaterm.Rule('llo', color=chromaterm.Color('f#321321'))
    config.add_rule(rule1)
    config.add_rule(rule2)

    data = 'hello'
    expected = [
        rule1.color.color_code, 'he', rule2.color.color_code, 'll',
        rule1.color.color_reset, 'o', rule2.color.color_reset
    ]

    assert repr(config.highlight(data)) == repr(''.join(expected))

    config.remove_rule(rule1)
    config.add_rule(rule1)

    assert repr(config.highlight(data)) == repr(''.join(expected))


def test_config_highlight_overlap_partial_type_mixed():
    """Two rules, partially overlapping matches, mixed color types. The different
    color types should not affect each other, but those that are the same should
    correctly update the reset that is in the middle of the other match to that
    of the match's color. Order of rules should not matter.
    1: ----------
    2:     ----------"""
    config = chromaterm.Config()

    rule1 = chromaterm.Rule('hell', color=chromaterm.Color('b#123123 bold'))
    rule2 = chromaterm.Rule('llo', color=chromaterm.Color('b#321321 italic'))
    config.add_rule(rule1)
    config.add_rule(rule2)

    data = 'hello'
    expected = [
        rule1.color.color_code, 'he', rule2.color.color_code, 'll', RESET_BOLD,
        rule2.color.color_types[0][1], 'o', rule2.color.color_reset
    ]

    assert repr(config.highlight(data)) == repr(''.join(expected))

    config.remove_rule(rule1)
    config.add_rule(rule1)

    assert repr(config.highlight(data)) == repr(''.join(expected))


def test_config_highlight_overlap_partial_type_same():
    """Two rules, partially overlapping matches, same color type. The reset of
    the first rule should be replaced with the color code of the second (most
    recent) rule to prevent the reset from interrupting the color. The behavior
    should be consistent regardless of the order of the rules.
    1: ----------
    2:     ----------"""
    config = chromaterm.Config()

    rule1 = chromaterm.Rule('hell', color=chromaterm.Color('b#123123'))
    rule2 = chromaterm.Rule('llo', color=chromaterm.Color('b#321321'))
    config.add_rule(rule1)
    config.add_rule(rule2)

    data = 'hello'
    expected = [
        rule1.color.color_code, 'he', rule2.color.color_code, 'll',
        rule2.color.color_code, 'o', rule2.color.color_reset
    ]

    assert repr(config.highlight(data)) == repr(''.join(expected))

    config.remove_rule(rule1)
    config.add_rule(rule1)

    assert repr(config.highlight(data)) == repr(''.join(expected))


def test_config_highlight_no_rules():
    """Highlight with config that has no rules – nothing is changed."""
    config = chromaterm.Config()
    assert repr(config.highlight('hello world')) == repr('hello world')


def test_config_read_only_colors():
    """Config's rules attribute is read only."""
    config = chromaterm.Config()

    with pytest.raises(AttributeError, match='can.t set attribute'):
        config.rules = []


def test_config_shallow_copied_rules():
    """Config's rules attribute should be shallow copied on read to prevent
    changes to the list's items. Modifying the content of the items, like
    colors[0].rgb = True is fine as it doesn't change the object type."""
    config = chromaterm.Config()
    assert config.rules is not config.rules


def test_config___call__():
    """Confim Config's decorator (__call__)."""
    config = chromaterm.Config()
    config.add_rule(chromaterm.Rule('hello', color=chromaterm.Color('bold')))
    config.add_rule(chromaterm.Rule('world', color=chromaterm.Color('italic')))

    @config
    def echo(*args):
        return ', '.join(args)

    assert repr(config.highlight('hello world')) == repr(echo('hello world'))


def test_config___repr__():
    """Confim Config's __repr__ format."""
    config = chromaterm.Config()
    assert repr(config) == 'Config()'


def test_config___str__():
    """Confim Config's __str__ format."""
    config = chromaterm.Config()
    assert str(config) == 'Config: 0 rules'

    config.add_rule(chromaterm.Rule('hello'))
    assert str(config) == 'Config: 1 rule'

    config.add_rule(chromaterm.Rule('hello'))
    assert str(config) == 'Config: 2 rules'
