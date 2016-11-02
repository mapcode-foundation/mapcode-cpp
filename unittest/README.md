# Unit Tests

To build the unit tests, execute:

    cd ../mapcodelib
    gcc -DDEBUG -O -c mapcoder.c
    cd ../unittest
    gcc -DDEBUG -O -DDEBUG unittest.c -lm -lpthread -o unittest ../mapcodelib/mapcoder.o

To execute the tests, simply execute:

    ./unittest

## Using `valgrind`  to Detect Memory Leaks

Compile and run as follows to use `valgrind` (http://valgrind.org) to detect memory leaks:

    cd ../mapcodelib
    gcc -g -O0 -c mapcoder.c
    cd ../unittest
    gcc -g -O0 unittest.c -lm -lpthread -o unittest ../mapcodelib/mapcoder.o
    valgrind --leak-check=yes ./unittest 

## Using the Address Sanitizer (from CLang) to Detect Memory Errors

Or, add `-fsanitize=address` to run the address sanitizer:

    cd ../mapcodelib
    gcc -O -c mapcoder.c
    cd ../unittest
    gcc -O unittest.c -lm -lpthread -fsanitize=address -o unittest ../mapcodelib/mapcoder.o

And add the environment variable `ASAN_OPTIONS` to your shell:

    ASAN_OPTIONS=debug=true:strict_string_checks=1:detect_stack_use_after_return=true:
        detect_invalid_pointer_pairs=99999:detect_container_overflow=true:
        detect_odr_violation=2:check_initialization_order=true:strict_init_order=true

## Using `gprof` to Profile the Library

Compile and run as follows to use `gprof` to profile the library:

    cd ../mapcodelib
    gcc -g -O0 -c mapcoder.c -pg
    cd ../unittest
    gcc -g -O0 unittest.c -lm -lpthread -o unittest ../mapcodelib/mapcoder.o -pg

## Using `gcov` to Show Test Coverage

Compile and run as follows to use `gcov` to show test coverage for the libray:

    cd ../mapcodelib
    gcc -fprofile-arcs -ftest-coverage -O0 -c mapcoder.c 
    cd ../unittest
    gcc  -fprofile-arcs -ftest-coverage -O0 unittest.c -lm -lpthread -o unittest ../mapcodelib/mapcoder.o -pg
    ./unittest
    cd ../mapcodelib
    gcov mapcoder.c
    cd ../unittest
    gcov unittest.c

The test coverage reports are the `*.gcov` text files.

## Using Microsoft Visual C++

If you use **Microsoft Visual C++**, you may need to add the following defines to your preprocessor
settings:

    NO_POSIX_THREADS
    _CRT_SECURE_NO_WARNINGS
    _CRT_NONSTDC_NO_DEPRECATE


