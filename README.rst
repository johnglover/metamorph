Metamorph
=========

Metamorph is a new open source library for performing high-level sound
transformations based on a sinusoids plus noise plus transients model.
It is written in C++, can be built as both a Python extension module
and a Csound opcode, and currently runs on Mac OS X and Linux.

It is designed to work primarily on monophonic, quasi-harmonic sound
sources and can be used in a non-real-time context to process pre-recorded
sound files or can operate in a real-time (streaming) mode.

Metamorph is available under the terms of the GNU General Public License (GPL).


C++ Library Dependencies
------------------------

* CMake_
* simpl_
* modal_
* notesegmentation_

.. _CMake: http://www.cmake.org
.. _simpl: http://simplsound.sourceforge.net
.. _modal: http://github.com/johnglover/modal
.. _notesegmentation: http://github.com/johnglover/notesegmentation


Additional Python Module Dependencies
-------------------------------------

* Python (>= 2.6.*)
* Cython_
* NumPy
* SciPy

.. _Cython: http://cython.org


Additional Csound Opcode Dependencies
-------------------------------------

* Csound5_

.. _Csound5: http://csounds.com

Installation
------------

**Note:** Currently Metamorph must be built from the source code, but I
hope to provide prebuilt binaries (in particular for the Csound opcodes)
soon.

To build and install the C++ module, from the metamorph root folder run:

::

    $ mkdir build
    $ cd build
    $ cmake ..
    $ make
    $ sudo make install

To build and install the Python module, from the metamorph root folder run:

::

    $ python setup.py build
    $ python setup.py install

To build the Csound opcodes, from the metamorph root folder run:

::

    $ cd csound
    $ mkdir build
    $ cd build
    $ cmake ..
    $ make


Using The Python Module
-----------------------

See the scripts in the examples folder.


Using the Csound Opcodes
------------------------

See ``csound/example.csd`` and ``csound/example_rt.csd``.

To run the Csound examples, Csound must be able to find the Metamorph
opcodes. You can either copy them to your $OPCODEDIR, or point Csound
to them directly at run time, eg:

::

    $ cd csound
    $ csound --opcode-lib=build/libmmop.dylib example.csd
