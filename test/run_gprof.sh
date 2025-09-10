#!/bin/sh
#
# Copyright (C) 2014-2025 Stichting Mapcode Foundation (http://www.mapcode.com)
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

OPTS="$(cat ./no_warnings.env)"
LIB="../mapcodelib/mapcoder.o"

TEST=$(which gprof)
if [ "$TEST" = "" ]
then
    echo "No gprof found on this machine - skipping script..."
    exit 1
fi

echo "!! -------------------------------------------------------------"
echo "Run gprof profiler..."
date
echo "!! -------------------------------------------------------------"

echo ""
echo "Run gprof with: -O0"
cd ../mapcodelib
gcc $OPTS -g -O0 -c mapcoder.c -pg
cd ../test
gcc $OPTS -g -O0 unittest.c -lm -lpthread -o unittest $LIB -pg
./unittest
gprof ./unittest
echo "!! -------------------------------------------------------------"

echo ""
echo "Run gprof with: -O3"
cd ../mapcodelib
gcc $OPTS -g -O3 -c mapcoder.c -pg
cd ../test
gcc $OPTS -g -O3 unittest.c -lm -lpthread -o unittest $LIB -pg
./unittest
gprof ./unittest
echo "!! -------------------------------------------------------------"
