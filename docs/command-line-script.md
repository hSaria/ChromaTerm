## Usage

Run any program and pipe it into `ct`, like so:

```shell
echo "Jul 14 12:28:19  Message from 1.2.3.4: Completed successfully" | ct
```

The script reads highlight rules from a configuration file called `.chromaterm.yml`,
located in your home directory. If the file is not there, a default one is copied.

!!! tip
    Set up functions, like `ssh() { /usr/bin/ssh "$@" | ct; }`, in your
    `.bash_profile` to always color the output of a program (`ssh` in this
    example).

---

## Highlight Rules

All of the highlight rules are placed under the `rules` list in the
configuration file. Here's an example config file:

```yaml linenums="1"
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

!!! note
    Check out the [`contrib/rules`](https://github.com/hSaria/ChromaTerm/tree/master/contrib/rules)
    directory in the project; it has topic-specific rules which are not in the
    defaults.

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

A color can be applied per RegEx group (see the 2nd example rule). Any group can
be referenced as long as it's in the regular expression.

---

## Controlling Terminal

When you use a pipe in your command, the program has the ability to detect that
its output is being redirected, causing it to act differently. For instance,
`less` is interactive when it's in a controlling terminal, but it will simply
echo out the file if it's being redirected.

To get around this, `ct` can spawn the program in a controlling terminal – or
rather make it seem like it is in one. For instance:

```shell
ct less file.txt
```
