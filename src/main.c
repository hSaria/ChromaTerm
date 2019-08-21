/* This program is protected under the GNU GPL (See COPYING) */

#include "defs.h"

struct globalData gd;

int main(int argc, char **argv) {
  fd_set readfds;
  PCRE_ERR_P errP;
  int c, bytesRead, errN;

  /* Set up default CT state */
  sprintf(gd.configFile, "%s/%s", getenv("HOME"), ".chromatermrc");
  gd.highlights = (struct highlight **)calloc(8, sizeof(struct highlight *));
  gd.highlightsSize = 8; /* initial size is 8, but is doubled when needed */
  gd.collidingActions = FALSE;

  /* Look ahead for the end of a color without a started before it. Also, look
   * for character-movement codes; those need to be skipped, too */
  PCRE_COMPILE(colorEndLookAhead, "^(?:(?!\\e\\[[1-9][0-9;]*m).)*?\\e\\[0m",
               &errN, &errP);
  PCRE_COMPILE(charMovement, "\\e\\[[0-9]*[A-D]", &errN, &errP);

  /* Parse the arguments (h is in there to prevent errors from '-h') */
  while ((c = getopt(argc, argv, "a c: d r h")) != -1) {
    switch (tolower(c)) {
    case 'a':
      gd.collidingActions = TRUE; /* Allow colliding actions */
      break;
    case 'c':
      strncpy(gd.configFile, optarg, BUFFER_SIZE);
      break;
    case 'd':
      colorDemo(); /* Print the available xterm256 colors */
      exitWithSignal(EXIT_SUCCESS);
    case 'r':
      system("pkill -SIGUSR1 '^ct$'"); /* Send SIGUSR1 to all CT instances */
      exitWithSignal(EXIT_SUCCESS);
    default:
      printf("ChromaTerm-- v%s\n", VERSION);
      printf("Usage: %s [-adr] [-c file]\n", argv[0]);
      printf("%7s %-5s Allow colliding actions\n", "-a", "");
      printf("%7s %-5s Override configuration file\n", "-c", "file");
      printf("%7s %-5s Demo the available custom color-codes\n", "-d", "");
      printf("%7s %-5s Reload the config file for all CT instances\n", "-r",
             "");

      exitWithSignal(2);
    }
  }

  /* Read configuration */
  l_readConfig(gd.configFile);

  signal(SIGINT, SIG_IGN);        /* Ignore interrupt */
  signal(SIGUSR1, reloadHandler); /* Config reloader */

  /* fd_set used for checking if there's more input */
  FD_ZERO(&readfds); /* Initialize */
  FD_SET(STDIN_FILENO, &readfds);

  /* MAIN LOGIC OF THE PROGRAM STARTS HERE */
  while ((bytesRead = (int)read(STDIN_FILENO, &gd.inputBuf[gd.inputBufLen],
                                INPUT_MAX - gd.inputBufLen - 1)) > 0) {
    /* Mandatoy wait before assuming no more output on the current line */
    struct timeval wait = {0, WAIT_FOR_NEW_LINE};

    gd.inputBufLen += bytesRead;

    /* Block for a small amount to see if there's more to read. If something
     * came up, stop waiting and start processing. */
    int rv = select(STDIN_FILENO + 1, &readfds, NULL, NULL, &wait);

    if (rv > 0) { /* More data came up while waiting */
      /* Failsafe: if the buffer is full, process all of pending output.
       * Otherwise, process until the line that doesn't end with \n. */
      processInput(INPUT_MAX - gd.inputBufLen <= 1 ? FALSE : TRUE);
    } else if (rv == 0) {  /* timed-out while waiting for FD (no more output) */
      processInput(FALSE); /* Process all that's left */
    } else if (rv < 0) {   /* error */
      perror("select returned < 0");
      exitWithSignal(EXIT_FAILURE);
    }
  }

  processInput(FALSE); /* Process anything that may be left in the buffer */

  exitWithSignal(EXIT_SUCCESS);
  return 0; /* Literally useless, but gotta make a warning shut up */
}

void colorDemo(void) {
  char buf[BUFFER_SIZE];

  substitute(
      "<g00> g00<g01> g01<g02> g02<g03> g03<g04> g04<g05> g05<g06> g06<g07> "
      "g07<g08> g08<g09> g09<g10> g10<g11> g11<g12> g12<g13> g13<g14> g14<g15> "
      "g15<g16> g16<g17> g17<g18> g18<g19> g19<g20> g20<g21> g21<g22> g22<g23> "
      "g23\n<G00> G00<G01> G01<G02> G02<G03> G03<G04> G04<G05> G05<G06> "
      "G06<G07> G07<G08> G08<G09> G09<G10> G10<G11> G11<G12> G12<G13> G13<G14> "
      "G14<G15> G15<G16> G16<G17> G17<G18> G18<G19> G19<G20> G20<G21> G21<G22> "
      "G22<G23> G23<088> \n<aaa> aaa<aab> aab<aac> aac<aad> aad<aae> aae<aaf> "
      "aaf<abf> abf<abe> abe<abd> abd<abc> abc<abb> abb<acb> acb<acc> acc<acd> "
      "acd<ace> ace<acf> acf<adf> adf<ade> ade<add> add<adc> adc<adb> adb<aeb> "
      "aeb<aec> aec<aed> aed<aee> aee<aef> aef<aff> aff<afe> afe<afd> afd<afc> "
      "afc<afb> afb<afa> afa<aea> aea<ada> ada<aca> aca<aba> aba\n<baa> "
      "baa<bab> bab<bac> bac<bad> bad<bae> bae<baf> baf<bbf> bbf<bbe> bbe<bbd> "
      "bbd<bbc> bbc<bbb> bbb<bcb> bcb<bcc> bcc<bcd> bcd<bce> bce<bcf> bcf<bdf> "
      "bdf<bde> bde<bdd> bdd<bdc> bdc<bdb> bdb<beb> beb<bec> bec<bed> bed<bee> "
      "bee<bef> bef<bff> bff<bfe> bfe<bfd> bfd<bfc> bfc<bfb> bfb<bfa> bfa<bea> "
      "bea<bda> bda<bca> bca<bba> bba\n<caa> caa<cab> cab<cac> cac<cad> "
      "cad<cae> cae<caf> caf<cbf> cbf<cbe> cbe<cbd> cbd<cbc> cbc<cbb> cbb<ccb> "
      "ccb<ccc> ccc<ccd> ccd<cce> cce<ccf> ccf<cdf> cdf<cde> cde<cdd> cdd<cdc> "
      "cdc<cdb> cdb<ceb> ceb<cec> cec<ced> ced<cee> cee<cef> cef<cff> cff<cfe> "
      "cfe<cfd> cfd<cfc> cfc<cfb> cfb<cfa> cfa<cea> cea<cda> cda<cca> cca<cba> "
      "cba\n<daa> daa<dab> dab<dac> dac<dad> dad<dae> dae<daf> daf<dbf> "
      "dbf<dbe> dbe<dbd> dbd<dbc> dbc<dbb> dbb<dcb> dcb<dcc> dcc<dcd> dcd<dce> "
      "dce<dcf> dcf<ddf> ddf<dde> dde<ddd> ddd<ddc> ddc<ddb> ddb<deb> deb<dec> "
      "dec<ded> ded<dee> dee<def> def<dff> dff<dfe> dfe<dfd> dfd<dfc> dfc<dfb> "
      "dfb<dfa> dfa<dea> dea<dda> dda<dca> dca<dba> dba\n<eaa> eaa<eab> "
      "eab<eac> eac<ead> ead<eae> eae<eaf> eaf<ebf> ebf<ebe> ebe<ebd> ebd<ebc> "
      "ebc<ebb> ebb<ecb> ecb<ecc> ecc<ecd> ecd<ece> ece<ecf> ecf<edf> edf<ede> "
      "ede<edd> edd<edc> edc<edb> edb<eeb> eeb<eec> eec<eed> eed<eee> eee<eef> "
      "eef<eff> eff<efe> efe<efd> efd<efc> efc<efb> efb<efa> efa<eea> eea<eda> "
      "eda<eca> eca<eba> eba\n<faa> faa<fab> fab<fac> fac<fad> fad<fae> "
      "fae<faf> faf<fbf> fbf<fbe> fbe<fbd> fbd<fbc> fbc<fbb> fbb<fcb> fcb<fcc> "
      "fcc<fcd> fcd<fce> fce<fcf> fcf<fdf> fdf<fde> fde<fdd> fdd<fdc> fdc<fdb> "
      "fdb<feb> feb<fec> fec<fed> fed<fee> fee<fef> fef<fff> fff<ffe> ffe<ffd> "
      "ffd<ffc> ffc<ffb> ffb<ffa> ffa<fea> fea<fda> fda<fca> fca<fba> fba<088> "
      "\n<aaa><AAA> AAA<AAB> AAB<AAC> AAC<AAD> AAD<AAE> AAE<AAF> AAF<ABF> "
      "ABF<ABE> ABE<ABD> ABD<ABC> ABC<ABB> ABB<ACB> ACB<ACC> ACC<ACD> ACD<ACE> "
      "ACE<ACF> ACF<ADF> ADF<ADE> ADE<ADD> ADD<ADC> ADC<ADB> ADB<AEB> AEB<AEC> "
      "AEC<AED> AED<AEE> AEE<AEF> AEF<AFF> AFF<AFE> AFE<AFD> AFD<AFC> AFC<AFB> "
      "AFB<AFA> AFA<AEA> AEA<ADA> ADA<ACA> ACA<ABA> ABA<088> \n<aaa><BAA> "
      "BAA<BAB> BAB<BAC> BAC<BAD> BAD<BAE> BAE<BAF> BAF<BBF> BBF<BBE> BBE<BBD> "
      "BBD<BBC> BBC<BBB> BBB<BCB> BCB<BCC> BCC<BCD> BCD<BCE> BCE<BCF> BCF<BDF> "
      "BDF<BDE> BDE<BDD> BDD<BDC> BDC<BDB> BDB<BEB> BEB<BEC> BEC<BED> BED<BEE> "
      "BEE<BEF> BEF<BFF> BFF<BFE> BFE<BFD> BFD<BFC> BFC<BFB> BFB<BFA> BFA<BEA> "
      "BEA<BDA> BDA<BCA> BCA<BBA> BBA<088> \n<aaa><CAA> CAA<CAB> CAB<CAC> "
      "CAC<CAD> CAD<CAE> CAE<CAF> CAF<CBF> CBF<CBE> CBE<CBD> CBD<CBC> CBC<CBB> "
      "CBB<CCB> CCB<CCC> CCC<CCD> CCD<CCE> CCE<CCF> CCF<CDF> CDF<CDE> CDE<CDD> "
      "CDD<CDC> CDC<CDB> CDB<CEB> CEB<CEC> CEC<CED> CED<CEE> CEE<CEF> CEF<CFF> "
      "CFF<CFE> CFE<CFD> CFD<CFC> CFC<CFB> CFB<CFA> CFA<CEA> CEA<CDA> CDA<CCA> "
      "CCA<CBA> CBA<088> \n<aaa><DAA> DAA<DAB> DAB<DAC> DAC<DAD> DAD<DAE> "
      "DAE<DAF> DAF<DBF> DBF<DBE> DBE<DBD> DBD<DBC> DBC<DBB> DBB<DCB> DCB<DCC> "
      "DCC<DCD> DCD<DCE> DCE<DCF> DCF<DDF> DDF<DDE> DDE<DDD> DDD<DDC> DDC<DDB> "
      "DDB<DEB> DEB<DEC> DEC<DED> DED<DEE> DEE<DEF> DEF<DFF> DFF<DFE> DFE<DFD> "
      "DFD<DFC> DFC<DFB> DFB<DFA> DFA<DEA> DEA<DDA> DDA<DCA> DCA<DBA> DBA<088> "
      "\n<aaa><EAA> EAA<EAB> EAB<EAC> EAC<EAD> EAD<EAE> EAE<EAF> EAF<EBF> "
      "EBF<EBE> EBE<EBD> EBD<EBC> EBC<EBB> EBB<ECB> ECB<ECC> ECC<ECD> ECD<ECE> "
      "ECE<ECF> ECF<EDF> EDF<EDE> EDE<EDD> EDD<EDC> EDC<EDB> EDB<EEB> EEB<EEC> "
      "EEC<EED> EED<EEE> EEE<EEF> EEF<EFF> EFF<EFE> EFE<EFD> EFD<EFC> EFC<EFB> "
      "EFB<EFA> EFA<EEA> EEA<EDA> EDA<ECA> ECA<EBA> EBA<088> \n<aaa><FAA> "
      "FAA<FAB> FAB<FAC> FAC<FAD> FAD<FAE> FAE<FAF> FAF<FBF> FBF<FBE> FBE<FBD> "
      "FBD<FBC> FBC<FBB> FBB<FCB> FCB<FCC> FCC<FCD> FCD<FCE> FCE<FCF> FCF<FDF> "
      "FDF<FDE> FDE<FDD> FDD<FDC> FDC<FDB> FDB<FEB> FEB<FEC> FEC<FED> FED<FEE> "
      "FEE<FEF> FEF<FFF> FFF<FFE> FFE<FFD> FFD<FFC> FFC<FFB> FFB<FFA> FFA<FEA> "
      "FEA<FDA> FDA<FCA> FCA<FBA> FBA<088>",
      buf);
  fprintf(stderr, "%s\n", buf);
}

void exitWithSignal(int exitSignal) {
  clearHighlights();            /* Delete all highlight rules */
  free(gd.highlights);          /* Free highlights */
  PCRE_FREE(colorEndLookAhead); /* Free the RegEx */
  PCRE_FREE(charMovement);      /* Free the RegEx */

  exit(exitSignal);
}

void reloadHandler(int signum) {
  if (signum == SIGUSR1) {
    clearHighlights();         /* Delete all highlight rules */
    l_readConfig(gd.configFile); /* Reload configuration file */
  }
}
