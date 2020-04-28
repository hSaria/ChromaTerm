"""A store for the default rules of ChromaTerm"""
import os

from chromaterm import Color, Rule

# pylint: disable=line-too-long

RULE_IPV4 = Rule(
    r'\b(?<!\.)((25[0-5]|(2[0-4]|[0-1]?\d)?\d)\.){3}(25[0-5]|(2[0-4]|[0-1]?\d)?\d)(/\d+)?(?!\.)\b',
    Color('f#00ffff'),
    'IPv4',
)

RULE_IPV6 = Rule(
    r'(?i)((?<=[\W])|^)(?<!:)(([\da-f]{1,4}:){7}[\da-f]{1,4}|([\da-f]{1,4}:){1,1}(:[\da-f]{1,4}){1,6}|([\da-f]{1,4}:){1,2}(:[\da-f]{1,4}){1,5}|([\da-f]{1,4}:){1,3}(:[\da-f]{1,4}){1,4}|([\da-f]{1,4}:){1,4}(:[\da-f]{1,4}){1,3}|([\da-f]{1,4}:){1,5}(:[\da-f]{1,4}){1,2}|([\da-f]{1,4}:){1,6}(:[\da-f]{1,4})|([\da-f]{1,4}:){1,7}:|:((:[\da-f]{1,4}){1,7}|:)|::(ffff(:0{1,4})?:)?((25[0-5]|(2[0-4]|[0-1]?\d)?\d)\.){3}(25[0-5]|(2[0-4]|[0-1]?\d)?\d)|([\da-f]{1,4}:){1,4}:((25[0-5]|(2[0-4]|[0-1]?\d)?\d)\.){3}(25[0-5]|(2[0-4]|[0-1]?\d)?\d))(%[\da-z]+)?(/\d+)?(?!:)((?=[\W])|$)',
    Color('f#ff00ff'),
    'IPv6 (boundaries don\'t work here as they can be in the start or end of the match, so using lookaheads and lookbehinds instead)',
)

RULE_MAC = Rule(
    r'(?i)\b((?<!:)([\da-f]{1,2}:){5}[\da-f]{1,2}(?!:)|(?<!\.)([\da-f]{4}\.){2}[\da-f]{4}(?!\.))\b',
    Color('f#5f61ad'),
    'MAC addresses',
)

RULE_DATE = Rule(
    r'(?i)((?<=[\W])|^)((\d{2}|\d{4})\-((0)?[1-9]|1[0-2])\-(3[0-1]|([1-2]\d)|(0)?[1-9])|(jan|feb|mar|apr|may|jun|jul|aug|sep|oct|nov|dec)\s(\d{4}|\s\d|(3[0-1]|([0-2]\d)))|((\d|(3[0-1]|([0-2]\d)))\s(jan|feb|mar|apr|may|jun|jul|aug|sep|oct|nov|dec)(\s\d{4})?))((?=[\WT_])|$)',
    Color('b#af5f00'),
    'Date in YYYY-MM-DD, YY-MM-DD, MMM (YYYY|DD), or DD MMM (YYYY)? formats',
)

RULE_TIME = Rule(
    r'\b((?<!:)((2[0-3])|[0-1]\d):[0-5]\d(:[0-5]\d)?((\.|,)\d{3,6})?(?!:))\b',
    Color('b#af5f00'),
    'Time in hh:mm:ss.SSSSSS format (sec, msec, and nsec optional)',
)

RULE_GENERIC_BAD = Rule(
    r'(?i)\b(password|abnormal(ly)?|down|los(t|s|ing)|err(or(s)?)?|(den(y|ies|ied)?)|reject(ing|ed)?|drop(ped|s)?|(err\-)?disable(d)?|(time(d)?(\-)?out)|fail(s|ed|iure)?|disconnect(ed)?|unreachable|invalid|bad|notconnect|unusable|block(ing|ed)?|blk|inaccessible|wrong|collision(s)?|unsynchronized|mismatch|runts|CRC)\b',
    Color('f#ff0000'),
    'Generics - Bad',
)

RULE_GENERIC_AMBIGIOUS_BAD = Rule(
    r"(?i)\b(no(t|pe)?|exit(ed)?|reset((t)?ing)?|discard(ed|ing)?|filter(ed)?|stop(p(ed|ing))?|never|can((')?t|not))\b",
    Color('f#865e12'),
    'Generics - Ambigious bad',
)

RULE_GENERIC_NOT_TOO_BAD = Rule(
    r'(?i)\b(warning(s)?)\b',
    Color('f#ffff00'),
    'Generics - Not too bad',
)

RULE_GENERIC_AMBIGIOUS_GOOD = Rule(
    r'(?i)\b(ye(s|a(h)?|p)?|started|running|can)\b',
    Color('f#085e0b'),
    'Generics - Ambigious good',
)

RULE_GENERIC_GOOD = Rule(
    r'(?i)\b(up|ok(ay)?|permit(ed|s)?|accept(s|ed)?|enable(d)?|online|succe((ss(ful|fully)?)|ed(ed)?)?|connect(ed)?|reachable|valid|forwarding|synchronized)\b',
    Color('f#00ff00'),
    'Generics - Good',
)


def generate_default_rules_yaml():
    """Returns a string describing the default configuration in YAML."""
    data = 'rules:'
    rule_format = """
- description: {}
  regex: {}
  color: {}
    """

    for rule in [
            RULE_IPV4, RULE_IPV6, RULE_MAC, RULE_DATE, RULE_TIME,
            RULE_GENERIC_BAD, RULE_GENERIC_AMBIGIOUS_BAD,
            RULE_GENERIC_NOT_TOO_BAD, RULE_GENERIC_AMBIGIOUS_GOOD,
            RULE_GENERIC_GOOD
    ]:
        data += rule_format.format(rule.description, rule.regex.pattern,
                                   rule.color.color)

    return data


def write_default_config(directory=None, name=None):
    """Writes the default configuration file if it doesn't exist.

    Args:
        directory (str): The directory where the file should be placed. Defaults
            to the home directory of the user.
        name (str): The name of the file. Defaults to '.chromaterm.yml'

    Returns:
        True if a file was written. False otherwise.
    """
    if not directory:
        directory = os.path.expanduser('~')

    if not name:
        name = '.chromaterm.yml'

    location = os.path.join(directory, name)

    # Already exists
    if os.access(location, os.F_OK):
        return False

    # No write permission in directory
    if not os.access(directory, os.W_OK):
        return False

    with open(location, 'w') as file:
        file.write(generate_default_rules_yaml())

    return True
