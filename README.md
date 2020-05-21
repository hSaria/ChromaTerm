# ChromaTerm

[![Build status](https://travis-ci.com/hSaria/ChromaTerm.svg?branch=master)](https://travis-ci.com/hSaria/ChromaTerm)
[![Language grade: Python](https://img.shields.io/lgtm/grade/python/g/hSaria/ChromaTerm.svg)](https://lgtm.com/projects/g/hSaria/ChromaTerm/context:python)
[![Coverage status](https://coveralls.io/repos/github/hSaria/ChromaTerm/badge.svg)](https://coveralls.io/github/hSaria/ChromaTerm)
[![Documentation status](https://readthedocs.org/projects/chromaterm/badge/?version=master)](https://chromaterm.readthedocs.io)
[![PyPI version](https://badge.fury.io/py/chromaterm.svg)](https://badge.fury.io/py/chromaterm)

ChromaTerm is a Python module and script used for coloring the output to
terminals.

## Installation

```shell
pip3 install chromaterm
```

## Command Line Script

You can pipe a program into `ct` to have its output colored according to
user-configurable rules. For instance, `ssh | ct`:

![alt text](https://github.com/hSaria/ChromaTerm/raw/master/.github/junos-show-interface.png "Example output")

Check out the script's
[documentation](https://chromaterm.readthedocs.io/command-line-script/)
for more details on usage.

## API

You can color your module's output with ChromaTerm's API. Here's a simple example:

```python
from chromaterm import Color

color = Color('bold')

# The string is bold-styled
color.print('Hello World!')
```

Have a look at the [introduction](https://chromaterm.readthedocs.io/api/introduction/)
for the API.

## Help

If you've got any questions or suggestions, please open up an
[issue](https://github.com/hSaria/ChromaTerm/issues/new/choose) (always
appreciated).
