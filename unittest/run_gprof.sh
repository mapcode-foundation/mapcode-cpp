#!/bin/sh
REPORT=_report_gprof.txt
OPTS="-Wall -Werror -Wno-pointer-to-int-cast"
LIB="../mapcodelib/mapcoder.o"

TEST=`which gprof`
if [ "$TEST" = "" ]
then
    echo "No gprof found on this machine - skipping script..."
    exit 1
fi

echo "!! -------------------------------------------------------------" | tee -a $REPORT
echo "Run gprof profiler..." | tee $REPORT
date | tee -a $REPORT
echo "!! -------------------------------------------------------------" | tee -a $REPORT

echo "" | tee -a $REPORT
echo "Run with: -O0" | tee -a $REPORT
cd ../mapcodelib
gcc $OPTS -g -O0 -c mapcoder.c -pg
cd ../unittest
gcc $OPTS -g -O0 unittest.c -lm -lpthread -o unittest $LIB -pg
./unittest | tee -a $REPORT
gprof ./unittest | tee -a $REPORT
echo "!! -------------------------------------------------------------" | tee -a $REPORT

echo "" | tee -a $REPORT
echo "Run with: -O3" | tee -a $REPORT
cd ../mapcodelib
gcc $OPTS -g -O3 -c mapcoder.c -pg
cd ../unittest
gcc $OPTS -g -O3 unittest.c -lm -lpthread -o unittest $LIB -pg
./unittest | tee -a $REPORT
gprof ./unittest | tee -a $REPORT
echo "!! -------------------------------------------------------------" | tee -a $REPORT

echo "" | tee -a $REPORT
echo "Report in: $REPORT"
