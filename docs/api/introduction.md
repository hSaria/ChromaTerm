## API

ChromaTerm can color the output of your module with minimal effort. The API
has three different ways in which you can highlight.

### [`Color`](../color)

A `Color` is the simplest way to highlight text.

```python linenums="1"
from chromaterm import Color

color = Color('bold')

# The string is bold-styled
color.print('Hello World!')
```

!!! tip
    See the [initializer of `Color`](../color/#chromaterm.Color.__init__) for
    more details on the format of a color.

---


### [`Rule`](../rule)

This class will only highlight the text matching a regular expression.

You can use a `Rule` to highlight significant words without having to go through
every message in your program.

```python linenums="1"
from chromaterm import Color, Rule

rule = Rule('World', color=Color('b#412100'))

# Only "World" is colored
rule.print('Hello World!')
```

---

### [`Config`](../config)

Multiple rules can be bundled into an instance of `Config`.

When highlighting, the text is first matched against all of the rules' regular
expressions and then the colors are added to the text. This makes matching more
accurate as the color (ANSI codes) of one rule won't interfere during the
matching of another.

```python linenums="1"
from chromaterm import Color, Config, Rule

config = Config()
config.add_rule(Rule('Goo', color=Color('underline')))
config.add_rule(Rule('Good', color=Color('b#00aa00')))

# "Goo" is underlined, but "Good" is green
config.print('Good')
```

---

## Decorators

All of the API classes can be used as decorators. This saves you from having to
explicitly reference the object. Here's an example with `Color`, but it's the
same case with `Rule` and `Config`.

```python linenums="1"
from chromaterm import Color

@Color('f#00dd00')
def multiply(x, y):
    return x * y

# The output will be colored green
print(multiply(5, 10))
```

---

## Group-specific Rules

The highlighting of a [Rule](../rule) can be further limited to groups within
the regular expression. By default, the entire match (group 0) is highlighted.

```python linenums="1"
from chromaterm import Color, Rule

rule = Rule('Hello (World)!')
rule.add_color(Color('underline'), group=1)

# Only "World" is underlined
rule.print('Hello World!')

# The regex did not match, and so the "World" group is not underlined
rule.print('Hi World!')
```

---

## Highlighting Precedence

!!! note
    The points below are very peculiar and ones you likely won't have to worry
    about; they're only here to document the behavior.

When the `Config` class has multiple rules and they match the same text, the
following occurs:

* If one match is inside another (encapsulates it), the color of the inner match
  remains unchanged.
* If two rules match the same text exactly, the latest rule added to the `Config`
  instance will color the match.
* If two rules have partially-overlapping matches, the overlap is colored by the
  trailing match (the latter one in the text).
