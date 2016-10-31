REPORT=report_valgrind.txt

echo "Run valgrind" | tee $REPORT

echo "" | tee -a $REPORT
echo "Run with: -O0" | tee -a report_valgrind.txt
cd ../mapcodelib
gcc -g -O0 -DDEBUG -c mapcoder.c
cd ../unittest
gcc -g -O0 -DDEBUG unittest.c -lm -lpthread -o unittest ../mapcodelib/mapcoder.o
valgrind --leak-check=yes ./unittest tee -a $REPORT
echo "----------------" tee -a $REPORT

echo "" | tee -a $REPORT
echo "Run with: -O3" | tee -a report_valgrind.txt
cd ../mapcodelib
gcc -g -O3 -c mapcoder.c
cd ../unittest
gcc -g -O3 unittest.c -lm -lpthread -o unittest ../mapcodelib/mapcoder.o
valgrind --leak-check=yes ./unittest tee -a $REPORT
echo "----------------" tee -a $REPORT

echo "" tee -a $REPORT
echo "Report in: $REPORT"
