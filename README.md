# Mapcode Library for C/C++

[![License](http://img.shields.io/badge/license-APACHE2-blue.svg)]()

**Copyright (C) 2014-2016 Stichting Mapcode Foundation (http://www.mapcode.com)**


**Online documentation: http://mapcode-foundation.github.io/mapcode-cpp/**

**Latest stable release: https://github.com/mapcode-foundation/mapcode-cpp/releases**

## The C/C++ Library: `mapcodelib/`

The directory 'mapcodelib' contains the files:

    mapcodelib/
      mapcoder.h                    <-- Header file with method prototypes and defines for caller.
      mapcoder.c                    <-- Implementation of mapcode routines.
      internal_*.h                  <-- Internal implementataion details for library.

      mapcode_alphabets.h           <-- Enumeration of supported alphabets (or scripts).
      mapcode_territories.h         <-- Enumeration of supported territories.
      
      mapcode_legacy.h              <-- Courtesy support for legacy calls, may be deprecated in future.

Together these provide routine to encode/decode Mapcodes.

Documentation, including example snippets of C source code, can be found in

    docs/
      mapcode_library_c.pdf         <-- PDF format.
      mapcode_library_c.docx        <-- Microsoft Word format.

A unit test can be found in the `test` subdirectory. Compile and run `unittest.c` to see 
if the library performs as expected:

    cd mapcodelib
    gcc -O -c mapcoder.c
    cd ../test
    gcc -O unittest.c -lm -lpthread -o unittest ../mapcodelib/mapcoder.o
    ./unittest

Check the `README.md` in directory `test` for more information.

Also see http://www.mapcode.com for background and reference materials.


## A Real-Life Example, The 'mapcode' Codec Tool: `utility/`

The directory 'utility' contains a Mapcode encoding/decoding utility, as an example
of how to use the library.

To build the original Mapcode tool, execute:

    cd mapcodelib
    gcc -O -c mapcoder.c
    cd ../utility
    gcc -O mapcode.cpp -o mapcode ../mapcodelib/mapcoder.o

For help, simply execute the binary file 'mapcode' without no arguments.
This tool provides a rather extensive command-line interface to encode and
decode Mapcodes. 

This produces the following help text:

    MAPCODE (version 2.5.2)
    Copyright (C) 2014-2016 Stichting Mapcode Foundation
    
    Usage:
        mapcode [-d| --decode] <default-territory> <mapcode> [<mapcode> ...]
    
           Decode a mapcode to a lat/lon. The default territory code is used if
           the mapcode is a shorthand local code
    
        mapcode [-e[0-8] | --encode[0-8]] <lat:-90..90> <lon:-180..180> [territory]>
    
           Encode a lat/lon to a mapcode. If the territory code is specified, the
           encoding will only succeeed if the lat/lon is located in the territory.
           You can specify the number of additional digits, 0, 1 or 2 (default 0)
           for high-precision mapcodes.
    
        mapcode [-t | --territories]
    
           Create a full set of territories in CSV format.
    
        mapcode [-a | --alphabets]
    
           Create a full set of alphabet tests in CSV format.
    
        mapcode [-b[XYZ] | --boundaries[XYZ]] [<extraDigits>]
        mapcode [-g[XYZ] | --grid[XYZ]]   <nrOfPoints> [<extraDigits>]
        mapcode [-r[XYZ] | --random[XYZ]] <nrOfPoints> [<extraDigits>] [<seed>]
    
           Create a test set of lat/lon pairs based on the mapcode boundaries database
           as a fixed 3D grid or random uniformly distributed set of lat/lons with their
           (x, y, z) coordinates and all mapcode aliases.
    
           <extraDigits>: 0-8; specifies additional accuracy, use 0 for standard.
           <seed> is an optional random seed, use 0 for arbitrary>.
           (You may wish to specify a specific seed to regenerate test cases).
    
           The output format is:
               <number-of-aliases> <lat-deg> <lon-deg> [<x> <y> <z>]
               <territory> <mapcode>      (repeated 'number-of-aliases' times)
                                          (empty lines and next record)
           Ranges:
               number-of-aliases : >= 1
               lat-deg, lon-deg  : [-90..90], [-180..180]
               x, y, z           : [-1..1]
    
           The lat/lon pairs will be distributed over the 3D surface of the Earth
           and the (x, y, z) coordinates are placed on a sphere with radius 1.
           The (x, y, z) coordinates are primarily meant for visualization of the data set.
    
           Example:
           mapcode -g    100 : produces a grid of 100 points as lat/lon pairs
           mapcode -gXYZ 100 : produces a grid of 100 points as (x, y, z) sphere coordinates)
    
           Notes on the use of stdout and stderr:
           stdout: used for outputting 3D Point data; stderr: used for statistics.
           You can redirect stdout to a destination file, while stderr will show progress.
    
           The result code is 0 when no error occurred, 1 if an input error occurred and 2
           if an internal error occurred.
       
## Compile Options for Microsoft Visual C++

If you use **Microsoft Visual C++**, you may need to add the following compiler directives to your build:

    -DNO_POSIX_THREADS
    -D_CRT_SECURE_NO_WARNINGS
    -D_CRT_NONSTDC_NO_DEPRECATE

## Reducing the Footprint of the Mapcode Library

The Mapcode C/C++ Library has includes a number of fixed data tables, which increase its footprint.
You may not require all of this data, so we've added some options for you to be able to reduce its
footprint, for example for embedded applications. 

You can specify the define `MAPCODE_NO_SUPPORT_ALL_LANGUAGES` to disable support for territory names
in all languages. 

Note that English names are always supported and it's also always possible to get territory names
in their locale language.
   
To add individual support support for other languages (of all territory names), use:
    
    -DMAPCODE_NO_SUPPORT_ALL_LANGUAGES   // If not defined, ALL languages are available.
    -DMAPCODE_SUPPORT_LANGUAGE_AF        // Add the languages you need. The names are the
    -DMAPCODE_SUPPORT_LANGUAGE_AR        // ISO 3166-2 character codes.
    -DMAPCODE_SUPPORT_LANGUAGE_BE
    -DMAPCODE_SUPPORT_LANGUAGE_CN
    -DMAPCODE_SUPPORT_LANGUAGE_CS
    -DMAPCODE_SUPPORT_LANGUAGE_DA
    -DMAPCODE_SUPPORT_LANGUAGE_DE
    -DMAPCODE_SUPPORT_LANGUAGE_EN
    -DMAPCODE_SUPPORT_LANGUAGE_ES
    -DMAPCODE_SUPPORT_LANGUAGE_FI
    -DMAPCODE_SUPPORT_LANGUAGE_FR
    -DMAPCODE_SUPPORT_LANGUAGE_HE
    -DMAPCODE_SUPPORT_LANGUAGE_HI
    -DMAPCODE_SUPPORT_LANGUAGE_HR
    -DMAPCODE_SUPPORT_LANGUAGE_ID
    -DMAPCODE_SUPPORT_LANGUAGE_IT
    -DMAPCODE_SUPPORT_LANGUAGE_JA
    -DMAPCODE_SUPPORT_LANGUAGE_KO
    -DMAPCODE_SUPPORT_LANGUAGE_NL
    -DMAPCODE_SUPPORT_LANGUAGE_NO
    -DMAPCODE_SUPPORT_LANGUAGE_PL
    -DMAPCODE_SUPPORT_LANGUAGE_PT
    -DMAPCODE_SUPPORT_LANGUAGE_RU
    -DMAPCODE_SUPPORT_LANGUAGE_SV
    -DMAPCODE_SUPPORT_LANGUAGE_SW
    -DMAPCODE_SUPPORT_LANGUAGE_TR
    -DMAPCODE_SUPPORT_LANGUAGE_UK

The list of support languages may grow over time.

## Release Notes

### 2.5.2

* Added unit test for floating point error with code "40822.schol".

* Added locale support.
 
* Added many languages.

* Hardened unit tests.

### 2.5.1

* Updated unit test to compile with plain C and added some test cases. 

### 2.5.0

* Added support for getting territory names in English and local alphabets. 

* Added much improved unit test scripts to run `gprof`, `valgrind`, the CLang address sanitize
and compare the output of the new library with and older version.

### 2.4.1

* Renamed `.h` files to `internal_*.h` unless they are relevant to the interface.

* Turned territories, alphabets and error codes into enums.

* Split off legacy stuff into `mapcode_legacy.h`. 

* Added `convertUtf8ToUtf16`, `convertUtf16ToUtf8`, `recognizeAlphabetUtf8`, `recogniseAlphabetUtf16`.

### 2.4.0

* Added scripts for Korean (Choson'gul / Hangul), Burmese, Khmer, Sinhalese, Thaana (Maldivan), 
Chinese (Zhuyin, Bopomofo), Tifinagh (Berber), Tamil, Amharic, Telugu, Odia, Kannada, Gujarati.

* Renamed constants to more correct terms (Malay to Malayalam, Hindi to Devanagari). 

* Added `getAlphabetsForTerritory(t)`, returning the most commonly used alphabets for territory `t`.

* Improved some characters for Arabic and Devanagari.

* Fixed Bengali to also support Assamese.

* Improved readability of implementation.
 
* Added conditional define option for `unittest`: `NO_POSIX_THREADS`. Add `-DNO_POSIX_THREADS`
to your compiler command-line to not use `pthreads.h`, for example, on MSVC platforms.

* Added conditional define option library: `NO_FAST_ENCODE`. Add `-DNO_FAST_ENCODE` to your
compiler command-line to not use the fast encoding (default is to use fast encoding). Using
fast encoding speeds up de encoding by a factor of 2. For normal use, there is no reason not
to use fast encoding. 

* Added `parseMapcodeString` to get individual mapcode elements, like the territory code,
the 'proper' mapcode (without the territory and precision extension) and the precision
extension parsed from a (user) input string.

* Additional bug fixes.

### 2.3.1

* Minor fixes to prevent compiler warnings.

* Minor fixes to prevent multiple inclusion of `.h` files.

* Minor data fix for Gansu Province, China (mountain range extending further west).

### 2.3.0

* Arabic script added. 

* Tibetan script changed so all characters can be easily typed on a computer keyboard.

* Greek, Hebrew and Arabic, the scripts that have (implied) vowels in mapcode sequences,
have been extended with ABJAD conversion, to prevent more than two consecutive non-digits 
from occurring. (As a result, mapcodes in Greek, Arab and Hebrew scripts are now often 
one character longer than in the roman script.)

### 2.2.2

* Removed minor warnings from CLion inspections.

### 2.2.1

* Solved 1-microdegree gap in a few spots on Earth, noticable now extreme precision is possible.

### 2.1.5

* Added stricter unit tests.

### 2.1.4

* Added `maxErrorInMetersz` to API.

* Moved recode logic into decoder.
 
* Adjusted unit test.

### 2.1.3

* Added useful routine `DistanceInMeters` to API.

### 2.1.2

* Rewrote fraction floating points to integer arithmetic.

### 2.1.1

* Added `unittest/` which can be compiled and executed to check
if the mapcode library performs as expected.

### 2.1.0

* Fixes floating point inaccuracy (prevent `encode(decode(M)) != M`).

### 2.0.3

* Added `#define FAST_ALPHA` for faster alphacode string recognition.

### 2.0.2.4

* Added point structs to source.

### 2.0.2.3

* Cleaned up source.

### 2.0.2.2

* Cleaned up source.

### 2.0.2.1

* Cleaned up source, removed legacy code.

* Renamed `dividemaps.h` to `mapcode_fast_encode.h`.

* Improved `convertToRoman` and `convertToAlphabet` to trim and skip leading territory.

* Rolled back 2.0.2 fix for letters "i" and "o" in Hindi, Bengali, Tibetan and Gurmukhi.

### 2.0.2

* Speed-up of encoding (4 times faster than 2.0.1).

* Added optional `mapcode_countrynames.h` (with the English short names, formal names, 
and name variants of all territories) and `mapcode_countrynames_short.h` (with only 
the short English names and name variants.

* Stricter processing of high-precision letters (now fails on illegal high-precision 
letters, which includes the letter Z).

* Fix for letters "i" and "o" in Hindi, Bengali, Tibetan and Gurmukhi;

* Fix for some unrecognized aliases in `convertTerritoryIsoNameToCode()`.
 
### 2.0.1

* Removed compiler warnings. 
 
### 2.0.0

* Support for high-precision mapcodes, with up to 8 character extensions (up to 10 micron accuracy).

* Fixes to the data rectangles (primarily intended for ISO proposal), see Word document for details.

### 1.50.1

* Bug fix for state IN-DD (in India).

* If the executable name contains `debug`, the self checking mechanism is activated. Note, however, that
this feature is under construction and produces false negative for certain decodes. 

### 1.50

* Major release. This version is not backwards compatible with mapcode 1.4x: is has dropped support for
Antartica AT0-8 codes and has a changed (improved) way of dealing with the Greek alphabet.

* Added 22-chararcter post-processing of all-digit mapcodes for the Greek alphabet.

* Retired legacy aliases EAZ and SKM, AU-QL, AU-TS, AU-NI and AU-JB.

* Retired legacy Antarctica claims AT0 through AT8.
 
### 1.41.1

* Bugfix for `utility/mapcode` utility (`-d` option was case sensitive).

### 1.41

* Added the India state Telangana (IN-TG), until 2014 a region in Adhra Pradesh.

* Fixed a problem with the `extraDigits` parameter of the `utility/mapcode` utility (maximum digits is 2, not 3).

### 1.40

* Added extraDigits parameter to allow high-precision mapcodes to be generated by the `utility/mapcode` utility.

### 1.33

* Fix to not remove valid results just across the edge of a territory.

* Improved interface readability and renamed methods to more readable forms.

### 1.32

* Added `encodeLatLonToSingleMapcode()`

* Fixed 1.29 so no country-wide alternative
is produced in edge cases; prevent FIJI failing to decode at exactly 180 degrees.

### 1.31

* Added `compareWithMapcodeFormat()`.

### 1.30

* IUpdated the documentation and extended it with examples and suggestions.

### 1.29

* Also generate country-wide alternative mapcodes for states.

### 1.28

* Bug fix for the needless generation of 7-letter alternatives to short mapcodes
in large states in India.

### 1.27

* Improved (faster) implementation of the function `isInArea`.

### 1.26

* Added alias OD ("Odisha") for indian state OR ("Orissa").

### 1.25

* Initial release to the public domain.


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


# Acknowledgements

Development of the C/C++ version of the Mapcode Library is kindly sponsored by 
[**JetBrains**](https://www.jetbrains.com).

[![CLion](docs/logo_CLion.png)](https://www.jetbrains.com/clion/)
