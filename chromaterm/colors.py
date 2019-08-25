#!/usr/bin/env python3
"""List of predefined colors and their codes."""

COLORS = {
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


def demo():
    """Print the 256-color demo."""
    # TODO
    print('Demo')


def get_code(color):
    """Return the ANSI code to be used when highlighting with `color` or None if
    the `color` is invalid."""
    if COLORS.get(color):
        return COLORS.get(color)

    return None
