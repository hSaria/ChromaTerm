/******************************************************************************
*   This program is protected under the GNU GPL (See COPYING)                 *
*                                                                             *
*   This program is free software; you can redistribute it and/or modify      *
*   it under the terms of the GNU General Public License as published by      *
*   the Free Software Foundation; either version 2 of the License, or         *
*   (at your option) any later version.                                       *
*                                                                             *
*   This program is distributed in the hope that it will be useful,           *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of            *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
*   GNU General Public License for more details.                              *
*                                                                             *
*   You should have received a copy of the GNU General Public License         *
*   along with this program; if not, write to the Free Software               *
*   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA *
******************************************************************************/

/******************************************************************************
*                  C H R O M A T E R M (C) 2013 (See CREDITS)                 *
******************************************************************************/

#include "defs.h"


struct help_type
{
	char                  * name;
	char                  * text;
};

/*
	This help table is a mess, but I got better things to do - Igor
*/

struct help_type help_table[] =
{
	{
		"ACTION",
		"<178>Command<078>: #action <178>{<078>search string<178>}<078> <178>{<078>commands<178>}<078> <178>{<078>priority<178>}<078>\n"
		"\n"
		"         Have the proxy search for a certain string of text from the program, if\n"
		"         the string is found it will execute the commands.  Variables %1 to %99\n"
		"         are substituted from the input string, and can be used in the command.\n"
		"\n"
		"         If the search string starts with ~, color codes must be matched, which\n"
		"         you can see by enabling: #config {convert meta} on.\n"
		"\n"
		"         The following Perl compatible regular expression options are available:\n"
		"\n"
		"       ^ force match of start of line.\n"
		"       $ force match of end of line.\n"
		"  %1-%99 lazy match of any text, available at %1-%99.\n"
		"      %0 should be avoided in triggers, and if left alone lists all matches.\n"
		"     { } embed a raw regular expression, available at %1-%99 + 1.\n"
		"         [ ] . + | ( ) ? * are treated as normal text unlessed used within\n"
		"         braces. Keep in mind that { } is replaced with ( ) automatically.\n"
		"\n"
		"         Of the following the (lazy) match is available at %1-%99 + 1\n"
		"\n"
		"      %w match zero to any number of letters.\n"
		"      %W match zero to any number of non letters.\n"
		"      %d match zero to any number of digits.\n"
		"      %D match zero to any number of non digits.\n"
		"      %s match zero to any number of spaces.\n"
		"      %S match zero to any number of non spaces.\n"
		"\n"
		"      %? match zero or one character.\n"
		"      %. match one character.\n"
		"      %+ match one to any number of characters.\n"
		"      %* match zero to any number of characters.\n"
		"\n"
		"      %i matching becomes case insensitive.\n"
		"      %I matching becomes case sensitive (default).\n"
		"\n"
		"         Actions can be triggered by the showme command.\n"
		"\n"
		"<178>Comment<078>: You can remove an action with the #unaction command.\n"
	},
	{
		"ALIAS",
		"<178>Command<078>: #alias <178>{<078>word<178>} {<078>commands<178>}<078>\n"
		"\n"
		"         Defines a word that actually means a different word. Useful for long\n"
		"         commands repeated often.  You can have multiple commands aliased to\n"
		"         a single word, if you enclose the alias in braces.\n"
		"\n"
		"         If the commands include variables %0-99, these are substituted as\n"
		"         part of the parsing.  %0 is set to all text after the word was input,\n"
		"         %1 is set to the first word following the aliased word, and %2 is the\n"
		"         second, and so on.\n"
		"\n"
		"         The word can include the %1 to %99 variables making an alias behave\n"
		"         like an action for complex input matching.\n"
		"\n"
		"<178>Example<078>: #alias gb get bread bag\n"
		"         Typing gb at the prompt would be sent as 'get bread bag'.\n"
		"\n"
		"<178>Example<078>: #alias ws {wake;stand}\n"
		"         Since ws aliases a multiple command, you must use the braces.\n"
		"\n"
		"<178>Example<078>: #alias heal cast 'heal' %1\n"
		"         Typing 'heal valgar' would be sent as 'cast 'heal' valgar'.\n"
		"\n"
		"<178>Comment<078>: See '#help action', for more information about triggers.\n"
		"\n"
		"<178>Comment<078>: You can remove an alias with the #unalias command.\n"
	},
	{
		"ALL",
		"<178>Command<078>: #all <178>{<078>string<178>}<078>\n"
		"\n"
		"         Sends a command to all active sessions.\n"
		"\n"
		"<178>Example<078>: #all quit\n"
		"         Sends 'quit' to all active sessions.\n"
	},
	{
		"BREAK",
		"<178>Command<078>: #break\n"
		"\n"
		"         The break command can be used inside the #FOREACH, #LOOP, #PARSE,\n"
		"         #WHILE and #SWITCH statements. When #BREAK is found the statement\n"
		"         being executed will be broken out of.\n"
		"\n"
		"<178>Example<078>: #while {1} {#math cnt $cnt + 1;#if {$cnt == 20} {#break}}\n"
	},
	{
		"CASE",
		"<178>Command<078>: #case <178>{<078>conditional<178>}<078> <178>{<078>arguments<178>}<078>\n"
		"\n"
		"         The case command must be used within the switch command. When the\n"
		"         conditional argument of the case command matches the conditional\n"
		"         argument of the switch command the body of the case is executed.\n"
	},
	{
		"CLASS",
		"<178>Command<078>: #class <178>{<078>name<178>}<078> <178>{<078>open<178>|<078>close<178>|<078>read filename<178>|<078>write filename<178>|<078>kill<178>}<078>\n"
		"\n"
		"         The {open} option will open a class, closing a previously opened\n"
		"         class. All triggers added afterwards are assigned to this class.\n"
		"         The {close} option will close the given class.\n"
		"         The {read} option will open the class, read, and close afterwards.\n"
		"         The {write} option will write all triggers of the given class to file.\n"
		"         The {kill} option will delete all triggers of the given class.\n"
		"\n"
		"         Keep in mind that the kill and read option are very fast allowing\n"
		"         them to be used to enable and disable classes.\n"
		"\n"
		"<178>Example<078>: #class extra kill;#class extra read extra.tin\n"
		"         Deletes all triggers of 'extra' class if any. Read 'extra.tin' file,\n"
		"         all triggers loaded will be assigned to the fresh new 'extra' class.\n"
	},

        {
                "COLORDEMO",
                "Greyscale\n"
                "<g00>g00 <g01>g01 <g02>g02 <g03>g03 <g04>g04 <g05>g05 <g06>g06 <g07>g07 <g08>g08 <g09>g09 <g10>g10 <g11>g11 <g12>g12 <g13>g13 <g14>g14 <g15>g15 <g16>g16 <g17>g17 <g18>g18 <g19>g19 <g20>g20 <g21>g21 <g22>g22 <g23>g23\n"
                "<G00>G00 <G01>G01 <G02>G02 <G03>G03 <G04>G04 <G05>G05 <G06>G06 <G07>G07 <G08>G08 <G09>G09 <G10>G10 <G11>G11 <G12>G12 <G13>G13 <G14>G14 <G15>G15 <G16>G16 <G17>G17 <G18>G18 <G19>G19 <G20>G20 <G21>G21 <G22>G22 <G23>G23<088>\n"
                "<fff>Character Color\n"
                "<aaa>aaa <aab>aab <aac>aac <aad>aad <aae>aae <aaf>aaf <abf>abf <abe>abe <abd>abd <abc>abc <abb>abb <acb>acb <acc>acc <acd>acd <ace>ace <acf>acf <adf>adf <ade>ade <add>add <adc>adc <adb>adb <aeb>aeb <aec>aec <aed>aed <aee>aee <aef>aef <aff>aff <afe>afe <afd>afd <afc>afc <afb>afb <afa>afa <aea>aea <ada>ada <aca>aca <aba>aba \n"
                "<baa>baa <bab>bab <bac>bac <bad>bad <bae>bae <baf>baf <bbf>bbf <bbe>bbe <bbd>bbd <bbc>bbc <bbb>bbb <bcb>bcb <bcc>bcc <bcd>bcd <bce>bce <bcf>bcf <bdf>bdf <bde>bde <bdd>bdd <bdc>bdc <bdb>bdb <beb>beb <bec>bec <bed>bed <bee>bee <bef>bef <bff>bff <bfe>bfe <bfd>bfd <bfc>bfc <bfb>bfb <bfa>bfa <bea>bea <bda>bda <bca>bca <bba>bba \n"
                "<caa>caa <cab>cab <cac>cac <cad>cad <cae>cae <caf>caf <cbf>cbf <cbe>cbe <cbd>cbd <cbc>cbc <cbb>cbb <ccb>ccb <ccc>ccc <ccd>ccd <cce>cce <ccf>ccf <cdf>cdf <cde>cde <cdd>cdd <cdc>cdc <cdb>cdb <ceb>ceb <cec>cec <ced>ced <cee>cee <cef>cef <cff>cff <cfe>cfe <cfd>cfd <cfc>cfc <cfb>cfb <cfa>cfa <cea>cea <cda>cda <cca>cca <cba>cba \n"
                "<daa>daa <dab>dab <dac>dac <dad>dad <dae>dae <daf>daf <dbf>dbf <dbe>dbe <dbd>dbd <dbc>dbc <dbb>dbb <dcb>dcb <dcc>dcc <dcd>dcd <dce>dce <dcf>dcf <ddf>ddf <dde>dde <ddd>ddd <ddc>ddc <ddb>ddb <deb>deb <dec>dec <ded>ded <dee>dee <def>def <dff>dff <dfe>dfe <dfd>dfd <dfc>dfc <dfb>dfb <dfa>dfa <dea>dea <dda>dda <dca>dca <dba>dba \n"
                "<eaa>eaa <eab>eab <eac>eac <ead>ead <eae>eae <eaf>eaf <ebf>ebf <ebe>ebe <ebd>ebd <ebc>ebc <ebb>ebb <ecb>ecb <ecc>ecc <ecd>ecd <ece>ece <ecf>ecf <edf>edf <ede>ede <edd>edd <edc>edc <edb>edb <eeb>eeb <eec>eec <eed>eed <eee>eee <eef>eef <eff>eff <efe>efe <efd>efd <efc>efc <efb>efb <efa>efa <eea>eea <eda>eda <eca>eca <eba>eba \n"
                "<faa>faa <fab>fab <fac>fac <fad>fad <fae>fae <faf>faf <fbf>fbf <fbe>fbe <fbd>fbd <fbc>fbc <fbb>fbb <fcb>fcb <fcc>fcc <fcd>fcd <fce>fce <fcf>fcf <fdf>fdf <fde>fde <fdd>fdd <fdc>fdc <fdb>fdb <feb>feb <fec>fec <fed>fed <fee>fee <fef>fef <fff>fff <ffe>ffe <ffd>ffd <ffc>ffc <ffb>ffb <ffa>ffa <fea>fea <fda>fda <fca>fca <fba>fba<088>\n"
                "<fff>Background Color\n"
                "<aaa><AAA>AAA <AAB>AAB <AAC>AAC <AAD>AAD <AAE>AAE <AAF>AAF <ABF>ABF <ABE>ABE <ABD>ABD <ABC>ABC <ABB>ABB <ACB>ACB <ACC>ACC <ACD>ACD <ACE>ACE <ACF>ACF <ADF>ADF <ADE>ADE <ADD>ADD <ADC>ADC <ADB>ADB <AEB>AEB <AEC>AEC <AED>AED <AEE>AEE <AEF>AEF <AFF>AFF <AFE>AFE <AFD>AFD <AFC>AFC <AFB>AFB <AFA>AFA <AEA>AEA <ADA>ADA <ACA>ACA <ABA>ABA \n"
                "<aaa><BAA>BAA <BAB>BAB <BAC>BAC <BAD>BAD <BAE>BAE <BAF>BAF <BBF>BBF <BBE>BBE <BBD>BBD <BBC>BBC <BBB>BBB <BCB>BCB <BCC>BCC <BCD>BCD <BCE>BCE <BCF>BCF <BDF>BDF <BDE>BDE <BDD>BDD <BDC>BDC <BDB>BDB <BEB>BEB <BEC>BEC <BED>BED <BEE>BEE <BEF>BEF <BFF>BFF <BFE>BFE <BFD>BFD <BFC>BFC <BFB>BFB <BFA>BFA <BEA>BEA <BDA>BDA <BCA>BCA <BBA>BBA \n"
                "<aaa><CAA>CAA <CAB>CAB <CAC>CAC <CAD>CAD <CAE>CAE <CAF>CAF <CBF>CBF <CBE>CBE <CBD>CBD <CBC>CBC <CBB>CBB <CCB>CCB <CCC>CCC <CCD>CCD <CCE>CCE <CCF>CCF <CDF>CDF <CDE>CDE <CDD>CDD <CDC>CDC <CDB>CDB <CEB>CEB <CEC>CEC <CED>CED <CEE>CEE <CEF>CEF <CFF>CFF <CFE>CFE <CFD>CFD <CFC>CFC <CFB>CFB <CFA>CFA <CEA>CEA <CDA>CDA <CCA>CCA <CBA>CBA \n"
                "<aaa><DAA>DAA <DAB>DAB <DAC>DAC <DAD>DAD <DAE>DAE <DAF>DAF <DBF>DBF <DBE>DBE <DBD>DBD <DBC>DBC <DBB>DBB <DCB>DCB <DCC>DCC <DCD>DCD <DCE>DCE <DCF>DCF <DDF>DDF <DDE>DDE <DDD>DDD <DDC>DDC <DDB>DDB <DEB>DEB <DEC>DEC <DED>DED <DEE>DEE <DEF>DEF <DFF>DFF <DFE>DFE <DFD>DFD <DFC>DFC <DFB>DFB <DFA>DFA <DEA>DEA <DDA>DDA <DCA>DCA <DBA>DBA \n"
                "<aaa><EAA>EAA <EAB>EAB <EAC>EAC <EAD>EAD <EAE>EAE <EAF>EAF <EBF>EBF <EBE>EBE <EBD>EBD <EBC>EBC <EBB>EBB <ECB>ECB <ECC>ECC <ECD>ECD <ECE>ECE <ECF>ECF <EDF>EDF <EDE>EDE <EDD>EDD <EDC>EDC <EDB>EDB <EEB>EEB <EEC>EEC <EED>EED <EEE>EEE <EEF>EEF <EFF>EFF <EFE>EFE <EFD>EFD <EFC>EFC <EFB>EFB <EFA>EFA <EEA>EEA <EDA>EDA <ECA>ECA <EBA>EBA \n"
                "<aaa><FAA>FAA <FAB>FAB <FAC>FAC <FAD>FAD <FAE>FAE <FAF>FAF <FBF>FBF <FBE>FBE <FBD>FBD <FBC>FBC <FBB>FBB <FCB>FCB <FCC>FCC <FCD>FCD <FCE>FCE <FCF>FCF <FDF>FDF <FDE>FDE <FDD>FDD <FDC>FDC <FDB>FDB <FEB>FEB <FEC>FEC <FED>FED <FEE>FEE <FEF>FEF <FFF>FFF <FFE>FFE <FFD>FFD <FFC>FFC <FFB>FFB <FFA>FFA <FEA>FEA <FDA>FDA <FCA>FCA <FBA>FBA<088> \n"
                "\n"
                "<178>Comment<078>: See '#help colors', for more information about using these color codes.\n"
        },

	{
		"COLORS",
		"<178>Syntax<078>:  <<888>xyz>  with x, y, z being parameters\n"
		"\n"
		"         Parameter 'x': VT100 code\n"
		"\n"
		"         0 - Reset all colors and codes to default\n"
		"         1 - Bold\n"
		"         2 - Dim\n"
		"         4 - Underscore\n"
		"         5 - Blink\n"
		"         7 - Reverse\n"
		"         8 - Skip (use previous code)\n"
		"\n"
		"         Parameter 'y':  Foreground color\n"
		"         Parameter 'z':  Background color\n"
		"\n"
		"         0 - Black                5 - Magenta\n"
		"         1 - Red                  6 - Cyan\n"
		"         2 - Green                7 - White\n"
		"         3 - Yellow               8 - Skip\n"
		"         4 - Blue                 9 - Default\n"
		"\n"
		"         For xterm 256 colors support use <<888>aaa> to <<888>fff> for RGB foreground\n"
		"         colors and <<888>AAA> to <<888>FFF> for RGB background colors. For the grayscale\n"
		"         foreground colors use <<888>g00> to <<888>g23>, for grayscale background colors\n"
		"         use <<888>G00> to <<888>G23>.\n"
		"\n"
		"         The tertiary colors are as follows:\n"
		"\n"
		"         <<888>acf> - Azure            <<888>afc> - Jade\n"
		"         <<888>caf> - Violet           <<888>cfa> - Lime\n"
		"         <<888>fac> - Pink             <<888>fca> - Orange\n"
		"\n"
		"<178>Example<078>: #showme <<888>acf>Azure    <<888>afc>Jade     <<888>caf>Violet\n"
		"<178>Example<078>: #showme <<888>cfa>Lime     <<888>fac>Pink     <<888>fca>Orange\n"
	},
	{
		"CONFIG",
		"<178>Command<078>: #config <178>{<078>option<178>}<078> <178>{<078>argument<178>}<078>\n"
		"\n"
		"         This allows you to configure various settings, the settings can be\n"
		"         written to file with the #write or #writesession command.\n"
		"\n"
		"         If you configure the global session (the one you see as you start up)\n"
		"         all sessions started will inherite these settings.\n"
		"\n"
		"         It's advised to make a configuration file to read on startup if you\n"
		"         do not like the default settings.\n"
		"\n"
		"         Config options which aren't listed by default:\n"
		"\n"
		"         #CONFIG {CONVERT META} {ON|OFF} Shows color codes and key bindings.\n"
		"         #CONFIG {COLOR PATCH}  {ON|OFF} Fixes color codes for some programs.\n"
		"         #CONFIG {LOG LEVEL}  {LOW|HIGH} LOW logs output before triggers.\n"
	},
	{
		"CONTINUE",
		"<178>Command<078>: #continue\n"
		"\n"
		"         The continue command can be used inside the #FOREACH, #LOOP, #PARSE,\n"
		"         #WHILE and #SWITCH commands. When #CONTINUE is found it'll skip\n"
		"         to the end of the command and proceed as normal, which may be to\n"
		"         reiterate the command.\n"
		"\n"
		"<178>Example<078>: #loop 1 10 cnt {#if {$cnt % 2 == 0} {#continue} {say $cnt}}\n"
	},
	{
		"CURSOR",
		"<178>Command<078>: #cursor <178>{<078>option<178>}<078> <178>{<078>argument<178>}<078>\n"
		"\n"
		"         Typing #cursor without an option will show all available cursor\n"
		"         options. The cursor command's primarly goal is adding customizable\n"
		"         input editing with macros.\n"
	},
	{
		"DEBUG",
		"<178>Command<078>: #debug <178>{<078>listname<178>}<078> <178>{<078>on<178>|<078>off<178>|<078>log<178>}<078>\n"
		"\n"
		"         Toggles a list on or off. With no argument it shows your current\n"
		"         settings, as well as the list names that you can debug.\n"
		"\n"
		"         If you for example set ACTIONS to ON you will get debug information\n"
		"         whenever an action is triggered.\n"
		"\n"
		"         #debug {listname} {log} will silently write debugging information to\n"
		"         the log file, you must be logging in order for this to work.\n"
		"\n"
		"         Not every list has debug support yet.\n"
	},
	{
		"DEFAULT",
		"<178>Command<078>: #default <178>{<078>commands<178>}<078>\n"
		"\n"
		"         The default command can only be used within the switch command. When\n"
		"         the conditional argument of non of the case commands matches the switch\n"
		"         command's conditional statement the default command is executed.\n"
	},
	{
		"DELAY",
		"<178>Command<078>: #delay <178>{<078>seconds<178>}<078> <178>{<078>command<178>}<078>\n"
		"<178>Command<078>: #delay <178>{<078>name<178>}<078> <178>{<078>command<178>}<078> <178>{<078>seconds<178>}<078> \n"
		"\n"
		"         Delay allows you to have the program wait the given amount of\n"
		"         second before executing the given command.\n"
		"\n"
		"         Floating point precision for miliseconds is possible.\n"
		"\n"
		"<178>Example<078>: #showme first;#delay {1} {#showme last}\n"
		"         This will print 'first', and 'last' around one second later.\n"
		"\n"
		"<178>Comment<078>: If you want to remove a delay with the #undelay command you can add\n"
		"         a name as the first argument, be aware this changes the syntax.\n"
	},
	{
		"ECHO",
		"<178>Command<078>: #echo <178>{<078>format<178>}<078> <178>{<078>argument1<178>} {<078>argument2<178>} {<078>etc<178>}<078>\n"
		"\n"
		"         Echo command displays text on the screen with formatting options. See\n"
		"         the help file for the format command for more informations.\n"
		"\n"
		"         The echo command does not trigger actions.\n"
		"\n"
		"         As with the #showme command you can split the {format} argument up into\n"
		"         two braced arguments, in which case the 2nd argument is the row number.\n"
		"\n"
		"<178>Example<078>: #echo {The current date is %t.} {%Y-%m-%d %H:%M:%S}\n"
		"         #echo {[%38s][%-38s]} {Hello World} {Hello World}\n"
		"         #echo {{this is %s on the top row} {-1}} {printed}\n"
	},
	{
		"ELSE",
		"<178>Command<078>: #else <178>{<078>commands<178>}<078>\n"
		"\n"
		"         The else statement should follow an #IF or #ELSEIF statement and is only\n"
		"         called if the proceeding #IF or #ELSEIF is false.\n"
		"\n"
		"<178>Example<078>: #if {1d2 == 1} {smile};#else {grin}\n"
	},
	{
		"ELSEIF",
		"<178>Command<078>: #elseif <178>{<078>conditional<178>}<078> <178>{<078>commands<178>}<078>\n"
		"\n"
		"         The elseif statement should follow an #IF or #ELSEIF statement and is only\n"
		"         called when the statement is true and the proceeding #IF and #ELSEIF statements\n"
		"         are false.\n"
		"\n"
		"<178>Example<078>: #if {1d3 == 1} {smirk};#elseif {1d2 == 1} {snicker}\n"
	},
	{
		"END",
		"<178>Command<078>: #end\n"
		"\n"
		"         Terminates the program and return to unix.  On most systems, ctrl-c\n"
		"         has the same result.\n"
	},
	{
		"ESCAPE CODES",
		"         You may use the escape character \\ for various special characters.\n"
		"\n"
		"         \\a   will beep the terminal.\n"
		"         \\c   will send a control character, \\ca for ctrl-a.\n"
		"         \\e   will start an escape sequence.\n"
		"         \\n   will send a line feed.\n"
		"         \\r   will send a carriage return.\n"
		"         \\t   will send a tab.\n"
		"         \\x   will print a hexadecimal value, \\xFF for example.\n"
		"         \\x7B will send the '{' character.\n"
		"         \\x7D will send the '}' character.\n"
		"\n"
		"         Ending a line with \\ will stop a line feed from being appended.\n"
		"         To escape arguments in an alias use %%0 %%1 %%2 etc.\n"
	},
	{
		"EVENT",
		"<178>Command<078>: #event <178>{<078>event type<178>}<078>\n"
		"\n"
		"         Events allow you to create triggers for predetermined client events.\n"
		"         Use #event without an argument to see a list of possible events with\n"
		"         a brief description. Use #event %* to see the current list of available\n"
		"         events you can use.\n"
		"\n"
		"         DATE                  %1 month - %3 day   %4 hour : %5 minute\n"
		"         DAY                   %3 day\n"
		"         HOUR                  %4 hour\n"
		"         MINUTE                %5 minute\n"
		"         MONTH                 %1 month\n"
		"         PROGRAM START         %0 client name %1 client version\n"
		"         PROGRAM TERMINATION\n"
		"         RECEIVED INPUT        %0 raw text\n"
		"         RECEIVED LINE         %0 raw text %1 plain text\n"
		"         RECEIVED OUTPUT       %0 raw text\n"
		"         RECEIVED PROMPT       %0 raw text %1 plain text\n"
		"         SCREEN RESIZE         %0 cols %1 rows\n"
		"         SECOND                %6 second\n"
		"         SEND OUTPUT           %0 raw text\n"
		"         SESSION ACTIVATED     %0 name\n"
		"         SESSION CONNECTED     %0 name %1 command %2 pid\n"
		"         SESSION DEACTIVATED   %0 name\n"
		"         SESSION DISCONNECTED  %0 name %1 command %2 pid\n"
		"         TIME                  %4 hour : %5 minute : %6 second\n"
		"         VARIABLE UPDATE <VAR>\n"
		"         WEEK                  %2 week\n"
		"         YEAR                  %0 year\n"
		"\n"
		"<178>Example<078>: #event {SESSION CONNECTED} {#read mychar.tin}\n"
		"\n"
		"<178>Comment<078>: You can remove an event with the #unevent command.\n"
	},
	{
		"FOREACH",
		"<178>Command<078>: #foreach <178>{<078>list<178>} {<078>variable<178>} {<078>commands<178>}<078>\n"
		"\n"
		"         For each item in the provided list the foreach statement will update\n"
		"         the given variable and execute the command part of the statement. List\n"
		"         elements must be separated by braces or semicolons.\n"
		"\n"
		"<178>Example<078>: #foreach {bob;tim;kim} {name} {tell $name Hello}\n"
		"<178>Example<078>: #foreach {{bob}{tim}{kim}} {name} {tell $name Hello}\n"
	},
	{
		"FORMAT",
		"<178>Command<078>: #format <178>{<078>variable<178>}<078> <178>{<078>format<178>}<078> <178>{<078>argument1<178>} {<078>argument2<178>} {<078>etc<178>}<078>\n"
		"\n"
		"         Allows you to store a string into a variable in the exact same way\n"
		"         C's sprintf works with a few enhancements and limitations such as\n"
		"         no integer operations and a maximum of 20 arguments. If you use format\n"
		"         inside an alias or action you must escape the %0-9 like: %+4s.\n"
		"\n"
		"<178>Example<078>: #format {test} {%+9s} {string}  pad string with up to 9 spaces\n"
		"         #format {test} {%-9s} {string}  post pad string with up to 9 spaces\n"
		"         #format {test} {%.8s} {string}  copy at most 8 characters\n"
		"         #format {test} {%a}   {number}  print corresponding ascii character\n"
		"         #format {test} {%c}   {string}  use a highlight color name\n"
		"         #format {test} {%d}   {number}  print a number with integer formatting\n"
		"         #format {test} {%g}   {number}  perform thousand grouping on {number}\n"
		"         #format {test} {%h}   {string}  turn text into a header line\n"
		"         #format {test} {%l}   {string}  lowercase text\n"
		"         #format {test} {%m}   {string}  perform mathematical calculation\n"
		"         #format {test} {%n}     {name}  capitalize the first letter\n"
		"         #format {test} {%p}   {string}  strip leading and trailing spaces\n"
		"         #format {test} {%r}   {string}  reverse text, hiya = ayih\n"
		"         #format {test} {%s}   {string}  print given string\n"
		"         #format {test} {%t}   {format}  display time with strftime format\n"
		"         #format {test} {%u}   {string}  uppercase text\n"
		"         #format {test} {%w}   {string}  wordwrap text\n"
		"         #format {test} {%A}     {char}  print corresponding ascii value\n"
		"         #format {cols} {%C}         {}  store the screen width in {cols}\n"
		"         #format {test} {%L}   {string}  store the string length in {test}\n"
		"         #format {rows} {%R}         {}  store the screen height in {rows}\n"
		"         #format {time} {%T}         {}  store the epoch time in {time}\n"
		"         #format {time} {%U}         {}  store the micro epoch time in {time}\n"
	},
	{
		"FUNCTION",
		"<178>Command<078>: #function <178>{<078>name<178>}<078> <178>{<078>operation<178>}<078>\n"
		"\n"
		"         Functions allow you to execute a script within a line of text, and\n"
		"         replace the function call with the line of text generated by the\n"
		"         function.\n"
		"\n"
		"         Be aware that each function should set the $result variable at the\n"
		"         end of the function, or call #return with the given result.\n"
		"\n"
		"         To use a function use the @ character before the function name.\n"
		"         The function arguments should be placed between braces behind the\n"
		"         function name with argument separated by semi-colons.\n"
		"\n"
		"         The function itself can use the provided arguments which are stored\n"
		"         in %1 to %9, with %0 holding all arguments.\n"
		"\n"
		"<178>Example<078>: #function {rnd} {#math {result} {1 d (%2 - %1 + 1) + %1 - 1}}\n"
		"         #showme A random number between 100 and 200: @rnd{100 200}\n"
		"\n"
		"<178>Example<078>: #function gettime {#format result %t %H:%M}\n"
		"         #showme The current time is @gettime{}\n"
		"\n"
		"<178>Comment<078>: You can remove a function with the #unfunction command.\n"
	},
	{
		"GAG",
		"<178>Command<078>: #gag <178>{<078>string<178>}<078>\n"
		"\n"
		"         Removes any line that contains the string.\n"
		"\n"
		"<178>Comment<078>: See '#help action', for more information about triggers.\n"
		"\n"
		"<178>Comment<078>: You can remove a gag with the #ungag command.\n"
	},

        {
                "GREETING",
                "<068>      #<068>##########################################################<068>#\n"
                "<068>      #<078>                                                          <068>#\n"
                "<068>      #<078>        C H R O M A T E R M   "CLIENT_VERSION"                    <068>#\n"
                "<068>      #<078>                                                          <068>#\n"
                "<068>      #<078>     Code by Peter Unold, Bill Reis, Igor van den Hoven.  <068>#\n"
                "<068>      #<078>        Produced by TunnelsUp.com     1992, 2013          <068>#\n"
                "<068>      #<068>##########################################################<068>#<088>\n"
        },

	{
		"GREETING",
		"<068>      #<068>##################################################################<068>#\n"
		"<068>      #<078>                                                                  <068>#\n"
		"<068>      #<078>                     D A Z Z L E + +   "CLIENT_VERSION"                   <068>#\n"
		"<068>      #<078>                                                                  <068>#\n"
		"<068>      #<078>         Code by Peter Unold, Bill Reis, Igor van den Hoven.      <068>#\n"
		"<068>      #<078>                                                                  <068>#\n"
		"<068>      #<078>                             1992, 2013                           <068>#\n"
		"<068>      #<068>##################################################################<068>#<088>\n"
	},

	{
		"HELP",
		"<178>Command<078>: #help <178>{<078>subject<178>}<078>\n"
		"\n"
		"         Without an argument #help will list all available help subjects.\n"
		"\n"
		"         Using #help %* will display all help entries.\n"
	},
	{
		"HIGHLIGHT",
		"<178>Command<078>: #highlight <178>{<078>string<178>}<078> <178>{<078>color names<178>}<078>\n"
		"\n"
		"         The highlight command is used to allow you to highlight strings of text\n"
		"         from the program.  Available ANSI color names are:\n"
		"\n"
		"         reset, light, faint, underscore, blink, reverse, dim,\n"
		"\n"
		"         black, red, green, yellow, blue, magenta, cyan, white,\n"
		"         b black, b red, b green, b yellow, b blue, b magenta, b cyan, b white\n"
		"\n"
		"         Available XTERM 256 color names are:\n"
		"\n"
		"         azure, ebony, jade, lime, orange, pink, silver, tan, violet,\n"
		"         light azure, light ebony, light jade, light lime, light orange,\n"
		"         light pink, light silver, light tan, light violet.\n"
		"\n"
		"         The %1-99 variables can be used as 'wildcards' that will match with any\n"
		"         text. They are useful for highlighting a complete line. The %0 variable\n"
		"         should never be used in highlights.\n"
		"\n"
		"         You may start the string to highlight with a ^ to only highlight text\n"
		"         if it begins the line.\n"
		"\n"
		"         Besides color names also <<888>abc> color codes can be used.\n"
		"\n"
		"<178>Example<078>: #high {Valgar} {reverse}\n"
		"         Prints every occurrence of 'Valgar' in reverse video.\n"
		"\n"
		"<178>Example<078>: #high {^You %1} {bold cyan}\n"
		"         Boldfaces any line that starts with 'You' in cyan.\n"
		"\n"
		"<178>Example<078>: #high {Bubba} {red underscore blink}\n"
		"         Highlights the name Bubba as blinking, red, underscored text\n"
		"\n"
		"<178>Comment<078>: See '#help action', for more information about triggers.\n"
		"\n"
		"<178>Comment<078>: See '#help substitute', for more advanced color substitution.\n"
		"\n"
		"<178>Comment<078>: This command only works with ANSI/VT100 terminals or emulators.\n"
		"\n"
		"<178>Comment<078>: You can remove a highlight with the #unhighlight command.\n"
	},
	{
		"IF",
		"<178>Command<078>: #if <178>{<078>conditional<178>}<078> <178>{<078>commands if true<178>}<078> <178>{<078>commands if false<178>}<078>\n"
		"\n"
		"         The 'if' command works similar to an 'if' statement in other languages,\n"
		"         and is strictly based on the way C handles its conditional statements.\n"
		"         When an 'if' command is encountered, the conditional statement is\n"
		"         evaluated, and if TRUE (any non-zero result) the commands are executed.\n"
		"\n"
		"         The 'if' statement is only evaluated if it is read, so you must nest\n"
		"         the 'if' statement inside another statement (most likely an 'action'\n"
		"         command). The conditional is evaluated exactly the same as in the\n"
		"         'math' command only instead of storing the result, the result is used\n"
		"         to determine whether to execute the commands.\n"
		"\n"
		"<178>Example<078>: #action {%0 gives you %1 gold coins.} {#if {%1>5000} {thank %0}}\n"
		"         If someone gives you more than 5000 coins, thank them.\n"
		"\n"
		"<178>Comment<078>: See '#help math', for more information.\n"
	},
	{
		"IGNORE",
		"<178>Command<078>: #ignore <178>{<078>listname<178>}<078> <178>{<078>on<178>|<078>off<178>}<078>\n"
		"\n"
		"         Toggles a list on or off. With no arguments it shows your current\n"
		"         settings, as well as the list names that you can ignore.\n"
		"\n"
		"         If you for example set ACTIONS to OFF actions will no longer trigger.\n"
		"         Not every list can be ignored.\n"
	},
	{
		"INFO",
		"<178>Command<078>: #info\n"
		"\n"
		"         Displays all the settings of every list available.\n"
		"\n"
		"Trivia:  Typing: #info cpu will show information about the cpu usage.\n"
	},
	{
		"KILL",
		"<178>Command<078>: #kill <178>{<078>list<178><178>} {<078>pattern<178>}<078>\n"
		"\n"
		"         Without an argument, the kill command clears all lists.  Useful if\n"
		"         you don't want to exit the program to reload your command files.\n"
		"\n"
		"         With one argument a specific list can be cleared.\n"
		"\n"
		"         With two arguments the triggers in the chosen list that match the\n"
		"         given pattern will be removed.\n"
		"\n"
		"<178>Example<078>: #kill alias %*test*\n"
	},
	{
		"LINE",
		"<178>Command<078>: #line <178>{<078>gag<178>|<078>log<178>|<078>logverbatim<178>}<078> <178>{<078>argument<178>}<078>\n"
		"\n"
		"         #line log {filename} {[text]}          Log the current or given line to\n"
		"                                                file.\n"
		"\n"
		"         #line logverbatim {filename} {[text]}  Like log with no substitutions.\n"
		"\n"
		"         #line gag                              Gag the next line.\n"
		"\n"
		"         #line ignore {argument}                Argument is executed without\n"
		"                                                any triggers being checked.\n"
		"\n"
		"         #line strip {argument}                 Strips the argument next\n"
		"                                                executes it a command.\n"
		"\n"
		"         #line substitute {options} {argument}  Substitutes the given options:\n"
		"                                                variables, functions, colors,\n"
		"                                                escapes, secure, eol, lnf, in\n"
		"                                                the given argument next executes\n"
		"                                                it as a command.\n"
		"\n"
		"         #line verbose {argument}               Argument is executed verbose.\n"
		"\n"
		"         When using #line log and logging in html format use \\c< \\c> \\c& \\c\" to\n"
		"         log a literal < > & and \".\n"
	},
	{
		"LIST",
		"<178>Command<078>: #list <178>{<078>variable<178>}<078> <178>{<078>option<178>}<078> <178>{<078>argument<178>}<078>\n"
		"\n"
		"         #list {var} {add} {item}               Add {item} to the list\n"
		"         #list {var} {clear}                    Empty the given list\n"
		"         #list {var} {create} {item}            Create a list using {item}\n"
		"         #list {var} {delete} {index} {number}  Delete the item at {index},\n"
		"                                                the {number} is optional.\n"
		"         #list {var} {insert} {index} {string}  Insert {string} at given index\n"
		"         #list {var} {find} {string} {variable} Return the found index\n"
		"         #list {var} {get} {index} {variable}   Copy an item to {variable}\n"
		"         #list {var} {set} {index} {string}     Change the item at {index}\n"
		"         #list {var} {size} {variable}          Copy list size to {variable}\n"
		"         #list {var} {sort} {string}            Insert item in alphabetic order\n"
		"         #list {var} {tokenize} {string}        Create a character list\n"
		"\n"
		"         The index should be between 1 and the list's length. You can also give\n"
		"         a negative value, in which case -1 equals the last item in the list, -2\n"
		"         the second last, etc.\n"
		"\n"
		"         When inserting an item a positive index will prepend the item at the\n"
		"         given index, while a negative index will append the item.\n"
		"\n"
		"         The add and create options allow using multiple items, as well\n"
		"         as semi-colon separated items.\n"
		"\n"
		"         A length of 0 is returned for an empty or non existant list.\n"
		"\n"
		"         You can directly access elements in a list variable using $var[1],\n"
		"         $var[2], $var[-1], etc.\n"
	},

	{
		"LOG",
		"<178>Command<078>: #log <178>{<078>append<178>|<078>overwrite<178>}<078> <178>{<078>filename<178>}<078>\n"
		"\n"
		"         Logs session to a file, you can set the data type to either plain,\n"
		"         raw, or html with the config command.\n"

	},

	{
		"LOOP",
		"<178>Command<078>: #loop <178>{<078><start><178>} {<078><finish><178>} {<078><variable><178>} {<078>commands<178>}<078>\n"
		"\n"
		"         Like a for statement, loop will loop from start to finish incrementing\n"
		"         or decrementing by 1 each time through.  The value of the loop counter\n"
		"         is stored in the provided variable, which you can use in the commands.\n"
		"\n"
		"<178>Example<078>: #loop 1 3 loop {get all $loop.corpse}\n"
		"         This equals 'get all 1.corpse;get all 2.corpse;get all 3.corpse'.\n"
		"\n"
		"<178>Example<078>: #loop 3 1 cnt {drop $cnt.key}\n"
		"         This equals 'drop 3.key;drop 2.key;drop 1.key'.\n"
	},
	{
		"MACRO",
		"<178>Command<078>: #macro <178>{<078>key sequence<178>}<078> <178>{<078>commands<178>}<078>\n"
		"\n"
		"         Macros allow you to make the program respond to function keys.\n"
		"\n"
		"         The key sequence send to the terminal when pressing a function key\n"
		"         differs for every OS and terminal. To find out what sequence is send\n"
		"         you can enable the CONVERT META config option.\n"
		"\n"
		"         Another option is pressing ctrl-v, which will enable CONVERT META for\n"
		"         the next key pressed.\n"
		"\n"
		"<178>Example<078>: #macro {(press ctrl-v)(press F1)} {#showme \\e[2J;#buffer lock}\n"
		"         Clear the screen and lock the window when you press F1, useful when the\n"
		"         boss is near.\n"
		"\n"
		"<178>Example<078>: #macro {\\eOM} {#cursor enter}\n"
		"         Makes the keypad's enter key work as an enter in keypad mode.\n"
		"\n"
		"<178>Comment<078>: Not all terminals properly initialize the keypad key sequences.\n"
		"         If this is the case you can still use the keypad, but instead of the\n"
		"         arrow keys use ctrl b, f, p, and n.\n"
		"\n"
		"<178>Comment<078>: You can remove a macro with the #unmacro command.\n"
	},

	{
		"MATH",
		"<178>Command<078>: #math <178>{<078>variable<178>}<078> <178>{<078>expression<178>}<078>\n"
		"\n"
		"         Performs math operations and stores the result in a variable.  The math\n"
		"         follows a C-like precedence, as follows, with the top of the list\n"
		"         having the highest priority.\n"
		"\n"
		"         Operators       Priority     Function\n"
		"         ------------------------------------------------\n"
		"         !               0            logical not\n"
		"         ~               0            bitwise not\n"
		"         *               1            integer multiply\n"
		"         /               1            integer divide\n"
		"         %               1            integer modulo\n"
		"         d               1            integer random dice roll\n"
		"         +               2            integer addition\n"
		"         -               2            integer subtraction\n"
		"         <<              3            bitwise shift\n"
		"         >>              3            bitwise shift\n"
		"         >               4            logical greater than\n"
		"         >=              4            logical greater than or equal\n"
		"         <               4            logical less than\n"
		"         <=              4            logical less than or equal\n"
		"         ==              5            logical equal (can use regex)\n"
		"         !=              5            logical not equal (can use regex)\n"
		"          &              6            bitwise and\n"
		"          ^              7            bitwise xor\n"
		"          |              8            bitwise or\n"
		"         &&              9            logical and\n"
		"         ^^             10            logical xor\n"
		"         ||             11            logical or\n"
		"\n"
		"         True is any non-zero number, and False is zero.  Parentheses () have\n"
		"         highest precedence, so inside the () is always evaluated first.\n"
		"         Strings must be enclosed in \" \" and use regex with == and !=,\n"
		"         in the case of <= and >= the alphabetic order is compared.\n"
		"\n"
		"<178>Example<078>: #math {heals} {$mana / 40}\n"
		"         Assuming there is a variable $mana, divides its value by 40 and stores\n"
		"         the result in $heals.\n"
		"\n"
		"<178>Example<078>: #action {^You receive %0 experience} {updatexp %0}\n"
		"         #alias updatexp {#math {xpneed} {$xpneed - %0}\n"
		"         Let's say you have a variable which stores xp needed for your next\n"
		"         level.  The above will modify that variable after every kill, showing\n"
		"         the amount still needed.\n"
		"\n"
		"<178>Example<078>: #action {%0 tells %1}\n"
		"           {#if {\"\%0\" == \"Bubba\" && $afk} {reply I'm away, my friend.}}\n"
		"         When you are away from keyboard, it will only reply to your friend.\n"
	},
	{
		"MESSAGE",
		"<178>Command<078>: #message <178>{<078>listname<178>}<078> <178>{<078>on<178>|<078>off<178>}<078>\n"
		"\n"
		"         This will show the message status of all your lists if typed without an\n"
		"         argument. If you set for example VARIABLES to OFF you will no longer be\n"
		"         spammed when correctly using the #VARIABLE and #UNVARIABLE commands.\n"
	},
	{
		"NOP",
		"<178>Command<078>: #nop <178>{<078>whatever<178>}<078>\n"
		"\n"
		"         Short for 'no operation', and is ignored by the client.  It is useful\n"
		"         for commenting in your coms file, any text after the nop and before a\n"
		"         semicolon or end of line is ignored. You shouldn't put braces { } in it\n"
		"         though, unless you close them properly.\n"
		"\n"
		"<178>Comment<078>: By using braces you can comment out multiple lines of code in a script\n"
		"         file.\n"
		"\n"
		"         For commenting out an entire trigger and especially large sections of\n"
		"         triggers you would want to use /* text */\n"
		"\n"
		"<178>Example<078>: #nop This is the start of my script file.\n"
	},
	{
		"READ",
		"<178>Command<078>: #read <178>{<078>filename<178>}<078>\n"
		"\n"
		"         Reads a commands file into memory.  The coms file is merged in with\n"
		"         the currently loaded commands.  Duplicate commands are overwritten.\n"
		"\n"
		"         If you uses braces, { and } you can use several lines for 1 commands.\n"
		"         This however means you must always match every { with a } for the read\n"
		"         command to work.\n"
		"\n"
		"         You can comment out triggers using /* text */\n"
	},
	{
		"REGEXP",
		"<178>Command<078>: #regexp <178>{<078>string<178>}<078> <178>{<078>expression<178>}<078> <178>{<078>true<178>}<078> <178>{<078>false<178>}<078>\n"
		"\n"
		"         Compares the string to the given regular expression.\n"
		"\n"
		"         Variables are stored in &1 to &99 with &0 holding the matched substring.\n"
		"\n"
		"       ^ force match of start of string.\n"
		"       $ force match of end of string.\n"
		"  %1-%99 lazy match of any text, available at &1-&99.\n"
		"      %0 should be avoided in triggers, and if left alone &0 lists all matches.\n"
		"     { } embed a raw regular expression, available at last &1-&99 + 1.\n"
		"         [ ] . + | ( ) ? * are treated as normal text unlessed used within\n"
		"         braces. Keep in mind that { } is replaced with ( ) automatically.\n"
		"\n"
		"         Of the following the lazy match is available at last &1-&99 + 1.\n"
		"\n"
		"      %w match zero to any number of letters.\n"
		"      %W match zero to any number of non letters.\n"
		"      %d match zero to any number of digits.\n"
		"      %D match zero to any number of non digits.\n"
		"      %s match zero to any number of spaces.\n"
		"      %S match zero to any number of non spaces.\n"
		"\n"
		"      %? match zero or one character.\n"
		"      %. match one character.\n"
		"      %+ match one to any number of characters.\n"
		"      %* match zero to any number of characters.\n"
		"\n"
		"      %i matching becomes case insensitive.\n"
		"      %I matching becomes case sensitive (default).\n"
		"\n"
		"<178>Example<078>: #regexp {bli bla blo} {bli {.*} blo} {#showme &1}\n"
	},
	{
		"REPLACE",
		"<178>Command<078>: #replace <178>{<078>variable<178>}<078> <178>{<078>oldtext<178>}<078> <178>{<078>newtext<178>}<078>\n"
		"\n"
		"         Searches the variable text replacing each occurance of 'oldtext' with\n"
		"         'newtext'.\n"
	},
	{
		"RETURN",
		"<178>Command<078>: #return <178>{<078>text<178>}<078>\n"
		"\n"
		"         This command can be used to break out of a command string being executed.\n"
		"         If used inside a #function you can use #return with an argument to both\n"
		"         break out of the function and set the result variable.\n"
	},
	{
		"RUN",
		"<178>Command<078>: #run <178>{<078>session name<178>}<078> <178>{<078>shell command<178>}<078>\n"
		"\n"
		"         The run command creates a session which runs the given command.\n"
		"\n"
		"         Sessions can be manipulated with the #all, #config, #end, #session,\n"
		"         #snoop, and #zap commands.\n"
		"\n"
		"<178>Example<078>: #run {somewhere} {ssh someone@somewhere.com}\n"
	},
	{
		"SCRIPT",
		"<178>Command<078>: #script <178>{<078>variable<178>}<178> {<078>shell command<178>}<078>\n"
		"\n"
		"         The script command works much like the system command except that it\n"
		"         treats the generated echos as commands if no variable is provided.\n"
		"\n"
		"         This is useful for running php, perl, ruby, and python scripts. You\n"
		"         can run these scrips either from file or from with #run if the\n"
		"         scripting language allows this.\n"
		"\n"
		"         If you provide a variable the output of the script is stored as a list.\n"
		"\n"
		"<178>Example<078>: #script {ruby -e 'print \"#showme hello world\"'}\n"
		"<178>Example<078>: #script {python -c 'print \"#showme hello world\"'}\n"
		"<178>Example<078>: #script {php -r 'echo \"#showme hello world\"'}\n"
		"<178>Example<078>: #script {path} {pwd};#showme The path is $path[1].\n"
	},

	{
		"SEND",
		"<178>Command<078>: #send <178>{<078>text<178>}<078>\n"
		"\n"
		"         Sends the text directly to the program, useful if you want to start with an\n"
		"         escape code or embed variables and functions.\n"
	},
	{
		"SESSION",
		"<178>Command<078>: #session <178>{<078>name<178>}<078> <178>{<078>command<178>}<078>\n"
		"\n"
		"         Without an argument #session shows the currently defined sessions.\n"
		"\n"
		"         If you provide a name the given session will be activated.\n"
		"\n"
		"         If you provide a name and command the command will be executed within the\n"
		"         given session.\n"
		"\n"
		"         The startup session is named 'gts'.\n"
	},
	{
		"SHOWME",
		"<178>Command<078>: #showme <178>{<078>string<178>}<078>\n"
		"\n"
		"         Display the string to the terminal, do not send to the program.  Useful\n"
		"         for status, warnings, etc.\n"
		"\n"
		"         Actions can be triggered by the showme command.\n"
		"\n"
		"<178>Example<078>: #action {You have new mail.} {#showme beep!\\a beep!\\a}\n"
	},
	{
		"SNOOP",
		"<178>Command<078>: #snoop <178>{<078>session name<178>}<078>\n"
		"\n"
		"         If there are multiple sessions active, this command allows you to see\n"
		"         what is going on the the sessions that are not currently active.  The\n"
		"         line of text from other sessions will be prefixed by the session's name.\n"
		"\n"
		"         You can toggle off snoop mode by executing #snoop a second time.\n"
	},
	{
		"STATEMENTS",
		"         The interpreter knows the following statements.\n"
		"<078>\n"
		"         #break\n"
		"         #case {value} {true}\n"
		"         #continue\n"
		"         #default {commands}\n"
		"         #else {commands}\n"
		"         #elseif {expression} {true}\n"
		"         #foreach {list} {variable} {commands}\n"
		"         #if {expression} {true}\n"
		"         #loop {min} {max} {variable} {commands}\n"
		"         #return {value}\n"
		"         #switch {expression} {commands}\n"
		"         #while {expression} {commands}\n"
	},
	{
		"SUBSTITUTE",
		"<178>Command<078>: #substitute <178>{<078>text<178>}<078> <178>{<078>new text<178>}<078>\n"
		"\n"
		"         Allows you to replace text from the program with different text.\n"
		"         This is helpful for complex coloring and making things more readable.\n"
		"         The %1-%99 variables can be used to capture text and use it as part of\n"
		"         the new output.\n"
		"\n"
		"         If only one argument is given, all active substitutions that match the\n"
		"         strings are displayed.  The '%*' char is valid in this instance.  See\n"
		"         '#help regex', for more information.\n"
		"\n"
		"         You can create a color substitution by starting the match with a tilda.\n"
		"\n"
		"         If no argument is given, all subs are displayed.\n"
		"\n"
		"<178>Example<078>: #sub {Zoe} {ZOE}\n"
		"         Any instance of Zoe will be replaced with ZOE.\n"
		"\n"
		"<178>Example<078>: #sub {~\\e[0;34m} {\\e[1;34m}\n"
		"         Replace generic dark blue color codes with bright blue ones.\n"
		"\n"
		"<178>Example<078>: #sub {%1massacres%2} {<<888>018>%1<<888>118>MASSACRES<<888>018>%2}\n"
		"         Replaces all occurrences of 'massacres' with 'MASSACRES' in red.\n"
		"\n"
		"<178>Comment<078>: See '#help colors', for more information.\n"
		"\n"
		"<178>Comment<078>: You can remove a substitution with the #unsubstitute command.\n"
	},
	{
		"SWITCH",
		"<178>Command<078>: #switch <178>{<078>conditional<178>}<078> <178>{<078>arguments<178>}<078>\n"
		"\n"
		"         The switch command works similar to the switch statement in other\n"
		"         languages. When the 'switch' command is encountered its body is parsed\n"
		"         and each 'case' command found will be compared to the conditional\n"
		"         argument of the switch and executed if there is a match.\n"
		"\n"
		"         If the 'default' command is found and no 'case' statement has been\n"
		"         matched the default command's argument is executed.\n"
		"\n"
		"<178>Example<078>: #switch {1d4} {#case 1 cackle;#case 2 smile;#default giggle}\n"
	},
	{
		"SYSTEM",
		"<178>Command<078>: #system <178>{<078>command<178>}<078>\n"
		"\n"
		"         Executes the command specified as a shell command.\n"
	},
	{
		"TICKER",
		"<178>Command<078>: #ticker <178>{<078>name<178>}<078> <178>{<078>commands<178>}<078> <178>{<078>interval in seconds<178>}<078>\n"
		"\n"
		"         Executes given command every # of seconds.\n"
		"\n"
		"<178>Comment<078>: Tickers don't work in the startup session.\n"
		"\n"
		"<178>Comment<078>: You can remove a ticker with the #unticker command.\n"
	},
	{
		"VARIABLE",
		"<178>Command<078>: #variable <178>{<078>variable name<178>}<078> <178>{<078>text to fill variable<178>}<078>\n"
		"\n"
		"         Variables differ from the %0-99 arguments in the fact that you can\n"
		"         specify a full word as a variable, and they stay in memory for the\n"
		"         full session unless they are changed.  They can be saved in the\n"
		"         coms file, and can be set to different values if you have two or\n"
		"         more sessions running at the same time.  Variables are global for\n"
		"         each session and can be accessed by adding a $ before the variable\n"
		"         name.\n"
		"\n"
		"<178>Example<078>: #alias {target} {#var target %0}\n"
		"         #alias {x}      {flame $target}\n"
		"\n"
		"         The name of a variable must exist of only letters and numbers in\n"
		"         order to be substituted.  If you do not meet these requirements do\n"
		"         not panic, simply encapsulate the variable in braces:\n"
		"\n"
		"<178>Example<078>: #variable {cool website} {http://tintin.sourceforge.net}\n"
		"         #showme I was on ${cool website} yesterday!.\n"
		"\n"
		"         Variables can be nested using brackets:\n"
		"\n"
		"<178>Example<078>: #var hp[self] 34;#var hp[target] 46\n"
		"\n"
		"         You can see the first nest of a variable using $variable[+1] and the\n"
		"         last nest using $variable[-1]. Using $variable[-2] will report the\n"
		"         second last variable, and so on. To show all indices use $variable[].\n"
		"         To show all values use $variable[%*] or a less generic regex.\n"
		"\n"
		"<178>Comment<078>: To see the internal index of a variable use &<variable name>\n"
		"\n"
		"<178>Comment<078>: A non existent nested variable will report itself as 0.\n"
		"\n"
		"<178>Comment<078>: You can remove a variable with the #unvariable command.\n"
	},
	{
		"WHILE",
		"<178>Command<078>: #while <178>{<078>conditional<178>} {<078>commands<178>}<078>\n"
		"\n"
		"         This command works similar to a 'while' statement in other languages.\n"
		"\n"
		"         When a 'while' command is encourated, the conditional is evaluated,\n"
		"         and if TRUE (any non-zero result) the commands are executed. The\n"
		"         'while' loop will be repeated indefinitely until the conditional is\n"
		"         FALSE or the #BREAK or #RETURN commands are found.\n"
		"\n"
		"         The 'while' statement is only evaluated if it is read, so you must\n"
		"         nest it inside a trigger, like an alias or action.\n"
		"\n"
		"         The conditional is evaluated exactly the same as in the 'math' command.\n"
                "\n"
                "<178>Example<078>: #math cnt 0;#while {$cnt < 20} {#math cnt $cnt + 1;say $cnt}\n"
		"\n"
		"<178>Comment<078>: See '#help math', for more information.\n"
	},
                                                                                                   
	{
		"WRITE",
		"<178>Command<078>: #write <178>{<078>filename<178>}<078>\n"
		"\n"
		"         Writes all current actions, aliases, subs, highlights, and variables\n"
		"         to a command file, specified by filename.\n"
	},
	{
		"ZAP",
		"<178>Command<078>: #zap {[session]}\n"
		"\n"
		"         Kill your current session.  If there is no current session, it will\n"
		"         cause the program to terminate. If you provide an argument it'll zap\n"
		"         the given session instead.\n"
	},
	{
		"",
		""
	}
};


DO_COMMAND(do_help)
{
	char left[BUFFER_SIZE], add[BUFFER_SIZE], buf[BUFFER_SIZE], *ptf, *pto;
	int cnt, found;

	arg = get_arg_in_braces(ses, arg, left, TRUE);

	if (*left == 0)
	{
		for (cnt = add[0] = 0 ; *help_table[cnt].name != 0 ; cnt++)
		{
			if ((int) strlen(add) + 19 > ses->cols)
			{
				display_puts2(ses, add);
				add[0] = 0;
			}
			cat_sprintf(add, "%19s", help_table[cnt].name);
		}
		display_puts(ses, add);
	}
	else
	{
		found = FALSE;

		for (cnt = 0 ; *help_table[cnt].name != 0 ; cnt++)
		{
			if (is_abbrev(left, help_table[cnt].name) || atoi(left) == cnt + 1 || match(ses, help_table[cnt].name, left, SUB_VAR|SUB_FUN))
			{
				substitute(ses, help_table[cnt].text, buf, SUB_COL);

				pto = buf;

				while (*pto)
				{
					ptf = strchr(pto, '\n');

					if (ptf == NULL)
					{
						break;
					}
					*ptf++ = 0;

					display_puts3(ses, pto);

					pto = ptf;
				}
//				display_puts2(ses, "");

				found = TRUE;

				if (is_abbrev(left, help_table[cnt].name))
				{
					break;
				}
			}
		}
		if (found == FALSE)
		{
			display_printf2(ses, "No help found for '%s'", left);
		}
	}
	return ses;
}
