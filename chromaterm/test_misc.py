#!/usr/bin/env python3
"""Miscellaneous tests."""

import os

import chromaterm.misc

DIR_FAKE = '.test_chromaterm'
FILE_FAKE = '.test_chromaterm.yml'


def test_write_default_config():
    """Write config file."""
    assert chromaterm.misc.write_default_config('.', FILE_FAKE) is True
    assert os.access(os.path.join('.', FILE_FAKE), os.F_OK)
    os.remove(FILE_FAKE)


def test_write_default_config_no_directory():
    """No directory for default config (e.g. no home)."""
    assert chromaterm.misc.write_default_config(None) is False


def test_write_default_config_exists():
    """Config file already exists."""
    open(FILE_FAKE, 'w').close()
    assert chromaterm.misc.write_default_config('.', FILE_FAKE) is False
    os.remove(FILE_FAKE)


def test_write_default_config_no_permission():
    """No write permission on directory."""
    os.mkdir(DIR_FAKE, mode=0o444)
    assert chromaterm.misc.write_default_config(DIR_FAKE) is False
    os.rmdir(DIR_FAKE)
