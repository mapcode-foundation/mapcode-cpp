# The 'mapcode' Codec Tool

This directory contains a Mapcode encoding/decoding utility.
To build the original Mapcode tool, execute:

  gcc -O mapcode.cpp -o mapcode

(Or if you need to make sure there are no warnings:)

  gcc -Wall -Wstrict-prototypes -Wshorten-64-to-32 -O mapcode.cpp -o mapcode

For help, simply execute the binary file 'mapcode' without no arguments.
This tool provides a rather extensive command-line interface to encode and
decode Mapcodes.

