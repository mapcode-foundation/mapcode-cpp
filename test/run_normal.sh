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

OPTS="-Wall -Wextra -Wno-pointer-to-int-cast"

echo "!! -------------------------------------------------------------"
echo "Run normal..."
date
echo "!! -------------------------------------------------------------"

echo ""
echo "Run normal with: -O0"
cd ../mapcodelib
gcc $OPTS -O0 -DDEBUG -c mapcoder.c
cd ../test
gcc $OPTS -O0 -DDEBUG unittest.c -lm -lpthread -o unittest ../mapcodelib/mapcoder.o
./unittest
echo "!! -------------------------------------------------------------"

echo ""
echo "Run normal with: -O3"
cd ../mapcodelib
gcc $OPTS -O3 -c mapcoder.c
cd ../test
gcc $OPTS -O3 unittest.c -lm -lpthread -o unittest ../mapcodelib/mapcoder.o
./unittest
echo "!! -------------------------------------------------------------"
