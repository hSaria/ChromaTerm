# ChromaTerm--
A tool for colorizing the output of a terminal.

- [About](#about)
- [Installation](#installation)
- [Usage](#usage)
- [Help](#help)


# About
ChromaTerm-- (ct) reads from standard input and highlights it according to user-configurable rules.

## Screenshots
Some quick examples. ChromaTerm-- uses Regex to find matches according to your rules.

<p><img src="https://raw.githubusercontent.com/hSaria/ChromaTerm--/master/images/junos-show-interface-brief.png"/><img width=422px height=425px align=left src="https://raw.githubusercontent.com/hSaria/ChromaTerm--/master/images/junos-show-route.png"/><img width=405px height=425px align=right src="https://raw.githubusercontent.com/hSaria/ChromaTerm--/master/images/ios-show-interface.png"/></p>

### <p>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</p>
# Installation [![Build Status](https://travis-ci.org/hSaria/ChromaTerm--.svg?branch=master)](https://travis-ci.org/hSaria/ChromaTerm--)
```
git clone https://github.com/hSaria/ChromaTerm--.git
cd ChromaTerm--/src/ && ./configure && make install
```

> You must have [PCRE2 or legacy PCRE](https://pcre.org) development installed. You can install PCRE2 with one of the following (depends on platform):
> - `brew install pcre2`
> - `apt install libpcre2-dev`
> - `yum install pcre2-devel`

> It is recommended that you use PCRE2 as you'll see a 16x to 20x increase in output processing speed. However, legacy PCRE is still very fast, so it's not the end of the world.

> You can uninstall ChromaTerm-- by running `make uninstall`


# Usage
- Create a configuration file at `~/.chromatermrc` and write a highlight rule to it. As an example, write `HIGHLIGHT {World} {blue}`
- Run `echo Hello, World! | ct` and you should see "World" get highlighted blue.

> If you're new, run `make ct-config` to copy the included configuration file to your home directory. Modify it to your liking

## Highlight rules
The syntax for a rule is: HIGHLIGHT {REGEX} {ACTION} {PRIORITY}

### REGEX
The RegEx engine used is PCRE (www.pcre.org). If supported, PCRE2 is used (much faster).

### ACTION
Once something matches the regex of a rule, the action of that rule is applied. The action can be a named (predefined) action or a custom one. Multiple actions can make up a single action as long as they are from the same category. For example: `{bold red}` or `{<fca><BAF>}`

The named actions are:
- VT100: bold, dim, underscore, blink, b black, b blue, b cyan, b green, b magenta, b red, b white, b yellow, black, blue, cyan, white, and yellow.
- xterm-256: b azure, b ebony, b jade, b lime, b orange, b pink, b  silver, b tan, b violet, azure, ebony, jade, light azure, light ebony, light jade, light lime, light orange, light pink, light silver, light tan, light violet, lime, orange, pink, silver, tan, and violet.

To use a custom action, look at `man ct` for more info on that.

### PRIORITY
(Optional) If a part of the text is matched by two rules, the rule with the lower priority value overrides the action of the higher priority value; lower priority value = higher precedence. Default priority is 1000.

# Help
ChromaTerm-- includes a manual; check out `man ct`.

## Questions, Suggestions, or Bugs
Please open up an issue (always appreciated).
