# ChromaTerm--
A tool for colorizing the output of a terminal.

- [About](#about)
- [Installation](#installation)
- [Usage](#usage)
- [Help](#help)

Quick summary of changes from the original ChromaTerm:
- Optimised the search through highlight rules which results in over **60% improvement in output processing speed**. This is very notable when you are using a lot of rules and/or outputting a long string of text.
- **Reduced the dependency** on many libraries. ChromaTerm-- should  require fewer, if any, additional installations.
- ChromaTerm-- **multithreads** the input and output operations which significantly increases the **responsiveness**. With that addition, polling at fixed intervals has been removed. This nearly negates the overhead of CT while wrapping around a process.
- Added an option to **toggle highlighting off**.
- Removed nearly every function apart from **Highlight**, which is the only thing I think this program should do. This significantly reduced the overhead on all operations of CT--.


# About
ChromaTerm-- (CT--) is a slimmed-down version of [ChromaTerm](https://github.com/tunnelsup/chromaterm). It essentially acts as a wrapper for a process (e.g. a shell). Any input to or output from the process is processed through CT--.
While running,  CT-- listens for a key command character and executes subsequent commands that allow you to configure CT--. Primarily, the `%highlight` command is the one you are after.

The original tool ([ChromaTerm by TunnelsUp](www.tunnelsup.com/chromaterm/)) has more features if you are interested, though it doesn't seem to be maintained anymore and is far less responsive.


## Screenshots
Below are screenshots of using chromaterm while SSH'd into a Cisco firewall.

<div align="center">
  <img src="https://raw.githubusercontent.com/hSaria/ChromaTerm--/master/images/junos-show-interface-brief.png"width="800"/>
  <div align="left" width="50%">
    <img src="https://raw.githubusercontent.com/hSaria/ChromaTerm--/master/images/junos-show-route.png" width="400"/>
  </div>
  <div align="right" width="50%">
    <img src="https://raw.githubusercontent.com/hSaria/ChromaTerm--/master/images/ios-show-interface.png" width="400"/>
  </div>
</div>


# Installation
```
git clone https://github.com/hSaria/ChromaTerm--.git
cd ChromaTerm--/src/ && ./configure && make
make install  # Optional: Move ct to the /usr/local/bin
```

> You can uninstall `ct` by running `make uninstall`

# Usage
You will need a terminal program that can handle VT100 and ANSI color codes (many of them do, if not all).
- You can start CT-- by `./ct` or just `ct` if installed using `make install`.

While running, type `%` at a new line then follow it with the required command. You can change the default command character (%); try `%config`.

> Before typing the command character, be sure to hit enter first and no other. This is a limitation which I documented in `read_key` function in io.c.

## Commands
Below are quick summaries for some of the important commands.

> All commands are:
> - autocompleted to the closest match
> - case-insensitive

#### `%highlight {condition} {action} {priority}` and `%unhighlight {condition}`
The output is scanned according to the condition. If a part of the text matches the condition, the action is taken on that text.
```
%highlight {([0-9]{2}:){2}[0-9]{2}} {bold green}
%highlight {(E|e)rr..} {<fca>}
```
The first will highlight "dd:dd:dd" bold green (d for digit). The second will highlight Err or err, and the following two characters. This is regular expression, so you can do a lot more.

You can remove a rule by using the `%unhighlight`.
```
%unhighlight {(E|e)rr..}
```

You can also globally toggle highlighting by using `%config highlight <on|off>`. The default is on.

> To sort the list of highlights, you can use the optional {priority} parameter. Smaller value is higher in the list.

#### `%read {file}` and `%write {file}`
You can read a configuration file while inside a session. Any rules will be **merged** with the existing ones. Furthermore, you can write the configuration of the current session to a file.

#### `%exit`
Exits CT--. The child process is terminated, too. `%quit` does the same thing.

## Configuration File
CT-- will look for a configuration file called `.chromatermrc` in the current directory then your home directory. It will load the first one it finds. The file should contain CT-- commands.

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

Additionally, there are help topics imbedded in the tool. The following commands show (1) the list of help topics, (2) the help for the `%highlight` command, and (3) the content of all help topics, respectively:
```
%help
%help highlight
%help all
```

## Questions or Bugs
To ask questions or submit bugs, please create an issue.
