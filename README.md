# ChromaTerm

[![PyPI version](https://badge.fury.io/py/chromaterm.svg)](https://badge.fury.io/py/chromaterm) [![Build Status](https://travis-ci.org/hSaria/ChromaTerm.svg?branch=master)](https://travis-ci.org/hSaria/ChromaTerm) [![Coverage Status](https://coveralls.io/repos/github/hSaria/ChromaTerm/badge.svg)](https://coveralls.io/github/hSaria/ChromaTerm)

A tool for colorizing the output of a terminal.

-   [About](#about)
-   [Installation](#installation)
-   [Usage](#usage)
-   [Highlight Rules](#highlight-rules)
    -   [Description](#description)
    -   [RegEx](#regex)
    -   [Color](#color)
        -   [Background and Foreground](#background-and-foreground)
        -   [Style](#style)
        -   [Group-Specific](#group-specific)
-   [Help](#help)

# About

ChromaTerm (`ct`) reads from standard input and colors it according to user-configurable rules.

Think of ChromaTerm like `grep`; just pipe things into it. However, unlike other programs which line-buffer, `ct` works with interactive applications, like `ssh`. In fact, I have `ssh() { /usr/bin/ssh $* | ct; }` in my `.bash_profile` to give my sessions color.

Here's an example using the rules in the default configuration file:

![alt text](https://github.com/hSaria/ChromaTerm/raw/master/.github/junos-show-interface.png "Example output")

# Installation

```shell
pip3 install chromaterm
```

> If you have the legacy version of ChromaTerm, be sure to uninstall it first.
>
> ```shell
> # If installed using HomeBrew
> brew uninstall chromaterm
>
> # If installed from source
> git clone -b legacy git@github.com:hSaria/ChromaTerm.git
> cd ChromaTerm/src/ && ./configure && make uninstall
> ```

# Usage

By default, ChromaTerm reads `.chromaterm.yml` in your home directory. As an example, run the following:

```shell
echo "Jul 14 12:28:19  Message from 1.2.3.4: Completed successfully" | ct
```

> The default config file is copied to your home directory if it's not there.

# Highlight Rules

All of the highlight rules are placed under the `rules` array in the configuration file. Here's an example config file:

```yaml
rules:
- description: My first rule colors the foreground
  regex: hello.+world
  color: f#ff0000

- description: Color the foreground and background for "there" and make it bold. Paint "buddy" red.
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

You can color the foreground and background simultaneously by separating them with a space, like `b#123456 f#abcdef`.

### Style

In addition to the foreground and background, the following styles are supported, though some terminals ignore them:

-   Blink
-   Bold
-   Italic
-   Strike
-   Underline

### Group-Specific

A color can be applied per RegEx group (see the 2nd example rule). You can apply as many groups as long as its in the RegEx.

# Help

If you've got any questions or suggestions, please open up an [issue](https://github.com/hSaria/ChromaTerm/issues/new/choose) (always appreciated).

There is no planned support for Windows due to the lack in standards support.
