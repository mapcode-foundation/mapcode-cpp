echo "Run all tests..."
date
echo ""
./run_normal.sh
./run_sanitizer.sh
./run_gprof.sh
./run_valgrind.sh
echo ""
echo "Done"
