# ChromaTerm

[![Build status](https://travis-ci.com/hSaria/ChromaTerm.svg?branch=master)](https://travis-ci.com/hSaria/ChromaTerm)
[![Language grade: Python](https://img.shields.io/lgtm/grade/python/g/hSaria/ChromaTerm.svg)](https://lgtm.com/projects/g/hSaria/ChromaTerm/context:python)
[![Coverage status](https://coveralls.io/repos/github/hSaria/ChromaTerm/badge.svg)](https://coveralls.io/github/hSaria/ChromaTerm)
[![PyPI version](https://badge.fury.io/py/chromaterm.svg)](https://badge.fury.io/py/chromaterm)

ChromaTerm (`ct`) reads from standard input and colors it according to user-configurable rules.

Think of ChromaTerm like `grep`; just pipe data into it. However, unlike most programs which line-buffer, `ct` works with interactive applications, like `ssh`.

Here's an example using the rules in the default configuration file:

![alt text](https://github.com/hSaria/ChromaTerm/raw/master/.github/junos-show-interface.png "Example output")

# Installation

```shell
pip3 install chromaterm
```

# Usage

By default, ChromaTerm reads `.chromaterm.yml` in your home directory. As an example, run the following:

```shell
echo "Jul 14 12:28:19  Message from 1.2.3.4: Completed successfully" | ct
```

> The default config file is copied to your home directory if it's not there.

Tip: set up functions, like `ssh() { /usr/bin/ssh "$@" | ct; }`, in your `.bash_profile` to always color the output of a program (`ssh` in this example).

# Highlight Rules

All of the highlight rules are placed under the `rules` list in the configuration file. Here's an example config file:

```yaml
rules:
- description: My first rule colors the foreground
  regex: hello.+world
  color: f#ff0000

- description: Color the foreground and background for "there" and make it bold. Paint "buddy" red
  regex: Hey (there), (buddy)
  color:
    1: b#abcabc f#123123 bold
    2: b#ff0000
```

## Description

Optional. It's purely for your sake.

## RegEx

The RegEx engine used is Python's [re](https://docs.python.org/3/library/re.html).

## Color

### Background and Foreground

The color is a hex string prefixed by `b` for background (e.g. `b#123456`) and `f` for foreground (e.g. `f#abcdef`).

### Style

In addition to the background and foreground, the following styles are supported, though some terminals ignore them:

-   Blink
-   Bold
-   Italic
-   Strike
-   Underline

### Group

A color can be applied per RegEx group (see the 2nd example rule). Any group can be referenced as long as it's in the RegEx.

# Help

If you've got any questions or suggestions, please open up an [issue](https://github.com/hSaria/ChromaTerm/issues/new/choose) (always appreciated).

Check out the [`contrib/rules`](https://github.com/hSaria/ChromaTerm/tree/master/contrib/rules) directory in the project; it has topic-specific rules which are not in the defaults.

There is no planned support for Windows due to the lack of standards.
