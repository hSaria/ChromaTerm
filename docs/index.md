[![Build status](https://travis-ci.com/hSaria/ChromaTerm.svg?branch=master)](https://travis-ci.com/hSaria/ChromaTerm)
[![Language grade: Python](https://img.shields.io/lgtm/grade/python/g/hSaria/ChromaTerm.svg)](https://lgtm.com/projects/g/hSaria/ChromaTerm/context:python)
[![Coverage status](https://coveralls.io/repos/github/hSaria/ChromaTerm/badge.svg)](https://coveralls.io/github/hSaria/ChromaTerm)
[![Documentation status](https://readthedocs.org/projects/chromaterm/badge/?version=master)](https://chromaterm.readthedocs.io)
[![PyPI version](https://badge.fury.io/py/chromaterm.svg)](https://badge.fury.io/py/chromaterm)

ChromaTerm is a utility for coloring output meant for terminals. You can interact
with it using a command line script and through an API.

## [Command Line Script](./command-line-script)

Included with ChromaTerm is a command line script, called `ct`, that can be used
to color the output of your programs according to user-configurable rules. You
can use the script to consistently highlight keywords across different programs.

Think of `ct` like `grep`; just pipe data into it. It even works with interactive
applications, like `ssh`.

## [API](./api/introduction)

If you have a Python module the output of which is meant for terminals, you may
be interested in extending it through ChromaTerm's API to highlight significant
output.
