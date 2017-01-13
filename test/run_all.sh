#!/bin/sh
OUT=_report.txt
echo "Run all tests..." | tee $OUT
date | tee -a $OUT
echo "" | tee -a $OUT
./run_compare.sh | tee -a $OUT
./run_normal.sh | tee -a $OUT
./run_sanitizer.sh | tee -a $OUT
./run_valgrind.sh | tee -a $OUT
./run_gcov.sh | tee -a $OUT
./run_gprof.sh | tee -a $OUT
echo ""  | tee -a $OUT
echo "Done" | tee -a $OUT
