#!/usr/bin/env python3
"""ChromaTerm setup"""
from setuptools import setup

# We're not dealing with the CLI script on Windows
REQUIRES = [
    'psutil; sys_platform != "win32"',
    'PyYAML; sys_platform != "win32"',
]
REQUIRES_PYTHON = '>=3.5.0'

with open('README.md', 'r') as f:
    LONG_DESCRIPTION = f.read()

setup(
    name='chromaterm',
    author='hSaria',
    author_email='ping@heysaria.com',
    classifiers=[
        'Intended Audience :: Information Technology',
        'Intended Audience :: System Administrators',
        'Intended Audience :: Telecommunications Industry',
        'License :: OSI Approved :: MIT License',
        'Programming Language :: Python :: 3', 'Topic :: Terminals',
        'Topic :: Utilities'
    ],
    description='Color your output to terminal',
    entry_points={'console_scripts': ['ct = chromaterm.cli:main']},
    license='MIT',
    install_requires=REQUIRES,
    long_description=LONG_DESCRIPTION,
    long_description_content_type='text/markdown',
    packages=['chromaterm'],
    python_requires=REQUIRES_PYTHON,
    url='https://github.com/hSaria/ChromaTerm',
    version='0.6.3',
)
