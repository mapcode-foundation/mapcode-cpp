#!/bin/sh
echo "Clean test files..."
if [ -f unittest.c ]
then
    rm -f _*.txt gmon.out *.gcda *.gcno *.gcov ../mapcodelib/*.gcda ../mapcodelib/*.gcno ../mapcodelib/*.gcov
fi
