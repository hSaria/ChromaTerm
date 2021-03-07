# ChromaTerm

[![Build status](https://img.shields.io/github/workflow/status/hSaria/ChromaTerm/CI/master)](https://github.com/hSaria/ChromaTerm/actions?query=workflow%3ACI)
[![Language grade: Python](https://img.shields.io/lgtm/grade/python/github/hSaria/ChromaTerm)](https://lgtm.com/projects/g/hSaria/ChromaTerm/context:python)
[![Coverage status](https://coveralls.io/repos/github/hSaria/ChromaTerm/badge.svg)](https://coveralls.io/github/hSaria/ChromaTerm)
[![Documentation status](https://readthedocs.org/projects/chromaterm/badge/?version=master)](https://chromaterm.readthedocs.io)
[![PyPI version](https://badge.fury.io/py/chromaterm.svg)](https://badge.fury.io/py/chromaterm)

ChromaTerm (`ct`) is a Python script that colors your output to terminal using
regular expressions. It works with interactive programs, like SSH.

## Installation

```shell
pip3 install chromaterm
```

## Usage

Think of `ct` like `grep`; just pipe data into it, like `ssh somewhere | ct`.

> If you always want to highlight a program, like `ssh`, you can set up a function
> in your `.bash_profile`, like `ssh() { /usr/bin/ssh "$@" | ct; }`.

![alt text](https://github.com/hSaria/ChromaTerm/raw/master/.github/junos-show-interface.png "Example output")

There are program that behave differently when piped, like `less`. In that case,
`ct` can hide the pipe by spawning your program. You just have to prefix the
command with `ct`, like `ct less file.txt`.

## Highlight Rules

ChromaTerm reads highlight rules from a configuration file called `.chromaterm.yml`,
located in your home directory. If the file is not there, a default one is created.

> Check out the [`contrib/rules`](https://github.com/hSaria/ChromaTerm/tree/master/contrib/rules)
> directory in the project; it some topic-specific rules.

The rules are stored as YAML, like so:

```yaml
rules:
- description: My first rule colors the foreground
  regex: hello.+world
  color: f#ff0000

- description: Make "there" bold and italic. Paint "buddy" red
  regex: Hey (there), (buddy)
  color:
    1: bold italic
    2: b#ff0000
```

### Description

Optional. It's purely for your sake.

### RegEx

The RegEx engine used is Python's [re](https://docs.python.org/3/library/re.html).

### Color

#### Background and Foreground

The color is a hex string prefixed by `b` for background (e.g. `b#123456`) and
`f` for foreground (e.g. `f#abcdef`).

#### Style

In addition to the background and foreground, the following styles are supported,
though some terminals ignore them:

 * Blink
 * Bold
 * Italic
 * Strike
 * Underline

### Group

A color can be applied per RegEx group (see the 2nd example rule). Any group in
the regular expression can be referenced.

## Help

If you've got any questions or suggestions, please open up an
[issue](https://github.com/hSaria/ChromaTerm/issues/new/choose) (always
appreciated).

### Windows support

To use ChromaTerm on Windows, you will need to run it with the
[Windows Subsystem for Linux (`WSL`)](https://docs.microsoft.com/en-us/windows/wsl/about)
