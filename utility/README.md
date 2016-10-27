# The 'mapcode' Codec Tool

This directory contains a Mapcode encoding/decoding utility.
To build the original Mapcode tool, execute:

    cd ../mapcodelib
    gcc -c mapcoder.c
    cd ../utility
    gcc -O mapcode.cpp -lm -lpthread -o mapcode ../mapcodelib/mapcoder.o

For help, simply execute:

    ./mapcode

As an example, type:

    ./mapcode --decode NLD XX.XX

This tool provides a rather extensive command-line interface to encode and
decode Mapcodes.

If you use **Microsoft Visual C++**, you may need to add the following defines to your preprocessor
settings:

    NO_POSIX_THREADS
    _CRT_SECURE_NO_WARNINGS
    _CRT_NONSTDC_NO_DEPRECATE


