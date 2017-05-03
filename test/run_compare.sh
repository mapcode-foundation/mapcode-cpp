#!/bin/sh
OPTS="-Wall -Werror -Wextra -Wno-pointer-to-int-cast"

NEW=../utility/mapcode
OLD=$HOME/bin/mapcode-2.3.1
NEWFILE=_new.txt
OLDFILE=_old.txt

OPTS1="--grid 1000000 8"
OPTS2="--random 1000000 8 1234"
OPTS3="--random 1000000 8 11223344"

TEST=$(which $OLD)
if [ "$TEST" = "" ]
then
    echo "No $OLD found on this machine - skipping script..."
    exit 1
fi

echo "!! -------------------------------------------------------------"
echo "Run compare with previous output..."
date
echo "!! -------------------------------------------------------------"

echo ""
echo "Run compare with: -O3"
cd ../mapcodelib
gcc $OPTS -O3 -c mapcoder.c
cd ../utility
gcc $OPTS -O3 mapcode.cpp -lm -o mapcode ../mapcodelib/mapcoder.o
cd ../test

TEST=$(which $NEW)
if [ "$TEST" = "" ]
then
    echo "No $NEW found on this machine - skipping script..."
    exit 1
fi

echo ""
echo "Execute: $NEW $OPTS1"
$NEW | grep version
$NEW $OPTS1 > $NEWFILE

echo ""
echo "Execute: $OLD $OPTS1"
$OLD | grep version
$OLD $OPTS1 > $OLDFILE
diff $NEWFILE $OLDFILE
if [ $? -ne 0 ]
then
    echo "ERROR: Diffs found with:" $OPTS1
    exit 1
else
    rm -f $NEWFILE $OLDFILE
fi

echo ""
echo "Execute: $NEW $OPTS2"
$NEW | grep version
$NEW $OPTS2 > $NEWFILE

echo ""
echo "Execute: $OLD $OPTS2"
$OLD | grep version
$OLD $OPTS2 > $OLDFILE
diff $NEWFILE $OLDFILE
if [ $? -ne 0 ]
then
    echo "ERROR: Diffs found with:" $OPTS2
    exit 1
else
    rm -f $NEWFILE $OLDFILE
fi

echo ""
echo "Execute: $NEW $OPTS3"
$NEW | grep version
$NEW $OPTS3 > $NEWFILE

echo ""
echo "Execute: $OLD $OPTS3"
$OLD | grep version
$OLD $OPTS3 > $OLDFILE
diff $NEWFILE $OLDFILE
if [ $? -ne 0 ]
then
    echo "ERROR: Diffs found with:" $OPTS3
    exit 1
else
    rm -f $NEWFILE $OLDFILE
fi
echo "!! -------------------------------------------------------------"

echo ""
echo "Report in: $REPORT"
