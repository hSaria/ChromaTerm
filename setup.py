import os

from setuptools import find_packages, setup


def get_version():
    base_path = os.path.abspath(os.path.dirname(__file__))
    module_path = os.path.join(base_path, 'chromaterm/__init__.py')

    with open(module_path, 'r', encoding='utf-8') as file:
        for line in file.readlines():
            if line.startswith('__version__'):
                return line.split('"' if '"' in line else "'")[1]

        raise RuntimeError('Unable to find version string.')


def get_long_description():
    with open('README.md', 'r', encoding='utf-8') as file:
        return file.read()


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
    long_description=get_long_description(),
    long_description_content_type='text/markdown',
    packages=find_packages(),
    python_requires='>=3.6.0',
    url='https://github.com/hSaria/ChromaTerm',
    version=get_version(),
)
