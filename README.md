# ChromaTerm

[![Build status](https://img.shields.io/github/workflow/status/hSaria/ChromaTerm/CI/master)](https://github.com/hSaria/ChromaTerm/actions?query=workflow%3ACI)
[![Language grade: Python](https://img.shields.io/lgtm/grade/python/github/hSaria/ChromaTerm)](https://lgtm.com/projects/g/hSaria/ChromaTerm/context:python)
[![Coverage status](https://coveralls.io/repos/github/hSaria/ChromaTerm/badge.svg)](https://coveralls.io/github/hSaria/ChromaTerm)
[![Downloads](https://static.pepy.tech/personalized-badge/chromaterm?period=total&units=international_system&left_color=grey&right_color=brightgreen&left_text=downloads)](https://pepy.tech/project/chromaterm)
[![PyPI version](https://badge.fury.io/py/chromaterm.svg)](https://badge.fury.io/py/chromaterm)

ChromaTerm (`ct`) is a Python script that colors your output to terminal using
regular expressions. It works with interactive programs, like SSH.

![alt text](https://github.com/hSaria/ChromaTerm/raw/master/.github/junos-show-interface.png "Example output")

## Installation

```shell
pip3 install chromaterm
```

## Usage

Think of `ct` like `grep`; just pipe data into it, like `ssh somewhere | ct`.

```shell
echo "Jul 14 12:28:19  Message from 1.2.3.4: Completed successfully" | ct
```

If you want to always highlight a program, you can set up a function in your
`.bash_profile`. For instance, here's one for `ssh`:
```shell
ssh() { /usr/bin/ssh "$@" | ct; }
```

Some programs behave differently when piped, like `less`. In that case, `ct` can
hide the pipe by spawning your program. You just have to prefix the command with
`ct`, like `ct less file.txt`.

## Highlight Rules

ChromaTerm reads highlight rules from a YAML configuration file, formatted like so:

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

ChromaTerm will look in the following locations for the config file and use the
first one it finds:

 * `$HOME/.chromaterm.yml`
 * `$XDG_CONFIG_HOME/chromaterm/chromaterm.yml` (if `$XDG_CONFIG_HOME` is not set,
 it defaults to `~/config`)
 * `/etc/chromaterm/chromaterm.yml`

If no file is found, a default one is created in your home directory.

> Check out [`contrib/rules`](https://github.com/hSaria/ChromaTerm/tree/master/contrib/rules);
> it has some topic-specific rules that are not included in the defaults.

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
 * Invert
 * Italic
 * Strike
 * Underline

### Group

Colors can be applied per RegEx group (see the 2nd example rule). Any group in
the RegEx can be referenced, even group `0` (entire match).

### Exclusive

When multiple rules match the same text, ChromaTerm highlights the text with all
of the colors of the matching rules. If you want the text to be highlighted only
by the first rule that matches it, use the `exclusive` flag on the rule, like so:

```yaml
- regex: hello
  color: bold
  exclusive: True
```

In the code above, no other rule will highlight "hello", unless it comes first
and has the `exclusive` flag set to `True`.

## Help

If you've got any questions or suggestions, please open up an
[issue](https://github.com/hSaria/ChromaTerm/issues/new/choose) (always
appreciated).

### Windows support

To use ChromaTerm on Windows, you will need to run it with the
[Windows Subsystem for Linux (`WSL`)](https://docs.microsoft.com/en-us/windows/wsl/about)
