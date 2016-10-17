/*
 * Copyright (C) 2014-2015 Stichting Mapcode Foundation (http://www.mapcode.com)
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

#ifndef __MAPCODER_H__
#define __MAPCODER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "mapcode_alphabets.h"
#include "mapcode_territory_alphabets.h"

#define mapcode_cversion "2.4.0"

#define UWORD                               unsigned short int  // 2-byte unsigned integer.

#define SUPPORT_FOREIGN_ALPHABETS           // Define to support additional alphabets.
#define SUPPORT_HIGH_PRECISION              // Define to enable high-precision extension logic.

#define MAX_NR_OF_MAPCODE_RESULTS           22          // Max. number of results ever returned by encoder (e.g. for 26.904899, 95.138515).
#define MAX_PROPER_MAPCODE_LEN              11          // Max. number of characters in a proper mapcode (including the dot, excl. precision extension).
#define MAX_PRECISION_DIGITS                8           // Max. number of extension characters (excluding the hyphen). Must be even.
#define MAX_PRECISION_FACTOR                810000      // 30 to the power (MAX_PRECISION_DIGITS/2).
#define MAX_ISOCODE_LEN                     7           // Max. number of characters of a valid territory code; although nothing longer than SIX characters is ever generated (RU-KAM), users can input SEVEN characters (RUS-KAM).
#define MAX_CLEAN_MAPCODE_LEN               (MAX_PROPER_MAPCODE_LEN + 1 + MAX_PRECISION_DIGITS)  // Max. number of characters in a clean mapcode (excluding zero-terminator).
#define MAX_MAPCODE_RESULT_LEN              (MAX_ISOCODE_LEN + 1 + MAX_CLEAN_MAPCODE_LEN + 1)    // Max. number of characters to store a single result (including zero-terminator).


/**
 * The type Mapcodes hold a number of mapcodes, for example from an encoding call.
 * If a result contains a space, it splits the territory alphacode from the mapcode.
 * International mapcodes do not specify a territory alphacode, nor a space.
 */
typedef struct {
    int count;                                                        // The number of mapcode results (length of array).
    char mapcode[MAX_NR_OF_MAPCODE_RESULTS][MAX_MAPCODE_RESULT_LEN];  // The mapcodes.
} Mapcodes;

typedef struct {
    char territoryISO[MAX_ISOCODE_LEN + 1];            // The (trimmed and uppercased) candidate territory.
    int territoryCode;                                 // The territory, as recognised and disambiguated, same as from getTerritoryCode().
    char properMapcode[MAX_PROPER_MAPCODE_LEN + 1];    // The (trimmed and uppercased) proper mapcode (excluding territory and precision extension).
    int properMapcodeLength;                           // Length of proper mapcode.
    int indexOfDot;                                    // Position of dot in properMapcode (a value between 2 and 5).
    char precisionExtension[MAX_PRECISION_DIGITS + 1]; // The (trimmed and uppercased) precision extension including hyphen; empty if precision 0.
} MapcodeElements;

/**
 * Encode a latitude, longitude pair (in degrees) to a set of Mapcodes.
 *
 * Arguments:
 *      mapcodes        - A pointer to a buffer to hold the mapcodes, allocated by the caller.
 *      lat             - Latitude, in degrees. Range: -90..90.
 *      lon             - Longitude, in degrees. Range: -180..180.
 *      territoryCode   - Territory code (obtained from getTerritoryCode), used as encoding context.
 *                        Pass 0 to get Mapcodes for all territories.
 *      extraDigits     - Number of extra "digits" to add to the generated mapcode. The preferred default is 0.
 *                        Other valid values are 1 and 2, which will add extra letters to the mapcodes to
 *                        make them represent the coordinate more accurately.
 *
 * Returns:
 *      Number of results stored in parameter results. Always >= 0 (0 if no encoding was possible or an error occurred).
 *      The results are stored as pairs (Mapcode, territory name) in:
 *          (results[0], results[1])...(results[(2 * N) - 2], results[(2 * N) - 1])
 */

int encodeLatLonToMapcodes(
        Mapcodes *mapcodes,
        double latDeg,
        double lonDeg,
        int territoryCode,
        int extraDigits);

/**
 * WARNING: This method is deprecated and should no longer be used, as it is not thread-safe. Use the version
 * specified above.
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
 *      territoryCode          - Territory code (obtained from getTerritoryCode), used as encoding context.
 *                               Pass 0 to get Mapcodes for all territories.
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
        int territoryCode,
        int extraDigits);

/**
 * Encode a latitude, longitude pair (in degrees) to a single Mapcode: the shortest possible for the given territory
 * (which can be 0 for all territories).
 *
 * Arguments:
 *      result          - Returned Mapcode. The caller must not allocate or de-allocated this string.
 *                        The resulting string MUST be allocated (and de-allocated) by the caller (contrary to
 *                        encodeLatLonToMapcodes!).
 *                        The caller should allocate at least MAX_MAPCODE_RESULT_LEN characters for the string.
 *      lat             - Latitude, in degrees. Range: -90..90.
 *      lon             - Longitude, in degrees. Range: -180..180.
 *      territoryCode   - Territory code (obtained from getTerritoryCode), used as encoding context.
 *      extraDigits     - Number of extra "digits" to add to the generated mapcode. The preferred default is 0.
 *                        Other valid values are 1 and 2, which will add extra letters to the mapcodes to
 *                        make them represent the coordinate more accurately.
 *
 * Returns:
 *      0 if encoding failed, or >0 if it succeeded.
 */
int encodeLatLonToSingleMapcode(
        char *mapcode,
        double latDeg,
        double lonDeg,
        int territoryCode,
        int extraDigits);

/**
 * Decode a Mapcode to  a latitude, longitude pair (in degrees).
 *
 * Arguments:
 *      lat             - Decoded latitude, in degrees. Range: -90..90.
 *      lon             - Decoded longitude, in degrees. Range: -180..180.
 *      mapcode         - Mapcode to decode.
 *      territoryCode   - Territory code (obtained from getTerritoryCode), used as decoding context.
 *                        Pass 0 if not available.
 *
 * Returns:
 *      0 if encoding succeeded, nonzero in case of error.
 */
int decodeMapcodeToLatLon(
        double *latDeg,
        double *lonDeg,
        const char *mapcode,
        int territoryCode);

/**
 * Checks if a string has the format of a Mapcode. (Note: The method is called compareXXX rather than hasXXX because
 * the return value '0' indicates the string has the Mapcode format, much like string comparison strcmp returns.)
 *
 * Arguments:
 *      asciiString         - Mapcode string to check.
 *      includesTerritory   - If 0, no territory is includes in the string. If 1, territory information is
 *                            supposed to be available in the string as well.
 * Returns:
 *      0 if the string has a correct Mapcode format; <0 if the string does not have a Mapcode format.
 *      Special value COMPARE_MAPCODE_MISSING_CHARACTERS (-999) indicates the string could be a Mapcode, but it seems
 *      to lack some characters.
 */
int compareWithMapcodeFormat(
        const char *asciiString,
        int includesTerritory);

/**
 * Parses a string into its mapcode components (returns nonzero in case the string is not a valid mapcode).
 *
 * Arguments:
 *      mapcodeFormat       - If not NULL, filled with analysis of the 'check' string if it was a correct mapcode.
 *                            Pass NULL to skip returning the additional analysis.
 *      asciiString         - Mapcode string to check.
 *      includesTerritory   - If 0, no territory is includes in the string. If non-0, territory information is
 *                            supposed to be available in the string as well. Pass non-0 when in doubt.
 *      territoryCode       - Territory code (obtained from getTerritoryCode), used as decoding context in case
 *                            the mapcode string does not contain a territory.
 *                            Pass 0 if not available.
 * Returns:
 *      Returns 0 if the string has a correct Mapcode format.
 *      Return <0 if the string does not have a Mapcode format.
 *      Special value COMPARE_MAPCODE_MISSING_CHARACTERS indicates the string could be a Mapcode, but it seems
 *      to lack some characters.
 *      If 0 is returned, the structure mapcodeFormat is correctly filled with (trimmed, uppercased) components
 *      id the passed pointer to mapcodeFormat was not NULL.
 */
int parseMapcodeString(
        MapcodeElements *mapcodeFormat,
        const char *asciiString,
        int containsTerritory,
        int territoryCode);

/**
 * Convert a territory name to a territory code.
 *
 * Arguments:
 *      territoryISO         - String starting with ISO code of territory (e.g. "USA" or "US-CA").
 *      parentTerritoryCode  - Parent territory code, or 0 if not available.
 *
 * Returns:
 *      Territory code >0 if succeeded, or <0 if failed.
 */
int getTerritoryCode(
        const char *territoryISO,
        int optionalTerritoryContext);

/**
 * Convert a territory code to a territory name.
 *
 * Arguments:
 *      territoryISO    - String to territory ISO code name result.
 *      territoryCode   - Territory code.
 *      userShortName   - Pass 0 for full name, 1 for short name (state codes may be ambiguous).
 *
 * Returns:
 *      Pointer to result. Empty if territoryCode illegal.
 */
char *getTerritoryIsoName(
        char *territoryISO,
        int territoryCode,
        int useShortName);

// the old, non-threadsafe routine which uses static storage, overwritten at each call:
const char *convertTerritoryCodeToIsoName(
        int territoryContext,
        int useShortName);

/**
 * Given a territory code, return the territory code itself it it was a country, or return its parent
 * country territory if it was a state.
 *
 * Arguments:
 *      territoryCode   - Country or state territory code.
 *
 * Returns:
 *      Territory code of the parent country (if the territoryCode indicated a state), or the territoryCode
 *      itself, if it was a country; <0 if the territoryCode was invalid.
 */
int getCountryOrParentCountry(int territoryCode);

/**
 * Given a territory code, return its parent country territory.
 *
 * Arguments:
 *      territoryCode   - State territory code.
 *
 * Returns:
 *      Territory code of the parent country; <0 if the territoryCode was not a state or it was invalid.
 */
int getParentCountryOf(int territoryCode);

/**
 * Returns the distance in meters between two coordinates (latitude/longitude pairs)
 * CAVEAT: only works for coordinates that are within a few miles from each other.
 */
double distanceInMeters(double latDeg1, double lonDeg1, double latDeg2, double lonDeg2);

/**
 * How far away, at worst, can a decoded mapcode be from the original encoded coordinate?
 * (which can be 0 for all territories).
 *
 * Arguments:
 *      extraDigits     - Number of extra "digits" in the mapcode. extra letters added to mapcodes
 *                        make them represent coordinates more accurately.
 *
 * Returns:
 *      The worst-case distance in meters between a decoded mapcode and the encoded coordinate.
 */
double maxErrorInMeters(int extraDigits);

/**
 * Is coordinate near more than one territory border?
 *
 * Arguments:
 *      lat             - Latitude, in degrees. Range: -90..90.
 *      lon             - Longitude, in degrees. Range: -180..180.
 *      territoryCode   - Territory code (obtained from getTerritoryCode).
 *
 * returns nonzero if coordinate is near more than one territory border
 *
 * Note that for the mapcode system, the following should hold: IF a point p has a 
 * mapcode M, THEN decode(M) delivers a point q within maxErrorInMeters() of p.
 * Furthermore, encode(q) must yield back M *unless* point q is near multiple borders.
 */
int multipleBordersNearby(
        double latDeg,
        double lonDeg,
        int territoryCode);

/**
 * Given a territory code, returns a structure defining which alphabets (in order of importance) are in common use in the territory
 *
 * Arguments:
 *      territoryCode   - territory code.
 *
 * Returns:
 *      A pointer to a TerritoryAlphabets structure (or NULL if territoryCode is invalid).
 */
const TerritoryAlphabets *getAlphabetsForTerritory(int territoryCode);

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
UWORD *convertToAlphabet(UWORD *utf16String, int maxLength, const char *asciiString, int alphabet);


/* DEPRECATED METHODS AND CONSTANT - WILL BE DROPPED IN FUTURE RELEASES. */

/**
 * DEPRECATED ODL VARIANT, NOT THREAD-SAFE:
 * Uses a pre-allocated static buffer, overwritten by the next call
 * Returns converted string. allocated by the library. String must NOT be
 * de-allocated by the caller. It will be overwritten by a subsequent call to this method!
 */
const char *decodeToRoman(const UWORD *utf16String);

/**
 * DEPRECATED ODL VARIANT, NOT THREAD-SAFE:
 * Returns converted string. allocated by the library. String must NOT be
 * de-allocated by the caller. It will be overwritten by a subsequent call to this method!
 */
const UWORD *encodeToAlphabet(const char *asciiString, int alphabet);


/**
 * List of #defines to support legacy systems.
 */
#define convertTerritoryIsoNameToCode getTerritoryCode
#define coord2mc(results, lat, lon, territoryCode)  encodeLatLonToMapcodes_Deprecated(results, lat, lon,territoryCode, 0)
#define coord2mc1(results, lat, lon, territoryCode) encodeLatLonToSingleMapcode(results, lat, lon, territoryCode, 0)
#define mc2coord decodeMapcodeToLatLon
#define lookslikemapcode compareWithMapcodeFormat
#define text2tc getTerritoryCode
#define tc2text convertTerritoryCodeToIsoName
#define tccontext getCountryOrParentCountry
#define tcparent getParentCountryOf
#define decode_to_roman decodeToRoman
#define encode_to_alphabet encodeToAlphabet
#define MAX_MAPCODE_TERRITORY_CODE MAX_CCODE
#define NR_BOUNDARY_RECS NR_RECS

#define MAX_LANGUAGES                  MAPCODE_ALPHABETS_TOTAL
#define MAPCODE_LANGUAGE_ROMAN         MAPCODE_ALPHABET_ROMAN
#define MAPCODE_LANGUAGE_GREEK         MAPCODE_ALPHABET_GREEK
#define MAPCODE_LANGUAGE_CYRILLIC      MAPCODE_ALPHABET_CYRILLIC
#define MAPCODE_LANGUAGE_HEBREW        MAPCODE_ALPHABET_HEBREW
#define MAPCODE_LANGUAGE_HINDI         MAPCODE_ALPHABET_DEVANAGARI
#define MAPCODE_ALPHABET_HINDI         MAPCODE_ALPHABET_DEVANAGARI
#define MAPCODE_LANGUAGE_MALAYALAM     MAPCODE_ALPHABET_MALAYALAM
#define MAPCODE_LANGUAGE_GEORGIAN      MAPCODE_ALPHABET_GEORGIAN
#define MAPCODE_LANGUAGE_KATAKANA      MAPCODE_ALPHABET_KATAKANA
#define MAPCODE_LANGUAGE_THAI          MAPCODE_ALPHABET_THAI
#define MAPCODE_LANGUAGE_LAO           MAPCODE_ALPHABET_LAO
#define MAPCODE_LANGUAGE_ARMENIAN      MAPCODE_ALPHABET_ARMENIAN
#define MAPCODE_LANGUAGE_BENGALI       MAPCODE_ALPHABET_BENGALI
#define MAPCODE_LANGUAGE_GURMUKHI      MAPCODE_ALPHABET_GURMUKHI
#define MAPCODE_LANGUAGE_TIBETAN       MAPCODE_ALPHABET_TIBETAN
#define MAPCODE_LANGUAGE_ARABIC        MAPCODE_ALPHABET_ARABIC

#ifdef __cplusplus
}
#endif
#endif
