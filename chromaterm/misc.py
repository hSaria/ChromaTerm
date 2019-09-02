#!/usr/bin/env python3
"""Miscellaneous functions unrelated to primary functions of ChromaTerm."""

import os

DEFAULT_CONFIG = r"""rules:
- description: IPv4
  regex: \b(?<!\.)((25[0-5]|(2[0-4]|[0-1]?\d)?\d)\.){3}(25[0-5]|(2[0-4]|[0-1]?\d)?\d)(/\d+)?(?!\.)\b
  color: f#00ffff

- description: IPv6 (boundaries don't work here as they can be in the start or end of the match, so using lookaheads and lookbehinds instead)
  regex: (?i)((?<=[\W])|^)(?<!:)(([\da-f]{1,4}:){7}[\da-f]{1,4}|([\da-f]{1,4}:){1,1}(:[\da-f]{1,4}){1,6}|([\da-f]{1,4}:){1,2}(:[\da-f]{1,4}){1,5}|([\da-f]{1,4}:){1,3}(:[\da-f]{1,4}){1,4}|([\da-f]{1,4}:){1,4}(:[\da-f]{1,4}){1,3}|([\da-f]{1,4}:){1,5}(:[\da-f]{1,4}){1,2}|([\da-f]{1,4}:){1,6}(:[\da-f]{1,4})|([\da-f]{1,4}:){1,7}:|:((:[\da-f]{1,4}){1,7}|:)|::(ffff(:0{1,4})?:)?((25[0-5]|(2[0-4]|[0-1]?\d)?\d)\.){3}(25[0-5]|(2[0-4]|[0-1]?\d)?\d)|([\da-f]{1,4}:){1,4}:((25[0-5]|(2[0-4]|[0-1]?\d)?\d)\.){3}(25[0-5]|(2[0-4]|[0-1]?\d)?\d))(%[\da-z]+)?(/\d+)?(?!:)((?=[\W])|$)
  color: f#ff00ff

- description: MAC addresses
  regex: (?i)\b((?<!:)([\da-f]{1,2}:){5}[\da-f]{1,2}(?!:)|(?<!\.)([\da-f]{4}\.){2}[\da-f]{4}(?!\.))\b
  color: f#5f61ad

- description: Date in YYYY-MM-DD, YY-MM-DD, MMM (YYYY|DD), or DD MMM (YYYY)? formats
  regex: (?i)((?<=[\W])|^)((\d{2}|\d{4})\-((0)?[1-9]|1[0-2])\-(3[0-1]|([1-2]\d)|(0)?[1-9])|(jan|feb|mar|apr|may|jun|jul|aug|sep|oct|nov|dec)\s(\d{4}|\s\d|(3[0-1]|([0-2]\d)))|((\d|(3[0-1]|([0-2]\d)))\s(jan|feb|mar|apr|may|jun|jul|aug|sep|oct|nov|dec)(\s\d{4})?))((?=[\WT_])|$)
  color: b#af5f00

- description: Time in hh:mm:ss.SSSSSS format (sec, msec, and nsec optional)
  regex: \b((?<!:)((2[0-3])|[0-1]\d):[0-5]\d(:[0-5]\d)?((\.|,)\d{3,6})?(?!:))\b
  color: b#af5f00

- description: Juniper interfaces
  regex: (?i)\b(((fe|ge|xe|et|gr|ip|lt|lsq|mt|sp|vcp)\-\d*/\d*/\d*)|(((b)?me|em|fab|fxp|fti|lo|pp(d|e)?|st|swfab)[0-2]|dsc|gre|ipip|irb|jsrv|lsi|mtun|pimd|pime|tap|vlan|vme|vtep)|((ae|reth)\d*))(\.\d*)?\b
  color: f#f07a7c

- description: Cisco interfaces
  regex: (?i)\b(((Hu(ndredGigabit)?|Fo(rtyGigabit)?|Te(nGigabit)?|Gi(gabit)?|Fa(st)?)(Ethernet)?)|Eth|Se(rial)?|Lo(opback)?|Tu(nnel)?|VL(AN)?|Po(rt-channel)?|Vi(rtual\-(Template|Access))?|Mu(ltilink)?|Di(aler)?|(B|N)VI)((\d*/){0,2}\d*)(\.\d*)?\b
  color: f#f07a7c

- description: Generics - Bad
  regex: (?i)\b(password|abnormal(ly)?|down|los(t|s|ing)|err(or(s)?)?|(den(y|ies|ied)?)|reject(ing|ed)?|drop(ped|s)?|(err\-)?disable(d)?|(time(d)?(\-)?out)|fail(s|ed|iure)?|disconnect(ed)?|unreachable|invalid|bad|notconnect|unusable|block(ing|ed)?|blk|inaccessible|wrong|collision(s)?|unsynchronized|mismatch|runts|CRC)\b
  color: f#ff0000

- description: Generics - Ambigious bad
  regex: (?i)\b(no(t|pe)?|exit(ed)?|reset((t)?ing)?|discard(ed|ing)?|filter(ed)?|stop(p(ed|ing))?|never|can((')?t|not))\b
  color: f#865e12

- description: Generics - Not too bad
  regex: (?i)\b(warning(s)?)\b
  color: f#ffff00

- description: Generics - Ambigious good
  regex: (?i)\b(ye(s|a(h)?|p)?|started|running|can)\b
  color: f#085e0b

- description: Generics - Good
  regex: (?i)\b(up|ok(ay)?|permit(ed|s)?|accept(s|ed)?|enable(d)?|online|succe((ss(ful|fully)?)|ed(ed)?)?|connect(ed)?|reachable|valid|forwarding|synchronized)\b
  color: f#00ff00

- description: Half-duplex
  regex: (?i)\b(half(\-)?duplex)\b
  color: f#ff0000

- description: Cisco Syslog facilities - Emergency to error
  regex: \b(%\w+\-[0-3]\-\w+)\b
  color: f#ff0000

- description: Cisco Syslog facilities - Warning to notice
  regex: \b(%\w+\-[4-5]\-\w+)\b
  color: f#ffff00

- description: Cisco Syslog facilities - Info to debug
  regex: \b(%\w+\-[6-7]\-\w+)\b
  color: f#65d7fd

- description: Spanning tree - Problematic states
  regex: \b(BKN|(LOOP|ROOT|TYPE|PVID)_Inc)\b
  color: f#ff0000

- description: Spanning tree - Forwarding states
  regex: \b(FWD|Root|Desg)\b
  color: f#00ff00

- description: OSPF - Transitional states
  regex: \b(ATTEMPT|INIT|EXCHANGE|LOADING)\b
  color: f#ffff00

- description: OSPF - Acceptable states
  regex: \b(2WAY|FULL)\b
  color: f#00ff00

- description: BGP - Transitional states
  regex: \b(Idle|Connect|Active|OpenSent|OpenConfirm)\b
  color: f#ffff00
"""


def write_default_config(directory=os.getenv('HOME'), name='.chromaterm.yml'):
    """Write the default configuration file if it doesn't exist."""
    if not directory or not name:
        return False

    location = os.path.join(directory, name)

    if os.access(location, os.F_OK):  # Already exists
        return False

    if not os.access(directory, os.W_OK):  # No write permission in directory
        return False

    with open(location, 'w') as file:
        file.write(DEFAULT_CONFIG)

    return True
