"""chromaterm tests"""
import pytest

import chromaterm

# pylint: disable=too-many-lines

# Aliases to two reset codes used during tests
RESET_BOLD = chromaterm.COLOR_TYPES['bold']['reset']
RESET_ITALIC = chromaterm.COLOR_TYPES['italic']['reset']


def make_sgr(*code_id):
    """Returns byte-array for an SGR of `code_id` (multiple are concatenated)."""
    return b''.join(b'\x1b[' + x + b'm' for x in code_id)


def test_color_known_colors():
    """Random known-good color codes."""
    colors = [
        'b#0973d8', 'b#8b6dd3', 'b#2867c7', 'b#18923a', 'b#636836', 'b#a0da5e',
        'b#b99153', 'b#fafb19', 'f#6cb0d7', 'f#6f5e3a', 'f#7d6256', 'f#15c93d',
        'f#45f2d7', 'f#50a910', 'f#1589b3', 'f#b8df23', 'f#d5a3bf', 'f#d7e764',
        'f#e002d7', 'f#f56726'
    ]
    codes = [
        b'48;5;33', b'48;5;140', b'48;5;32', b'48;5;35', b'48;5;101',
        b'48;5;156', b'48;5;179', b'48;5;226', b'38;5;117', b'38;5;101',
        b'38;5;102', b'38;5;41', b'38;5;87', b'38;5;70', b'38;5;38',
        b'38;5;190', b'38;5;182', b'38;5;228', b'38;5;201', b'38;5;208'
    ]

    for color_str, color_code in zip(colors, codes):
        color = chromaterm.Color(color_str, rgb=False)
        assert color.color_code == make_sgr(color_code)


def test_color_known_grayscale():
    """Random known-good grayscale codes."""
    colors = [
        'b#000000', 'b#5d5d5d', 'b#373737', 'b#c8c8c8', 'b#cecece', 'b#d7d7d7',
        'b#d8d8d8', 'b#fcfcfc', 'f#0b0b0b', 'f#000000', 'f#2b2b2b', 'f#2f2f2f',
        'f#4c4c4c', 'f#4d4d4d', 'f#9d9d9d', 'f#808080'
    ]
    codes = [
        b'48;5;232', b'48;5;240', b'48;5;237', b'48;5;250', b'48;5;251',
        b'48;5;252', b'48;5;252', b'48;5;255', b'38;5;233', b'38;5;232',
        b'38;5;236', b'38;5;236', b'38;5;239', b'38;5;239', b'38;5;246',
        b'38;5;244'
    ]

    for color_str, color_code in zip(colors, codes):
        color = chromaterm.Color(color_str, rgb=False)
        assert color.color_code == make_sgr(color_code)


def test_color_decode_sgr_bg():
    """Background colors and reset are being detected."""
    for code in [b'48;5;12', b'48;2;1;1;1', b'49', b'101']:
        colors = chromaterm.Color.decode_sgr(make_sgr(code))
        assert len(colors) == 1

        for color_code, _, color_type in colors:
            assert color_code == make_sgr(code)
            assert color_type == 'bg'


def test_color_decode_sgr_fg():
    """Foreground colors and reset are being detected."""
    for code in [b'38;5;12', b'38;2;1;1;1', b'39', b'91']:
        colors = chromaterm.Color.decode_sgr(make_sgr(code))
        assert len(colors) == 1

        for color_code, _, color_type in colors:
            assert color_code == make_sgr(code)
            assert color_type == 'fg'


def test_color_decode_sgr_styles_blink():
    """Blink and its reset are being detected."""
    for code in [b'5', b'25']:
        colors = chromaterm.Color.decode_sgr(make_sgr(code))
        assert len(colors) == 1

        for color_code, _, color_type in colors:
            assert color_code == make_sgr(code)
            assert color_type == 'blink'


def test_color_decode_sgr_styles_bold():
    """Bold and its reset are being detected."""
    for code in [b'1', b'2', b'22']:
        colors = chromaterm.Color.decode_sgr(make_sgr(code))
        assert len(colors) == 1

        for color_code, _, color_type in colors:
            assert color_code == make_sgr(code)
            assert color_type == 'bold'


def test_color_decode_sgr_styles_italic():
    """Italic and its reset are being detected."""
    for code in [b'3', b'23']:
        colors = chromaterm.Color.decode_sgr(make_sgr(code))
        assert len(colors) == 1

        for color_code, _, color_type in colors:
            assert color_code == make_sgr(code)
            assert color_type == 'italic'


def test_color_decode_sgr_styles_strike():
    """Strike and its reset are being detected."""
    for code in [b'9', b'29']:
        colors = chromaterm.Color.decode_sgr(make_sgr(code))
        assert len(colors) == 1

        for color_code, _, color_type in colors:
            assert color_code == make_sgr(code)
            assert color_type == 'strike'


def test_color_decode_sgr_styles_underline():
    """Underline and its reset are being detected."""
    for code in [b'4', b'24']:
        colors = chromaterm.Color.decode_sgr(make_sgr(code))
        assert len(colors) == 1

        for color_code, _, color_type in colors:
            assert color_code == make_sgr(code)
            assert color_type == 'underline'


def test_color_decode_sgr_full_reset():
    """Full reset detection."""
    for code in [b'00', b'0', b'']:
        colors = chromaterm.Color.decode_sgr(make_sgr(code))
        assert len(colors) == 1

        for color_code, color_reset, color_type in colors:
            assert color_code == make_sgr(b'0')
            assert color_reset is True
            assert color_type is None


def test_color_decode_sgr_malformed():
    """Malformed colors."""
    for code in [b'38;5', b'38;2;1;1', b'38;5;123;38;2;1;1']:
        colors = chromaterm.Color.decode_sgr(make_sgr(code))
        assert len(colors) == 1

        for color_code, color_reset, color_type in colors:
            assert color_code == make_sgr(code)
            assert color_reset is False
            assert color_type is None


def test_color_decode_sgr_split_compound():
    """Split the a compound SGR into discrete SGR's."""
    codes = [b'1', b'33', b'40']
    types = ['bold', 'fg', 'bg']

    colors = chromaterm.Color.decode_sgr(make_sgr(b'1;33;40'))
    assert len(colors) == 3

    for index, (color_code, is_reset, color_type) in enumerate(colors):
        assert color_code == make_sgr(codes[index])
        assert is_reset is False
        assert color_type == types[index]


def test_color_decode_sgr_unrecognized():
    """An SGR that's valid, but the type isn't recognized during decoding."""
    colors = chromaterm.Color.decode_sgr(make_sgr(b'7'))
    assert len(colors) == 1

    for color_code, color_reset, color_type in colors:
        assert color_code == make_sgr(b'7')
        assert color_reset is False
        assert color_type is None


def test_color_change_color():
    """Confirm that the color_code is updated when the color or rgb is changed."""
    color = chromaterm.Color('b#123123', rgb=False)

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
    color = chromaterm.Color('b#123123', rgb=False)

    assert color.color == 'b#123123'
    assert color.color_code == make_sgr(b'48;5;22')
    assert color.color_reset == make_sgr(b'49')
    assert color.color_types == [('bg', make_sgr(b'48;5;22'))]
    assert color.rgb is False


def test_color_format_color_foreground():
    """Foreground color."""
    color = chromaterm.Color('f#123123', rgb=False)

    assert color.color == 'f#123123'
    assert color.color_code == make_sgr(b'38;5;22')
    assert color.color_reset == make_sgr(b'39')
    assert color.color_types == [('fg', make_sgr(b'38;5;22'))]
    assert color.rgb is False


def test_color_format_color_multiple():
    """Multiple different color types. Also tests case insensitivity"""
    color = chromaterm.Color('f#123123 b#aBCDef bOLd', rgb=False)

    assert color.color == 'f#123123 b#abcdef bold'
    assert color.color_code == make_sgr(b'38;5;22', b'48;5;189', b'1')
    assert color.color_reset == make_sgr(b'22', b'49', b'39')
    assert color.color_types == [
        ('fg', make_sgr(b'38;5;22')),
        ('bg', make_sgr(b'48;5;189')),
        ('bold', make_sgr(b'1')),
    ]
    assert color.rgb is False


def test_color_format_color_rgb():
    """RGB color."""
    color = chromaterm.Color('b#123123', rgb=True)

    assert color.color == 'b#123123'
    assert color.color_code == make_sgr(b'48;2;18;49;35')
    assert color.color_reset == make_sgr(b'49')
    assert color.color_types == [('bg', make_sgr(b'48;2;18;49;35'))]
    assert color.rgb is True


def test_color_format_color_style():
    """Style color."""
    styles = {k: v for k, v in chromaterm.COLOR_TYPES.items() if v.get('code')}

    for style in styles:
        color = chromaterm.Color(style, rgb=False)

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


def test_rule_get_matches():
    """Get matches of rule that only colors the default regex group."""
    color = chromaterm.Color('bold')
    rule = chromaterm.Rule('hello|world', color=color)

    data = b'hello world'
    expected = [(0, 5, color), (6, 11, color)]

    assert rule.get_matches(data) == expected


def test_rule_get_matches_groups():
    """Get matches of rule that colors default and specific regex groups."""
    color1 = chromaterm.Color('bold')
    color2 = chromaterm.Color('b#123123')
    color3 = chromaterm.Color('f#321321')
    rule = chromaterm.Rule('(hello) (world)', color=color1)
    rule.set_color(color2, group=1)
    rule.set_color(color3, group=2)

    data = b'hello world'
    expected = [(0, 11, color1), (0, 5, color2), (6, 11, color3)]

    assert rule.get_matches(data) == expected


def test_rule_get_matches_groups_optional_not_matches():
    """Get matches of rule that has a specific regex group that is optional. When
    the optional group is not in the match, the color should not be inserted."""
    color = chromaterm.Color('bold')
    rule = chromaterm.Rule('hello(,)? world')
    rule.set_color(color, group=1)

    data = b'hello, world'
    expected = [(5, 6, color)]

    assert rule.get_matches(data) == expected

    data = b'hello world'
    expected = []

    assert rule.get_matches(data) == expected


def test_rule_get_matches_no_colors():
    """Get matches of rule that has no colors – nothing is changed."""
    assert chromaterm.Rule('hello').get_matches(b'hello') == []


def test_rule_get_matches_zero_length_match():
    """Get matches of rule that has a regex that matches zero-length data."""
    color = chromaterm.Color('bold')
    rule = chromaterm.Rule('hello() world')
    rule.set_color(color, group=1)

    assert rule.get_matches(b'hello world') == []


def test_rule_set_color_clear_existing():
    """Clear the color of a rule by setting it to None."""
    color = chromaterm.Color('bold')
    rule = chromaterm.Rule('(hello|world)')

    rule.set_color(color, group=0)
    rule.set_color(color, group=1)
    assert list(rule.colors) == [0, 1]

    rule.set_color(None, group=0)
    assert list(rule.colors) == [1]


def test_rule_set_color_default_group():
    """Add to a rule a color to the default regex group (0)."""
    rule = chromaterm.Rule('hello')
    assert rule.color is None

    color = chromaterm.Color('bold')
    rule.set_color(color)
    assert rule.color is color
    assert rule.colors.get(0) is color


def test_rule_set_color_specific_group():
    """Add to a rule a color to a specific regex group. Additionally, ensure
    that the dict keys are ordered according to the group number."""
    rule = chromaterm.Rule('he(llo)')
    assert rule.color is None

    color = chromaterm.Color('bold')
    rule.set_color(color, group=1)
    assert rule.color is None
    assert rule.colors.get(1) is color

    rule.set_color(color)
    assert list(rule.colors) == [0, 1]


def test_rule_set_color_invalid_type_color():
    """Add to a rule a color with invalid group type."""
    rule = chromaterm.Rule('hello')

    with pytest.raises(TypeError, match='color must be a chromaterm.Color'):
        rule.set_color(True)


def test_rule_set_color_invalid_type_group():
    """Add to a rule a color with invalid color type."""
    rule = chromaterm.Rule('hello')

    with pytest.raises(TypeError, match='group must be an integer'):
        rule.set_color(chromaterm.Color('bold'), group='3')


def test_rule_set_color_invalid_value_group():
    """Add to a rule a color with invalid group value."""
    rule = chromaterm.Rule('hello')

    with pytest.raises(ValueError, match='regex only has 0 group'):
        rule.set_color(chromaterm.Color('bold'), group=2)


def test_rule_change_color():
    """Confirm that a color change overrides the old one."""
    rule = chromaterm.Rule('hello', color=chromaterm.Color('bold'))

    old_color = rule.color
    rule.color = chromaterm.Color('b#123123')
    assert old_color is not rule.color


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
    with pytest.raises(TypeError, match='regex must be a string'):
        chromaterm.Rule(True)


def test_config_highlight():
    """Highlight with one rule."""
    config = chromaterm.Config()

    rule = chromaterm.Rule('hello', color=chromaterm.Color('b#123123'))
    config.rules.append(rule)

    data = b'hello world'
    expected = [
        rule.color.color_code, b'hello', rule.color.color_reset, b' world'
    ]

    assert config.highlight(data) == b''.join(expected)


def test_config_highlight_adjoin_type_different():
    """Two rules with different color types, and one ending where the other
    starts. Both are applied without any overlap in the codes, independent of the
    order.
    1: -------
    2:        -------"""
    config = chromaterm.Config()

    rule1 = chromaterm.Rule('he', color=chromaterm.Color('b#123123'))
    rule2 = chromaterm.Rule('llo', color=chromaterm.Color('f#321321'))
    config.rules.append(rule1)
    config.rules.append(rule2)

    data = b'hello'
    expected = [
        rule1.color.color_code, b'he', rule1.color.color_reset,
        rule2.color.color_code, b'llo', rule2.color.color_reset
    ]

    assert config.highlight(data) == b''.join(expected)

    config.rules.remove(rule1)
    config.rules.append(rule1)

    assert config.highlight(data) == b''.join(expected)


def test_config_highlight_adjoin_type_mixed():
    """Two rules with mixed color types, with one ending where the other starts.
    Both are applied without any overlap in the codes, independent of the
    order.
    1: -------
    2:        -------"""
    config = chromaterm.Config()

    rule1 = chromaterm.Rule('he', color=chromaterm.Color('b#123123 italic'))
    rule2 = chromaterm.Rule('llo', color=chromaterm.Color('b#321321 bold'))
    config.rules.append(rule1)
    config.rules.append(rule2)

    data = b'hello'
    expected = [
        rule1.color.color_code, b'he', rule1.color.color_reset,
        rule2.color.color_code, b'llo', rule2.color.color_reset
    ]

    assert config.highlight(data) == b''.join(expected)

    config.rules.remove(rule1)
    config.rules.append(rule1)

    assert config.highlight(data) == b''.join(expected)


def test_config_highlight_adjoin_type_same():
    """Two rules with same color type, with one ending where the other starts.
    Both are applied without any overlap in the codes, independent of the order.
    1: -------
    2:        -------"""
    config = chromaterm.Config()

    rule1 = chromaterm.Rule('he', color=chromaterm.Color('b#123123'))
    rule2 = chromaterm.Rule('llo', color=chromaterm.Color('b#321321'))
    config.rules.append(rule1)
    config.rules.append(rule2)

    data = b'hello'
    expected = [
        rule1.color.color_code, b'he', rule1.color.color_reset,
        rule2.color.color_code, b'llo', rule2.color.color_reset
    ]

    assert config.highlight(data) == b''.join(expected)

    config.rules.remove(rule1)
    config.rules.append(rule1)

    assert config.highlight(data) == b''.join(expected)


def test_config_highlight_common_beginning_type_different():
    """Two rules with different color types, and both sharing the same start of
    a match. The most recent rule will be closer to the match's start.
    1: --------------
    2: -------"""
    config = chromaterm.Config()

    rule1 = chromaterm.Rule('hello', color=chromaterm.Color('b#123123'))
    rule2 = chromaterm.Rule('he', color=chromaterm.Color('f#321321'))
    config.rules.append(rule1)
    config.rules.append(rule2)

    data = b'hello'
    expected = [
        rule1.color.color_code, rule2.color.color_code, b'he',
        rule2.color.color_reset, b'llo', rule1.color.color_reset
    ]

    assert config.highlight(data) == b''.join(expected)

    config.rules.remove(rule1)
    config.rules.append(rule1)

    # Flip start color
    expected[0], expected[1] = expected[1], expected[0]

    assert config.highlight(data) == b''.join(expected)


def test_config_highlight_common_beginning_type_mixed():
    """Two rules with different color types, and both sharing the same start of
    a match. The most recent rule will be closer to the match's start.
    1: --------------
    2: -------"""
    config = chromaterm.Config()

    rule1 = chromaterm.Rule('hello', color=chromaterm.Color('b#123123 italic'))
    rule2 = chromaterm.Rule('he', color=chromaterm.Color('b#321321 bold'))
    config.rules.append(rule1)
    config.rules.append(rule2)

    data = b'hello'
    expected = [
        rule1.color.color_code, rule2.color.color_code, b'he', RESET_BOLD,
        rule1.color.color_types[0][1], b'llo', rule1.color.color_reset
    ]

    assert config.highlight(data) == b''.join(expected)


def test_config_highlight_common_beginning_type_same():
    """Two rules with same color type, and both sharing the same start of a
    match. The most recent rule will be closer to the match's start.
    1: --------------
    2: -------"""
    config = chromaterm.Config()

    rule1 = chromaterm.Rule('hello', color=chromaterm.Color('b#123123'))
    rule2 = chromaterm.Rule('he', color=chromaterm.Color('b#321321'))
    config.rules.append(rule1)
    config.rules.append(rule2)

    data = b'hello'
    expected = [
        rule1.color.color_code, rule2.color.color_code, b'he',
        rule1.color.color_code, b'llo', rule1.color.color_reset
    ]

    assert config.highlight(data) == b''.join(expected)


def test_config_highlight_common_end_type_different():
    """Two rules with different color types, and both sharing the same end of a
    match. The most recent rule will be closer to the match's end.
    1: --------------
    2:        -------"""
    config = chromaterm.Config()

    rule1 = chromaterm.Rule('hello', color=chromaterm.Color('b#123123'))
    rule2 = chromaterm.Rule('llo', color=chromaterm.Color('f#321321'))
    config.rules.append(rule1)
    config.rules.append(rule2)

    data = b'hello'
    expected = [
        rule1.color.color_code, b'he', rule2.color.color_code, b'llo',
        rule2.color.color_reset, rule1.color.color_reset
    ]

    assert config.highlight(data) == b''.join(expected)

    config.rules.remove(rule1)
    config.rules.append(rule1)

    # Flip end color
    expected[-1], expected[-2] = expected[-2], expected[-1]

    assert config.highlight(data) == b''.join(expected)


def test_config_highlight_common_end_type_mixed():
    """Two rules with different color types, and both sharing the same end of a
    match. The most recent rule will be closer to the match's end.
    1: --------------
    2:        -------"""
    config = chromaterm.Config()

    rule1 = chromaterm.Rule('hello', color=chromaterm.Color('b#123123 italic'))
    rule2 = chromaterm.Rule('llo', color=chromaterm.Color('b#321321 bold'))
    config.rules.append(rule1)
    config.rules.append(rule2)

    data = b'hello'
    expected = [
        rule1.color.color_code, b'he', rule2.color.color_code, b'llo',
        RESET_BOLD, rule1.color.color_types[0][1], rule1.color.color_reset
    ]

    assert config.highlight(data) == b''.join(expected)


def test_config_highlight_common_end_type_same():
    """Two rules with same color type, and both sharing the same end of a
    match. The most recent rule will be closer to the match's end.
    1: --------------
    2:        -------"""
    config = chromaterm.Config()

    rule1 = chromaterm.Rule('hello', color=chromaterm.Color('b#123123'))
    rule2 = chromaterm.Rule('llo', color=chromaterm.Color('b#321321'))
    config.rules.append(rule1)
    config.rules.append(rule2)

    data = b'hello'
    expected = [
        rule1.color.color_code, b'he', rule2.color.color_code, b'llo',
        rule1.color.color_code, rule1.color.color_reset
    ]

    assert config.highlight(data) == b''.join(expected)


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
    config.rules.append(rule1)
    config.rules.append(rule2)
    config.rules.append(rule3)

    data = b'hello world'
    expected = [
        rule3.color.color_code, b'he', rule2.color.color_code, b'l',
        rule1.color.color_code, b'lo wo', rule1.color.color_reset, b'r',
        rule2.color.color_reset, b'ld', rule3.color.color_reset
    ]

    assert config.highlight(data) == b''.join(expected)

    config.rules.remove(rule1)
    config.rules.remove(rule2)
    config.rules.append(rule2)
    config.rules.append(rule1)

    assert config.highlight(data) == b''.join(expected)


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
    config.rules.append(rule1)
    config.rules.append(rule2)
    config.rules.append(rule3)

    data = b'hello world'
    expected = [
        rule3.color.color_code, b'he', rule2.color.color_code, b'l',
        rule1.color.color_code, b'lo wo', RESET_ITALIC,
        rule2.color.color_types[0][1], b'r', rule2.color.color_types[1][1],
        chromaterm.COLOR_TYPES[rule2.color.color_types[0][0]]['reset'], b'ld',
        rule3.color.color_reset
    ]

    assert config.highlight(data) == b''.join(expected)

    config.rules.remove(rule1)
    config.rules.remove(rule2)
    config.rules.append(rule2)
    config.rules.append(rule1)

    assert config.highlight(data) == b''.join(expected)


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
    config.rules.append(rule1)
    config.rules.append(rule2)
    config.rules.append(rule3)

    data = b'hello world'
    expected = [
        rule3.color.color_code, b'he', rule2.color.color_code, b'l',
        rule1.color.color_code, b'lo wo', rule2.color.color_code, b'r',
        rule3.color.color_code, b'ld', rule3.color.color_reset
    ]

    assert config.highlight(data) == b''.join(expected)

    config.rules.remove(rule1)
    config.rules.remove(rule2)
    config.rules.append(rule2)
    config.rules.append(rule1)

    assert config.highlight(data) == b''.join(expected)


def test_config_highlight_overlap_full_type_different():
    """Two rules, fully overlapping matches, different color types. Should not
    affect each other as they have different types. Most recent rule should be
    closest to the match.
    1: ----------
    2: ----------"""
    config = chromaterm.Config()

    rule1 = chromaterm.Rule('hello', color=chromaterm.Color('b#123123'))
    rule2 = chromaterm.Rule('hello', color=chromaterm.Color('f#321321'))
    config.rules.append(rule1)
    config.rules.append(rule2)

    data = b'hello'
    expected = [
        rule1.color.color_code, rule2.color.color_code, b'hello',
        rule2.color.color_reset, rule1.color.color_reset
    ]

    assert config.highlight(data) == b''.join(expected)


def test_config_highlight_overlap_full_type_mixed():
    """Two rules, fully overlapping matches, mixed color types. The different
    color types should not affect each other, but those that are the same should
    update the oldest reset with the most recent rule's color code.
    1: ----------
    2: ----------"""
    config = chromaterm.Config()

    rule1 = chromaterm.Rule('hello', color=chromaterm.Color('b#123123 bold'))
    rule2 = chromaterm.Rule('hello', color=chromaterm.Color('b#321321 italic'))
    config.rules.append(rule1)
    config.rules.append(rule2)

    data = b'hello'

    expected = [
        rule1.color.color_code, rule2.color.color_code, b'hello', RESET_ITALIC,
        rule1.color.color_types[0][1], rule1.color.color_reset
    ]

    assert config.highlight(data) == b''.join(expected)


def test_config_highlight_overlap_full_type_same():
    """Two rules, fully overlapping matches, same color type. The reset of the
    first rule should be replaced with the color code of the second (most recent)
    rule to prevent the reset from interrupting the color.
    1: ----------
    2: ----------"""
    config = chromaterm.Config()

    rule1 = chromaterm.Rule('hello', color=chromaterm.Color('b#123123'))
    rule2 = chromaterm.Rule('hello', color=chromaterm.Color('b#321321'))
    config.rules.append(rule1)
    config.rules.append(rule2)

    data = b'hello'
    expected = [
        rule1.color.color_code, rule2.color.color_code, b'hello',
        rule1.color.color_code, rule1.color.color_reset
    ]

    assert config.highlight(data) == b''.join(expected)


def test_config_highlight_overlap_partial_type_different():
    """Two rules, partially overlapping matches, different color types. The rules
    should not affect each other. Tested in reverse order, too.
    1: ----------
    2:     ----------"""
    config = chromaterm.Config()

    rule1 = chromaterm.Rule('hell', color=chromaterm.Color('b#123123'))
    rule2 = chromaterm.Rule('llo', color=chromaterm.Color('f#321321'))
    config.rules.append(rule1)
    config.rules.append(rule2)

    data = b'hello'
    expected = [
        rule1.color.color_code, b'he', rule2.color.color_code, b'll',
        rule1.color.color_reset, b'o', rule2.color.color_reset
    ]

    assert config.highlight(data) == b''.join(expected)

    config.rules.remove(rule1)
    config.rules.append(rule1)

    assert config.highlight(data) == b''.join(expected)


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
    config.rules.append(rule1)
    config.rules.append(rule2)

    data = b'hello'
    expected = [
        rule1.color.color_code, b'he', rule2.color.color_code, b'll',
        RESET_BOLD, rule2.color.color_types[0][1], b'o',
        rule2.color.color_reset
    ]

    assert config.highlight(data) == b''.join(expected)

    config.rules.remove(rule1)
    config.rules.append(rule1)

    assert config.highlight(data) == b''.join(expected)


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
    config.rules.append(rule1)
    config.rules.append(rule2)

    data = b'hello'
    expected = [
        rule1.color.color_code, b'he', rule2.color.color_code, b'll',
        rule2.color.color_code, b'o', rule2.color.color_reset
    ]

    assert config.highlight(data) == b''.join(expected)

    config.rules.remove(rule1)
    config.rules.append(rule1)

    assert config.highlight(data) == b''.join(expected)


def test_config_highlight_no_rules():
    """Highlight with config that has no rules – nothing is changed."""
    config = chromaterm.Config()
    assert config.highlight(b'hello world') == b'hello world'


def test_config_highlight_tracking_common_beginning_type_different():
    """A rule with a match that has a color of a different type just before its
    start. The rule's color is closer to the match and the reset is unaffected by
    the existing color.
    1: x-------------"""
    config = chromaterm.Config()

    rule = chromaterm.Rule('hello', color=chromaterm.Color('b#123123'))
    config.rules.append(rule)

    data = b'\x1b[33mhello'
    expected = [
        b'\x1b[33m', rule.color.color_code, b'hello', rule.color.color_reset
    ]

    assert config.highlight(data) == b''.join(expected)


def test_config_highlight_tracking_common_beginning_type_same():
    """A rule with a match that has a color of the same type just before its
    start. The rule's color is closer to the match and the reset used is the
    existing color.
    1: x-------------"""
    config = chromaterm.Config()

    rule = chromaterm.Rule('hello', color=chromaterm.Color('f#321321'))
    config.rules.append(rule)

    data = b'\x1b[33mhello'
    expected = [b'\x1b[33m', rule.color.color_code, b'hello', b'\x1b[33m']

    assert config.highlight(data) == b''.join(expected)


def test_config_highlight_tracking_common_end_type_different():
    """A rule with a match that has a color of a different type just after its
    end. The rule's reset is closer to the match and the reset is unaffected by
    the existing color.
    1: -------------x"""
    config = chromaterm.Config()

    rule = chromaterm.Rule('hello', color=chromaterm.Color('b#123123'))
    config.rules.append(rule)

    data = b'hello\x1b[33m'
    expected = [
        rule.color.color_code, b'hello', rule.color.color_reset, b'\x1b[33m'
    ]

    assert config.highlight(data) == b''.join(expected)


def test_config_highlight_tracking_common_end_type_same():
    """A rule with a match that has a color of the same type just after its end.
    The rule's reset is closer to the match and is unaffected by the existing
    color.
    1: -------------x"""
    config = chromaterm.Config()

    rule = chromaterm.Rule('hello', color=chromaterm.Color('f#321321'))
    config.rules.append(rule)

    data = b'hello\x1b[33m'
    expected = [
        rule.color.color_code, b'hello', rule.color.color_reset, b'\x1b[33m'
    ]

    assert config.highlight(data) == b''.join(expected)


def test_config_highlight_tracking_full_reset_beginning():
    """A rule with a match that has a full reset just before the start of the
    match. The rule's color is closer to the match and the reset used is the
    default for that color type.
    1: R-------------"""
    config = chromaterm.Config()

    rule = chromaterm.Rule('hello', color=chromaterm.Color('f#321321'))
    config.rules.append(rule)

    data = b'\x1b[0mhello'
    expected = [
        b'\x1b[0m', rule.color.color_code, b'hello',
        chromaterm.COLOR_TYPES[rule.color.color_types[0][0]]['reset']
    ]

    assert config.highlight(data) == b''.join(expected)


def test_config_highlight_tracking_full_reset_end():
    """A rule with a match that has a full reset just after the end of the match.
    The rule's reset is closer to the match and the reset used is the default for
    that color type.
    1: -------------R"""
    config = chromaterm.Config()

    rule = chromaterm.Rule('hello', color=chromaterm.Color('f#321321'))
    config.rules.append(rule)

    data = b'hello\x1b[0m'
    expected = [
        rule.color.color_code, b'hello',
        chromaterm.COLOR_TYPES[rule.color.color_types[0][0]]['reset'],
        b'\x1b[0m'
    ]

    assert config.highlight(data) == b''.join(expected)


def test_config_highlight_tracking_full_reset_middle():
    """A rule with a match that has a full reset in the middle of it. The full
    reset is changed to the color code of the match and the reset of the match
    is changed to a full reset.
    1: ------R-------"""
    config = chromaterm.Config()

    rule = chromaterm.Rule('hello', color=chromaterm.Color('f#321321'))
    config.rules.append(rule)

    data = b'hel\x1b[0mlo'
    expected = [
        rule.color.color_code, b'hel', rule.color.color_code, b'lo', b'\x1b[0m'
    ]

    assert config.highlight(data) == b''.join(expected)


def test_config_highlight_tracking_malformed():
    """A rule with a match that has a malformed SGR in the middle. It should be
    ignored and inserted back into the match. Highlighting from the rule should
    still go through.
    1: x-------------"""
    config = chromaterm.Config()

    rule = chromaterm.Rule('hello', color=chromaterm.Color('b#123123'))
    config.rules.append(rule)

    data = b'he\x1b[38;5mllo'
    expected = [
        rule.color.color_code, b'he', b'\x1b[38;5m', b'llo',
        rule.color.color_reset
    ]

    assert config.highlight(data) == b''.join(expected)


def test_config_highlight_tracking_mixed_full_reset():
    """Track multiple color types and ensure a full reset only defaults the types
    that were not updated by other colors in the data."""
    config = chromaterm.Config()

    rule1 = chromaterm.Rule('hello', color=chromaterm.Color('f#321321'))
    rule2 = chromaterm.Rule('world', color=chromaterm.Color('b#123123'))
    config.rules.append(rule1)
    config.rules.append(rule2)

    data = b'\x1b[33mhello\x1b[0m there \x1b[43mworld'
    expected = [
        b'\x1b[33m', rule1.color.color_code, b'hello', b'\x1b[33m', b'\x1b[0m',
        b' there ', b'\x1b[43m', rule2.color.color_code, b'world', b'\x1b[43m'
    ]

    assert config.highlight(data) == b''.join(expected)

    # The color of rule1 was reset to its default because a full reset came after
    # it, but the color of rule2 was already updated so it wasn't affected by the
    # full reset
    data = b'hello there world'
    expected = [
        rule1.color.color_code, b'hello', rule1.color.color_reset, b' there ',
        rule2.color.color_code, b'world', b'\x1b[43m'
    ]

    assert config.highlight(data) == b''.join(expected)


def test_config_highlight_tracking_multiline_type_different():
    """Ensure that data with an existing color is tracked across highlights and
    does not affect the reset of a color of a different type."""
    config = chromaterm.Config()

    rule = chromaterm.Rule('hello', color=chromaterm.Color('b#123123'))
    config.rules.append(rule)

    # Inject a foreground color to have it tracked
    assert config.highlight(b'\x1b[33m') == b'\x1b[33m'

    data = b'hello'
    expected = [rule.color.color_code, b'hello', rule.color.color_reset]

    assert config.highlight(data) == b''.join(expected)


def test_config_highlight_tracking_multiline_type_same():
    """Ensure that data with an existing color is tracked across highlights and
    affects the reset of a color of the same type."""
    config = chromaterm.Config()

    rule = chromaterm.Rule('hello', color=chromaterm.Color('f#321321'))
    config.rules.append(rule)

    # Inject a foreground color to have it tracked
    assert config.highlight(b'\x1b[33m') == b'\x1b[33m'

    data = b'hello'
    expected = [rule.color.color_code, b'hello', b'\x1b[33m']

    assert config.highlight(data) == b''.join(expected)
