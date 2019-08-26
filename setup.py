from setuptools import setup

REQUIRES = ['regex', 'PyYAML']
REQUIRES_PYTHON = '>=3.6.0'

with open('README.md', 'r') as f:
    long_description = f.read()

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
    description='Colorize your output using RegEx',
    license='GPLv2',
    long_description=long_description,
    long_description_content_type='text/markdown',
    requires=REQUIRES,
    packages=['chromaterm'],
    python_requires=REQUIRES_PYTHON,
    scripts=['ct'],
    url='https://github.com/hSaria/ChromaTerm',
    version='0.3.94',
)
