# ChromaTerm--
A tool for colorizing the output of a terminal.

- [About](#about)
- [Installation](#installation)
- [Usage](#usage)
- [Help](#help)

Quick summary of changes from the original ChromaTerm:
- Optimised the search through highlight rules which results in over **60% improvement in output processing speed**. This is very notable when you have a lot of rules and/or outputting a large amount of text.
- **Reduced the dependency** on many libraries. ChromaTerm-- should  require fewer, if any, additional installations.
- ChromaTerm-- **multithreads** the input and output operations which significantly increases the **responsiveness**. With that addition, polling at fixed intervals has been removed. This nearly negates the overhead of CT while wrapping around a process.
- Added an option to **toggle highlighting off**.
- Removed nearly every function apart from **Highlight**, which is the only thing I think this program should do. This significantly reduced the overhead on all operations of CT--.


# About
ChromaTerm-- (CT--) is a slimmed-down version of [ChromaTerm](https://github.com/tunnelsup/chromaterm). It essentially acts as a wrapper for a process (e.g. a shell). Any input to or output from the process is processed through CT--.
While running,  CT-- listens for a key command character and executes subsequent commands that allow you to configure CT--. You are probably after the `%highlight` command.


## Screenshots
A few different examples.

<p><img src="https://raw.githubusercontent.com/hSaria/ChromaTerm--/master/images/junos-show-interface-brief.png"/><img width=422px height=425px align=left src="https://raw.githubusercontent.com/hSaria/ChromaTerm--/master/images/junos-show-route.png"/><img width=405px height=425px align=right src="https://raw.githubusercontent.com/hSaria/ChromaTerm--/master/images/ios-show-interface.png"/></p>


# Installation
```
git clone https://github.com/hSaria/ChromaTerm--.git
cd ChromaTerm--/src/ && ./configure && make install
```

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

> All commands are:
> - autocompleted to the closest match, and
> - case-insensitive.

#### `%highlight {condition} {action} {priority}` and `%unhighlight {condition}`
Output from the child process is scanned according to the `{condition}`. Any text that matches the condition will be highlighted with the specified `{action}`.
```
%highlight {(\d{2}:){2}[\d{2}} {bold green}
%highlight {(E|e)rr..} {<fca>}
```
The first will highlight "dd:dd:dd" bold green (d for digit). The second will highlight Err or err, and the following two characters. This is PCRE **regular expression**, so you can do a lot more.

You can remove a rule by using the `%unhighlight`.
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
CT-- will look for a configuration file called `.chromatermrc` in the current working directory (`pwd`) then your home directory. It will load the CT-- commands from the first one it finds.

You may also override which configuration file is loaded by using the `-c` parameter. For example:
```
ct -c $HOME/.config/ChromaTerm--/.chromatermrc
```

> There is a sample file in the project. Feel free to use it.


# Help

## Parameters
For a list of the available CT-- parameters:
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
