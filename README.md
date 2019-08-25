# ChromaTerm

A tool for colorizing the output of a terminal.

-   [About](#about)
-   [Installation](#installation)
-   [Usage](#usage)
-   [Highlight Rules](#highlight-rules)
    -   [Types](#types)
        -   [Simple](#simple)
        -   [Complex](#complex)
    -   [Syntax](#syntax)
        -   [Description](#description)
        -   [RegEx](#regex)
        -   [Color](#color)
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

# Usage

By default, ChromaTerm reads a YAML file at `~/.chromatermrc` which would have your highlight rules. As an example, run the following:

    echo "Jul 14 12:28:19: Message from 1.2.3.4" | ct

Think of ChromaTerm like `grep`; just pipe things into it. However, unlike other programs which line-buffer, `ct` works with interactive applications, like `ssh`. In fact, I have this in my .bash_profile `ssh() { /usr/bin/ssh $* | ct; }`.

> During installation, the default config file was copied to `~/.chromatermrc`; modify it to your liking.

# Highlight Rules

All of the highlight rules are placed under the `highlights` array in the configuration file.

## Types

### Simple

A simple rule matches using `regex` and highlights the match according to `color`. For example:

```yaml
- description: My first rule
  regex: hello.+world
  color: red
```

### Complex

A complex rule can color sub-groups differently. For example:

```yaml
- description: My first rule
  regex: hey (there)
  color:
    0: green
    1: blue
```

## Syntax

### Description

Optional. It's purely for your sake.

### RegEx

The RegEx engine used is [Python RegEx](https://pypi.org/project/regex/), not to be confused with the native Python `re`. It was chosen because it has support for variable-length look-behinds.

### Color

Once something matches the RegEx of a rule, the color is applied to the match. For complex rules, each group's color is applied to the respective group.

The color can be a named (predefined) action or a custom one. Multiple colors can be used, like `bold red` or `<fca><BAF>`. The named actions are:

-   VT100: bold, dim, underscore, blink, b black, b blue, b cyan, b green, b magenta, b red, b white, b yellow, black, blue, cyan, white, and yellow.
-   xterm-256: b azure, b ebony, b jade, b lime, b orange, b pink, b  silver, b tan, b violet, azure, ebony, jade, light azure, light ebony, light jade, light lime, light orange, light pink, light silver, light tan, light violet, lime, orange, pink, silver, tan, and violet.

> Terminals that support xterm-256 codes will support VT100 codes.

To use a custom action, `man ct` has more info on that. Run `ct --demo` to see the full range of colors.

# Help

ChromaTerm includes a manual; check out `man ct`.

## Questions, Suggestions, or Bugs

Please open up an issue (always appreciated).
