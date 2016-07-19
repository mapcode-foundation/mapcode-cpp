# Unit Tests

To build the unit tests, execute:

    cd unittest
    gcc -O unittest.c -o unittest

(Or if you need to make sure there are no warnings:)

    gcc -Wall -Wstrict-prototypes -Wshorten-64-to-32 -O unittest.c -o unittest

To execute the tests, simply execute:

    ./unittest
