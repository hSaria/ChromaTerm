#!/usr/bin/env python3
"""Miscellaneous functions unrelated to primary functions of ChromaTerm."""

import os

DEFAULT_CONFIG = r"""rules:
- description: Password
  regex: (?i)password
  color: f#ff0000

- description: IPv4
  regex: (?i)((?<=\W)|^)(?<!\.)((25[0-5]|(2[0-4]|[0-1]?\d)?\d)\.){3}(25[0-5]|(2[0-4]|[0-1]?\d)?\d)(/\d+)?(?!\.)((?=\W)|$)
  color: f#00ffff

- description: IPv6
  regex: (?i)((?<=\W)|^)(?<!:)(::(ffff(:0{1,4})?:)?((25[0-5]|(2[0-4]|[0-1]?\d)?\d)\.){3}(25[0-5]|(2[0-4]|[0-1]?\d)?\d)|((?P<h>[\da-f]{1,4}):){1,4}:((25[0-5]|(2[0-4]|[0-1]?\d)?\d)\.){3}(25[0-5]|(2[0-4]|[0-1]?\d)?\d)|(([0-9a-f]):){7}([0-9a-f])|([0-9a-f]):((:([0-9a-f])){1,6})|(([0-9a-f]):){1,2}(:([0-9a-f])){1,5}|(([0-9a-f]):){1,3}(:([0-9a-f])){1,4}|(([0-9a-f]):){1,4}(:([0-9a-f])){1,3}|(([0-9a-f]):){1,5}(:([0-9a-f])){1,2}|(([0-9a-f]):){1,6}:([0-9a-f])|:((:([0-9a-f])){1,7}|:)|(([0-9a-f]):){1,7}:)(%[\da-z]+)?(/\d*)?(?!:)((?=\W)|$)
  color: f#ff00ff

- description: MAC
  regex: (?i)((?<=\W)|^)((?<!:)([0-9a-f]{1,2}:){5}[0-9a-f]{1,2}(?!:)|(?<!\.)([0-9a-f]{4}\.){2}[0-9a-f]{4}(?!\.))((?=\W)|$)
  color: f#5f61ad

- description: Date in (YYYY-MM-DD|MMM (YYYY|DD)|DD MMM (YYYY)?)[\WT_]hh:mm:ss.SSSSSS format (date, sec, msec, and nsec optional)
  regex: (?i)((?<=\W)|^)(?<!:)((\d{2,4}\-((0)?[1-9]|1[0-2])\-(3[0-1]|([1-2]\d)|(0)?[1-9])|(?P<MMM>jan|feb|mar|apr|may|jun|jul|aug|sep|oct|nov|dec)\s(\d{4}|\s\d|(3[0-1]|([0-2]\d)))|(\d|(3[0-1]|([0-2]\d)))\s(?P=MMM)(\s\d{4})?)[\WT_])?((2[0-3])|[0-1]\d):[0-5]\d(:[0-5]\d)?((\.|,)\d{3,6})?(?!:)((?=\W)|$)
  color: b#af5f00

- description: Juniper interfaces
  regex: (?i)((?<=\W)|^)(((fe|ge|xe|et|gr|ip|lt|lsq|mt|sp|vcp)\-\d*/\d*/\d*)|(((b)?me|em|fab|fxp|fti|lo|pp(d|e)?|st|swfab)[0-2]|dsc|gre|ipip|irb|jsrv|lsi|mtun|pimd|pime|tap|vlan|vme|vtep)|((ae|reth)\d*))(\.\d*)?((?=\W)|$)
  color: f#f07a7c

- description: Cisco interfaces
  regex: (?i)((?<=\W)|^)(((Hu(ndredGigabit)?|Fo(rtyGigabit)?|Te(nGigabit)?|Gi(gabit)?|Fa(st)?)(Ethernet)?)|Eth|Se(rial)?|Lo(opback)?|Tu(nnel)?|VL(AN)?|Po(rt-channel)?|Vi(rtual\-(Template|Access))?|Mu(ltilink)?|Di(aler)?|(B|N)VI)((\d*/){0,2}\d*)(\.\d*)?((?=\W)|$)
  color: f#f07a7c

- description: Generics - Bad
  regex: (?i)((?<=\W)|^)(abnormal(ly)?|down|los(t|s|ing)|err(or(s)?)?|(den(y|ies|ied)?)|reject(ing|ed)?|drop(ped|s)?|(err\-)?disable(d)?|(time(d)?(\-)?out)|fail(s|ed|iure)?|disconnect(ed)?|unreachable|invalid|bad|notconnect|unusable|block(ing|ed)?|blk|inaccessible|wrong|collision(s)?|unsynchronized|mismatch|runts|CRC)((?=\W)|$)
  color: f#ff0000

- description: Generics - Ambigious bad
  regex: (?i)((?<=\W)|^)(no(t|pe)?|exit(ed)?|reset((t)?ing)?|discard(ed|ing)?|filter(ed)?|stop(p(ed|ing))?|never|can((')?t|not))((?=\W)|$)
  color: f#865e12

- description: Generics - Not too bad
  regex: (?i)((?<=\W)|^)(warning(s)?)((?=\W)|$)
  color: f#ffff00

- description: Generics - Ambigious good
  regex: (?i)((?<=\W)|^)(ye(s|a(h)?|p)?|started|running|can)((?=\W)|$)
  color: f#085e0b

- description: Generics - Good
  regex: (?i)((?<=\W)|^)(up|ok(ay)?|permit(ed|s)?|accept(s|ed)?|enable(d)?|online|succe((ss(ful|fully)?)|ed(ed)?)?|connect(ed)?|reachable|valid|forwarding|synchronized)((?=\W)|$)} {green} {1000}
  color: f#00ff00

- description: Half-duplex
  regex: (?i)((?<=\W)|^)(half(\-)?duplex)((?=\W)|$)
  color: f#ff0000

- description: Cisco Syslog facilities - Emergency to error
  regex: ((?<=\W)|^)(%\w*\-[0-3]\-\w*)((?=\W)|$)
  color: f#ff0000

- description: Cisco Syslog facilities - Warning to notice
  regex: ((?<=\W)|^)(%\w*\-[4-5]\-\w*)((?=\W)|$)
  color: f#ffff00

- description: Cisco Syslog facilities - Info to debug
  regex: ((?<=\W)|^)(%\w*\-[6-7]\-\w*)((?=\W)|$)
  color: f#65d7fd

- description: Spanning tree - Problematic states
  regex: ((?<=\W)|^)(BKN|(LOOP|ROOT|TYPE|PVID)_Inc)((?=\W)|$)
  color: f#ff0000

- description: Spanning tree - Forwarding states
  regex: ((?<=\W)|^)(FWD|Root|Desg)((?=\W)|$)
  color: f#00ff00

- description: OSPF - Transitional states
  regex: ((?<=\W)|^)(ATTEMPT|INIT|EXCHANGE|LOADING)((?=\W)|$)
  color: f#ffff00

- description: OSPF - Acceptable states
  regex: ((?<=\W)|^)(2WAY|FULL)((?=\W)|$)
  color: f#00ff00

- description: BGP - Transitional states
  regex: ((?<=\W)|^)(Idle|Connect|Active|OpenSent|OpenConfirm)((?=\W)|$)
  color: f#ffff00
"""


def write_default_config(directory=os.getenv('HOME'), file='.chromaterm.yml'):
    """Write the default configuration file if it doesn't exist."""
    if not directory or not file:
        return False

    location = os.path.join(directory, file)

    if os.access(location, os.F_OK):  # Already exists
        return False

    if not os.access(directory, os.W_OK):  # No write permission in directory
        return False

    with open(location, 'w') as file:
        file.write(DEFAULT_CONFIG)

    return True
