# Unit Tests

To build the unit tests, execute:

    cd unittest
    gcc -O unittest.c -lm -lpthread -o unittest

(Or if you need to make sure there are no warnings:)

    gcc -Wall -O unittest.c -lm -lpthread -o unittest

To execute the tests, simply execute:

    ./unittest

If you use **Microsoft Visual C++**, you may need to add the following defines to your preprocessor
settings:

    NO_POSIX_THREADS
    _CRT_SECURE_NO_WARNINGS
    _CRT_NONSTDC_NO_DEPRECATE


