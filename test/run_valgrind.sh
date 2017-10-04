#!/bin/sh
OPTS="-Wall -Wextra -Wno-pointer-to-int-cast"
LIB="../mapcodelib/mapcoder.o"

TEST=$(which valgrind)
if [ "$TEST" = "" ]
then
    echo "No valgrind found on this machine - skipping script..."
    exit 1
fi

echo "!! -------------------------------------------------------------"
echo "Run valgrind"
date
echo "!! -------------------------------------------------------------"

echo ""
echo "Run valgrind with: -O0"
cd ../mapcodelib
gcc $OPTS -g -O0 -c mapcoder.c
cd ../test
gcc $OPTS -g -O0 unittest.c -lm -lpthread -o unittest $LIB
valgrind --leak-check=yes ./unittest
echo "!! -------------------------------------------------------------"
