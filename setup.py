import os
import shutil

from setuptools import setup
from setuptools.command.install import install

REQUIRES = ['PyYAML']
REQUIRES_PYTHON = '>=3.6.0'

with open('README.md', 'r') as f:
    LONG_DESCRIPTION = f.read()


class InstallCommands(install):
    """Custom commands to be run during installation."""
    def run(self):
        install.run(self)
        copy_default_config()


def copy_default_config():
    """Copy the default configuration file to the home directory if it doesn't
    exist."""
    file = '.chromaterm.yml'
    home = os.getenv('HOME')

    try:
        if os.access(home + '/' + file, os.F_OK):  # Already exists
            return

        print('Copying {} -> {}'.format(file, home))
        shutil.copy(file, home)
    except Exception as exception:
        print(exception)


setup(
    name='chromaterm',
    author='hSaria',
    author_email='ping@heysaria.com',
    classifiers=[
        'Intended Audience :: Information Technology',
        'Intended Audience :: System Administrators',
        'Intended Audience :: Telecommunications Industry',
        'License :: OSI Approved :: GNU General Public License v2 (GPLv2)',
        'Programming Language :: Python :: 3', 'Topic :: Terminals',
        'Topic :: Utilities'
    ],
    cmdclass={'install': InstallCommands},
    description='Colorize your output using RegEx',
    license='GPLv2',
    long_description=LONG_DESCRIPTION,
    long_description_content_type='text/markdown',
    requires=REQUIRES,
    packages=['chromaterm'],
    python_requires=REQUIRES_PYTHON,
    scripts=['ct'],
    url='https://github.com/hSaria/ChromaTerm',
    version='0.3.96',
)
