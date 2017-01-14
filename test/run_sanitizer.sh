#!/bin/sh
OPTS="-Wall -Werror -Wextra -Wpedantic -Wno-pointer-to-int-cast"
LIB="../mapcodelib/mapcoder.o"

export ASAN_OPTIONS=debug=true:strict_string_checks=1:detect_stack_use_after_return=true:detect_invalid_pointer_pairs=99999:detect_container_overflow=true:detect_odr_violation=2:check_initialization_order=true:strict_init_order=true

TEST=`which clang`
if [ "$TEST" = "" ]
then
    echo "No clang found on this machine - skipping script..."
    exit 1
fi

echo "!! -------------------------------------------------------------"
echo "Run address sanitizer..."
date
echo "!! -------------------------------------------------------------"

# No optimize
echo ""
echo "Run address sanitizer with: -O0"
cd ../mapcodelib
gcc $OPTS -O0 -DDEBUG -c mapcoder.c
cd ../test
gcc $OPTS -O0 -DDEBUG unittest.c -lm -lpthread -fsanitize=address -o unittest $LIB
./unittest
echo "!! -------------------------------------------------------------"

# Optimize 2
echo ""
echo "Run address sanitizer with: -O2"
cd ../mapcodelib
gcc $OPTS -O2 -c mapcoder.c
cd ../test
gcc $OPTS -O2 unittest.c -lm -lpthread -fsanitize=address -o unittest $LIB
./unittest
echo "!! -------------------------------------------------------------"

# Optimize 3
echo ""
echo "Run address sanitizer with: -O3"
cd ../mapcodelib
gcc $OPTS -O3 -c mapcoder.c
cd ../test
gcc $OPTS -O3 unittest.c -lm -lpthread -fsanitize=address -o unittest $LIB
./unittest
echo "!! -------------------------------------------------------------"

echo ""
echo "Report in: $REPORT"
