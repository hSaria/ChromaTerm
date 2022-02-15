from setuptools import setup

with open('README.md', 'r') as f:
    LONG_DESCRIPTION = f.read()

setup(
    name='chromaterm',
    author='hSaria',
    author_email='sariahajjar@gmail.com',
    classifiers=[
        'Intended Audience :: Information Technology',
        'Intended Audience :: System Administrators',
        'Intended Audience :: Telecommunications Industry',
        'License :: OSI Approved :: MIT License',
        'Programming Language :: Python :: 3', 'Topic :: Terminals',
        'Topic :: Utilities'
    ],
    description='Color your Terminal with Regex!',
    entry_points={'console_scripts': ['ct = chromaterm.__main__:main']},
    install_requires=['psutil', 'PyYAML>=5.1'],
    license='MIT',
    long_description=LONG_DESCRIPTION,
    long_description_content_type='text/markdown',
    packages=['chromaterm'],
    python_requires='>=3.6.0',
    url='https://github.com/hSaria/ChromaTerm',
    version='0.10.1',
)
