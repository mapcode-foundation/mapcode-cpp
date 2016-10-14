# Unit Tests

To build the unit tests, execute:

    cd unittest
    gcc -O unittest.c -lm -lpthread -o unittest

(Or if you need to make sure there are no warnings:)

    gcc -Wall -O unittest.c -lm -lpthread -o unittest

To execute the tests, simply execute:

    ./unittest
