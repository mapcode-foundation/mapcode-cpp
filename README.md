# Mapcode Library for C/C++

Copyright (C) 2014-2015 Stichting Mapcode Foundation (http://www.mapcode.com)

----

**Online documentation can be found at: http://mapcode-foundation.github.io/mapcode-cpp/**


## The C/C++ Library: `mapcodelib/`

The directory 'mapcodelib' contains the files:

	mapcoder.h      <-- Header file with method prototypes and defines for caller.
	mapcoder.c
	basics.h

Together these provide routine to encode/decode Mapcodes.

Documentation, including example snippets of C source code, can be found in

	mapcode_library.doc     (Microsoft Word format).

Also see www.mapcode.com for background and reference materials.

Note: this version may be restricted to a particular area of the Earth!
In that case, basics.h will state a version number of the for:
	#define mapcode_cversion "1.2xxx"
where "xxx" states the geographical limitation.

## A Real-Life Example, The 'mapcode' Codec Tool: `example/`

The directory 'example' contains a Mapcode encoding/decoding utility, as an example
of how to use the library.

To build the original Mapcode tool, execute:

  cd example
  gcc mapcode.cpp -o mapcode

For help, simply execute the binary file 'mapcode' without no arguments.
This tool provides a rather extensive command-line interface to encode and
decode Mapcodes.


## Release Notes

* 1.41

    Added the India state Telangana (IN-TG), until 2014 a region in Adhra Pradesh.

* 1.40

    Added extraDigits parameter to allow high-precision mapcodes to be generated.

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

## License

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
