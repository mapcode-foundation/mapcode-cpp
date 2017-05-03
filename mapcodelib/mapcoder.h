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

#ifndef __MAPCODER_H__
#define __MAPCODER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "mapcode_territories.h"
#include "mapcode_alphabets.h"

// English is always defined - this is the fallback language for any unknown language.
#define MAPCODE_SUPPORT_LANGUAGE_EN
#define DEFAULT_TERRITORY_FULL_NAME TERRITORY_FULL_NAME_EN

#ifndef MAPCODE_NO_SUPPORT_ALL_LANGUAGES
#define MAPCODE_SUPPORT_LANGUAGE_AF
#define MAPCODE_SUPPORT_LANGUAGE_AR
#define MAPCODE_SUPPORT_LANGUAGE_BE
#define MAPCODE_SUPPORT_LANGUAGE_CN
#define MAPCODE_SUPPORT_LANGUAGE_CS
#define MAPCODE_SUPPORT_LANGUAGE_DA
#define MAPCODE_SUPPORT_LANGUAGE_DE
#define MAPCODE_SUPPORT_LANGUAGE_EN
#define MAPCODE_SUPPORT_LANGUAGE_ES
#define MAPCODE_SUPPORT_LANGUAGE_FI
#define MAPCODE_SUPPORT_LANGUAGE_FR
#define MAPCODE_SUPPORT_LANGUAGE_HE
#define MAPCODE_SUPPORT_LANGUAGE_HI
#define MAPCODE_SUPPORT_LANGUAGE_HR
#define MAPCODE_SUPPORT_LANGUAGE_ID
#define MAPCODE_SUPPORT_LANGUAGE_IT
#define MAPCODE_SUPPORT_LANGUAGE_JA
#define MAPCODE_SUPPORT_LANGUAGE_KO
#define MAPCODE_SUPPORT_LANGUAGE_NL
#define MAPCODE_SUPPORT_LANGUAGE_NO
#define MAPCODE_SUPPORT_LANGUAGE_PL
#define MAPCODE_SUPPORT_LANGUAGE_PT
#define MAPCODE_SUPPORT_LANGUAGE_RU
#define MAPCODE_SUPPORT_LANGUAGE_SV
#define MAPCODE_SUPPORT_LANGUAGE_SW
#define MAPCODE_SUPPORT_LANGUAGE_TR
#define MAPCODE_SUPPORT_LANGUAGE_UK
#endif

#define MAPCODE_C_VERSION                   "2.5.3"
#define UWORD                               unsigned short int  // 2-byte unsigned integer.

#define MAX_NR_OF_MAPCODE_RESULTS           22                  // Max. number of results ever returned by encoder (e.g. for 26.904899, 95.138515).
#define MAX_PRECISION_DIGITS                8                   // Max. number of extension characters (excluding the hyphen). Must be even.

#define MAX_PROPER_MAPCODE_ASCII_LEN        11          // Max. chars in a proper mapcode (including the dot, excl. precision extension).
#define MAX_ISOCODE_ASCII_LEN               7           // Max. chars in a valid ISO3166 territory code.
#define MAX_CLEAN_MAPCODE_ASCII_LEN         (MAX_PROPER_MAPCODE_ASCII_LEN + 1 + MAX_PRECISION_DIGITS)       // Max. chars in a clean mapcode (excluding zero-terminator).
#define MAX_MAPCODE_RESULT_ASCII_LEN        (MAX_ISOCODE_ASCII_LEN + 1 + MAX_CLEAN_MAPCODE_ASCII_LEN + 1)   // Max. chars to store a single result (including zero-terminator).
#define MAX_TERRITORY_FULLNAME_UTF8_LEN     111 // Max. number of characters to store the longest possible territory name (in UTF8)

#define MAX_MAPCODE_RESULT_UTF8_LEN         (MAX_MAPCODE_RESULT_ASCII_LEN * 3) // One mapcode character can become at most 3 UTF8characters.
#define MAX_MAPCODE_RESULT_UTF16_LEN        (MAX_MAPCODE_RESULT_ASCII_LEN)     // Each mapcode character can become one UTF16 word.


/**
 * The type Mapcodes hold a number of mapcodes, for example from an encoding call.
 * If a result contains a space, that space seperates the territory ISO3166 code from the mapcode.
 * International mapcodes never include a territory ISO3166 code, nor a space.
 */
typedef struct {
    int count;                                                              // The number of mapcode results (length of array).
    char mapcode[MAX_NR_OF_MAPCODE_RESULTS][MAX_MAPCODE_RESULT_ASCII_LEN];   // The mapcodes.
} Mapcodes;


/**
 * The MapcodeElement structure is returned by decodeXXX and can be used to inspect or clean up the
 * mapcode input. The field territoryISO contains the cleaned up territory code from the input, but
 * the code may be abbreviated, or even missing (if it wasn't available in the input).
 *
 * If you want to get a full territory code, use:
 * char isoName[MAX_ISOCODE_ASCII_LEN + 1];
 * getTerritoryIsoName(isoName, mapcodeElement.territoryCode, 0)
 */
typedef struct {
    char territoryISO[MAX_ISOCODE_ASCII_LEN + 1];           // The (trimmed and uppercased) territory code, from the input.
    enum Territory territoryCode;                           // The territory, as recognized and disambiguated from territoryISO.
    char properMapcode[MAX_PROPER_MAPCODE_ASCII_LEN + 1];   // The (romanised) mapcode excl. territory or extension.
    int indexOfDot;                                         // Position of dot in properMapcode (a value between 2 and 5).
    char precisionExtension[MAX_PRECISION_DIGITS + 1];      // The (romanised) extension (excluding the hyphen).
} MapcodeElements;


/**
 * List of error return codes (negative except for ERR_OK = 0)
 */
enum MapcodeError {

    // note: an incomplete mapcode could "become" complete by adding letters.

    ERR_MAPCODE_INCOMPLETE = -999,   // not enough letters (yet) after dot

    // format errors.

    ERR_ALL_DIGIT_CODE = -299,       // mapcode consists only of digits
    ERR_INVALID_MAPCODE_FORMAT,      // string not recognized as mapcode format
    ERR_INVALID_CHARACTER,           // mapcode contains an invalid character
    ERR_BAD_ARGUMENTS,               // an argument is invalid (e.g. NULL)
    ERR_INVALID_ENDVOWELS,           // mapcodes ends in UE or UU
    ERR_EXTENSION_INVALID_LENGTH,    // precision extension too long, or empty
    ERR_EXTENSION_INVALID_CHARACTER, // bad precision extension character (e.g. Z)
    ERR_UNEXPECTED_DOT,              // mapcode dot can not be in this position
    ERR_DOT_MISSING,                 // mapcode dot not found
    ERR_UNEXPECTED_HYPHEN,           // hyphen can not be in this position
    ERR_INVALID_VOWEL,               // vowel in invalid location, or missing
    ERR_BAD_TERRITORY_FORMAT,        // mapcode territory badly formatted
    ERR_TRAILING_CHARACTERS,         // characters found trailing the mapcode

    // parse errors.

    ERR_UNKNOWN_TERRITORY = -199,    // mapcode territory not recognized

    // other errors.

    ERR_BAD_MAPCODE_LENGTH = -99,    // proper mapcode too short or too long
    ERR_MISSING_TERRITORY,           // mapcode can not be decoded without territory
    ERR_EXTENSION_UNDECODABLE,       // extension does not decode to valid coordinate
    ERR_MAPCODE_UNDECODABLE,         // mapcode does not decode inside territory
    ERR_BAD_COORDINATE,              // latitude or longitude is NAN or infinite

    // all OK.

    ERR_OK = 0
};


/**
 * Encode a latitude, longitude pair (in degrees) to a set of Mapcodes.
 *
 * Arguments:
 *      mapcodes        - A pointer to a buffer to hold the mapcodes, allocated by the caller.
 *      lat             - Latitude, in degrees. Range: -90..90.
 *      lon             - Longitude, in degrees. Range: -180..180.
 *      territory       - Territory (e.g. as from getTerritoryCode), used as encoding context.
 *                        Pass TERRITORY_NONE or TERRITORY_UNKNOWN to get Mapcodes for all territories.
 *      extraDigits     - Number of extra "digits" to add to the generated mapcode. The preferred default is 0.
 *                        Other valid values are 1 to 8, which will add extra letters to the mapcodes to
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
        enum Territory territory,
        int extraDigits);


/**
 * Encode a latitude, longitude pair (in degrees) to a single Mapcode: the shortest possible for the given territory
 * (which can be 0 for all territories).
 *
 * Arguments:
 *      result          - Returned Mapcode. The caller must not allocate or de-allocated this string.
 *                        The resulting string MUST be allocated (and de-allocated) by the caller (contrary to
 *                        encodeLatLonToMapcodes!).
 *                        The caller should allocate at least MAX_MAPCODE_RESULT_ASCII_LEN characters for the string.
 *      lat             - Latitude, in degrees. Range: -90..90.
 *      lon             - Longitude, in degrees. Range: -180..180.
 *      territory       - Territory (e.g. as obtained from getTerritoryCode), used as encoding context.
 *                        Pass TERRITORY_NONE or TERRITORY_UNKNOWN to get Mapcodes for all territories.
 *      extraDigits     - Number of extra "digits" to add to the generated mapcode. The preferred default is 0.
 *                        Other valid values are 1 to 8, which will add extra letters to the mapcodes to
 *                        make them represent the coordinate more accurately.
 *
 * Returns:
 *      Number of results. <=0 if encoding failed, or 1 if it succeeded.
 */
int encodeLatLonToSingleMapcode(
        char *mapcode,
        double latDeg,
        double lonDeg,
        enum Territory territory,
        int extraDigits);


/**
 * Decode a utf8 or ascii Mapcode to  a latitude, longitude pair (in degrees).
 *
 * Arguments:
 *      lat             - Decoded latitude, in degrees. Range: -90..90.
 *      lon             - Decoded longitude, in degrees. Range: -180..180.
 *      utf8string      - Mapcode to decode (ascii or utf8 string).
 *      territory       - Territory (e.g. as obtained from getTerritoryCode), used as decoding context.
 *                        Pass TERRITORY_NONE if not available.
 *      mapcodeElements - If not NULL, filled with analysis of the string (unless an error was encountered).
 *
 * Returns:
 *      ERR_OK if encoding succeeded.
 */
enum MapcodeError decodeMapcodeToLatLonUtf8(
        double *latDeg,
        double *lonDeg,
        const char *utf8string,
        enum Territory territory,
        MapcodeElements *mapcodeElements);


/**
 * Decode a utf16 Mapcode to  a latitude, longitude pair (in degrees).
 *
 * Arguments:
 *      lat             - Decoded latitude, in degrees. Range: -90..90.
 *      lon             - Decoded longitude, in degrees. Range: -180..180.
 *      mapcodeElements - If not NULL, filled with analysis of the string (unless an error was encountered)
 *      utf8string      - Mapcode to decode (ascii or utf8 string).
 *      territory       - Territory (e.g. as obtained from getTerritoryCode), used as decoding context.
 *                        Pass TERRITORY_NONE if not available.
 *
 * Returns:
 *      ERR_OK if encoding succeeded.
 */
enum MapcodeError decodeMapcodeToLatLonUtf16(
        double *latDeg,
        double *lonDeg,
        const UWORD *utf16string,
        enum Territory territory,
        MapcodeElements *mapcodeElements);


/**
 * Checks if a string has the format of a Mapcode. (Note: The method is called compareXXX rather than hasXXX because
 * the return value ERR_OK indicates the string has the Mapcode format, much like string comparison strcmp returns.)
 *
 * Arguments:
 *      utf8String/utf16String - Mapcode string to check, in UTF8 or UTF16 format.
 *
 * Returns:
 *      ERR_OK if the string has a correct Mapcode format, another ERR_XXX value if the string does
 *      not have a Mapcode format.
 *      Special value ERR_MAPCODE_INCOMPLETE indicates the string could be a Mapcode, but it seems
 *      to lack some characters.
 *      NOTE: a correct Mapcode format does not in itself guarantee the mapcode will decode to
 *      a valid coordinate!
 */
enum MapcodeError compareWithMapcodeFormatUtf8(const char *utf8String);

enum MapcodeError compareWithMapcodeFormatUtf16(const UWORD *utf16String);


/**
 * Convert an ISO3166 territory code to a territory.
 *
 * Arguments:
 *      territoryISO         - String starting with ISO3166 code of territory (e.g. "USA" or "US-CA").
 *      parentTerritoryCode  - Parent territory, or TERRITORY_NONE if not available.
 *
 * Returns:
 *      Territory (> _TERRITORY_MIN) if succeeded, or TERRITORY_NONE if failed.
 */
enum Territory getTerritoryCode(
        const char *territoryISO,
        enum Territory optionalTerritoryContext);


/**
 * Convert a territory to a territory name.
 *
 * Arguments:
 *      territoryISO    - String to territory ISO code name result.
 *      territory       - Territory to get the name of.
 *      userShortName   - Pass 0 for full name, 1 for short name (state codes may be ambiguous).
 *
 * Returns:
 *      Pointer to result. String will be empty if territory illegal.
 */
char *getTerritoryIsoName(
        char *territoryISO,
        enum Territory territory,
        int useShortName);


/**
 * Given a territory, return the territory itself it it was a country, or return its parent
 * territory if it was a subdivision (e.g. a state).
 *
 * Arguments:
 *      territory   - territory (either a country or a subdivision, e.g. a state).
 *
 * Returns:
 *      Territory of the parent country (if the territory has one), or the territory itself.
 *      TERRITORY_NONE if the territory was invalid.
 */
enum Territory getCountryOrParentCountry(enum Territory territory);


/**
 * Given a territory, return its parent country.
 *
 * Arguments:
 *      territory   - territory to get the parent of.
 *
 * Returns:
 *      Territory of the parent country.
 *      TERRITORY_NONE if the territory was not a subdivision, or invalid.
 */
enum Territory getParentCountryOf(enum Territory territory);


/**
 * Returns the distance in meters between two coordinates (latitude/longitude pairs).
 * Important: only accurate for coordinates within a few kilometers from each other.
 */
double distanceInMeters(double latDeg1, double lonDeg1, double latDeg2, double lonDeg2);


/**
 * Returns how far, at worst, a decoded mapcode can be from the original encoded coordinate.
 *
 * Arguments:
 *      extraDigits     - Number of extra "digits" in the mapcode. Extra letters added to mapcodes
 *                        make them represent coordinates more accurately. Must be >= 0.
 *
 * Returns:
 *      The worst-case distance in meters between a decoded mapcode and the encoded coordinate.
 */
double maxErrorInMeters(int extraDigits);


/**
 * Returns whether a coordinate is near more than one territory border.
 *
 * Arguments:
 *      lat             - Latitude, in degrees. Range: -90..90.
 *      lon             - Longitude, in degrees. Range: -180..180.
 *      territory       - Territory
 *
 * Return value:
 *      0 if coordinate is NOT near more than one territory border, non-0 otherwise.
 *
 * Note that for the mapcode system, the following should hold: IF a point p has a 
 * mapcode M, THEN decode(M) delivers a point q within maxErrorInMeters() of p.
 * Furthermore, encode(q) must yield back M *unless* point q is near multiple borders.
 */
int multipleBordersNearby(
        double latDeg,
        double lonDeg,
        enum Territory territory);


/**
 * Returns territory names in English. There's always at least 1 alternative (with index 0).
 *
 *   Arguments:
 *       territoryName - Target string, allocated by caller to be at least MAX_TERRITORY_FULLNAME_ASCII_LEN + 1 bytes.
 *       territory     - Territory to get name for.
 *       alternative   - Which name to get, must be >= 0 (0 = default, 1 = first alternative, 2 = second etc.).
 *
 *   Return value:
 *       0 if the alternative does not exist (territoryName will be empty).
 *       non-0 if the alternative exists (territoryName contains name).
 */
int getFullTerritoryNameEnglish(
        char *territoryName,
        enum Territory territory,
        int alternative);


/**
 * Returns territory names in the local language. There are two variants of this call. One returns local
 * territory names in a specified alphabet only. The other simply returns the local names, regardless
 * of its alphabet. There is always at least 1 alternative, with index 0.
 *
 *   Arguments:
 *       territoryName - Target string, allocated by caller to be at least MAX_TERRITORY_FULLNAME_UTF8_LEN + 1 bytes.
 *       territory     - Territory to get name for.
 *       alternative   - Which name to get, must be >= 0 (0 = default, 1 = first alternative, 2 = second etc.).
 *       alphabet      - Alphabet to use for territoryName. Must be a valid alphabet value.
 *
 *   Return value:
 *       0 if the alternative does not exist (territoryName will be empty).
 *       non-0 if the alternative exists (territoryName contains name).
 */
int getFullTerritoryNameLocalUtf8(
        char *territoryName,
        enum Territory territory,
        int alternative);

int getFullTerritoryNameLocalInAlphabetUtf8(
        char *territoryName,
        enum Territory territory,
        int alternative,
        enum Alphabet alphabet);


/**
 * Returns territory names in a specific locale. There are two variants of this call. One returns
 * territory names in a specified alphabet only. The other simply returns the local names, regardless
 * of its alphabet. There is always at least 1 alternative, with index 0.
 *
 *   Arguments:
 *       territoryName - Target string, allocated by caller to be at least MAX_TERRITORY_FULLNAME_UTF8_LEN + 1 bytes.
 *       territory     - Territory to get name for.
 *       alternative   - Which name to get, must be >= 0 (0 = default, 1 = first alternative, 2 = second etc.).
 *       locale        - A locale (e.g. "en_US" for U.S. English); use NULL for local territory names.
 *       alphabet      - Alphabet to use for territoryName. Must be a valid alphabet value.
 *
 *   Return value:
 *       0 if the alternative does not exist (territoryName will be empty).
 *       non-0 if the alternative exists (territoryName contains name).
 */
int getFullTerritoryNameInLocaleUtf8(
        char *territoryName,
        enum Territory territory,
        int alternative,
        const char *locale);

int getFullTerritoryNameInLocaleInAlphabetUtf8(
        char *territoryName,
        enum Territory territory,
        int alternative,
        const char *locale,
        enum Alphabet alphabet);


/**
 * This struct contains the returned alphabest for getAlphabetsForTerritory. The 'count' specifies
 * how many alphabets are listed in 'alphabet', range [1, MAX_ALPHABETS_PER_TERRITORY].
 */
#define MAX_ALPHABETS_PER_TERRITORY 3

typedef struct {
    int count;
    enum Alphabet alphabet[MAX_ALPHABETS_PER_TERRITORY];
} TerritoryAlphabets;


/**
 * Given a territory, returns a structure defining which alphabets (in order of importance)
 * are in common use in the territory.
 *
 * Arguments:
 *      territory   - Territory to get the common alphabets for.
 *
 * Returns:
 *      A pointer to a TerritoryAlphabets structure, or NULL if the territory is invalid.
 *      (The pointer is owned by the library and should not be dealloacted by the caller.)
 */
const TerritoryAlphabets *getAlphabetsForTerritory(enum Territory territory);


/**
 * Encode a string to Alphabet characters for a language.
 *
 * Arguments:
 *      utf8String   - Buffer to be filled with the Unicode string result.
 *                     Must have capacity for MAX_MAPCODE_RESULT_UTF8_LEN + 1 characters.
 *      asciiString  - ASCII string to encode (must be < MAX_MAPCODE_RESULT_ASCII_LEN characters).
 *      alphabet     - Alphabet to use.
 *
 * Returns:
 *      Encode UTF8 string (pointer to utf8String buffer), allocated and deallocated by the caller.
 */
char *convertMapcodeToAlphabetUtf8(char *utf8String, const char *asciiString, enum Alphabet alphabet);


/**
 * Encode a string to Alphabet characters for a language.
 *
 * Arguments:
 *      utf16String  - Buffer to be filled with the Unicode string result.
 *                     Must have capacity for MAX_MAPCODE_RESULT_UTF16_LEN 16-bit characters.
 *      asciiString  - ASCII string to encode (must be < MAX_MAPCODE_RESULT_ASCII_LEN characters).
 *      alphabet     - Alphabet to use.
 *
 * Returns:
 *      Encode UTF16 string (pointer to utf16String buffer), allocated and deallocated by the caller.
 */
UWORD *convertMapcodeToAlphabetUtf16(UWORD *utf16String, const char *asciiString, enum Alphabet alphabet);


#ifdef __cplusplus
}
#endif

#endif
