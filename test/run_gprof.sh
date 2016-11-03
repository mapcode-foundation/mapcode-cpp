#!/bin/sh
OPTS="-Wall -Werror -Wextra -Wno-pointer-to-int-cast"
LIB="../mapcodelib/mapcoder.o"

TEST=`which gprof`
if [ "$TEST" = "" ]
then
    echo "No gprof found on this machine - skipping script..."
    exit 1
fi

echo "!! -------------------------------------------------------------"
echo "Run gprof profiler..."
date
echo "!! -------------------------------------------------------------"

echo ""
echo "Run gprof with: -O0"
cd ../mapcodelib
gcc $OPTS -g -O0 -c mapcoder.c -pg
cd ../test
gcc $OPTS -g -O0 unittest.c -lm -lpthread -o unittest $LIB -pg
./unittest
gprof ./unittest
echo "!! -------------------------------------------------------------"

echo ""
echo "Run gprof with: -O3"
cd ../mapcodelib
gcc $OPTS -g -O3 -c mapcoder.c -pg
cd ../test
gcc $OPTS -g -O3 unittest.c -lm -lpthread -o unittest $LIB -pg
./unittest
gprof ./unittest
echo "!! -------------------------------------------------------------"

echo ""
echo "Report in: $REPORT"
