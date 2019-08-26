# ChromaTerm

A tool for colorizing the output of a terminal.

-   [About](#about)
-   [Installation](#installation)
-   [Usage](#usage)
-   [Highlight Rules](#highlight-rules)
    -   [Description](#description)
    -   [RegEx](#regex)
    -   [Color](#color)
    -   [Group](#group)
-   [Help](#help)

# About

ChromaTerm (`ct`) reads from standard input and highlights according to user-configurable rules. Here's an example using the rules in the included configuration file:

![alt text](https://github.com/hSaria/ChromaTerm/raw/master/.github/junos-show-interface.png "Example output")

# Installation

    pip3 install chromaterm

> If you have the legacy version of ChromaTerm, be sure to uninstall it first.
>
>     # If installed using HomeBrew
>     brew uninstall chromaterm
>
>     # If installed from source
>     git clone -b legacy git@github.com:hSaria/ChromaTerm.git
>     cd ChromaTerm--/src/ && ./configure && make uninstall

# Usage

By default, ChromaTerm reads a YAML file at `~/.chromatermrc` which would have your highlight rules. As an example, run the following:

    echo "Jul 14 12:28:19: Message from 1.2.3.4" | ct

Think of ChromaTerm like `grep`; just pipe things into it. However, unlike other programs which line-buffer, `ct` works with interactive applications, like `ssh`. In fact, I have this in my .bash_profile `ssh() { /usr/bin/ssh $* | ct; }`.

> During installation, the default config file was copied to `~/.chromatermrc`; modify it to your liking.

# Highlight Rules

All of the highlight rules are placed under the `rules` array in the configuration file. Here's an example config file:

```yaml
rules:
- description: My first rule colors the foreground
  regex: hello.+world
  color: f#ff0000

- description: Background this time, but just for "there"
  regex: Hey (there), buddy
  color: b#ff0000
  group: 1
```

## Description

Optional. It's purely for your sake.

## RegEx

The RegEx engine used is [Python RegEx](https://pypi.org/project/regex/), not to be confused with the native Python `re`. It was chosen because it has support for variable-length look-behinds.

## Color

The color is a hex string prefixed by `b` for background (e.g. `b#123456`) and `f` for foreground (e.g. `f#abcdef`).

You can have the foreground and background colored by separating them with a space, like `b#123456 f#abcdef`.

## Group

Optional. By default, the entire match is colored. That can changed to a specific group in the `regex`.

# Help

Please open up an issue (always appreciated).
