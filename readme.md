# ChromaTerm--
A tool for colorizing the output of a terminal.

- [About](#about)
- [Installation](#installation)
- [Usage](#usage)
- [Help](#help)


# About
ChromaTerm-- (CT--) is a process-wrapper that highlights any output according to user-configurable rules.

## Screenshots
Some quick examples. CT-- uses Regex to find matches according to your rules.

<p><img src="https://raw.githubusercontent.com/hSaria/ChromaTerm--/master/images/junos-show-interface-brief.png"/><img width=422px height=425px align=left src="https://raw.githubusercontent.com/hSaria/ChromaTerm--/master/images/junos-show-route.png"/><img width=405px height=425px align=right src="https://raw.githubusercontent.com/hSaria/ChromaTerm--/master/images/ios-show-interface.png"/></p>


# Installation
```
git clone https://github.com/hSaria/ChromaTerm--.git
cd ChromaTerm--/src/ && ./configure && make install
```

> You must have PCRE2 development installed. E.g. `brew install pcre2` or `apt install libpcre2-dev`. For more info, see https://pcre.org

> The `install` parameter is optional; it will copy `ct` to /usr/local/bin

> You can uninstall `ct` by running `make uninstall`


# Usage
- Simply type `ct` and you'll be in a shell which is wrapped by CT--.

While running, type `%` after a **new line** (`\n`) then follow it with a command. For a list of available commands:
```
%commands
```


## Commands
Below are quick summaries for some of the important commands.

> All commands are autocompleted to the closest match and case-insensitive.

#### `%highlight {condition} {action} {priority}` and `%unhighlight {condition}`
Output from the child process is scanned according to the `{condition}`. Any text that matches the condition will be highlighted with the specified `{action}`.
```
%highlight {(\d{2}:){2}[\d{2}} {bold green}
%highlight {(E|e)rr..} {<fca>}
```
The first will highlight "dd:dd:dd" bold green (d for digit). The second will highlight Err or err, and the following two characters. CT-- uses a **Perl-Compatible Regular Expressions** engine.

You can remove a rule by using `%unhighlight`.
```
%unhighlight {(E|e)rr..}
```

You can globally toggle highlighting by using `%config highlight <on|off>`.

> The highlight rules can be sorted using the optional `{priority}` parameter. Smaller value is higher in the list.

#### `%read {file}` and `%write {file}`
You can read a configuration file while inside a session. Commands will be **merged** with any existing ones. Additionally, you can write the configuration of the current session to a file.

#### `%exit`
Exits CT--. The child process is terminated, too. `%quit` does the same thing.

## Configuration File
At launch, CT-- will look for a configuration file called `.chromatermrc` in the current working directory (`pwd`) then your home directory. It will load the CT-- commands from the first one it finds.

You may also override which configuration file is loaded by using the `-c` parameter. For example:
```
ct -c $HOME/.config/ChromaTerm--/myAwesomeFirewall
```

> There is a sample file in the project. Feel free to use it.


# Help
CT-- offer a lot more than I have written here. If you want to know more, use the commands below.

## Parameters
For a list of the available CT-- arguments:
```
ct -h
```

## Commands
To get a list of the available CT-- commands:
```
%commands
```

There are help topics embedded in the tool. The following commands show (1) the list of help topics, (2) the help for the `%highlight` command, and (3) the content of all help topics, respectively:
```
%help
%help highlight
%help all
```

## Questions, Suggestions, or Bugs
Please open up an issue (always appreciated).
