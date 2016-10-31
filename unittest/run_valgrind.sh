REPORT=report_valgrind.txt
OPTS="-Wall -Werror -Wno-pointer-to-int-cast"
LIB="../mapcodelib/mapcoder.o"

echo "Run valgrind" | tee $REPORT
date | tee -a $REPORT

TEST=`which valgrind`
if [ "$TEST" = "" ]
then
    echo "No valgrind found on this machine - skipping script..."
    exit 1
fi

echo "" | tee -a $REPORT
echo "Run with: -O0" | tee -a report_valgrind.txt
cd ../mapcodelib
gcc $OPTS -g -O0 -DDEBUG -c mapcoder.c
cd ../unittest
gcc $OPTS -g -O0 -DDEBUG unittest.c -lm -lpthread -o unittest $LIB
valgrind --leak-check=yes ./unittest | tee -a $REPORT
echo "----------------" tee -a $REPORT

echo "" | tee -a $REPORT
echo "Run with: -O3" | tee -a report_valgrind.txt
cd ../mapcodelib
gcc $OPTS -g -O3 -c mapcoder.c
cd ../unittest
gcc $OPTS -g -O3 unittest.c -lm -lpthread -o unittest $LIB
valgrind --leak-check=yes ./unittest | tee -a $REPORT
echo "----------------" tee -a $REPORT

echo "" tee -a $REPORT
echo "Report in: $REPORT"
