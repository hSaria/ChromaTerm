'''A store for the default rules of ChromaTerm'''
import os

import yaml

from chromaterm import Color, Palette, Rule

# pylint: disable=line-too-long

PALETTE = Palette()

# https://coolors.co/c68c6c-00e0d1-ef2e9f-5698c8-a35a00-df99f0-03d28d
PALETTE.add_color('type-1', '#dc8968')
PALETTE.add_color('type-2', '#00e0d1')
PALETTE.add_color('type-3', '#ef2e9f')
PALETTE.add_color('type-4', '#5698c8')
PALETTE.add_color('type-5', '#a35a00')
PALETTE.add_color('type-6', '#df99f0')
PALETTE.add_color('type-7', '#03d28d')

# https://coolors.co/c71800-c96901-ca9102-cab902-a2bc02-79bf02-28c501
PALETTE.add_color('status-1', '#c71800')
PALETTE.add_color('status-2', '#c96901')
PALETTE.add_color('status-3', '#ca9102')
PALETTE.add_color('status-4', '#cab902')
PALETTE.add_color('status-5', '#a2bc02')
PALETTE.add_color('status-6', '#79bf02')
PALETTE.add_color('status-7', '#28c501')

RULE_NUMBERS = Rule(
    r'\b(?<!\.)\d+(\.\d+)?(?!\.)\b',
    Color('f.type-1', palette=PALETTE),
    'Numbers',
)

RULE_URL = Rule(
    r'''(?ix)\b
    ((htt|ft|lda)ps?|telnet|ssh)://  # Scheme
    ([-%:\w\\/]{1,256}@)?  # User info
    [-\w]{1,63}(\.[-\w]{1,63}){0,126}(:\d{1,5})?  # Host and port
    (/[-+=~@%&?#.:;,\w\\/()]*)?  # Path, query, and fragment
((?=[.:;,)])|\b)  # Avoid highlighting trailing path characters by matching them in a lookahead
''',
    Color('f.type-4', palette=PALETTE),
    'URL',
    exclusive=True,
)

RULE_IPV4 = Rule(
    r'\b(?<!\.)((25[0-5]|(2[0-4]|[0-1]?\d)?\d)\.){3}(25[0-5]|(2[0-4]|[0-1]?\d)?\d)(/\d+)?\b',
    Color('f.type-2', palette=PALETTE),
    'IPv4',
    exclusive=True,
)

RULE_IPV6 = Rule(
    r'''(?ix)(?<![\w:])(
    ([\da-f]{1,4}:){7}[\da-f]{1,4}|  # 1:2:3:4:5:6:7:8
    [\da-f]{1,4}:(:[\da-f]{1,4}){1,6}|  # 1::3:4:5:6:7:8
    ([\da-f]{1,4}:){1,2}(:[\da-f]{1,4}){1,5}|  # 1:2::4:5:6:7:8
    ([\da-f]{1,4}:){1,3}(:[\da-f]{1,4}){1,4}|  # 1:2:3::5:6:7:8
    ([\da-f]{1,4}:){1,4}(:[\da-f]{1,4}){1,3}|  # 1:2:3:4::6:7:8
    ([\da-f]{1,4}:){1,5}(:[\da-f]{1,4}){1,2}|  # 1:2:3:4:5::7:8
    ([\da-f]{1,4}:){1,6}:[\da-f]{1,4}|  # 1:2:3:4:5:6::8
    ([\da-f]{1,4}:){1,7}:|  # 1:2:3:4:5:6:7::
    :((:[\da-f]{1,4}){1,7}|:)  # ::2:3:4:5:6:7:8
)(:(?=\W))?  # \W is an exclusive-flag hack to color the : before an IPv4-embedded address
(%[\da-z]+)?  # Zone index
(/\d+)?  # Prefix length
(?!:?\w)
''',
    Color('f.type-3', palette=PALETTE),
    'IPv6',
    exclusive=True,
)

RULE_MAC = Rule(
    r'''(?ix)\b(
    (?<!:)([\da-f]{1,2}:){5}[\da-f]{1,2}(?!:)|  # 11:22:33:aa:bb:cc
    (?<!\.)([\da-f]{4}\.){2}[\da-f]{4}(?!\.)  # 1122.33aa.bbcc
)\b
''',
    Color('f.type-4', palette=PALETTE),
    'MAC address',
    exclusive=True,
)

RULE_DATE = Rule(
    r'''(?ix)\b(
    (\d{2}|\d{4})(?P<sep1>[-/])(0?[1-9]|1[0-2])(?P=sep1)(3[0-1]|[1-2]\d|0?[1-9])|  # YYYY-MM-DD, YY-MM-DD, YYYY/MM/DD, YY/MM/DD
    (3[0-1]|[1-2]\d|0?[1-9])(?P<sep2>[-/])(0?[1-9]|1[0-2])(?P=sep2)(\d{2}|\d{4})|  # DD-MM-YYYY, DD-MM-YY, DD/MM/YYYY, DD/MM/YY
    (0?[1-9]|1[0-2])(?P<sep3>[-/])(3[0-1]|[1-2]\d|0?[1-9])(?P=sep3)(\d{2}|\d{4})|  # MM-DD-YYYY, MM-DD-YY, MM/DD/YYYY, MM/DD/YY
    (jan|feb|mar|apr|may|jun|jul|aug|sep|oct|nov|dec)\s+(  # MMM
        (3[0-1]|[1-2]\d|0?[1-9])(\s+\d{4})?|\d{4}  # DD (YYYY)?, YYYY
    )|(3[0-1]|[1-2]\d|0?[1-9])\s(jan|feb|mar|apr|may|jun|jul|aug|sep|oct|nov|dec)(?!\s+(3[0-1]|[1-2]\d|0?[1-9])([^\w:]|$))(\s+\d{4})?  # DD MMM (YYYY)?
)((?=[\WT_])|$)
''',
    Color('b.type-5', palette=PALETTE),
    'Date',
    exclusive=True,
)

RULE_TIME = Rule(
    r'''(?ix)(?<![\.:])(\b|(?<=T))
    (2[0-3]|[0-1]\d):[0-5]\d  # Hours and minutes
    (:[0-5]\d([\.,]\d{3,6})?)?  # (Seconds (sub-seconds, 3 to 6 decimal places)?)?
    ([\-\+](\d{2}|\d{4})|Z)?  # (Timezone)?
(?![\.:])\b
''',
    Color('b.type-5', palette=PALETTE),
    'Time',
    exclusive=True,
)

RULE_SIZE = Rule(
    r'(?i)\b\d+(\.\d+)?\s?((([KMGTPEZY](i?B)?)|B)(ps)?)\b',
    {
        0: Color('f.type-6', palette=PALETTE),
        2: Color('bold'),
    },
    'Size, like 123G 123Gb 123Gib 1.23G 123Gbps',
    exclusive=True,
)

RULE_GENERIC_BAD = Rule(
    r'(?i)\b(password|abnormal(ly)?|down|los(t|ing)|err(ors?)?|(den(y|ies|ied)?)|reject(ing|ed)?|drop(ped|s)?|(err\-)?disabled?|(timed?\-?out)|fail(s|ed|iure)?|disconnect(ed)?|unreachable|invalid|bad|notconnect|unusable|blk|inaccessible|wrong|collisions?|unsynchronized|mismatch|runts)\b',
    Color('f.status-1', palette=PALETTE),
    'Generic - Bad',
)

RULE_GENERIC_AMBIGIOUS_BAD = Rule(
    r'(?i)\b(no(pe)?|exit(ed)?|reset(t?ing)?|discard(ed|ing)?|block(ed|ing)?|filter(ed|ing)?|stop(p(ed|ing))?|never|bad)\b',
    Color('f.status-3', palette=PALETTE),
    'Generic - Ambigious bad',
)

RULE_GENERIC_NOT_TOO_BAD = Rule(
    r'(?i)\b(warnings?)\b',
    Color('f.status-4', palette=PALETTE),
    'Generic - Not too bad',
)

RULE_GENERIC_AMBIGIOUS_GOOD = Rule(
    r'(?i)\b(ye(s|ah?|p)?|start(ed|ing)?|running|good)\b',
    Color('f.status-6', palette=PALETTE),
    'Generic - Ambigious good',
)

RULE_GENERIC_GOOD = Rule(
    r'(?i)\b(up|ok(ay)?|permit(ed|s)?|accept(s|ed)?|enabled?|online|succe((ss(ful|fully)?)|ed(ed)?)?|connect(ed)?|reachable|valid|forwarding|synchronized)\b',
    Color('f.status-7', palette=PALETTE),
    'Generic - Good',
)


def generate_default_rules_yaml():
    '''Returns a YAML string of the default configuration.'''
    data = yaml.dump({'palette': PALETTE.colors}, sort_keys=False) + '\n'
    data += 'rules:\n'

    for rule in [
            RULE_NUMBERS, RULE_URL, RULE_IPV4, RULE_IPV6, RULE_MAC, RULE_DATE,
            RULE_TIME, RULE_SIZE, RULE_GENERIC_BAD, RULE_GENERIC_AMBIGIOUS_BAD,
            RULE_GENERIC_NOT_TOO_BAD, RULE_GENERIC_AMBIGIOUS_GOOD,
            RULE_GENERIC_GOOD
    ]:
        rule_dict = {'description': rule.description, 'regex': rule.regex}

        if list(rule.colors) == [0]:
            rule_dict['color'] = rule.color.color
        else:
            rule_dict['color'] = {k: v.color for k, v in rule.colors.items()}

        if rule.exclusive:
            rule_dict['exclusive'] = True

        data += yaml.dump([rule_dict], sort_keys=False) + '\n'

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


def yaml_str_presenter(dumper, data):
    '''YAML string representer that uses the `|` style when data is multiline.'''
    style = '|' if len(data.splitlines()) > 1 else None
    return dumper.represent_scalar('tag:yaml.org,2002:str', data, style=style)


yaml.add_representer(str, yaml_str_presenter)
