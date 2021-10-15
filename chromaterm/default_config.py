'''A store for the default rules of ChromaTerm'''
import os

from chromaterm import Color, Rule

# pylint: disable=line-too-long

RULE_NUMBERS = Rule(
    r'\b(?<!\.)\d+(\.\d+)?(?!\.)\b',
    Color('f#91490a'),
    'Numbers',
)

RULE_IPV4 = Rule(
    r'\b(?<!\.)((25[0-5]|(2[0-4]|[0-1]?\d)?\d)\.){3}(25[0-5]|(2[0-4]|[0-1]?\d)?\d)(/\d+)?(?!\.)\b',
    Color('f#00ffff'),
    'IPv4',
    exclusive=True,
)

RULE_IPV6 = Rule(
    r'(?i)(?<![\w:])(([\da-f]{1,4}:){7}[\da-f]{1,4}|[\da-f]{1,4}:(:[\da-f]{1,4}){1,6}|([\da-f]{1,4}:){1,2}(:[\da-f]{1,4}){1,5}|([\da-f]{1,4}:){1,3}(:[\da-f]{1,4}){1,4}|([\da-f]{1,4}:){1,4}(:[\da-f]{1,4}){1,3}|([\da-f]{1,4}:){1,5}(:[\da-f]{1,4}){1,2}|([\da-f]{1,4}:){1,6}:[\da-f]{1,4}|([\da-f]{1,4}:){1,7}:|:((:[\da-f]{1,4}){1,7}|:)|::(ffff(:0{1,4})?:)?((25[0-5]|(2[0-4]|[0-1]?\d)?\d)\.){3}(25[0-5]|(2[0-4]|[0-1]?\d)?\d)|([\da-f]{1,4}:){1,4}:((25[0-5]|(2[0-4]|[0-1]?\d)?\d)\.){3}(25[0-5]|(2[0-4]|[0-1]?\d)?\d))(%[\da-z]+)?(/\d+)?(?![\w:])',
    Color('f#ff00ff'),
    'IPv6 (boundaries don\'t work here as they can be in the start or end of the match, so using lookaheads and lookbehinds instead)',
    exclusive=True,
)

RULE_MAC = Rule(
    r'(?i)\b((?<!:)([\da-f]{1,2}:){5}[\da-f]{1,2}(?!:)|(?<!\.)([\da-f]{4}\.){2}[\da-f]{4}(?!\.))\b',
    Color('f#5f61ad'),
    'MAC addresses',
    exclusive=True,
)

RULE_DATE = Rule(
    r'(?i)((?<=\W)|^)((\d{2}|\d{4})\-(0?[1-9]|1[0-2])\-(3[0-1]|([1-2]\d)|0?[1-9])|(jan|feb|mar|apr|may|jun|jul|aug|sep|oct|nov|dec)\s+(\d{4}|(3[0-1]|([1-2]\d)|0?[1-9]))|((3[0-1]|([1-2]\d)|0?[1-9])\s(jan|feb|mar|apr|may|jun|jul|aug|sep|oct|nov|dec)(\s+\d{4})?))((?=[\WT_])|$)',
    Color('b#af5f00'),
    'Date in YYYY-MM-DD, YY-MM-DD, MMM (YYYY|DD), or DD MMM (YYYY)? formats',
    exclusive=True,
)

RULE_TIME = Rule(
    r'(\b|(?<=T))(?<![\.:])((2[0-3])|[0-1]\d):[0-5]\d(:[0-5]\d([\.,]\d{3,6})?)?([\-\+](\d{2}|\d{4}))?(?![\.:])\b',
    Color('b#af5f00'),
    'Time in hh:mm:ss.SSSSSS-ZZZZ format (sec, msec, nsec, and timezone offset optional)',
    exclusive=True,
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
    '''Returns a string describing the default configuration in YAML.'''
    data = 'rules:'

    for rule in [
            RULE_NUMBERS, RULE_IPV4, RULE_IPV6, RULE_MAC, RULE_DATE, RULE_TIME,
            RULE_GENERIC_BAD, RULE_GENERIC_AMBIGIOUS_BAD,
            RULE_GENERIC_NOT_TOO_BAD, RULE_GENERIC_AMBIGIOUS_GOOD,
            RULE_GENERIC_GOOD
    ]:
        data += f'''
- description: {rule.description}
  regex: {rule.regex.pattern.decode()}
  color: {rule.color.color}
'''

        if rule.exclusive:
            data += f'  exclusive: {rule.exclusive}\n'

    return data


def write_default_config(path):
    '''Writes a default config file if it doesn't exist.

    Args:
        path (str): The location to which the default config file is written.

    Returns:
        True if the file did not exist and was written. False otherwise.
    '''
    # Already exists
    if os.access(path, os.F_OK):
        return False

    # No write permission in directory
    if not os.access(os.path.dirname(path) or os.path.curdir, os.W_OK):
        return False

    with open(path, 'w', encoding='utf-8') as file:
        file.write(generate_default_rules_yaml())

    return True
