/*
 * Copyright (C) 2014 Stichting Mapcode Foundation (http://www.mapcode.com)
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

#define RELEASENEAT                         // Use clean code (mapcoder.c).
#define UWORD           unsigned short int  // 2-byte unsigned integer.
#define SUPPORT_FOREIGN_ALPHABETS

#define MAX_NR_OF_MAPCODE_RESULTS           48          // Max. number of results returned by encoder.
#define MAX_MAPCODE_RESULT_LEN              24          // Max. number of characters in a single result.
#define COMPARE_MAPCODE_MISSING_CHARACTERS  -999        // Used for compareWithMapcodeFormat.

/**
 * Encode a latitude, longitude pair (in degrees) to a set of Mapcodes.
 *
 * Arguments:
 *      results         - Results set of Mapcodes. The caller must pass an array of at least MAX_NR_OF_MAPCODE_RESULTS
 *                        string points, which must NOT be allocated or de-allocated by the caller.
 *                        The resulting strings are statically allocated by the library and will be overwritten
 *                        by the next call to this method!
 *      lat             - Latitude, in degrees. Range: -90..90.
 *      lon             - Longitude, in degrees. Range: -180..180.
 *      territoryCode   - Territory code (obtained from convertTerritoryIsoNameToCode), used as encoding context.
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
    char**  results,
    double  lat,
    double  lon,
    int     territoryCode,
    int     extraDigits);

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
 *      territoryCode   - Territory code (obtained from convertTerritoryIsoNameToCode), used as encoding context.
 *                        Pass 0 to get the shortest Mapcode for all territories.
 *      extraDigits     - Number of extra "digits" to add to the generated mapcode. The preferred default is 0.
 *                        Other valid values are 1 and 2, which will add extra letters to the mapcodes to
 *                        make them represent the coordinate more accurately.
 *
 * Returns:
 *      0 if encoding failed, or >0 if it succeeded.
 */
int encodeLatLonToSingleMapcode(
    char* result,
    double lat,
    double lon,
    int territoryCode,
    int extraDigits);

/**
 * Decode a Mapcode to  a latitude, longitude pair (in degrees).
 *
 * Arguments:
 *      lat             - Decoded latitude, in degrees. Range: -90..90.
 *      lon             - Decoded longitude, in degrees. Range: -180..180.
 *      mapcode         - Mapcode to decode.
 *      territoryCode   - Territory code (obtained from convertTerritoryIsoNameToCode), used as decoding context.
 *                        Pass 0 if not available.
 *
 * Returns:
 *      0 if encoding succeeded, nonzero in case of error
 */
int decodeMapcodeToLatLon(
    double*     lat,
    double*     lon,
    const char* mapcode,
    int         territoryCode);

/**
 * Checks if a string has the format of a Mapcode. (Note: The method is called compareXXX rather than hasXXX because
 * the return value '0' indicates the string has the Mapcode format, much like string comparison strcmp returns.)
 *
 * Arguments:
 *      check               - Mapcode string to check.
 *      includesTerritory   - If 0, no territory is includes in the string. If 1, territory information is
 *                            supposed to be available in the string as well.
 * Returns:
 *      0 if the string has a correct Mapcode format; <0 if the string does not have a Mapcode format.
 *      Special value COMPARE_MAPCODE_MISSING_CHARACTERS (-999) indicates the string could be a Mapcode, but it seems
 *      to lack some characters.
 */
int compareWithMapcodeFormat(
    const char* check,
    int         includesTerritory);

/**
 * Convert a territory name to a territory code.
 *
 * Arguments:
 *      isoNam           - Territory name to convert.
 *      parentTerritory  - Parent territory code, or 0 if not available.
 *
 * Returns:
 *      Territory code >= 0 if succeeded, or <0 if failed.
 */
int convertTerritoryIsoNameToCode(
    const char* isoName,
    int         parentTerritoryCode);

/**
 * Convert a territory name to a territory code.
 *
 * Arguments:
 *      territoryCode   - Territory code.
 *      format          - Pass 0 for full name, 1 for short name (state codes may be ambiguous).
 *
 * Returns:
 *      Static result string or 0 if failed. The string is allocated by the library and must NOT be
 *      de-allocated by the caller. It will be overwritten by a subsequent call to this method!
 */
const char *convertTerritoryCodeToIsoName(
    int territoryCode,
    int format);

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
int getParentCountryOfState(int territoryCode);

/**
 * Alphabets:
 */
#define MAPCODE_ALPHABETS_TOTAL        14

#define MAPCODE_ALPHABET_ROMAN         0
#define MAPCODE_ALPHABET_GREEK         1
#define MAPCODE_ALPHABET_CYRILLIC      2
#define MAPCODE_ALPHABET_HEBREW        3
#define MAPCODE_ALPHABET_HINDI         4
#define MAPCODE_ALPHABET_MALAI         5
#define MAPCODE_ALPHABET_GEORGIAN      6
#define MAPCODE_ALPHABET_KATAKANA      7
#define MAPCODE_ALPHABET_THAI          8
#define MAPCODE_ALPHABET_LAO           9
#define MAPCODE_ALPHABET_ARMENIAN      10
#define MAPCODE_ALPHABET_BENGALI       11
#define MAPCODE_ALPHABET_GURMUKHI      12
#define MAPCODE_ALPHABET_TIBETAN       13


/**
 * Decode a string to Roman characters.
 *
 * Arguments:
 *      string - String to decode.
 *
 * Returns:
 *      Decoded string. The string is allocated by the library and must NOT be
 *      de-allocated by the caller. It will be overwritten by a subsequent call to this method!
 */
const char* decodeToRoman(const UWORD* string);

/**
 * Encode a string to Alphabet characters for a language.
 *
 * Arguments:
 *      string      - String to encode.
 *      alphabet    - Alphabet to use. Currently supported are:
 *                      0 = roman, 2 = cyrillic, 4 = hindi, 12 = gurmukhi.
 *
 * Returns:
 *      Encoded string. The string is allocated by the library and must NOT be
 *      de-allocated by the caller. It will be overwritten by a subsequent call to this method!
 */
const UWORD* encodeToAlphabet(const char* mapcode, int alphabet);


/**
 * list of #defines to support legacy systems
 */
#define coord2mc(results,lat,lon,territoryCode)  encodeLatLonToMapcodes(results,lat,lon,territoryCode,0)
#define coord2mc1(results,lat,lon,territoryCode) encodeLatLonToSingleMapcode(results,lat,lon,territoryCode,0)
#define mc2coord decodeMapcodeToLatLon
#define lookslikemapcode compareWithMapcodeFormat
#define text2tc convertTerritoryIsoNameToCode
#define tc2text convertTerritoryCodeToIsoName
#define tccontext getCountryOrParentCountry
#define tcparent getParentCountryOfState
#define decode_to_roman decodeToRoman
#define encode_to_alphabet encodeToAlphabet
#define MAX_MAPCODE_TERRITORY_CODE MAX_CCODE
#define NR_BOUNDARY_RECS NR_RECS

#define MAX_LANGUAGES                  MAPCODE_ALPHABETS_TOTAL
#define MAPCODE_LANGUAGE_ROMAN         MAPCODE_ALPHABET_ROMAN
#define MAPCODE_LANGUAGE_GREEK         MAPCODE_ALPHABET_GREEK
#define MAPCODE_LANGUAGE_CYRILLIC      MAPCODE_ALPHABET_CYRILLIC
#define MAPCODE_LANGUAGE_HEBREW        MAPCODE_ALPHABET_HEBREW
#define MAPCODE_LANGUAGE_HINDI         MAPCODE_ALPHABET_HINDI
#define MAPCODE_LANGUAGE_MALAI         MAPCODE_ALPHABET_MALAI
#define MAPCODE_LANGUAGE_GEORGIAN      MAPCODE_ALPHABET_GEORGIAN
#define MAPCODE_LANGUAGE_KATAKANA      MAPCODE_ALPHABET_KATAKANA
#define MAPCODE_LANGUAGE_THAI          MAPCODE_ALPHABET_THAI
#define MAPCODE_LANGUAGE_LAO           MAPCODE_ALPHABET_LAO
#define MAPCODE_LANGUAGE_ARMENIAN      MAPCODE_ALPHABET_ARMENIAN
#define MAPCODE_LANGUAGE_BENGALI       MAPCODE_ALPHABET_BENGALI
#define MAPCODE_LANGUAGE_GURMUKHI      MAPCODE_ALPHABET_GURMUKHI
#define MAPCODE_LANGUAGE_TIBETAN       MAPCODE_ALPHABET_TIBETAN
