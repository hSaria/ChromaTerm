#!/usr/bin/env python3
"""Miscellaneous tests."""

import os

import chromaterm
import chromaterm.misc

CONFIG = chromaterm.parse_config(chromaterm.misc.DEFAULT_CONFIG)
TEMP_DIR = '.test_chromaterm'
TEMP_FILE = '.test_chromaterm.yml'


def assert_highlight(positives, negatives, rule, permutate=True):
    """Assert that all positives are highlighted while negatives are not."""
    assert rule

    config = chromaterm.get_default_config()
    config['rules'] = [rule]

    for entry in permutate_data(positives) if permutate else positives:
        assert chromaterm.highlight(config, entry) != entry

    for entry in permutate_data(negatives) if permutate else negatives:
        assert chromaterm.highlight(config, entry) == entry


def find_rule(query):
    """Return the first parsed rule with a description that includes `query`."""
    for rule in CONFIG['rules']:
        if query in rule['description']:
            return rule
    return None


def permutate_data(data):
    """Return a list with `data` permutated. `data` can a string or list."""
    output = []

    for entry in data if isinstance(data, list) else [data]:
        output.append(entry)  # Plain entry
        output.append('hello ' + entry)  # Start changed
        output.append(entry + ' world')  # End changed
        output.append('hello ' + entry + ' world')  # Start and end changed

    return output


def test_default_config_ipv4():
    """Default rule: IPv4 addresses."""
    positives = ['192.168.2.1', '255.255.255.255', '=240.3.2.1', '1.2.3.4/32']
    negatives = ['192.168.2.1.', '1.2.3.4.5', '256.255.255.255', '1.2.3']
    rule = find_rule('IPv4')

    assert_highlight(positives, negatives, rule)


def test_default_config_ipv6():
    """Default rule: IPv6 addresses."""
    positives = [
        'A:b:3:4:5:6:7:8', 'A::', 'A:b:3:4:5:6:7::', 'A::8', '::b:3:4:5:6:7:8',
        '::8', 'A:b:3:4:5:6::8', 'A:b:3:4:5::7:8', 'A:b:3:4::6:7:8', '::',
        'A:b:3::5:6:7:8', 'A:b::4:5:6:7:8', 'A::3:4:5:6:7:8', 'A::7:8', 'A::8',
        'A:b:3:4:5::8', 'A::6:7:8', 'A:b:3:4::8', 'A::5:6:7:8', 'A:b:3::8',
        'A::4:5:6:7:8', 'A:b::8', 'A:b:3:4:5:6:7:8/64', '::255.255.255.255',
        '::ffff:255.255.255.255', 'fe80::1%tun', '::ffff:0:255.255.255.255',
        '00A:db8:3:4::192.0.2.33', '64:ff9b::192.0.2.33'
    ]
    negatives = [
        ':::', '1:2', '1:2:3', '1:2:3:4', '1:2:3:4:5', '1:2:3:4:5:6:7',
        '1:2:3:4:5:6:7:8:9', '1:2:3:4:5:6:7::8', 'abcd:xyz::1', 'fe80:1%tun'
    ]
    rule = find_rule('IPv6')

    assert_highlight(positives, negatives, rule)


def test_default_config_mac():
    """Default rule: MAC addresses."""
    positives = ['0A:23:45:67:89:AB', '0a:23:45:67:89:ab', '0a23.4567.89ab']
    negatives = [
        '0A:23:45:67:89', '0A:23:45:67:89:AB:', '0A23.4567.89.AB',
        '0a23.4567.89ab.', '0a:23:45:67:xy:zx', '0a23.4567.xyzx'
    ]
    rule = find_rule('MAC')

    assert_highlight(positives, negatives, rule)


def test_default_config_date():
    """Default rule: Date."""
    positives = [
        '2019-12-31', '2019-12-31', 'jan 2019', 'feb 2019', 'Mar 2019',
        'apr 2019', 'MAY 2019', 'Jun 2019', 'jul 2019', 'AUG 19', 'sep 20',
        'oct 21', 'nov 22', 'dec 23', '24 jan', '25 feb 2019'
    ]
    negatives = [
        '201-12-31', '2019-13-31', '2019-12-32', 'xyz 2019', 'Jun 201',
        'xyz 26', 'jun 32', '32 jun'
    ]
    rule = find_rule('Date')

    assert_highlight(positives, negatives, rule)


def test_default_config_time():
    """Default rule: Time."""
    positives = ['23:59', '23:01', '23:01:01', '23:01:01.123']
    negatives = ['24:59', '23:60', '23:1', '23:01:1', '23:01:01:']
    rule = find_rule('Time')

    assert_highlight(positives, negatives, rule)


def test_find_rule():
    """Verify find rule is able to locate rules in the default configuration."""
    assert find_rule('BGP')['description'] == 'BGP - Transitional states'
    assert find_rule('A fake rule that does not exist') is None


def test_write_default_config():
    """Write config file."""
    assert chromaterm.misc.write_default_config('.', TEMP_FILE + '1') is True
    assert os.access(os.path.join('.', TEMP_FILE + '1'), os.F_OK)
    os.remove(TEMP_FILE + '1')


def test_write_default_config_no_directory():
    """No directory for default config (e.g. no home)."""
    assert chromaterm.misc.write_default_config(None) is False


def test_write_default_config_exists():
    """Config file already exists."""
    open(TEMP_FILE + '2', 'w').close()
    assert chromaterm.misc.write_default_config('.', TEMP_FILE + '2') is False
    os.remove(TEMP_FILE + '2')


def test_write_default_config_no_permission():
    """No write permission on directory."""
    os.mkdir(TEMP_DIR + '1', mode=0o444)
    assert chromaterm.misc.write_default_config(TEMP_DIR + '1') is False
    os.rmdir(TEMP_DIR + '1')
