# ChromaTerm--
A tool for colorizing the output of a terminal.

- [About](#about)
- [Installation](#installation)
- [Usage](#usage)
- [Help](#help)

Quick summary of changes from the original ChromaTerm:
- Changed the regex library (was PCRE, now POSIX regex). **Extended regular expressions** are enabled.
- Optimised the search through highlight rules which results in over **60% improvement in output processing speed**. This is very notable when you are using a lot of rules and/or outputting a long string of text.
- **Reduced the dependency** on many libraries. ChromaTerm-- should  require fewer, if any, additional installations.
- ChromaTerm-- **multithreads** the input and output operations which significantly increases the **responsiveness**. With that addtion, polling at fixed intervals has been removed. This nearly negates the overhead of CT while wrapping around a process.
- Added an option to **toggle highlighting off**.
- Removed nearly every function apart from **Highlight**, which is the only thing I think this program should do. This significantly reduced the overhead on all operations of CT.


# About
ChromaTerm-- (CT--) is a slimmed-down version of [ChromaTerm](https://github.com/tunnelsup/chromaterm). It essentially acts as a wrapper for a process (e.g. a shell). Any activity within the process is ran through CT--.
While running,  CT-- listens for keywords and executes commands that allow you to configure CT--. Primarily, the `#highlight` command is the one you are after.

The original tool ([ChromaTerm by TunnelsUp](www.tunnelsup.com/chromaterm/)) has way more features if you are interested, though it doesn't seem to be maintained anymore and is far less responsive.


## Screenshots
Below are screenshots of using chromaterm while SSH'd into a Cisco firewall.

![Chromaterm IMG](http://tunnelsup.com/images/chroma1.PNG)

![Chromaterm IMG](http://tunnelsup.com/images/chroma2.PNG)

![Chromaterm IMG](http://tunnelsup.com/images/chroma3.PNG)


# Installation
```
git clone https://github.com/hSaria/ChromaTerm--.git
cd ChromaTerm--/src/
./configure
make
make install  # Optional: Move ct to the /usr/local/bin
```


# Usage
You will need a terminal program that can handle VT100 and ANSI color codes (many of them do, if not all).
- You can start CT-- by `./ct` or just `ct` if installed using `make install`.

## Modes
You have two options for running ChromaTerm--:
- Interactive: Open up CT-- without running any process. Good for testing commands.
- Direct: Run your commands then exit. Great for using `ct` as part of a script.

The only difference is that the **direct** mode runs with the `-e` flag. For example:
```
ct -c ~/.custom_chromatermrc -e "#run /bin/bash"
```

This example will open up CT--, load a custom configuration file (more on that later), then run a process (bash, in this case). Once the process closes, CT-- will too.

You can still run commands even while a process is running; type # on a new line that is empty then follow it with the require command. You can change the default command character (#); try `#config`.

## Commands
Below are quick summaries for some of the important commands.

> All commands are:
> - autocompleted to the closest match
> - case-insensitive

> Before writing a CT-- command, be sure to hit enter first. This is a limitation which I documented in `read_key` function in input.c.

#### `#highlight {condition} {action}` and `#unhighlight {condition}`
The output is scanned according to the condition. If a part of the text matches the condition, the action is taken on that text.
```
#highlight {([0-9]{2}:){2}[0-9]{2}} {bold green}
#highlight {(E|e)rr..} {<fca>}
```
The first will find the time in the format of "HH:MM:SS" and highlight it bold green. The second will highlight Err or err, and the following two characters. This is regular expression, so you can do a lot more.

You can remove a rule by using the `#unhighlight`. 
```
#unhighlight {(E|e)rr..}
```

You can also globally toggle highlighting by using `#config highlight <on|off>`. The default is on.

#### `#read {file}` and `#write {file}`
You can read a configuration file while inside a session. Any rules will be **merged** with the existing ones. Furthermore, you can write the configuration of the current session to a file.

#### `#run {process} ...`
This command will run a process and pass any arguments to it. CT will close once the process dies.

#### `#exit`
Exits CT--. The child process is terminated, too. `#quit` does the same thing.

## Configuration File
CT-- will look for a configuration file called `.chromatermrc` in the current directory then your home directory. It will load the first one it finds. The file should contain CT-- commands.

You may also override which configuration file is loaded by using the `-c` parameter. For example:
```
ct -c $HOME/.config/ChromaTerm--/.chromatermrc
```

There is a sample file in the project. Feel free to use it.


# Help

## Parameters
For a list of the available CT-- parameters:
```
ct -h
```

## Commands
To get a list of the available CT-- commands:
```
#commands
```

Additionally, there are help topics within the tool. The following commands show (1) the list of help topics, (2) the help for the `#highlight` command, and (3) all of content of the help topics, respectively:
```
#help
#help highlight
#help all
```

## Questions or Bugs
To ask questions or submit bugs, please create an issue on **this** fork.
