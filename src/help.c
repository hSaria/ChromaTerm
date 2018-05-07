/* This program is protected under the GNU GPL (See COPYING) */

#include "defs.h"

struct help_type {
  char *name;
  char *text;
};

struct help_type help_table[] = {
    {"ALL", ""},
    {"COLORDEMO",
     "<g00>g00<g01>g01<g02>g02<g03>g03<g04>g04<g05>g05<g06>g06<g07>g07"
     "<g08>g08<g09>g09<g10>g10<g11>g11<g12>g12<g13>g13<g14>g14<g15>g15"
     "<g16>g16<g17>g17<g18>g18<g19>g19<g20>g20<g21>g21<g22>g22<g23>g23\n"
     "<G00>G00<G01>G01<G02>G02<G03>G03<G04>G04<G05>G05<G06>G06<G07>G07"
     "<G08>G08<G09>G09<G10>G10<G11>G11<G12>G12<G13>G13<G14>G14<G15>G15"
     "<G16>G16<G17>G17<G18>G18<G19>G19<G20>G20<G21>G21<G22>G22"
     "<G23>G23<088>\n"
     "<aaa>aaa<aab>aab<aac>aac<aad>aad<aae>aae<aaf>aaf<abf>abf<abe>abe"
     "<abd>abd<abc>abc<abb>abb<acb>acb<acc>acc<acd>acd<ace>ace<acf>acf"
     "<adf>adf<ade>ade<add>add<adc>adc<adb>adb<aeb>aeb<aec>aec<aed>aed"
     "<aee>aee<aef>aef<aff>aff<afe>afe<afd>afd<afc>afc<afb>afb<afa>afa"
     "<aea>aea<ada>ada<aca>aca<aba>aba\n"
     "<baa>baa<bab>bab<bac>bac<bad>bad<bae>bae<baf>baf<bbf>bbf<bbe>bbe"
     "<bbd>bbd<bbc>bbc<bbb>bbb<bcb>bcb<bcc>bcc<bcd>bcd<bce>bce<bcf>bcf"
     "<bdf>bdf<bde>bde<bdd>bdd<bdc>bdc<bdb>bdb<beb>beb<bec>bec<bed>bed"
     "<bee>bee<bef>bef<bff>bff<bfe>bfe<bfd>bfd<bfc>bfc<bfb>bfb<bfa>bfa"
     "<bea>bea<bda>bda<bca>bca<bba>bba\n"
     "<caa>caa<cab>cab<cac>cac<cad>cad<cae>cae<caf>caf<cbf>cbf<cbe>cbe"
     "<cbd>cbd<cbc>cbc<cbb>cbb<ccb>ccb<ccc>ccc<ccd>ccd<cce>cce<ccf>ccf"
     "<cdf>cdf<cde>cde<cdd>cdd<cdc>cdc<cdb>cdb<ceb>ceb<cec>cec<ced>ced"
     "<cee>cee<cef>cef<cff>cff<cfe>cfe<cfd>cfd<cfc>cfc<cfb>cfb<cfa>cfa"
     "<cea>cea<cda>cda<cca>cca<cba>cba\n"
     "<daa>daa<dab>dab<dac>dac<dad>dad<dae>dae<daf>daf<dbf>dbf<dbe>dbe"
     "<dbd>dbd<dbc>dbc<dbb>dbb<dcb>dcb<dcc>dcc<dcd>dcd<dce>dce<dcf>dcf"
     "<ddf>ddf<dde>dde<ddd>ddd<ddc>ddc<ddb>ddb<deb>deb<dec>dec<ded>ded"
     "<dee>dee<def>def<dff>dff<dfe>dfe<dfd>dfd<dfc>dfc<dfb>dfb<dfa>dfa"
     "<dea>dea<dda>dda<dca>dca<dba>dba\n"
     "<eaa>eaa<eab>eab<eac>eac<ead>ead<eae>eae<eaf>eaf<ebf>ebf<ebe>ebe"
     "<ebd>ebd<ebc>ebc<ebb>ebb<ecb>ecb<ecc>ecc<ecd>ecd<ece>ece<ecf>ecf"
     "<edf>edf<ede>ede<edd>edd<edc>edc<edb>edb<eeb>eeb<eec>eec<eed>eed"
     "<eee>eee<eef>eef<eff>eff<efe>efe<efd>efd<efc>efc<efb>efb<efa>efa"
     "<eea>eea<eda>eda<eca>eca<eba>eba\n"
     "<faa>faa<fab>fab<fac>fac<fad>fad<fae>fae<faf>faf<fbf>fbf<fbe>fbe"
     "<fbd>fbd<fbc>fbc<fbb>fbb<fcb>fcb<fcc>fcc<fcd>fcd<fce>fce<fcf>fcf"
     "<fdf>fdf<fde>fde<fdd>fdd<fdc>fdc<fdb>fdb<feb>feb<fec>fec<fed>fed"
     "<fee>fee<fef>fef<fff>fff<ffe>ffe<ffd>ffd<ffc>ffc<ffb>ffb<ffa>ffa"
     "<fea>fea<fda>fda<fca>fca<fba>fba<088>\n"
     "<aaa><AAA>AAA<AAB>AAB<AAC>AAC<AAD>AAD<AAE>AAE<AAF>AAF<ABF>ABF"
     "<ABE>ABE<ABD>ABD<ABC>ABC<ABB>ABB<ACB>ACB<ACC>ACC<ACD>ACD<ACE>ACE"
     "<ACF>ACF<ADF>ADF<ADE>ADE<ADD>ADD<ADC>ADC<ADB>ADB<AEB>AEB<AEC>AEC"
     "<AED>AED<AEE>AEE<AEF>AEF<AFF>AFF<AFE>AFE<AFD>AFD<AFC>AFC<AFB>AFB"
     "<AFA>AFA<AEA>AEA<ADA>ADA<ACA>ACA<ABA>ABA<088>\n"
     "<aaa><BAA>BAA<BAB>BAB<BAC>BAC<BAD>BAD<BAE>BAE<BAF>BAF<BBF>BBF"
     "<BBE>BBE<BBD>BBD<BBC>BBC<BBB>BBB<BCB>BCB<BCC>BCC<BCD>BCD<BCE>BCE"
     "<BCF>BCF<BDF>BDF<BDE>BDE<BDD>BDD<BDC>BDC<BDB>BDB<BEB>BEB<BEC>BEC"
     "<BED>BED<BEE>BEE<BEF>BEF<BFF>BFF<BFE>BFE<BFD>BFD<BFC>BFC<BFB>BFB"
     "<BFA>BFA<BEA>BEA<BDA>BDA<BCA>BCA<BBA>BBA<088>\n"
     "<aaa><CAA>CAA<CAB>CAB<CAC>CAC<CAD>CAD<CAE>CAE<CAF>CAF<CBF>CBF"
     "<CBE>CBE<CBD>CBD<CBC>CBC<CBB>CBB<CCB>CCB<CCC>CCC<CCD>CCD<CCE>CCE"
     "<CCF>CCF<CDF>CDF<CDE>CDE<CDD>CDD<CDC>CDC<CDB>CDB<CEB>CEB<CEC>CEC"
     "<CED>CED<CEE>CEE<CEF>CEF<CFF>CFF<CFE>CFE<CFD>CFD<CFC>CFC<CFB>CFB"
     "<CFA>CFA<CEA>CEA<CDA>CDA<CCA>CCA<CBA>CBA<088>\n"
     "<aaa><DAA>DAA<DAB>DAB<DAC>DAC<DAD>DAD<DAE>DAE<DAF>DAF<DBF>DBF"
     "<DBE>DBE<DBD>DBD<DBC>DBC<DBB>DBB<DCB>DCB<DCC>DCC<DCD>DCD<DCE>DCE"
     "<DCF>DCF<DDF>DDF<DDE>DDE<DDD>DDD<DDC>DDC<DDB>DDB<DEB>DEB<DEC>DEC"
     "<DED>DED<DEE>DEE<DEF>DEF<DFF>DFF<DFE>DFE<DFD>DFD<DFC>DFC<DFB>DFB"
     "<DFA>DFA<DEA>DEA<DDA>DDA<DCA>DCA<DBA>DBA<088>\n"
     "<aaa><EAA>EAA<EAB>EAB<EAC>EAC<EAD>EAD<EAE>EAE<EAF>EAF<EBF>EBF"
     "<EBE>EBE<EBD>EBD<EBC>EBC<EBB>EBB<ECB>ECB<ECC>ECC<ECD>ECD<ECE>ECE"
     "<ECF>ECF<EDF>EDF<EDE>EDE<EDD>EDD<EDC>EDC<EDB>EDB<EEB>EEB<EEC>EEC"
     "<EED>EED<EEE>EEE<EEF>EEF<EFF>EFF<EFE>EFE<EFD>EFD<EFC>EFC<EFB>EFB"
     "<EFA>EFA<EEA>EEA<EDA>EDA<ECA>ECA<EBA>EBA<088>\n"
     "<aaa><FAA>FAA<FAB>FAB<FAC>FAC<FAD>FAD<FAE>FAE<FAF>FAF<FBF>FBF"
     "<FBE>FBE<FBD>FBD<FBC>FBC<FBB>FBB<FCB>FCB<FCC>FCC<FCD>FCD<FCE>FCE"
     "<FCF>FCF<FDF>FDF<FDE>FDE<FDD>FDD<FDC>FDC<FDB>FDB<FEB>FEB<FEC>FEC"
     "<FED>FED<FEE>FEE<FEF>FEF<FFF>FFF<FFE>FFE<FFD>FFD<FFC>FFC<FFB>FFB"
     "<FFA>FFA<FEA>FEA<FDA>FDA<FCA>FCA<FBA>FBA<099>\n\n\n"},
    {"COLORS",
     "<078>Syntax<078>:  <<888>xyz>  with x, y, z being parameters\n"
     "\n"
     "         Parameter 'x': VT100 code\n"
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
     "         0 - Black                5 - Magenta\n"
     "         1 - Red                  6 - Cyan\n"
     "         2 - Green                7 - White\n"
     "         3 - Yellow               8 - Skip\n"
     "         4 - Blue                 9 - Default\n"
     "\n"
     "         For xterm 256 colors support use <<888>aaa> to "
     "<<888>fff> for RGB foreground\n"
     "         colors and <<888>AAA> to <<888>FFF> for RGB "
     "background colors. For the grayscale\n"
     "         foreground colors use <<888>g00> to <<888>g23>, for "
     "grayscale background colors\n"
     "         use <<888>G00> to <<888>G23>.\n"
     "\n"
     "         You can apply multiple actions (e.g. foreground and background "
     "colors).\n"
     "         For example: <<888>FCA><<888>baf>\n"
     "\n"
     "         Use <178>#help colordemo<078> to get a long (not complete) list "
     "of colors<099>\n\n\n"},
    {"CONFIG",
     "<028>Command<078>: #config <178>{<078>option<178>}<078> "
     "<178>{<078>argument<178>}<078>\n"
     "\n"
     "         This allows you to configure various settings, the settings can "
     "be\n"
     "         written to file with the <178>#write<078> command.\n"
     "\n"
     "         It's advised to make a configuration file to read on startup if "
     "you\n"
     "         do not like the default settings.\n"
     "\n"
     "         For debugging purposes, this hidden command can show color and "
     "codes:\n"
     "         <178>#CONFIG {CONVERT META} {ON|OFF}<099>\n\n\n"},
    {"EXIT", "<028>Command<078>: #exit\n"
             "\n"
             "         Terminates the program and any child process.\n"
             "         You can also use <178>#quit<078>\n\n\n"},
    {"HELP", "<028>Command<078>: #help <178>{<078>subject<178>}<078>\n"
             "\n"
             "         Without an argument #help will list all available help "
             "subjects.\n"
             "\n"
             "         Using <178>#help all<078> will display all help "
             "entries.<099>\n\n\n"},
    {"HIGHLIGHT",
     "<028>Command<078>: #highlight <178>{<078>regular expression<178>}<078> "
     "<178>{<078>color names<178>}<078> <178>{<078>priority<178>}<078>\n"
     "\n"
     "         The highlight command is used to allow you to highlight strings "
     "of text\n"
     "         from the program.  Available ANSI color names are:\n"
     "         reset, light, faint, underscore, blink, reverse, dim,\n"
     "         black, red, green, yellow, blue, magenta, cyan, white,\n"
     "         b black, b red, b green, b yellow, b blue, b magenta, b cyan, b "
     "white\n"
     "\n"
     "         Available XTERM 256 color names are:\n"
     "         azure, ebony, jade, lime, orange, pink, silver, tan, violet,\n"
     "         light azure, light ebony, light jade, light lime, light "
     "orange,\n"
     "         light pink, light silver, light tan, light violet.\n"
     "\n"
     "         Besides color names also <<888>abc> color codes can be used. \n"
     "         Use the <178>#help color<078> for more info on that.\n"
     "\n"
     "         You may start the string to highlight with a ^ to only "
     "highlight text\n"
     "         if it begins the line.\n"
     "\n"
     "         You may specify a priority to give a highlight precedence.\n"
     "         Default priority is 5. Lower is better.\n"
     "\n"
     "<078>Example<078>: #high {Valgar} {reverse}\n"
     "         Prints every occurrence of 'Valgar' in reverse video.\n"
     "<078>Example<078>: #high {^(P|p)assword} {bold yellow}\n"
     "         Boldfaces any line that starts with 'Password' or 'password' in "
     "yellow.\n"
     "<078>Example<078>: #high {Bubba} {red underscore blink}\n"
     "         Highlights the name Bubba as blinking, red, underscored text\n"
     "<078>Comment<078>: This command only works with ANSI/VT100 terminals or "
     "emulators.\n"
     "<078>Comment<078>: You can remove a highlight with the "
     "<178>#unhighlight<078> "
     "command.<099>\n\n\n"},
    {"READ",
     "<028>Command<078>: #read <178>{<078>filename<178>}<078>\n"
     "\n"
     "         Reads commands from a file.  The commands file is "
     "merged in with\n"
     "         the currently loaded commands (duplicate commands are "
     "overwritten).\n"
     "\n"
     "         When reading a file, the first character that is not a part of "
     "a comment\n"
     "         is considered the command character.\n"
     "\n"
     "         Commad char must me a punctuation: \n"
     "                 !@#$%%^&*-+=',.\"\\/:;?_`<>()[]{}|~,\n"
     "\n"
     "         You can comment out triggers using /* text */<099>\n\n\n"},
    {"RUN", "<028>Command<078>: #run <178>{<078>shell command<178>}<078>\n"
            "\n"
            "         The run command calls the shell command which it wraps "
            "around."
            "\n"
            "         You can only run one process per CT instance."
            "\n"
            "<078>Example<078>: #run {ssh someone@somewhere.com}<099>\n\n\n"},
    {"WRITE", "<028>Command<078>: #write <178>{<078>filename<178>}<078>\n"
              "\n"
              "         Writes all current configuration and highlight rules "
              "to the specified \n"
              "         filename.<099>\n\n\n"},
    {"", ""}};

DO_COMMAND(do_help) {
  char left[BUFFER_SIZE], add[BUFFER_SIZE], buf[BUFFER_SIZE], *ptf;
  int cnt;

  get_arg_in_braces(arg, left, GET_ONE);

  if (*left == 0) {
    display_header(" HELP TOPICS ");
    for (cnt = add[0] = 0; *help_table[cnt].name != 0; cnt++) {
      if ((int)strlen(add) + 19 > gts->cols) {
        display_printf(add);
        add[0] = 0;
      }
      cat_sprintf(add, "%19s", help_table[cnt].name);
    }
    display_printf(add);
    display_header("");
  } else {
    int found = FALSE;
    for (cnt = 0; *help_table[cnt].name != 0; cnt++) {
      if (is_abbrev(left, help_table[cnt].name) || is_abbrev(left, "all")) {
        found = TRUE;

        substitute(help_table[cnt].text, buf);
        char *pto = buf;

        while (*pto) {
          ptf = strchr(pto, '\n');

          if (ptf == NULL) {
            break;
          }
          *ptf++ = 0;

          display_printf(pto);

          pto = ptf;
        }
      }
    }

    if (!found) {
      display_printf("#HELP: No help found for topic '%s'", left);
    }
  }
}
