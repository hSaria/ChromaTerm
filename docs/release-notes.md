# Release Notes

## ChromaTerm v0.6

### v0.6.0 – 2020-04-30

* Introduced an API for ChromaTerm; you can now color the output of your own module.
* Added documentation (hosted at [Read the Docs](https://chromaterm.readthedocs.io)).

---

## ChromaTerm v0.5

### v0.5.9 – 2020-02-28

* [#84](https://github.com/hSaria/ChromaTerm/issues/84) - Improved the accuracy of highlighting input that does not end with a new line.

---

### v0.5.8 – 2020-02-09

* [#80](https://github.com/hSaria/ChromaTerm/issues/80) - Some programs spawned by CT, like `screen`, were able to escape highlighting.
* [#81](https://github.com/hSaria/ChromaTerm/issues/81) - Operating System Command (OSC) codes, like setting the title, were not ignored.
* [#83](https://github.com/hSaria/ChromaTerm/issues/83) - Handling of text, specifically while typing, is now more consistent.

---

### v0.5.7 – 2020-01-20

* Cleaned up the `--reload` option as it was throwing errors from the `psutil` module on some platforms.

---

### v0.5.6 – 2019-12-04

* Moved the networking rules out of the defaults; there's a new `contrib/rules` directory that contains topic-specific rules. If you don't have any custom rules, you may want to delete your existing configuration file to get this cleaner version.

---

### v0.5.5 – 2019-11-03

* Fixed zero-length matches getting erroneously colored.
* Improved error handling when a rule's `regex` is an integer.
* Minor performance optimizations.
* Module restructuring to make the code a bit cleaner.
* Added Python 3.8 support (was already supported – now added to CI workflows).

---

### v0.5.4 – 2019-10-21

* Better handling of broken pipes, like when piping into `head`.
* The output of the help section is a bit clearer regarding the `program` argument.

---

### v0.5.3 – 2019-10-21

* Fixed bug with color detection following changes for [#75](https://github.com/hSaria/ChromaTerm/issues/75).

---

### v0.5.2 – 2019-10-13

* [#74](https://github.com/hSaria/ChromaTerm/issues/74) - ChromaTerm can now run a program and make it look like it lives on the controlling terminal. Simply prepend your command with `ct`. This is useful for coloring programs like `ct less file.log`. You should still pipe when possible as it's simpler and more logical to look at.
* [#75](https://github.com/hSaria/ChromaTerm/issues/75) - Some special ANSI codes were not being handled (ignored) appropriately, causing some rules to incorrectly not match.

Thanks to [@aaronols](https://github.com/aaronols) for raising the issues above.

---

### v0.5.1 – 2019-10-09

* [#73](https://github.com/hSaria/ChromaTerm/issues/73) - Bold style wasn't being turned off correctly. Thanks to [@heigren](https://github.com/heigren) for pointing it out.

---

### v0.5.0 – 2019-10-05

* Removed support for the deprecated `group` key in a rule; per-group coloring is still supported, just not using the `group` key.
* Removed uninstallation note for legacy ChromaTerm.
* A bit o' cleanup.

---

## ChromaTerm v0.4

### v0.4.8 – 2019-09-17

* Fixed a bug when the input contains multiple combined colors (multiple SGR's into a single ANSI escape code).

---

### v0.4.7 – 2019-09-17

* Fixed a rare bug where the type of a color was not updated if it was a full reset in the middle of the new color.
* Optimized color handling and tracking.
* Update license classifier for PyPI.
* Removed a bit of redundant code and simplified color type internals.

---

### v0.4.6 – 2019-09-13

* Improved handling of existing colors in the input data; `ct` will now detect the type(s) of a compound ANSI code (e.g. foreground and background colors combined into a single SGR sequence).

---

### v0.4.5 – 2019-09-13

* Significantly improved the quality of color tracking by doing it per type (i.e. independent foreground, background, and per-style color tracking). This fixes a bug when using, for example, `vi` which, on some platforms, injects some color-resets at start-up to ensure deterministic output.
* Adjusted the chunk size per read to increase responsiveness when coloring lots of output.
* Improved performance by removing some redundant operations.
* Changed license to MIT. The master branch of this project is not related to the original ChromaTerm; I prefer to be more relaxed with my code and its license.
* Minor cleanup of readme.

---

### v0.4.4 – 2019-09-06

* Any existing colors are temporarily removed from the input before RegEx matching occurs, making the RegEx of the rules more accurate.
* Minor code cleanup.

---

### v0.4.3 – 2019-09-05

* No operational changes. Just a bit of cleanup on readme and the setup script.

---

### v0.4.2 – 2019-09-05

* [#71](https://github.com/hSaria/ChromaTerm/issues/71) - Added `--reload` program argument to instruct all other ChromaTerm instances to reload their config.
* [#72](https://github.com/hSaria/ChromaTerm/issues/72) - Support for terminal styles (e.g. bold).
* Multiple colors are compiled into a single Control Sequence Introducer (CSI).
* Added deprecation message to group key.

Once again, thanks to [@Ren60FHk](https://github.com/Ren60FHk) for the suggestions.

---

### v0.4.1 – 2019-09-05

* [#70](https://github.com/hSaria/ChromaTerm/issues/70) - Support for multiple groups per rule. Thanks [@Ren60FHk](https://github.com/Ren60FHk) for the suggestion.
* Highlighting using a group-specific rule to an optional RegEx group which is not part of the match outputs malformed data.

---

### v0.4.0 – 2019-09-03

With a major rewrite into Python 3, the new code is leaps and bounds better than what it was.

* Configuration file format standardized to YAML. This will make any future changes seamless as well as backwards- and forwards-compatible. It's also nicer to read.
* Simple installation via PyPI with `pip3 install chromaterm`.
* The color of a rule can optionally be applied to a specific group. By default, it'll color the entire match.
* Color format changed to hex. It is automatically downscaled to the closest `xterm-256` color.
* RGB support. The support in your terminal is automatically detected but can be force-enabled with `--rgb`.
* Simplified the rule definitions by removing the priority as it is no longer required (see next point).
* Highlight rules can now overlap (collide). Previously, there was a mechanism to prevent this as it wasn't handled very nicely. But not anymore – ChromaTerm will color text as instructed by the rules.
* The default highlight rules have been improved to cover more edge-cases, while being simpler.
* Much better handling of colors in the input. Now, the color of unhighlighted text is restored to what it was. Previously, it was always reset to the default terminal text color.
* ChromaTerm now takes into account ANSI movement sequences, reducing false-positive (mis)matching.

Huge thanks for [@cpriest](https://github.com/cpriest) for his suggestions; He took the initiative of raising the issues.
