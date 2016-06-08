# Mapcode Library for C/C++

Copyright (C) 2014-2015 Stichting Mapcode Foundation (http://www.mapcode.com)

----

**Online documentation: http://mapcode-foundation.github.io/mapcode-cpp/**

**Latest stable release: https://github.com/mapcode-foundation/mapcode-cpp/releases**

# The C/C++ Library: `mapcodelib/`

The directory 'mapcodelib' contains the files:

    mapcoder.h                    <-- Header file with method prototypes and defines for caller.
    mapcoder.c
    basics.h

    mapcode_fastencode.h          <-- include unless FAST_ENCODE is undefined 
    mapcode_fastalpha.h           <-- needed only if FAST_ALPHA is defined
    
    mapcode_countrynames.h        <-- optional array with english territory names, including official names
    mapcode_countrynames_short.h  <-- optional array with english territory names

Together these provide routine to encode/decode Mapcodes.

Documentation, including example snippets of C source code, can be found in

    mapcode_library.doc     (Microsoft Word format).

A unit test can be found in the unittest\ subdirectory.
Compile and run unittest\unittest/c to see if the library performs as expected.

Also see www.mapcode.com for background and reference materials.

Note: this version may be restricted to a particular area of the Earth!
In that case, basics.h will state a version number of the for:

    #define mapcode_cversion "1.2xxx"
    
where "xxx" states the geographical limitation.


# A Real-Life Example, The 'mapcode' Codec Tool: `utility/`

The directory 'utility' contains a Mapcode encoding/decoding utility, as an example
of how to use the library.

To build the original Mapcode tool, execute:

    cd utility
    gcc -O mapcode.cpp -o mapcode

(Or if you need to make sure there are no warnings:)

    gcc -Wall -Wstrict-prototypes -Wshorten-64-to-32 -O mapcode.cpp -o mapcode

For help, simply execute the binary file 'mapcode' without no arguments.
This tool provides a rather extensive command-line interface to encode and
decode Mapcodes.


# Release Notes

* 2.2.2 (NOT RELEASED YET)

    Removed minor warnings from CLion inspections.
    
* 2.2.1

    Solved 1-microdegree gap in a few spots on Earth, noticable now extreme precision is possible.

* 2.1.5

    Added stricter unit tests.

* 2.1.4

    Added `maxErrorInMetersz` to API.
    
    Moved recode logic into decoder.
    
    Adjusted unit test.

* 2.1.3

    Added useful routine `DistanceInMeters` to API.

* 2.1.2

    Rewrote fraction floating points to integer arithmetic.

* 2.1.1

    Added `unittest/` which can be compiled and executed to check
    if the mapcode library performs as expected.

* 2.1.0

    Fixes floating point inaccuracy (prevent `encode(decode(M)) != M`).

* 2.0.3

    Added `#define FAST_ALPHA` for faster alphacode string recognition.

* 2.0.2.4

    Added point structs to source.

* 2.0.2.3

    Cleaned up source.

* 2.0.2.2

    Cleaned up source.

* 2.0.2.1

    Cleaned up source, removed legacy code.
    
    Renamed `dividemaps.h` to `mapcode_fast_encode.h`.
     
    Improved `convertToRoman` and `convertToAlphabet` to trim and skip leading territory.

    Rolled back 2.0.2 fix for letters "i" and "o" in Hindi, Bengali, Tibetan and Gurmukhi.

* 2.0.2

    Speed-up of encoding (4 times faster than 2.0.1).

    Added optional `mapcode_countrynames.h` (with the English short names, formal names, 
    and name variants of all territories) and `mapcode_countrynames_short.h` (with only 
    the short English names and name variants.

    Stricter processing of high-precision letters (now fails on illegal high-precision 
    letters, which includes the letter Z).

    Fix for letters "i" and "o" in Hindi, Bengali, Tibetan and Gurmukhi;

    Fix for some unrecognized aliases in `convertTerritoryIsoNameToCode()`.
    
* 2.0.1

    Removed compiler warnings. 
    
* 2.0.0

    Support for high-precision mapcodes, with up to 8 character extensions (up to 10 micron accuracy).
    
    Fixes to the data rectangles (primarily intended for ISO proposal), see Word document for details.

* 1.50.1

    Bug fix for state IN-DD (in India).
    
    If the executable name contains "debug", the self checking mechanism is activated. Note, however, that
    this feature is under construction and produces false negative for certain decodes. 
    
* 1.50

    Major release. This version is not backwards compatible with mapcode 1.4x: is has dropped support for
    Antartica AT0-8 codes and has a changed (improved) way of dealing with the Greek alphabet.

    Added 22-chararcter post-processing of all-digit mapcodes for the Greek alphabet.

    Retired legacy aliases EAZ and SKM, AU-QL, AU-TS, AU-NI and AU-JB.

    Retired legacy Antarctica claims AT0 through AT8.
    
* 1.41.1

    Bugfix for “utility/mapcode” utility (-d option was case sensitive).

* 1.41

    Added the India state Telangana (IN-TG), until 2014 a region in Adhra Pradesh.

    Fixed a problem with the "extraDigits" parameter of the “utility/mapcode" utility (maximum digits is 2, not 3).

* 1.40

    Added extraDigits parameter to allow high-precision mapcodes to be generated by the “utility/mapcode” utility.

* 1.33

    Fix to not remove valid results just across the edge of a territory.
    Improved interface readability and renamed methods to more readable forms.

* 1.32

    Added encodeLatLonToSingleMapcode(); fixed 1.29 so no country-wide alternative
    is produced in edge cases; prevent FIJI failing to decode at exactly 180 degrees.

* 1.31

    Added compareWithMapcodeFormat().

* 1.30

    IUpdated the documentation and extended it with examples and suggestions.

* 1.29

    Also generate country-wide alternative mapcodes for states.

* 1.28

    Bug fix for the needless generation of 7-letter alternatives to short mapcodes
    in large states in India.

* 1.27

    Improved (faster) implementation of the function isInArea.

* 1.26

    Added alias OD ("Odisha") for indian state OR ("Orissa").

* 1.25

    Initial release to the public domain.

# License

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

