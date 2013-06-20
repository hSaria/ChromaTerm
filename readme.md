## About
Chromaterm is a terminal colorization tool that runs on linux and is produced by TunnelsUp.com. It essentially acts as a wrapper for the linux shell. Once it starts it then starts a new shell. Any activity within that new shell will be ran through Chromaterm. Chromaterm listens for keywords, that are user defined, and will highlight them with user defined colors.

This can be extremely helpful especially when accessing Cisco routers and firewalls from a central Linux server. Colorizing the ssh screen of a Cisco CLI output is very convenient. 

## Screenshots
Below are screenshots of using chromaterm while SSH'd into a Cisco firewall.<br>
![Chromaterm IMG](http://tunnelsup.com/images/chroma1.PNG)<br>
![Chromaterm IMG](http://tunnelsup.com/images/chroma2.PNG)<br>
![Chromaterm IMG](http://tunnelsup.com/images/chroma3.PNG)


## Install
Installation is easy.

- Download the files from github. Either by using the download link or by doing `git clone https://github.com/tunnelsup/chromaterm.git`

- `cd chromaterm/src/` Go into the src directory.

- `./configure` Configure the program.

- `make` Create the binary called ct.

- `make install` Optional. It will move ct to the ~ directory.

Once installation is complete a new file called `ct` will be in the src/ directory. Move this to your home directory. You can start the program by doing the following:

`./ct <config_file>`

## Creating the Config file
Use your text editor of choice to create a file called ct.cfg and put the following in it.

```
#run session bash
#config regex on
#event {SESSION DISCONNECTED} {#end}

#highlight {%d.%d.%d.%d} {bold yellow}
#highlight { any } {bold white}
#highlight {{permit(ted)*}} {bold green}
#highlight {{(d|D)eny}} {bold red}
#highlight {{ (E|e)rr..}} {bold white}
#highlight {INSIDE} {bold blue}
#highlight {OUTSIDE} {bold green}
#highlight {DMZ} {bold magenta}

#substitute {^Cisco %1 Version %2, %3} {Cisco %1 <134>Version %2<088>, %3}
#substitute {%1pkts encaps: %d, %3} {%1pkts encaps: <150>%2<088>, %3}
#substitute {%1pkts decaps: %d, %3} {%1pkts decaps: <120>%2<088>, %3}
#substitute {%1 uptime is %2} {%1 uptime is <150>%2<088>}
```
The `highlight` keyword will simply look for the text in the first argument and colorize it using the color chosen in the second argument.

The `substitute` keyword will search and replace text display. %1, %2 etc are variables that are stored. They can then be called later to colorize a variable.


## Usage
You will need a terminal program that can handle VT100 and ANSI color codes. Such programs that can do this are putty, SecureCRT, or any native Linux terminal.

Start chromaterm using the following command: `./ct ct.cfg`

Once Chromaterm is running use the `#help` command to display help. Some useful help commands:

`#help highlight`<br>
`#help substitute`<br>
`#help colors`<br>
`#help colordemo`

To exit chromaterm type:<br>
`#end`

You can then edit the ct.cfg file to your satisfaction to add more keyword highlighting or change colors. A sample ct.cfg file is included in the files which is what I use as my config.

Now that it's running you can test it by telnetting or ssh'ing into a device and watch how higlighted keywords defined in the config file will become colorized.

## Further Help
Official website is found here:<br>
[http://www.tunnelsup.com/tup/2013/06/16/chromaterm](http://www.tunnelsup.com/tup/2013/06/16/chromaterm)<br>
Use the comment section on the bottom of the page to ask questions or submit bugs.


