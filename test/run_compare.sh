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
NEW=../mapcode-2.5.6
OLD=../mapcode-2.5.2
NEWFILE=_new.txt
OLDFILE=_old.txt

OPTS1="--grid 1000000 8"
OPTS2="--random 1000000 8 1234"
OPTS3="--random 1000000 8 11223344"

comparison_error() {
    echo "If you want to run the comparison test script, please create symbolic links"
    echo "to two version of the mapcode utility:"
    echo "  $OLD"
    echo "  $NEW"
    echo "These must be placed in the root directory of this project."
    exit
}

TEST=$(which $OLD)
if [ "$TEST" = "" ]
then
    echo "No $OLD found on this machine - skipping script..."
    comparison_error
    exit 1
fi

echo "!! -------------------------------------------------------------"
echo "Run compare with previous output..."
date
echo "!! -------------------------------------------------------------"

echo ""
echo "Run compare with: -O3"
cd ../mapcodelib
gcc $OPTS -O3 -c mapcoder.c
cd ../utility
gcc $OPTS -O3 mapcode.cpp -lm -o mapcode ../mapcodelib/mapcoder.o
cd ../test

TEST=$(which $NEW)
if [ "$TEST" = "" ]
then
    echo "No $NEW found on this machine - skipping script..."
    comparison_error
    exit 1
fi

echo ""
echo "Execute: $NEW $OPTS1"
$NEW | grep version
$NEW $OPTS1 > $NEWFILE

echo ""
echo "Execute: $OLD $OPTS1"
$OLD | grep version
$OLD $OPTS1 > $OLDFILE
diff $NEWFILE $OLDFILE
if [ $? -ne 0 ]
then
    echo "ERROR: Diffs found with:" $OPTS1
    exit 1
else
    rm -f $NEWFILE $OLDFILE
fi

echo ""
echo "Execute: $NEW $OPTS2"
$NEW | grep version
$NEW $OPTS2 > $NEWFILE

echo ""
echo "Execute: $OLD $OPTS2"
$OLD | grep version
$OLD $OPTS2 > $OLDFILE
diff $NEWFILE $OLDFILE
if [ $? -ne 0 ]
then
    echo "ERROR: Diffs found with:" $OPTS2
    exit 1
else
    rm -f $NEWFILE $OLDFILE
fi

echo ""
echo "Execute: $NEW $OPTS3"
$NEW | grep version
$NEW $OPTS3 > $NEWFILE

echo ""
echo "Execute: $OLD $OPTS3"
$OLD | grep version
$OLD $OPTS3 > $OLDFILE
diff $NEWFILE $OLDFILE
if [ $? -ne 0 ]
then
    echo "ERROR: Diffs found with:" $OPTS3
    exit 1
else
    rm -f $NEWFILE $OLDFILE
fi
echo "!! -------------------------------------------------------------"
