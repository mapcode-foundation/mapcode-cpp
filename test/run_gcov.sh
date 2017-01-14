#!/bin/sh
OPTS="-Wall -Werror -Wextra -Wno-pointer-to-int-cast -fprofile-arcs -ftest-coverage"
LIB="../mapcodelib/mapcoder.o"

TEST=`which gcov`
if [ "$TEST" = "" ]
then
    echo "No gcov found on this machine - skipping script..."
    exit 1
fi

echo "!! -------------------------------------------------------------"
echo "Run gcov unittest.coverage..."
date
echo "!! -------------------------------------------------------------"

echo ""
echo "Run gcov with: -O0"
cd ../mapcodelib
gcc $OPTS -O0 -c mapcoder.c
cd ../test
gcc $OPTS -O0 unittest.c -lm -lpthread -o unittest $LIB
./unittest
cd ../mapcodelib
gcov mapcoder.c
cd ../test
gcov unittest.c
echo "!! -------------------------------------------------------------"
echo "!! Coverage reports in: *.gcov files"
echo "!! -------------------------------------------------------------"

echo ""
echo "Report in: $REPORT"
