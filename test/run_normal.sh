#!/bin/sh
OPTS="-Wall -Werror -Wextra -Wpedantic -Wno-pointer-to-int-cast"

echo "!! -------------------------------------------------------------"
echo "Run normal..."
date
echo "!! -------------------------------------------------------------"

echo ""
echo "Run normal with: -O0"
cd ../mapcodelib
gcc $OPTS -O0 -DDEBUG -c mapcoder.c
cd ../test
gcc $OPTS -O0 -DDEBUG unittest.c -lm -lpthread -o unittest ../mapcodelib/mapcoder.o
./unittest
echo "!! -------------------------------------------------------------"

echo ""
echo "Run normal with: -O3"
cd ../mapcodelib
gcc $OPTS -O3 -c mapcoder.c
cd ../test
gcc $OPTS -O3 unittest.c -lm -lpthread -o unittest ../mapcodelib/mapcoder.o
./unittest
echo "!! -------------------------------------------------------------"

echo ""
echo "Report in: $REPORT"
