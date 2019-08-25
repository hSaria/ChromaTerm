#!/usr/bin/env python3
"""List of predefined colors and their codes."""

import re

PREDEFINED_COLORS = {
    'bold': '<188>',
    'dim': '<288>',
    'underscore': '<488>',
    'blink': '<588>',
    'b azure': '<ABD>',
    'b black': '<880>',
    'b blue': '<884>',
    'b cyan': '<886>',
    'b ebony': '<G04>',
    'b green': '<882>',
    'b jade': '<ADB>',
    'b lime': '<BDA>',
    'b magenta': '<885>',
    'b orange': '<DBA>',
    'b pink': '<DAB>',
    'b red': '<881>',
    'b silver': '<CCC>',
    'b tan': '<CBA>',
    'b violet': '<BAD>',
    'b white': '<887>',
    'b yellow': '<883>',
    'azure': '<abd>',
    'black': '<808>',
    'blue': '<848>',
    'cyan': '<868>',
    'ebony': '<g04>',
    'green': '<828>',
    'jade': '<adb>',
    'light azure': '<acf>',
    'light ebony': '<bbb>',
    'light jade': '<afc>',
    'light lime': '<cfa>',
    'light orange': '<fca>',
    'light pink': '<fac>',
    'light silver': '<eee>',
    'light tan': '<eda>',
    'light violet': '<caf>',
    'lime': '<bda>',
    'magenta': '<858>',
    'orange': '<dba>',
    'pink': '<dab>',
    'red': '<818>',
    'silver': '<ccc>',
    'tan': '<cba>',
    'violet': '<bad>',
    'white': '<878>',
    'yellow': '<838>',
}


def decode_color_code(color):
    """Convert from <xyz> notation to ANSI code."""
    code = ''

    for match in re.findall(r'<([0-9a-fA-FgG]{3})>', color):
        start = '\033['

        if re.match('^[0-9]{3}$', match) and match != '888':  # VT-100 codes
            codes = []

            for target, char in zip(['', '3', '4'], match):
                if char != '8':
                    codes.append(target + char)

            code += start + ';'.join(codes) + 'm'
        else:  # xterm-256 Colors
            # Foregroud if lower, otherwise for background
            target = '38;5;' if match[0].islower() else '48;5;'

            if re.match('^[gG][0-9]{2}$', match):  # Grayscale
                color_id = 232 + int(match[1:3])

                if not 232 <= color_id <= 255:  # Invalid
                    return None
            elif re.match('^[a-fA-F]{3}$', match):  # Colors
                # Must be all lower-case or all upper-case; not mixed-case
                if not match.isupper() and not match.islower():
                    return None

                color_id = 16
                for base, char in zip([36, 6, 1], match):
                    char_id = ord(char) - ord('a' if match.islower() else 'A')
                    color_id += base * char_id
            else:
                return None  # Unknown

            code += start + target + str(color_id) + 'm'

    return code


def demo():
    """Print the xterm-256 demo."""
    # TODO
    print('Demo')


def get_code(color):
    """Return the ANSI code to be used when highlighting with `color` or None if
    the `color` is invalid."""
    # Matches the native color code
    if re.search(r'^(<[0-9a-fA-FgG]{3}>(\s|,)*)+$', color):
        return decode_color_code(color)

    # Attempt to match named (predefined) colors
    code = ''

    while True:
        found = False
        color = color.strip(', ')

        if not color:
            break

        for predefined_color in PREDEFINED_COLORS:
            if color.startswith(predefined_color):
                code += PREDEFINED_COLORS[predefined_color]
                color = color[len(predefined_color):]
                found = True
                break

        if not found:
            return None

    return decode_color_code(code)
