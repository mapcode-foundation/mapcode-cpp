#!/bin/sh
OPTS="-Wall -Werror -Wno-pointer-to-int-cast"
LIB="../mapcodelib/mapcoder.o"

TEST=`which valgrind`
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
echo "Run with: -O0"
cd ../mapcodelib
gcc $OPTS -g -O0 -c mapcoder.c
cd ../unittest
gcc $OPTS -g -O0 unittest.c -lm -lpthread -o unittest $LIB
valgrind --leak-check=yes ./unittest
echo "!! -------------------------------------------------------------"

echo "" tee -a $REPORT
echo "Report in: $REPORT"
