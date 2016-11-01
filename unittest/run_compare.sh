#!/bin/sh
REPORT=_report_compare.txt
OPTS="-Wall -Werror -Wno-pointer-to-int-cast"

NEW=../utility/mapcode
OLD=$HOME/bin/mapcode-2.3.1

OPTS1="--grid 1000000 8"
OPTS2="--random 1000000 8 1234"
OPTS3="--random 1000000 8 11223344"

TEST=`which $OLD`
if [ "$TEST" = "" ]
then
    echo "No $OLD found on this machine - skipping script..."
    exit 1
fi

echo "!! -------------------------------------------------------------" | tee -a $REPORT
echo "Run compare with previous output..." | tee $REPORT
date | tee -a $REPORT
echo "!! -------------------------------------------------------------" | tee -a $REPORT

echo "" | tee -a $REPORT
echo "Run with: -O3" | tee -a $REPORT
cd ../mapcodelib
gcc $OPTS -O3 -c mapcoder.c
cd ../utility
gcc $OPTS -O3 mapcode.cpp -lm -o mapcode ../mapcodelib/mapcoder.o
cd ../unittest

TEST=`which $NEW`
if [ "$TEST" = "" ]
then
    echo "No $NEW found on this machine - skipping script..."
    exit 1
fi

echo ""
echo "Execute: $NEW $OPTS1" | tee -a $REPORT
$NEW | grep version | tee -a $REPORT
$NEW $OPTS1 > _new_1.txt | tee -a $REPORT

echo ""
echo "Execute: $OLD $OPTS1" | tee -a $REPORT
$OLD | grep version | tee -a $REPORT
$OLD $OPTS1 > _old_1.txt | tee -a $REPORT
diff _new_1.txt _old_1.txt | tee -a $REPORT
if [ $? -ne 0 ]
then
    echo "ERROR: Diffs found with: " $OPTS1 | tee -a $REPORT
fi

echo ""
echo "Execute: $NEW $OPTS2" | tee -a $REPORT
$NEW | grep version | tee -a $REPORT
$NEW $OPTS2 > _new_2.txt | tee -a $REPORT

echo ""
echo "Execute: $OLD $OPTS2" | tee -a $REPORT
$OLD | grep version | tee -a $REPORT
$OLD $OPTS2 > _old_2.txt | tee -a $REPORT
diff _new_2.txt _old_2.txt | tee -a $REPORT
if [ $? -ne 0 ]
then
    echo "ERROR: Diffs found with: " $OPTS2 | tee -a $REPORT
fi

echo ""
echo "Execute: $NEW $OPTS3" | tee -a $REPORT
$NEW | grep version | tee -a $REPORT
$NEW $OPTS3 > _new_3.txt | tee -a $REPORT

echo ""
echo "Execute: $OLD $OPTS3" | tee -a $REPORT
$OLD | grep version | tee -a $REPORT
$OLD $OPTS3 > _old_3.txt | tee -a $REPORT
diff _new_3.txt _old_3.txt | tee -a $REPORT
if [ $? -ne 0 ]
then
    echo "ERROR: Diffs found with: " $OPTS3 | tee -a $REPORT
fi

echo "!! -------------------------------------------------------------" | tee -a $REPORT

echo "" | tee -a $REPORT
echo "Report in: $REPORT"
