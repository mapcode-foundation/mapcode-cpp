#!/bin/sh
echo "Clean test files..."
if [ -f unittest.c ]
then
    rm -f _*.txt gmon.out
fi
