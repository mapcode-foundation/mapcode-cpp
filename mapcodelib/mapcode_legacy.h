/*
 * Copyright (C) 2014-2017 Stichting Mapcode Foundation (http://www.mapcode.com)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __MAPCODE_LEGACY_H__
#define __MAPCODE_LEGACY_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "mapcoder.h"
#include "mapcode_territories.h"
#include "mapcode_alphabets.h"


/**
 * List of #defines to support legacy systems.
 */
#define decodeMapcodeToLatLon(latDeg, lonDeg, mapcode, territory) decodeUtf8MapcodeToLatLon(latDeg, lonDeg, NULL, mapcode, territory)
#define compareWithMapcodeFormat(utf8, canContainTerritory)    compareUtf8WithMapcodeFormat(utf8)

#define convertTerritoryIsoNameToCode           getTerritoryCode
#define coord2mc(results, lat, lon, territory)  encodeLatLonToMapcodes_Deprecated(results, lat, lon,territory, 0)
#define coord2mc1(results, lat, lon, territory) encodeLatLonToSingleMapcode(results, lat, lon, territory, 0)
#define mc2coord                                decodeMapcodeToLatLon
#define lookslikemapcode                        compareWithMapcodeFormat
#define text2tc                                 getTerritoryCode
#define tc2text                                 convertTerritoryCodeToIsoName
#define tccontext                               getCountryOrParentCountry
#define tcparent                                getParentCountryOf
#define decode_to_roman                         decodeToRoman
#define encode_to_alphabet                      encodeToAlphabet
#define MAX_MAPCODE_TERRITORY_CODE              (_TERRITORY_MAX - _TERRITORY_MIN - 1)
#define MAX_CCODE                               (_TERRITORY_MAX - _TERRITORY_MIN - 1)
#define NR_BOUNDARY_RECS                        MAPCODE_BOUNDARY_MAX
#define NR_RECS                                 MAPCODE_BOUNDARY_MAX

#define COMPARE_MAPCODE_MISSING_CHARACTERS      ERR_MAPCODE_INCOMPLETE

#define MAX_PROPER_MAPCODE_LEN                  MAX_PROPER_MAPCODE_ASCII_LEN
#define MAX_ISOCODE_LEN                         MAX_ISOCODE_ASCII_LEN
#define MAX_CLEAN_MAPCODE_LEN                   MAX_CLEAN_MAPCODE_ASCII_LEN
#define MAX_MAPCODE_RESULT_LEN                  MAX_MAPCODE_RESULT_ASCII_LEN

#define MAX_LANGUAGES                           _ALPHABET_MAX
#define MAPCODE_LANGUAGE_ROMAN                  ALPHABET_ROMAN
#define MAPCODE_LANGUAGE_GREEK                  ALPHABET_GREEK
#define MAPCODE_LANGUAGE_CYRILLIC               ALPHABET_CYRILLIC
#define MAPCODE_LANGUAGE_HEBREW                 ALPHABET_HEBREW
#define MAPCODE_LANGUAGE_HINDI                  ALPHABET_DEVANAGARI
#define ALPHABET_HINDI                          ALPHABET_DEVANAGARI
#define MAPCODE_LANGUAGE_MALAYALAM              ALPHABET_MALAYALAM
#define MAPCODE_LANGUAGE_GEORGIAN               ALPHABET_GEORGIAN
#define MAPCODE_LANGUAGE_KATAKANA               ALPHABET_KATAKANA
#define MAPCODE_LANGUAGE_THAI                   ALPHABET_THAI
#define MAPCODE_LANGUAGE_LAO                    ALPHABET_LAO
#define MAPCODE_LANGUAGE_ARMENIAN               ALPHABET_ARMENIAN
#define MAPCODE_LANGUAGE_BENGALI                ALPHABET_BENGALI
#define MAPCODE_LANGUAGE_GURMUKHI               ALPHABET_GURMUKHI
#define MAPCODE_LANGUAGE_TIBETAN                ALPHABET_TIBETAN
#define MAPCODE_LANGUAGE_ARABIC                 ALPHABET_ARABIC
// Some alphabets are missing because they were never supported in the legacy library.


/**
 * DEPRECATED OLD VARIANT, NOT THREAD-SAFE:
 *
 * Encode a latitude, longitude pair (in degrees) to a set of Mapcodes. Not thread-safe!
 *
 * Arguments:
 *      mapcodesAndTerritories - Results set of mapcodes and territories.
 *                               The caller must pass an array of at least 2 * MAX_NR_OF_MAPCODE_RESULTS
 *                               string points, which must NOT be allocated or de-allocated by the caller.
 *                               The resulting strings are statically allocated by the library and will be overwritten
 *                               by the next call to this method!
 *      lat                    - Latitude, in degrees. Range: -90..90.
 *      lon                    - Longitude, in degrees. Range: -180..180.
 *      territory              - Territory (e.g. as obtained from getTerritoryCode), used as encoding context.
 *                               Pass TERRITORY_NONE or TERRITORY_UNKNOWN to get Mapcodes for all territories.
 *      extraDigits            - Number of extra "digits" to add to the generated mapcode. The preferred default is 0.
 *                               Other valid values are 1 and 2, which will add extra letters to the mapcodes to
 *                               make them represent the coordinate more accurately.
 *
 * Returns:
 *      Number of results stored in parameter results. Always >= 0 (0 if no encoding was possible or an error occurred).
 *      The results are stored as pairs (Mapcode, territory name) in:
 *          (results[0], results[1])...(results[(2 * N) - 2], results[(2 * N) - 1])
 */
int encodeLatLonToMapcodes_Deprecated(     // Warning: this method is deprecated and not thread-safe.
        char **mapcodesAndTerritories,
        double latDeg,
        double lonDeg,
        enum Territory territory,
        int extraDigits);


/**
 * DEPRECATED OLD VARIANT, NOT THREAD-SAFE:
 *
 * Convert a territory to a territory name.
 * Non-threadsafe routine which uses static storage, overwritten at each call.
 *
 * Arguments:
 *      territory       - Territory to get the name of.
 *      userShortName   - Pass 0 for full name, 1 for short name (state codes may be ambiguous).
 *
 * Returns:
 *      Pointer to result. String will be empty if territory illegal.
 */
const char *convertTerritoryCodeToIsoName_Deprecated(
        enum Territory territory,
        int useShortName);


/**
 * Decode a string to Roman characters.
 *
 * Arguments:
 *      asciiString - Buffer to be filled with the ASCII string result.
 *      maxLength   - Size of asciiString buffer.
 *      utf16String - Unicode string to decode, allocated by caller.
 *
 * Returns:
 *      Pointer to same buffer as asciiString (allocated by caller), which holds the result.
 */
char *convertToRoman(char *asciiString, int maxLength, const UWORD *utf16String);



/**
 * Encode a string to Alphabet characters for a language.
 *
 * Arguments:
 *      utf16String  - Buffer to be filled with the Unicode string result.
 *      asciiString  - ASCII string to encode.
 *      maxLength    - Size of utf16String buffer.
 *      alphabet     - Alphabet to use.
 *
 * Returns:
 *      Encoded Unicode string, points at buffer from 'utf16String', allocated/deallocated by caller.
 */
UWORD *convertToAlphabet(UWORD *utf16String, int maxLength, const char *asciiString, enum Alphabet alphabet);


/**
 * DEPRECATED OLD VARIANT, NOT THREAD-SAFE:
 *
 * Uses a pre-allocated static buffer, overwritten by the next call
 * Returns converted string. allocated by the library.
 *
 * String must NOT be de-allocated by the caller.
 * It will be overwritten by a subsequent call to this method!
 */
const char *decodeToRoman_Deprecated(const UWORD *utf16String);


/**
 * DEPRECATED OLD VARIANT, NOT THREAD-SAFE:
 *
 * Returns converted string. allocated by the library.
 *
 * String must NOT be de-allocated by the caller.
 * It will be overwritten by a subsequent call to this method!
 */
const UWORD *encodeToAlphabet_Deprecated(const char *asciiString, enum Alphabet alphabet);


#ifdef __cplusplus
}
#endif

#endif // __MAPCODE_LEGACY_H__
