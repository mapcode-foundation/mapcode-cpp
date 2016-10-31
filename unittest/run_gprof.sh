REPORT=report_gprof.txt

echo "Run gprof profiler..." | tee $REPORT
date | tee -a $REPORT

echo "" | tee -a $REPORT
echo "Run with: -O0" | tee -a $REPORT
cd ../mapcodelib
gcc -g -O0 -c mapcoder.c -pg
cd ../unittest
gcc -g -O0 unittest.c -lm -lpthread -o unittest ../mapcodelib/mapcoder.o -pg
./unittest
gprof ./unittest | tee -a $REPORT
echo "----------------" | tee -a $REPORT

echo "" | tee -a $REPORT
echo "Run with: -O3" | tee -a $REPORT
cd ../mapcodelib
gcc -g -O3 -c mapcoder.c -pg
cd ../unittest
gcc -g -O3 unittest.c -lm -lpthread -o unittest ../mapcodelib/mapcoder.o -pg
./unittest
gprof ./unittest | tee -a $REPORT
echo "----------------" | tee -a $REPORT

echo "" | tee -a $REPORT
echo "Report in: $REPORT"
