#!/usr/bin/env python

"""
setup.py file for SWIG example
"""

import sys
import re
import os.path
from distutils.core import setup, Extension
from distutils.command.install_lib import install_lib as _install_lib

# PACKAGES = find_packages()

def get_rootdir():
    return os.path.abspath(os.path.join(os.path.dirname(__file__), '../..'))
def get_includedir():
    return '.'

def get_swigdir():
    return os.path.join(get_rootdir(), 'swig')

#import os; os.environ['CC'] = 'g++'; os.environ['CXX'] = 'g++';
#os.environ['CPP'] = 'g++'; os.environ['LDSHARED'] = 'g++'

def batch_rename(src, dst, src_dir_fd=None, dst_dir_fd=None):
    '''Same as os.rename, but returns the renaming result.'''
    os.rename(src, dst,
              src_dir_fd=src_dir_fd,
              dst_dir_fd=dst_dir_fd)
    return dst

class _CommandInstallCythonized(_install_lib):
    def __init__(self, *args, **kwargs):
        _install_lib.__init__(self, *args, **kwargs)

    def install(self):
        # let the distutils' install_lib do the hard work
        outfiles = _install_lib.install(self)
        # batch rename the outfiles:
        # for each file, match string between
        # second last and last dot and trim it
        matcher = re.compile('\.([^.]+)\.so$')
        return [batch_rename(file, re.sub(matcher, '.so', file))
                for file in outfiles]

additional_include_dirs = []
library_dirs = None
extra_compile_args = None
libs = []

if sys.platform.startswith("darwin"):
    # On recent macos versions (mojave) it is necessary to specify that libc++ is used instead of libstdc++.
    # Furthermore, '-Wl,-undefined,dynamic_lookup' is necessary to link the right libraries.
    libs += ["-stdlib=libc++", '-Wl,-undefined,dynamic_lookup']
    extra_compile_args = ["-stdlib=libc++"]

with open('README.md') as reader:
        readme = reader.read()

simstring_module = Extension(
    'quickumls_simstring._simstring',
    sources = [
        'quickumls_simstring/export.cpp',
        'quickumls_simstring/export_wrap.cpp',
    ],
    include_dirs=[get_includedir()] + additional_include_dirs,
    library_dirs=library_dirs,
    extra_link_args=libs,
    extra_compile_args=extra_compile_args,
    language='c++',
    )

setup(
    name = 'quickumls_simstring',
    url = 'https://github.com/Georgetown-IR-Lab/simstring',
    version = '1.1.5r1',
    description=(
        'Clone of simstring designed to work with QuickUMLS. ' 
        'Original version here: http://chokkan.org/software/simstring/'
    ),
    long_description=readme,
    packages=['quickumls_simstring'],
    author = 'Naoaki Okazaki & Blink Health & Luca Soldaini',
    author_email = 'luca@ir.cs.georgetown.edu',
    ext_modules = [simstring_module],
    cmdclass={
        'install_lib': _CommandInstallCythonized,
    },
)

