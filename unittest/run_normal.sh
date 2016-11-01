#!/bin/sh
REPORT=report_normal.txt
OPTS="-Wall -Werror -Wno-pointer-to-int-cast"

echo "!! -------------------------------------------------------------" | tee -a $REPORT
echo "Run normal..." | tee $REPORT
date | tee -a $REPORT
echo "!! -------------------------------------------------------------" | tee -a $REPORT

echo "" | tee -a $REPORT
echo "Run with: -O0" | tee -a $REPORT
cd ../mapcodelib
gcc $OPTS -O0 -DDEBUG -c mapcoder.c
cd ../unittest
gcc $OPTS -O0 -DDEBUG unittest.c -lm -lpthread -o unittest ../mapcodelib/mapcoder.o
./unittest | tee -a $REPORT
echo "!! -------------------------------------------------------------" | tee -a $REPORT

echo "" | tee -a $REPORT
echo "Run with: -O3" | tee -a $REPORT
cd ../mapcodelib
gcc $OPTS -O3 -c mapcoder.c
cd ../unittest
gcc $OPTS -O3 unittest.c -lm -lpthread -o unittest ../mapcodelib/mapcoder.o
./unittest | tee -a $REPORT
echo "!! -------------------------------------------------------------" | tee -a $REPORT

echo "" | tee -a $REPORT
echo "Report in: $REPORT"
