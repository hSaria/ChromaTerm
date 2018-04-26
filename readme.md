# ChromaTerm--
A tool for colorizing the output of a terminal.

- [About](#about)
- [Installation](#installation)
- [Usage](#usage)
- [Help](#help)


# About
ChromaTerm-- (CT--) is a slimmed-down version of [ChromaTerm](https://github.com/tunnelsup/chromaterm). It essentially acts as a wrapper for a process (e.g. a shell). Any activity within the process is ran through CT--. 
While running,  CT-- listens for keywords and executes commands that allow you to configure CT--. Primarily, the `#highlight` command is the one you are after.

The original tool ([ChromaTerm by TunnelsUp](www.tunnelsup.com/chromaterm/)) has way more features if you are interested. This fork just removes a lot of those features (I am only interested in the #highlight and any supporting commands for it).

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
> NOTE: the [pcre library](https://pcre.org) is required. (`homebrew install pcre` works.)


# Usage
You will need a terminal program that can handle VT100 and ANSI color codes (many of them do).
- You can start CT-- by `./ct` or just `ct` if installed to /usr/local/bin.

## Modes
You have two options for running ChromaTerm--:
- Interactive: Open up CT-- without running any process. Good for testing commands.
- Direct: Run your commands then exit. Great for using `ct` as part of a script.

The only difference is that the **direct** mode runs with the `-e` flag. For example:
```
ct -e "#highlight {%d.%d.%d.%d} {bold green}; #write {./chromatermrc_for_ipv4}; #run /bin/bash"
```

This example will open up CT--, create a new highlight rule, write all rules and configuration to a file, run a process (bash, in this case). Once the process closes, CT-- will close, too. 

You can still run commands even while a process is running; type # on a new line that is empty then follow it with the require command. You can change the default command character (#); see `#help config`.

## Commands
Below are quick summaries for some of the important commands.

> All commands are:
> - autocompleted to the closest match
> - case-insensitive

> Before writing a CT-- command, be sure to hit enter first. This is a limitation which I documenated in `read_key` function in input.c.

#### `#highlight {condition} {action}` and `#unhighlight {condition}`
The output is scanned according to the condition. If a part of the text matches the condition, the action is takes on that text. 
```
#highlight {%d.%d.%d.%d} {bold green}
#highlight {{(E|e)rr..}} {bold red}
```
The first will find four digits separated by dots then highlight them bold yellow. The second has a regular expression (enclosed by two sets of curly backets).

You can remove a rule by using the `#unhighlight`.
```
#unhighlight {{(E|e)rr..}}
#unhighlight %*
```
The first will remove a specific rule, while the second will remove all highlight rules.

#### `#read {file}` and `#write {file}`
You can read a configuration file while inside a session. Any rules will be **merged** with the existing ones. Furthermore, you can write the configuration of the current session to a file.

#### `#run {process} ...`
This command will run a process and pass any arguments to it.

#### `#exit`
Exits CT--. The child process is terminated, too. If the child process dies, CT-- will automatically exit.

## Configuration File
CT-- will look for a configuration file called `.chromatermrc` in the current directory then the home directory, and will load the first one it finds. The file should contain CT-- commands.

You may also override which configuration file is loaded by using the `-c` parameter. For example:
```
ct -c $HOME/.config/chromaterm--/.chromatermrc 
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
#help %*
```

## Questions or Bugs
To ask questions or submit bugs, please create an issue on **this** fork.
