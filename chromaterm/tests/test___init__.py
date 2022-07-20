'''chromaterm tests'''
import pytest

import chromaterm

# pylint: disable=too-many-lines

# Aliases to two reset codes used during tests
RESET_BOLD = chromaterm.COLOR_TYPES['bold']['reset']
RESET_ITALIC = chromaterm.COLOR_TYPES['italic']['reset']


def make_sgr(*code_id):
    '''Returns byte-array for an SGR of `code_id` (multiple are concatenated).'''
    return b''.join(b'\x1b[' + x + b'm' for x in code_id)


def test_color_known_colors():
    '''Random known-good color codes.'''
    known_colors = [('644db0', 61), ('18ce9f', 43), ('7318d8', 56),
                    ('5c96f6', 69), ('1ca4c3', 37), ('d9dcc1', 187),
                    ('8bfe8c', 120), ('33f743', 83), ('36d840', 77),
                    ('5baf98', 72), ('e5a667', 179), ('255bfe', 27),
                    ('53e7b4', 79), ('7126d5', 56), ('60c51c', 76),
                    ('bb9465', 137), ('15f502', 46), ('673f0b', 58),
                    ('818fbb', 103), ('38fad2', 86), ('7ba6f1', 111),
                    ('b6bffb', 147), ('cdf107', 190), ('4340ad', 61),
                    ('c9b248', 179), ('903a02', 94), ('084bef', 27),
                    ('a7dca2', 151), ('bf9a28', 136), ('20ca8e', 42),
                    ('670f41', 53), ('94b8e9', 110), ('2c2d27', 235),
                    ('d56561', 167), ('042e6b', 17), ('e7122a', 160),
                    ('3420d6', 56), ('eae8fb', 255), ('04cc5e', 41),
                    ('6a0577', 54), ('6c6e1f', 58), ('aa1ff2', 129),
                    ('8d33db', 98), ('85f7c2', 121), ('b55896', 132),
                    ('254118', 235), ('9c6c95', 132), ('a877ab', 139),
                    ('7e4739', 95), ('4e8867', 65), ('7fbe9a', 108),
                    ('e6da8e', 186), ('c355d7', 134), ('582017', 52),
                    ('93051d', 88), ('3ac6a4', 79), ('fae310', 220),
                    ('08f2d7', 50), ('09fbf1', 51), ('1ab465', 35),
                    ('cf4341', 167), ('042ba4', 19), ('5310e0', 56),
                    ('6d732e', 58), ('6d7cec', 69), ('c428d9', 164),
                    ('4fd7f4', 81), ('09de3e', 41), ('410cfd', 57),
                    ('e41749', 161), ('dc4578', 168), ('ebf705', 190),
                    ('ce22b2', 163), ('ccf0ba', 193), ('a2a315', 142),
                    ('db3a0e', 166), ('8ff3dd', 122), ('866a61', 95),
                    ('25b6b4', 37), ('adbbb1', 249), ('d8272a', 160),
                    ('2c72d8', 26), ('e401df', 164), ('10081b', 233),
                    ('b3bf40', 143), ('0b572e', 22), ('d3a6de', 182),
                    ('71659e', 61), ('260ff7', 21), ('81bb90', 108),
                    ('8d7b5e', 101), ('ebf324', 190), ('1d6925', 22),
                    ('e966c9', 170), ('f0e172', 221), ('7b2f0f', 88),
                    ('fea6d8', 218), ('065694', 24), ('1e2e82', 18),
                    ('ab4e33', 131)]

    for hex_code, xterm256_code in known_colors:
        color = chromaterm.Color(f'f#{hex_code}', rgb=False)
        assert color.color_code == make_sgr(b'38;5;%d' % xterm256_code)


def test_color_decode_sgr_bg():
    '''Background colors and reset are being detected.'''
    for code in [b'48;5;12', b'48;2;1;1;1', b'49', b'101']:
        colors = chromaterm.Color.decode_sgr(make_sgr(code))
        assert len(colors) == 1

        for color_code, _, color_type in colors:
            assert color_code == make_sgr(code)
            assert color_type == 'bg'


def test_color_decode_sgr_fg():
    '''Foreground colors and reset are being detected.'''
    for code in [b'38;5;12', b'38;2;1;1;1', b'39', b'91']:
        colors = chromaterm.Color.decode_sgr(make_sgr(code))
        assert len(colors) == 1

        for color_code, _, color_type in colors:
            assert color_code == make_sgr(code)
            assert color_type == 'fg'


def test_color_decode_sgr_styles_blink():
    '''Blink and its reset are being detected.'''
    for code in [b'5', b'25']:
        colors = chromaterm.Color.decode_sgr(make_sgr(code))
        assert len(colors) == 1

        for color_code, _, color_type in colors:
            assert color_code == make_sgr(code)
            assert color_type == 'blink'


def test_color_decode_sgr_styles_bold():
    '''Bold and its reset are being detected.'''
    for code in [b'1', b'2', b'22']:
        colors = chromaterm.Color.decode_sgr(make_sgr(code))
        assert len(colors) == 1

        for color_code, _, color_type in colors:
            assert color_code == make_sgr(code)
            assert color_type == 'bold'


def test_color_decode_sgr_styles_invert():
    '''Invert and its reset are being detected.'''
    for code in [b'7', b'27']:
        colors = chromaterm.Color.decode_sgr(make_sgr(code))
        assert len(colors) == 1

        for color_code, _, color_type in colors:
            assert color_code == make_sgr(code)
            assert color_type == 'invert'


def test_color_decode_sgr_styles_italic():
    '''Italic and its reset are being detected.'''
    for code in [b'3', b'23']:
        colors = chromaterm.Color.decode_sgr(make_sgr(code))
        assert len(colors) == 1

        for color_code, _, color_type in colors:
            assert color_code == make_sgr(code)
            assert color_type == 'italic'


def test_color_decode_sgr_styles_strike():
    '''Strike and its reset are being detected.'''
    for code in [b'9', b'29']:
        colors = chromaterm.Color.decode_sgr(make_sgr(code))
        assert len(colors) == 1

        for color_code, _, color_type in colors:
            assert color_code == make_sgr(code)
            assert color_type == 'strike'


def test_color_decode_sgr_styles_underline():
    '''Underline and its reset are being detected.'''
    for code in [b'4', b'24']:
        colors = chromaterm.Color.decode_sgr(make_sgr(code))
        assert len(colors) == 1

        for color_code, _, color_type in colors:
            assert color_code == make_sgr(code)
            assert color_type == 'underline'


def test_color_decode_sgr_full_reset():
    '''Full reset detection.'''
    for code in [b'00', b'0', b'']:
        colors = chromaterm.Color.decode_sgr(make_sgr(code))
        assert len(colors) == 1

        for color_code, color_reset, color_type in colors:
            assert color_code == make_sgr(b'0')
            assert color_reset is True
            assert color_type is None


def test_color_decode_sgr_malformed():
    '''Malformed colors.'''
    for code in [b'38;5', b'38;2;1;1', b'38;5;123;38;2;1;1', b'>1;']:
        colors = chromaterm.Color.decode_sgr(make_sgr(code))
        assert len(colors) == 1

        for color_code, color_reset, color_type in colors:
            assert color_code == make_sgr(code)
            assert color_reset is False
            assert color_type is None


def test_color_decode_sgr_split_compound():
    '''Split the a compound SGR into discrete SGR's.'''
    codes = [b'1', b'33', b'40']
    types = ['bold', 'fg', 'bg']

    colors = chromaterm.Color.decode_sgr(make_sgr(b'1;33;40'))
    assert len(colors) == 3

    for index, (color_code, is_reset, color_type) in enumerate(colors):
        assert color_code == make_sgr(codes[index])
        assert is_reset is False
        assert color_type == types[index]


def test_color_decode_sgr_unrecognized():
    '''An SGR that's valid, but the type isn't recognized during decoding.'''
    colors = chromaterm.Color.decode_sgr(make_sgr(b'60'))
    assert len(colors) == 1

    for color_code, color_reset, color_type in colors:
        assert color_code == make_sgr(b'60')
        assert color_reset is False
        assert color_type is None


def test_color_decode_sgr_is_reset():
    '''Colors and styles with `is_reset=True` should always be marked as resets.'''
    for style, code in zip(['fg', 'bg', 'bold'],
                           [b'38;5;12', b'48;5;12', b'1']):
        colors = chromaterm.Color.decode_sgr(make_sgr(code), is_reset=True)
        assert len(colors) == 1

        for color_code, color_reset, color_type in colors:
            assert color_code == make_sgr(code)
            assert color_reset
            assert color_type == style


def test_color_change_color():
    '''Confirm that the color_code is updated when the color or rgb is changed.'''
    color = chromaterm.Color('b#123123', rgb=False)

    # Change from background to foreground
    old_color_code = color.color_code
    color.color = 'f#123123'
    assert color.color_code != old_color_code

    # Toggle rgb
    old_color_code = color.color_code
    color.rgb = not color.rgb
    assert color.color_code != old_color_code


def test_color_duplicate_style():
    '''Duplicate styles are ignored as opposed to added multiple times.'''
    for name, color_type in chromaterm.COLOR_TYPES.items():
        # Only styles have a 'code' key
        if color_type.get('code'):
            color = chromaterm.Color(f'{name} {name}')

            assert color.color == name
            assert color.color_code == color_type['code']
            assert color.color_reset == color_type['reset']


def test_color_palette():
    '''Use a palette color.'''
    palette = chromaterm.Palette()
    palette.add_color('red', '#ff0000')

    color = chromaterm.Color('b.red bold', palette=palette, rgb=False)

    assert color.color == 'b.red bold'
    assert color.color_code == make_sgr(b'48;5;196', b'1')
    assert color.color_reset == make_sgr(b'22', b'49')
    assert color.color_types == [('bg', make_sgr(b'48;5;196')),
                                 ('bold', make_sgr(b'1'))]


def test_color_invalid_value_palette_missing():
    '''Use a palette color, but don't specify a pleatte.'''
    with pytest.raises(ValueError, match='no palette specified'):
        chromaterm.Color('b.red bold')


def test_color_format_color_background():
    '''Background color.'''
    color = chromaterm.Color('b#123123', rgb=False)

    assert color.color == 'b#123123'
    assert color.color_code == make_sgr(b'48;5;235')
    assert color.color_reset == make_sgr(b'49')
    assert color.color_types == [('bg', make_sgr(b'48;5;235'))]
    assert color.rgb is False


def test_color_format_color_foreground():
    '''Foreground color.'''
    color = chromaterm.Color('f#123123', rgb=False)

    assert color.color == 'f#123123'
    assert color.color_code == make_sgr(b'38;5;235')
    assert color.color_reset == make_sgr(b'39')
    assert color.color_types == [('fg', make_sgr(b'38;5;235'))]
    assert color.rgb is False


def test_color_format_color_multiple():
    '''Multiple different color types. Also tests case insensitivity'''
    color = chromaterm.Color('f#123123 b#aBCDef bOLd', rgb=False)

    assert color.color == 'f#123123 b#abcdef bold'
    assert color.color_code == make_sgr(b'38;5;235', b'48;5;153', b'1')
    assert color.color_reset == make_sgr(b'22', b'49', b'39')
    assert color.color_types == [
        ('fg', make_sgr(b'38;5;235')),
        ('bg', make_sgr(b'48;5;153')),
        ('bold', make_sgr(b'1')),
    ]
    assert color.rgb is False


def test_color_format_color_rgb():
    '''RGB color.'''
    color = chromaterm.Color('b#123123', rgb=True)

    assert color.color == 'b#123123'
    assert color.color_code == make_sgr(b'48;2;18;49;35')
    assert color.color_reset == make_sgr(b'49')
    assert color.color_types == [('bg', make_sgr(b'48;2;18;49;35'))]
    assert color.rgb is True


def test_color_format_color_style():
    '''Style color.'''
    styles = {k: v for k, v in chromaterm.COLOR_TYPES.items() if v.get('code')}

    for style in styles:
        color = chromaterm.Color(style, rgb=False)

        assert color.color == style
        assert color.color_code == styles[style]['code']
        assert color.color_reset == styles[style]['reset']
        assert color.color_types == [(style, styles[style]['code'])]
        assert color.rgb is False


def test_color_invalid_type_color():
    '''Color with an int for color.'''
    with pytest.raises(TypeError, match='color must be a string'):
        chromaterm.Color(123)


def test_color_invalid_type_rgb():
    '''Color with an int for rgb.'''
    with pytest.raises(TypeError, match='rgb must be a boolean'):
        chromaterm.Color('bold', rgb=1)


def test_color_invalid_value_background():
    '''Color with an incorrect background color.'''
    with pytest.raises(ValueError, match='invalid color format'):
        chromaterm.Color('b#12312')

    with pytest.raises(ValueError, match='invalid color format'):
        chromaterm.Color('b#12312x')


def test_color_invalid_value_foreground():
    '''Color with an incorrect foreground color.'''
    with pytest.raises(ValueError, match='invalid color format'):
        chromaterm.Color('f#12312')

    with pytest.raises(ValueError, match='invalid color format'):
        chromaterm.Color('f#12312x')


def test_color_invalid_value_style():
    '''Color with an incorrect style.'''
    with pytest.raises(ValueError, match='invalid color format'):
        chromaterm.Color('something')


def test_color_invalid_value_duplicate_background():
    '''Color with multiple background colors.'''
    with pytest.raises(ValueError, match='one foreground and one background'):
        chromaterm.Color('b#123123 b#456456')


def test_color_invalid_value_duplicate_foreground():
    '''Color with multiple background colors.'''
    with pytest.raises(ValueError, match='one foreground and one background'):
        chromaterm.Color('f#123123 f#456456')


def test_palette_add_color():
    '''Confirm a color and its value are added to the palette.'''
    palette = chromaterm.Palette()
    palette.add_color('red', '#ff0000')

    assert palette.colors['red'] == '#ff0000'


def test_palette_add_color_invalid_type_name():
    '''Add a color with an invalid name type.'''
    with pytest.raises(TypeError, match='color name must be a string'):
        chromaterm.Palette().add_color(1, '#ff0000')


def test_palette_add_color_invalid_type_value():
    '''Add a color with an invalid value type.'''
    with pytest.raises(TypeError, match='color value must be a string'):
        chromaterm.Palette().add_color('red', 1)


def test_palette_add_color_invalid_value_name():
    '''Add a color with an invalid name value.'''
    with pytest.raises(ValueError, match='name accepts alphanumerics, dashes'):
        chromaterm.Palette().add_color('red$', '#ff0000')


def test_palette_add_color_invalid_value_name_duplicate():
    '''Add a color with a name that's already in use.'''
    palette = chromaterm.Palette()
    palette.add_color('red', '#ff0000')

    with pytest.raises(ValueError, match='same name already exists'):
        palette.add_color('red', '#ff0000')


def test_palette_add_color_invalid_value_name_reserved():
    '''Add a color with a reserved name.'''
    for color_type in chromaterm.COLOR_TYPES:
        with pytest.raises(ValueError, match='color name is reserved'):
            chromaterm.Palette().add_color(color_type, '#ff0000')


def test_palette_add_color_invalid_value_value():
    '''Add a color with an invalid value value.'''
    with pytest.raises(ValueError, match='must be in `#123abc` format'):
        chromaterm.Palette().add_color('red', 'h#ff0000')


def test_palette_resolve():
    '''Resolve a color name using a palette'''
    palette = chromaterm.Palette()
    palette.add_color('red', '#ff0000')
    palette.add_color('green', '#00ff00')
    palette.add_color('blue', '#0000ff')

    assert palette.resolve('b.red') == 'b#ff0000'
    assert palette.resolve('f.green bold') == 'f#00ff00 bold'
    assert palette.resolve('123 f.blue italic') == '123 f#0000ff italic'
    assert palette.resolve('f#0000ff bold') == 'f#0000ff bold'
    assert palette.resolve('random') == 'random'


def test_palette_resolve_invalid_type_name():
    '''Resolve a color name with the wrong type.'''
    with pytest.raises(TypeError, match='color must be a string'):
        chromaterm.Palette().resolve(False)


def test_palette_resolve_invalid_value_name_not_in_palette():
    '''Resolve a color name that isn't in the palette.'''
    with pytest.raises(ValueError, match='not in palette'):
        chromaterm.Palette().resolve('b.red')


def test_rule_get_matches(pcre):
    '''Get matches of rule that only colors the default regex group.'''
    color = chromaterm.Color('bold')
    rule = chromaterm.Rule('hello|world', color, pcre=pcre)

    data = b'hello world'
    expected = [(0, 5, color), (6, 11, color)]

    assert rule.get_matches(data) == expected


def test_rule_get_matches_new_line(pcre):
    '''Attempt to match a new line, which shouldn't be possible as it's considered
    as a separator (i.e. not passed to be highlighted). New lines are only used
    to replace data matched by exclusive rules.'''
    rule = chromaterm.Rule(r'hell\Wo', chromaterm.Color('bold'), pcre=pcre)

    assert not rule.get_matches(b'hell\no')


def test_rule_get_matches_groups(pcre):
    '''Get matches of rule that colors default and specific regex groups.'''
    colors = {
        0: chromaterm.Color('bold'),
        1: chromaterm.Color('b#123123'),
        2: chromaterm.Color('f#456456')
    }
    rule = chromaterm.Rule('(h)(i)', colors, pcre=pcre)

    data = b'hi'
    expected = [(0, 2, colors[0]), (0, 1, colors[1]), (1, 2, colors[2])]

    assert rule.get_matches(data) == expected


def test_rule_get_matches_groups_optional_no_matches(pcre):
    '''Get matches of rule that has a specific regex group that is optional. When
    the optional group is not in the match, the color should not be inserted.'''
    color = chromaterm.Color('bold')
    rule = chromaterm.Rule('hello(,)? world', {1: color}, pcre=pcre)

    data = b'hello, world'
    expected = [(5, 6, color)]

    assert rule.get_matches(data) == expected

    data = b'hello world'
    expected = []

    assert rule.get_matches(data) == expected


def test_rule_get_matches_no_colors(pcre):
    '''Get matches of rule that has no colors – nothing is changed.'''
    assert not chromaterm.Rule('hello', pcre=pcre).get_matches(b'hello')


def test_rule_get_matches_zero_length_match(pcre):
    '''Get matches of rule that has a regex that matches zero-length data.'''
    color = chromaterm.Color('bold')
    rule = chromaterm.Rule('hello() world', {1: color}, pcre=pcre)

    assert not rule.get_matches(b'hello world')


def test_rule_set_color_clear_existing(pcre):
    '''Clear the color of a rule by setting it to None.'''
    color = chromaterm.Color('bold')
    colors = {0: color, 1: color}
    rule = chromaterm.Rule('(hello|world)', colors, pcre=pcre)

    assert list(rule.colors) == [0, 1]
    rule.set_color(None, group=0)
    assert list(rule.colors) == [1]


def test_rule_set_color_default_group(pcre):
    '''Add to a rule a color to the default regex group (0).'''
    rule = chromaterm.Rule('hello', pcre=pcre)
    assert rule.color is None

    color = chromaterm.Color('bold')
    rule.set_color(color)
    assert rule.color is color
    assert rule.colors.get(0) is color


def test_rule_set_color_named_group(pcre):
    '''Reference a group by name. It should be converted to an index.'''
    rule = chromaterm.Rule('(?P<hi>hello)', pcre=pcre)
    assert rule.color is None

    color = chromaterm.Color('bold')
    rule.set_color(color, group='hi')
    assert rule.color is None
    assert rule.colors.get(1) is color

    rule.set_color(color)
    assert list(rule.colors) == [0, 1]


def test_rule_set_color_specific_group(pcre):
    '''Add to a rule a color to a specific regex group. Additionally, ensure
    that the dict keys are ordered according to the group number.'''
    rule = chromaterm.Rule(r'he(llo)\1', pcre=pcre)
    assert rule.color is None

    color = chromaterm.Color('bold')
    rule.set_color(color, group=1)
    assert rule.color is None
    assert rule.colors.get(1) is color

    rule.set_color(color)
    assert list(rule.colors) == [0, 1]


def test_rule_set_color_invalid_type_color(pcre):
    '''Add to a rule a color with invalid group type.'''
    rule = chromaterm.Rule('hello', pcre=pcre)

    with pytest.raises(TypeError, match='color must be a chromaterm.Color'):
        rule.set_color(True)


def test_rule_set_color_invalid_type_group(pcre):
    '''Add to a rule a color with invalid color type.'''
    rule = chromaterm.Rule('hello', pcre=pcre)

    with pytest.raises(TypeError, match='group must be an integer or a strin'):
        rule.set_color(chromaterm.Color('bold'), group=1.1)


def test_rule_set_color_invalid_value_group_index(pcre):
    '''Reference the index of a group that doesn't exist.'''
    rule = chromaterm.Rule(r'he(llo)\1', pcre=pcre)

    with pytest.raises(ValueError, match='regex has 1 group'):
        rule.set_color(chromaterm.Color('bold'), group=2)


def test_rule_set_color_invalid_value_group_name(pcre):
    '''Reference the name of a group that doesn't exist.'''
    rule = chromaterm.Rule('(?P<hi>hello)', pcre=pcre)

    with pytest.raises(ValueError, match='named group .+ not in regex'):
        rule.set_color(chromaterm.Color('bold'), group='bla')


def test_rule_change_color(pcre):
    '''Confirm that a color change overrides the old one.'''
    rule = chromaterm.Rule('hello', chromaterm.Color('bold'), pcre=pcre)

    old_color = rule.color
    rule.color = chromaterm.Color('b#123123')
    assert old_color is not rule.color


def test_rule_change_pcre(pcre):
    '''Confirm that a change in the PCRE flag recompiles the regex.'''
    # pylint: disable=protected-access
    rule = chromaterm.Rule('hello', chromaterm.Color('bold'), pcre=pcre)

    old_regex_object = rule._regex_object
    rule.pcre = not rule.pcre
    assert old_regex_object is not rule._regex_object


def test_rule_format_color_dict(pcre):
    '''Accept a dictionary of groups and colors.'''
    colors = {1: chromaterm.Color('bold'), 2: chromaterm.Color('italic')}
    rule = chromaterm.Rule('h(e)(llo)', colors, pcre=pcre)

    assert rule.colors[1] is colors[1]
    assert rule.colors[2] is colors[2]


def test_rule_invalid_type_color(pcre):
    '''Rule with an invalid color type.'''
    with pytest.raises(TypeError, match='color must be a chromaterm.Color'):
        chromaterm.Rule('hello', True, pcre=pcre)


def test_rule_invalid_type_description(pcre):
    '''Rule with an invalid description type.'''
    with pytest.raises(TypeError, match='description must be a string'):
        chromaterm.Rule('hello', description=True, pcre=pcre)


def test_rule_invalid_type_exclusive(pcre):
    '''Rule with an invalid exclusive type.'''
    with pytest.raises(TypeError, match='exclusive must be a boolean'):
        chromaterm.Rule('hello', exclusive='', pcre=pcre)


def test_rule_invalid_type_pcre():
    '''Rule with an invalid pcre type.'''
    with pytest.raises(TypeError, match='pcre must be a boolean'):
        chromaterm.Rule('hello', pcre='')


def test_rule_invalid_type_regex(pcre):
    '''Rule with an invalid regex type.'''
    with pytest.raises(TypeError, match='regex must be a string'):
        chromaterm.Rule(True, pcre=pcre)


def test_config_highlight(pcre):
    '''Highlight with one rule.'''
    rule = chromaterm.Rule('hello', chromaterm.Color('b#123123'), pcre=pcre)
    config = chromaterm.Config()
    config.rules.append(rule)

    data = b'hello world'
    expected = [
        rule.color.color_code, b'hello', rule.color.color_reset, b' world'
    ]

    assert config.highlight(data) == b''.join(expected)


def test_config_highlight_adjoin_type_different(pcre):
    '''Two rules with different color types, and one ending where the other
    starts. Both are applied without any overlap in the codes, independent of the
    order.
    1: -------
    2:        -------'''
    rule1 = chromaterm.Rule('he', chromaterm.Color('b#123123'), pcre=pcre)
    rule2 = chromaterm.Rule('llo', chromaterm.Color('f#456456'), pcre=pcre)
    config = chromaterm.Config()
    config.rules.append(rule1)
    config.rules.append(rule2)

    data = b'hello'
    expected = [
        rule1.color.color_code, b'he', rule1.color.color_reset,
        rule2.color.color_code, b'llo', rule2.color.color_reset
    ]

    assert config.highlight(data) == b''.join(expected)
    config.rules.reverse()
    assert config.highlight(data) == b''.join(expected)


def test_config_highlight_adjoin_type_mixed(pcre):
    '''Two rules with mixed color types, with one ending where the other starts.
    Both are applied without any overlap in the codes, independent of the
    order.
    1: -------
    2:        -------'''
    rule1 = chromaterm.Rule('he',
                            chromaterm.Color('b#123123 blink'),
                            pcre=pcre)
    rule2 = chromaterm.Rule('llo',
                            chromaterm.Color('b#456456 bold'),
                            pcre=pcre)
    config = chromaterm.Config()
    config.rules.append(rule1)
    config.rules.append(rule2)

    data = b'hello'
    expected = [
        rule1.color.color_code, b'he', rule1.color.color_reset,
        rule2.color.color_code, b'llo', rule2.color.color_reset
    ]

    assert config.highlight(data) == b''.join(expected)
    config.rules.reverse()
    assert config.highlight(data) == b''.join(expected)


def test_config_highlight_adjoin_type_same(pcre):
    '''Two rules with same color type, with one ending where the other starts.
    Both are applied without any overlap in the codes, independent of the order.
    1: -------
    2:        -------'''
    rule1 = chromaterm.Rule('he', chromaterm.Color('b#123123'), pcre=pcre)
    rule2 = chromaterm.Rule('llo', chromaterm.Color('b#456456'), pcre=pcre)
    config = chromaterm.Config()
    config.rules.append(rule1)
    config.rules.append(rule2)

    data = b'hello'
    expected = [
        rule1.color.color_code, b'he', rule1.color.color_reset,
        rule2.color.color_code, b'llo', rule2.color.color_reset
    ]

    assert config.highlight(data) == b''.join(expected)
    config.rules.reverse()
    assert config.highlight(data) == b''.join(expected)


def test_config_highlight_common_beginning_type_different(pcre):
    '''Two rules with different color types, and both sharing the same start of
    a match. The most recent rule will be closer to the match's start.
    1: --------------
    2: -------'''
    rule1 = chromaterm.Rule('hello', chromaterm.Color('b#123123'), pcre=pcre)
    rule2 = chromaterm.Rule('he', chromaterm.Color('f#456456'), pcre=pcre)
    config = chromaterm.Config()
    config.rules.append(rule1)
    config.rules.append(rule2)

    data = b'hello'
    expected = [
        rule1.color.color_code, rule2.color.color_code, b'he',
        rule2.color.color_reset, b'llo', rule1.color.color_reset
    ]

    assert config.highlight(data) == b''.join(expected)
    config.rules.reverse()
    expected[0], expected[1] = expected[1], expected[0]
    assert config.highlight(data) == b''.join(expected)


def test_config_highlight_common_beginning_type_mixed(pcre):
    '''Two rules with different color types, and both sharing the same start of
    a match. The most recent rule will be closer to the match's start.
    1: --------------
    2: -------'''
    rule1 = chromaterm.Rule('hello',
                            chromaterm.Color('b#123123 italic'),
                            pcre=pcre)
    rule2 = chromaterm.Rule('he', chromaterm.Color('b#456456 bold'), pcre=pcre)
    config = chromaterm.Config()
    config.rules.append(rule1)
    config.rules.append(rule2)

    data = b'hello'
    expected = [
        rule1.color.color_code, rule2.color.color_code, b'he', RESET_BOLD,
        rule1.color.color_types[0][1], b'llo', rule1.color.color_reset
    ]

    assert config.highlight(data) == b''.join(expected)


def test_config_highlight_common_beginning_type_same(pcre):
    '''Two rules with same color type, and both sharing the same start of a
    match. The most recent rule will be closer to the match's start.
    1: --------------
    2: -------'''
    rule1 = chromaterm.Rule('hello', chromaterm.Color('b#123123'), pcre=pcre)
    rule2 = chromaterm.Rule('he', chromaterm.Color('b#456456'), pcre=pcre)
    config = chromaterm.Config()
    config.rules.append(rule1)
    config.rules.append(rule2)

    data = b'hello'
    expected = [
        rule1.color.color_code, rule2.color.color_code, b'he',
        rule1.color.color_code, b'llo', rule1.color.color_reset
    ]

    assert config.highlight(data) == b''.join(expected)


def test_config_highlight_common_end_type_different(pcre):
    '''Two rules with different color types, and both sharing the same end of a
    match. The most recent rule will be closer to the match's end.
    1: --------------
    2:        -------'''
    rule1 = chromaterm.Rule('hello', chromaterm.Color('b#123123'), pcre=pcre)
    rule2 = chromaterm.Rule('llo', chromaterm.Color('f#456456'), pcre=pcre)
    config = chromaterm.Config()
    config.rules.append(rule1)
    config.rules.append(rule2)

    data = b'hello'
    expected = [
        rule1.color.color_code, b'he', rule2.color.color_code, b'llo',
        rule2.color.color_reset, rule1.color.color_reset
    ]

    assert config.highlight(data) == b''.join(expected)
    config.rules.reverse()
    expected[-1], expected[-2] = expected[-2], expected[-1]
    assert config.highlight(data) == b''.join(expected)


def test_config_highlight_common_end_type_mixed(pcre):
    '''Two rules with different color types, and both sharing the same end of a
    match. The most recent rule will be closer to the match's end.
    1: --------------
    2:        -------'''
    rule1 = chromaterm.Rule('hello',
                            chromaterm.Color('b#123123 italic'),
                            pcre=pcre)
    rule2 = chromaterm.Rule('llo',
                            chromaterm.Color('b#456456 bold'),
                            pcre=pcre)
    config = chromaterm.Config()
    config.rules.append(rule1)
    config.rules.append(rule2)

    data = b'hello'
    expected = [
        rule1.color.color_code, b'he', rule2.color.color_code, b'llo',
        RESET_BOLD, rule1.color.color_types[0][1], rule1.color.color_reset
    ]

    assert config.highlight(data) == b''.join(expected)


def test_config_highlight_common_end_type_same(pcre):
    '''Two rules with same color type, and both sharing the same end of a
    match. The most recent rule will be closer to the match's end.
    1: --------------
    2:        -------'''
    rule1 = chromaterm.Rule('hello', chromaterm.Color('b#123123'), pcre=pcre)
    rule2 = chromaterm.Rule('llo', chromaterm.Color('b#456456'), pcre=pcre)
    config = chromaterm.Config()
    config.rules.append(rule1)
    config.rules.append(rule2)

    data = b'hello'
    expected = [
        rule1.color.color_code, b'he', rule2.color.color_code, b'llo',
        rule1.color.color_code, rule1.color.color_reset
    ]

    assert config.highlight(data) == b''.join(expected)


def test_config_highlight_encapsulate_type_different(pcre):
    '''Three rules, with matches that encapsulate each other, different color
    types. None of them should affect the rest as they are of different types. The
    order in which the rules are applied should not matter.
    1:     --
    2:   ------
    3: ----------'''
    rule1 = chromaterm.Rule('lo wo', chromaterm.Color('b#123123'), pcre=pcre)
    rule2 = chromaterm.Rule('llo wor', chromaterm.Color('f#456456'), pcre=pcre)
    rule3 = chromaterm.Rule('hello world', chromaterm.Color('bold'), pcre=pcre)
    config = chromaterm.Config()
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
    config.rules.reverse()
    assert config.highlight(data) == b''.join(expected)


def test_config_highlight_encapsulate_type_mixed(pcre):
    '''Three rules, with matches that encapsulate each other, mixed color types.
    The colors that are different should not affect each other, but those that
    are the same should correctly track each others colors and update their
    resets appropriately.
    1:     --
    2:   ------
    3: ----------'''
    rule1 = chromaterm.Rule('lo wo',
                            chromaterm.Color('b#123123 italic'),
                            pcre=pcre)
    rule2 = chromaterm.Rule('llo wor',
                            chromaterm.Color('b#456456 bold'),
                            pcre=pcre)
    rule3 = chromaterm.Rule('hello world', chromaterm.Color('bold'), pcre=pcre)
    config = chromaterm.Config()
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
    config.rules.reverse()
    assert config.highlight(data) == b''.join(expected)


def test_config_highlight_encapsulate_type_same(pcre):
    '''Three rules, with matches that encapsulate each other, same color type.
    The reset of a match would be updated to the color of the encapsulating one.
    Once the second match updates the reset of the first one to a color, it is no
    longer considered a reset, and therefore the third match would not update it
    to its own color. The order in which the rules are applied should not matter.
    1:     --
    2:   ------
    3: ----------'''
    rule1 = chromaterm.Rule('lo wo', chromaterm.Color('b#123123'), pcre=pcre)
    rule2 = chromaterm.Rule('llo wor', chromaterm.Color('b#456456'), pcre=pcre)
    rule3 = chromaterm.Rule('hello world',
                            chromaterm.Color('b#abcabc'),
                            pcre=pcre)
    config = chromaterm.Config()
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
    config.rules.reverse()
    assert config.highlight(data) == b''.join(expected)


def test_config_highlight_exclusive(pcre):
    '''Two exclusive rules, partially overlapping matches. The first rule to match
    the input gets to color it.
    1:     ----------
    2: ----------'''
    rule1 = chromaterm.Rule('llo',
                            chromaterm.Color('bold'),
                            exclusive=True,
                            pcre=pcre)
    rule2 = chromaterm.Rule('hell',
                            chromaterm.Color('italic'),
                            exclusive=True,
                            pcre=pcre)
    config = chromaterm.Config()
    config.rules.append(rule1)
    config.rules.append(rule2)

    data = b'hello'
    expected = [b'he', rule1.color.color_code, b'llo', rule1.color.color_reset]

    assert config.highlight(data) == b''.join(expected)
    config.rules.reverse()
    expected = [rule2.color.color_code, b'hell', rule2.color.color_reset, b'o']
    assert config.highlight(data) == b''.join(expected)


def test_config_highlight_overlap_full_type_different(pcre):
    '''Two rules, fully overlapping matches, different color types. Should not
    affect each other as they have different types. Most recent rule should be
    closest to the match.
    1: ----------
    2: ----------'''
    rule1 = chromaterm.Rule('hello', chromaterm.Color('b#123123'), pcre=pcre)
    rule2 = chromaterm.Rule('hello', chromaterm.Color('f#456456'), pcre=pcre)
    config = chromaterm.Config()
    config.rules.append(rule1)
    config.rules.append(rule2)

    data = b'hello'
    expected = [
        rule1.color.color_code, rule2.color.color_code, b'hello',
        rule2.color.color_reset, rule1.color.color_reset
    ]

    assert config.highlight(data) == b''.join(expected)


def test_config_highlight_overlap_full_type_mixed(pcre):
    '''Two rules, fully overlapping matches, mixed color types. The different
    color types should not affect each other, but those that are the same should
    update the oldest reset with the most recent rule's color code.
    1: ----------
    2: ----------'''
    rule1 = chromaterm.Rule('hello',
                            chromaterm.Color('b#123123 bold'),
                            pcre=pcre)
    rule2 = chromaterm.Rule('hello',
                            chromaterm.Color('b#456456 italic'),
                            pcre=pcre)
    config = chromaterm.Config()
    config.rules.append(rule1)
    config.rules.append(rule2)

    data = b'hello'
    expected = [
        rule1.color.color_code, rule2.color.color_code, b'hello', RESET_ITALIC,
        rule1.color.color_types[0][1], rule1.color.color_reset
    ]

    assert config.highlight(data) == b''.join(expected)


def test_config_highlight_overlap_full_type_same(pcre):
    '''Two rules, fully overlapping matches, same color type. The reset of the
    first rule should be replaced with the color code of the second (most recent)
    rule to prevent the reset from interrupting the color.
    1: ----------
    2: ----------'''
    rule1 = chromaterm.Rule('hello', chromaterm.Color('b#123123'), pcre=pcre)
    rule2 = chromaterm.Rule('hello', chromaterm.Color('b#456456'), pcre=pcre)
    config = chromaterm.Config()
    config.rules.append(rule1)
    config.rules.append(rule2)

    data = b'hello'
    expected = [
        rule1.color.color_code, rule2.color.color_code, b'hello',
        rule1.color.color_code, rule1.color.color_reset
    ]

    assert config.highlight(data) == b''.join(expected)


def test_config_highlight_overlap_partial_type_different(pcre):
    '''Two rules, partially overlapping matches, different color types. The rules
    should not affect each other. Tested in reverse order, too.
    1: ----------
    2:     ----------'''
    rule1 = chromaterm.Rule('hell', chromaterm.Color('b#123123'), pcre=pcre)
    rule2 = chromaterm.Rule('llo', chromaterm.Color('f#456456'), pcre=pcre)
    config = chromaterm.Config()
    config.rules.append(rule1)
    config.rules.append(rule2)

    data = b'hello'
    expected = [
        rule1.color.color_code, b'he', rule2.color.color_code, b'll',
        rule1.color.color_reset, b'o', rule2.color.color_reset
    ]

    assert config.highlight(data) == b''.join(expected)
    config.rules.reverse()
    assert config.highlight(data) == b''.join(expected)


def test_config_highlight_overlap_partial_type_mixed(pcre):
    '''Two rules, partially overlapping matches, mixed color types. The different
    color types should not affect each other, but those that are the same should
    correctly update the reset that is in the middle of the other match to that
    of the match's color. Order of rules should not matter.
    1: ----------
    2:     ----------'''
    rule1 = chromaterm.Rule('hell',
                            chromaterm.Color('b#123123 bold'),
                            pcre=pcre)
    rule2 = chromaterm.Rule('llo',
                            chromaterm.Color('b#456456 italic'),
                            pcre=pcre)
    config = chromaterm.Config()
    config.rules.append(rule1)
    config.rules.append(rule2)

    data = b'hello'
    expected = [
        rule1.color.color_code, b'he', rule2.color.color_code, b'll',
        RESET_BOLD, rule2.color.color_types[0][1], b'o',
        rule2.color.color_reset
    ]

    assert config.highlight(data) == b''.join(expected)
    config.rules.reverse()
    assert config.highlight(data) == b''.join(expected)


def test_config_highlight_overlap_partial_type_same(pcre):
    '''Two rules, partially overlapping matches, same color type. The reset of
    the first rule should be replaced with the color code of the second (most
    recent) rule to prevent the reset from interrupting the color. The behavior
    should be consistent regardless of the order of the rules.
    1: ----------
    2:     ----------'''
    rule1 = chromaterm.Rule('hell',
                            chromaterm.Color('b#123123 bold'),
                            pcre=pcre)
    rule2 = chromaterm.Rule('llo',
                            chromaterm.Color('b#456456 bold'),
                            pcre=pcre)
    config = chromaterm.Config()
    config.rules.append(rule1)
    config.rules.append(rule2)

    data = b'hello'
    expected = [
        rule1.color.color_code, b'he', rule2.color.color_code, b'll',
        rule2.color.color_types[1][1], rule2.color.color_types[0][1], b'o',
        rule2.color.color_reset
    ]

    assert config.highlight(data) == b''.join(expected)
    config.rules.reverse()
    assert config.highlight(data) == b''.join(expected)


def test_config_highlight_no_rules():
    '''Highlight with config that has no rules – nothing is changed.'''
    config = chromaterm.Config()
    assert config.highlight(b'hello world') == b'hello world'


def test_config_highlight_tracking_common_beginning_type_different(pcre):
    '''A rule with a match that has a color of a different type just before its
    start. The rule's color is closer to the match and the reset is unaffected by
    the existing color.
    1: x-------------'''
    rule = chromaterm.Rule('hello', chromaterm.Color('b#123123'), pcre=pcre)
    config = chromaterm.Config()
    config.rules.append(rule)

    data = b'\x1b[33mhello'
    expected = [
        b'\x1b[33m', rule.color.color_code, b'hello', rule.color.color_reset
    ]

    assert config.highlight(data) == b''.join(expected)


def test_config_highlight_tracking_common_beginning_type_same(pcre):
    '''A rule with a match that has a color of the same type just before its
    start. The rule's color is closer to the match and the reset used is the
    existing color.
    1: x-------------'''
    rule = chromaterm.Rule('hello', chromaterm.Color('f#456456'), pcre=pcre)
    config = chromaterm.Config()
    config.rules.append(rule)

    data = b'\x1b[33mhello'
    expected = [b'\x1b[33m', rule.color.color_code, b'hello', b'\x1b[33m']

    assert config.highlight(data) == b''.join(expected)


def test_config_highlight_tracking_common_end_type_different(pcre):
    '''A rule with a match that has a color of a different type just after its
    end. The rule's reset is closer to the match and the reset is unaffected by
    the existing color.
    1: -------------x'''
    rule = chromaterm.Rule('hello', chromaterm.Color('b#123123'), pcre=pcre)
    config = chromaterm.Config()
    config.rules.append(rule)

    data = b'hello\x1b[33m'
    expected = [
        rule.color.color_code, b'hello', rule.color.color_reset, b'\x1b[33m'
    ]

    assert config.highlight(data) == b''.join(expected)


def test_config_highlight_tracking_common_end_type_same(pcre):
    '''A rule with a match that has a color of the same type just after its end.
    The rule's reset is closer to the match and is unaffected by the existing
    color.
    1: -------------x'''
    rule = chromaterm.Rule('hello', chromaterm.Color('f#456456'), pcre=pcre)
    config = chromaterm.Config()
    config.rules.append(rule)

    data = b'hello\x1b[33m'
    expected = [
        rule.color.color_code, b'hello', rule.color.color_reset, b'\x1b[33m'
    ]

    assert config.highlight(data) == b''.join(expected)


def test_config_highlight_tracking_full_reset_beginning(pcre):
    '''A rule with a match that has a full reset just before the start of the
    match. The rule's color is closer to the match and the reset used is the
    default for that color type.
    1: R-------------'''
    rule = chromaterm.Rule('hello', chromaterm.Color('f#456456'), pcre=pcre)
    config = chromaterm.Config()
    config.rules.append(rule)

    data = b'\x1b[0mhello'
    expected = [
        b'\x1b[0m', rule.color.color_code, b'hello',
        chromaterm.COLOR_TYPES[rule.color.color_types[0][0]]['reset']
    ]

    assert config.highlight(data) == b''.join(expected)


def test_config_highlight_tracking_full_reset_end(pcre):
    '''A rule with a match that has a full reset just after the end of the match.
    The rule's reset is closer to the match and the reset used is the default for
    that color type.
    1: -------------R'''
    rule = chromaterm.Rule('hello', chromaterm.Color('f#456456'), pcre=pcre)
    config = chromaterm.Config()
    config.rules.append(rule)

    data = b'hello\x1b[0m'
    expected = [
        rule.color.color_code, b'hello',
        chromaterm.COLOR_TYPES[rule.color.color_types[0][0]]['reset'],
        b'\x1b[0m'
    ]

    assert config.highlight(data) == b''.join(expected)


def test_config_highlight_tracking_full_reset_middle(pcre):
    '''A rule with a match that has a full reset in the middle of it. The full
    reset is changed to the color code of the match and the reset of the match
    is changed to a full reset.
    1: ------R-------'''
    rule = chromaterm.Rule('hello', chromaterm.Color('f#456456'), pcre=pcre)
    config = chromaterm.Config()
    config.rules.append(rule)

    data = b'hel\x1b[0mlo'
    expected = [
        rule.color.color_code, b'hel', rule.color.color_code, b'lo', b'\x1b[0m'
    ]

    assert config.highlight(data) == b''.join(expected)


def test_config_highlight_tracking_malformed(pcre):
    '''A rule with a match that has a malformed SGR in the middle. It should be
    ignored and inserted back into the match. Highlighting from the rule should
    still go through.
    1: x-------------'''
    rule = chromaterm.Rule('hello', chromaterm.Color('b#123123'), pcre=pcre)
    config = chromaterm.Config()
    config.rules.append(rule)

    data = b'he\x1b[38;5mllo'
    expected = [
        rule.color.color_code, b'he', b'\x1b[38;5m', b'llo',
        rule.color.color_reset
    ]

    assert config.highlight(data) == b''.join(expected)


def test_config_highlight_tracking_mixed_full_reset(pcre):
    '''Track multiple color types and ensure a full reset only defaults the types
    that were not updated by other colors in the data.'''
    rule1 = chromaterm.Rule('hello', chromaterm.Color('f#456456'), pcre=pcre)
    rule2 = chromaterm.Rule('world', chromaterm.Color('b#123123'), pcre=pcre)
    config = chromaterm.Config()
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


def test_config_highlight_tracking_multiline_type_different(pcre):
    '''Ensure that data with an existing color is tracked across highlights and
    does not affect the reset of a color of a different type.'''
    rule = chromaterm.Rule('hello', chromaterm.Color('b#123123'), pcre=pcre)
    config = chromaterm.Config()
    config.rules.append(rule)

    # Inject a foreground color to have it tracked
    assert config.highlight(b'\x1b[33m') == b'\x1b[33m'

    data = b'hello'
    expected = [rule.color.color_code, b'hello', rule.color.color_reset]

    assert config.highlight(data) == b''.join(expected)


def test_config_highlight_tracking_multiline_type_same(pcre):
    '''Ensure that data with an existing color is tracked across highlights and
    affects the reset of a color of the same type.'''
    rule = chromaterm.Rule('hello', chromaterm.Color('f#456456'), pcre=pcre)
    config = chromaterm.Config()
    config.rules.append(rule)

    # Inject a foreground color to have it tracked
    assert config.highlight(b'\x1b[33m') == b'\x1b[33m'

    data = b'hello'
    expected = [rule.color.color_code, b'hello', b'\x1b[33m']

    assert config.highlight(data) == b''.join(expected)
