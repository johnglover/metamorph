'''
Metamorph is an open source library for performing high-level sound
transformations based on a sinusoids plus noise plus transients model.
It is designed to work primarily on monophonic sounds and can be used in
both real-time or non-real-time contexts.
'''

from distutils.core import setup
from distutils.extension import Extension
from Cython.Distutils import build_ext
import numpy

try:
    numpy_include = numpy.get_include()
except AttributeError:
    numpy_include = numpy.get_numpy_include()

doc_lines = __doc__.split('\n')
include_dirs = ['/usr/local/include/simpl/',
                '/usr/local/include',
                numpy_include]
libs = ['notesegmentation', 'simpl']

metamorph = Extension(
    'metamorph.fx',
    sources=['metamorph/fx.pyx',
             'src/fx.cpp',
             'src/transformations.cpp',
             'src/time_scale.cpp',
             'src/spec_env.cpp',
             'src/exceptions.cpp'],
    include_dirs=['src'] + include_dirs,
    libraries=libs,
    language='c++'
)

setup(
    name='metamorph',
    description=doc_lines[0],
    long_description='\n'.join(doc_lines[2:]),
    url='http://github.com/johnglover/metamorph',
    download_url='http://github.com/johnglover/metamorph',
    license='GPL',
    author='John Glover',
    author_email='john.c.glover@nuim.ie',
    platforms=['Linux', 'Mac OS-X', 'Unix'],
    version='1.0',
    packages=['metamorph'],
    ext_modules=[metamorph],
    cmdclass={'build_ext': build_ext}
)
