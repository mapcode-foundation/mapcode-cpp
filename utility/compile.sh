#!/bin/sh
echo "Compiling..."
cd ../mapcodelib
gcc -O3 -c mapcoder.c
cd ../utility
gcc -O3 mapcode.cpp -lm -o mapcode ../mapcodelib/mapcoder.o

