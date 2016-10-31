REPORT=report_sanitizer.txt
export ASAN_OPTIONS=debug=true:strict_string_checks=1:detect_stack_use_after_return=true:detect_invalid_pointer_pairs=99999:detect_container_overflow=true:detect_odr_violation=2:check_initialization_order=true:strict_init_order=true

echo "Run address sanitizer..." | tee $REPORT
date | tee -a $REPORT

# No optimize
echo "" | tee -a $REPORT
echo "Run with: -O0" | tee -a $REPORT
cd ../mapcodelib
gcc -O0 -DDEBUG -c mapcoder.c
cd ../unittest
gcc -O0 -DDEBUG unittest.c -lm -lpthread -fsanitize=address -o unittest 
./unittest | tee -a $REPORT
echo "----------------" | tee -a $REPORT sanitize.txt

# Optimize 1
echo "" | tee -a $REPORT
echo "Run with: -O1" | tee -a $REPORT sanitize.txt
cd ../mapcodelib
gcc -O1 -c mapcoder.c
cd ../unittest
gcc -O1 unittest.c -lm -lpthread -fsanitize=address -o unittest 
./unittest | tee -a $REPORT
echo "----------------" | tee -a $REPORT sanitize.txt

# Optimize 2
echo "" | tee -a $REPORT
echo "Run with: -O2" | tee -a $REPORT sanitize.txt
cd ../mapcodelib
gcc -O2 -c mapcoder.c
cd ../unittest
gcc -O2 unittest.c -lm -lpthread -fsanitize=address -o unittest 
./unittest | tee -a $REPORT
echo "----------------" | tee -a $REPORT sanitize.txt

# Optimize 3
echo "" | tee -a $REPORT
echo "Run with: -O3" | tee -a $REPORT sanitize.txt
cd ../mapcodelib
gcc -O3 -c mapcoder.c
cd ../unittest
gcc -O3 unittest.c -lm -lpthread -fsanitize=address -o unittest 
./unittest | tee -a $REPORT
echo "----------------" | tee -a $REPORT sanitize.txt

echo "" | tee -a $REPORT
echo "Report in: $REPORT"
