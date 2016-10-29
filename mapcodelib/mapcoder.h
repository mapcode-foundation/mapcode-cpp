/*
 * Copyright (C) 2014-2016 Stichting Mapcode Foundation (http://www.mapcode.com)
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

#ifndef NO_SUPPORT_ALPHABETS

#include "mapcode_alphabets.h"

#endif


#define MAPCODE_C_VERSION          "2.4.1"
#define UWORD                      unsigned short int  // 2-byte unsigned integer.
#define MAX_NR_OF_MAPCODE_RESULTS  22          // Max. number of results ever returned by encoder (e.g. for 26.904899, 95.138515).
#define MAX_PROPER_MAPCODE_LEN     11          // Max. number of characters in a proper mapcode (including the dot, excl. precision extension).
#define MAX_PRECISION_DIGITS       8           // Max. number of extension characters (excluding the hyphen). Must be even.
#define MAX_ISOCODE_LEN            7           // Max. number of characters of a valid ISO3166 territory code; although nothing longer than SIX characters is ever generated (RU-KAM), users can input SEVEN characters (RUS-KAM).
#define MAX_TERRITORY_FULLNAME_LEN 79          // Max. number of characters to store the longest possible territory name (in any alphabet, excl. 0-terminator).
#define MAX_CLEAN_MAPCODE_LEN      (MAX_PROPER_MAPCODE_LEN + 1 + MAX_PRECISION_DIGITS)  // Max. number of characters in a clean mapcode (excluding zero-terminator).
#define MAX_MAPCODE_RESULT_LEN     (MAX_ISOCODE_LEN + 1 + MAX_CLEAN_MAPCODE_LEN + 1)    // Max. number of characters to store a single result (including zero-terminator).


/**
 * The type Mapcodes hold a number of mapcodes, for example from an encoding call.
 * If a result contains a space, that space seperates the territory ISO3166 code from the mapcode.
 * International mapcodes never include a territory ISO3166 code, nor a space.
 */
typedef struct {
    int count;                                                        // The number of mapcode results (length of array).
    char mapcode[MAX_NR_OF_MAPCODE_RESULTS][MAX_MAPCODE_RESULT_LEN];  // The mapcodes.
} Mapcodes;


typedef struct {
    char territoryISO[MAX_ISOCODE_LEN + 1];            // The (trimmed and uppercased) candidate territory ISO3166 code.
    enum Territory territoryCode;                      // The territory, as recognized and disambiguated from territoryISO.
    char properMapcode[MAX_PROPER_MAPCODE_LEN +
                       1];    // The (trimmed and uppercased) candidate proper mapcode (ex territory or extension).
    int indexOfDot;                                    // Position of dot in properMapcode (a value between 2 and 5).
    char precisionExtension[MAX_PRECISION_DIGITS + 1]; // The (trimmed and uppercased) candidate ex excluding hyphen.
} MapcodeElements;


/**
 * List of error return codes (negative except for ERR_OK = 0)
 */
enum MapcodeError {

    // Note: an incomplete mapcode could "become" complete by adding letters.

    ERR_MAPCODE_INCOMPLETE = -999,   // not enough letters (yet) after dot

    // Format errors.

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

    // Parse errors.

    ERR_UNKNOWN_TERRITORY = -199,    // mapcode territory not recognized

    // other errors

    ERR_BAD_MAPCODE_LENGTH = -99,    // proper mapcode too short or too long
    ERR_MISSING_TERRITORY,           // mapcode can not be decoded without territory
    ERR_EXTENSION_UNDECODABLE,       // extension does not decode to valid coordinate
    ERR_MAPCODE_UNDECODABLE,         // mapcode does not decode inside territory
    ERR_BAD_COORDINATE,              // latitude or longitude is NAN or infinite

    // All OK.

    ERR_OK = 0,
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
 *                        The caller should allocate at least MAX_MAPCODE_RESULT_LEN characters for the string.
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
 * Decode a Mapcode to  a latitude, longitude pair (in degrees).
 *
 * Arguments:
 *      lat             - Decoded latitude, in degrees. Range: -90..90.
 *      lon             - Decoded longitude, in degrees. Range: -180..180.
 *      mapcode         - Mapcode to decode.
 *      territory       - Territory (e.g. as obtained from getTerritoryCode), used as decoding context.
 *                        Pass TERRITORY_NONE if not available.
 *
 * Returns:
 *      0 if encoding succeeded, nonzero in case of error.
 */
enum MapcodeError decodeMapcodeToLatLon(
        double *latDeg,
        double *lonDeg,
        const char *mapcode,
        enum Territory territory);


/**
 * Checks if a string has the format of a Mapcode. (Note: The method is called compareXXX rather than hasXXX because
 * the return value '0' indicates the string has the Mapcode format, much like string comparison strcmp returns.)
 *
 * Arguments:
 *      asciiString             - Mapcode string to check.
 *      stringIncludesTerritory - Pass 0 if you are certain that no territory is (or should be) included in the string. 
 *                                Pass 1 if territory information MAY be available in the string.
 * Returns:
 *      0 if the string has a correct Mapcode format; <0 if the string does not have a Mapcode format.
 *      Special value ERR_MAPCODE_INCOMPLETE (-999) indicates the string could be a Mapcode, but it seems
 *      to lack some characters.
 *      NOTE: a correct Mapcode format does not in itself guarantee the mapcode will decode to a valid coordinate!
 */
enum MapcodeError compareWithMapcodeFormat(
        const char *asciiString,
        int stringIncludesTerritory);


/**
 * Parses a string into its mapcode components, separating the territory, the 'proper' mapcode (without the
 * territory and the precision extension) and the precision extension.
 *
 * Arguments:
 *      mapcodeFormat             - If not NULL, filled with analysis of the 'check' string if it was a correct mapcode.
 *                                  Pass NULL to skip returning the additional analysis.
 *      asciiString               - Mapcode string to check.
 *      stringIncludesTerritory   - Pass 0 if you are certain that no territory is (or should be) included in the string. 
 *                                  Pass 1 if territory information MAY be available in the string.
 *      territory                 - Territory (e.g. as obtained from getTerritoryCode), used as decoding context in case
 *                                  the mapcode string does not contain an (unambiguous) ISO3166 territory code.
 *                                  Pass 0 if not available.
 * Returns:
 *      Returns 0 if the string has a correct Mapcode format.
 *      Return <0 if the string does not have a Mapcode format.
 *      Special value ERR_MAPCODE_INCOMPLETE indicates the string could be a Mapcode, but it seems
 *      to lack some characters.
 *      If 0 is returned, the structure mapcodeFormat is correctly filled with (trimmed, uppercased) components
 *      if the passed pointer to mapcodeFormat was not NULL.
 *      NOTE: a correct Mapcode format does not in itself guarantee the mapcode will decode to a valid coordinate!
 */
enum MapcodeError parseMapcodeString(
        MapcodeElements *mapcodeElements,
        const char *asciiString,
        int stringIncludesTerritory,
        enum Territory territory);

/**
 * Convert an ISO3166 territory code to a territory.
 *
 * Arguments:
 *      territoryISO         - String starting with ISO3166 code of territory (e.g. "USA" or "US-CA").
 *      parentTerritoryCode  - Parent territory, or TERRITORY_NONE if not available.
 *
 * Returns:
 *      Territory (>0) if succeeded, or <0 (TERRITORY_NONE) if failed.
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
 *      territory   - territory (either a country or a subdivision, e.g. a state)
 *
 * Returns:
 *      Territory of the parent country (if the territory has one), or the territory itself.
 *      <0 (TERRITORY_NONE) if the territory was invalid.
 */
enum Territory getCountryOrParentCountry(enum Territory territory);


/**
 * Given a territory, return its parent country.
 *
 * Arguments:
 *      territory   - territory to get the parent of
 *
 * Returns:
 *      Territory of the parent country; 
 *      <0 (TERRITORY_NONE) if the territory was not a subdivision, or invalid.
 */
enum Territory getParentCountryOf(enum Territory territory);


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
 *                        make them represent coordinates more accurately. Must be >= 0.
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
 *      territory       - Territory
 *
 * Return value:
 *      non-0 if coordinate is near more than one territory border
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
 * Returns territory names in English or in the local language.
 *
 *   Arguments:
 *       territoryName - Target string, allocated by caller to be at least MAX_TERRITORY_NAME_LENGTH + 1 bytes.
 *       territory     - Territory to get name for.
 *       alternative   - Which name to get, must be >= 0 (0 = default, 1 = first alternative, 2 = second etc.).
 *
 *   Return value:
 *       non-0 if more alternatives are available (call again with alternative + 1).
 *       0 if no more alternatives are available.
*/

int getFullTerritoryNameEnglish(char *territoryName, enum Territory territory, int alternative);


/* ----------------------------------------------------------------------------
 * ALPHABET SUPPORT
 * ----------------------------------------------------------------------------
 *
 * Use -DNO_SUPPORT_ALPHABETS as a compiler option to switch off alphabet support for
 * more alphabets. If NO_SUPPORT_ALPHABETS is not defined, alphabets other than ROMAN
 * are supported.
 * ----------------------------------------------------------------------------
 */

#ifndef NO_SUPPORT_ALPHABETS

/**
 * Returns territory names in the local language.
 *
 *   Arguments:
 *       territoryName - Target string, allocated by caller to be at least MAX_TERRITORY_NAME_LENGTH + 1 bytes.
 *       territory     - Territory to get name for.
 *       alternative   - Which name to get, must be >= 0 (0 = default, 1 = first alternative, 2 = second etc.).
 *       alphabet      - Alphabet to use for territoryName.
 *
 *   Return value:
 *       non-0 if more alternatives are available (call again with alternative + 1).
 *       0 if no more alternatives are available.
*/
int getFullTerritoryNameLocal(char *territoryName, enum Territory territory, int alternative, enum Alphabet alphabet);


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
 * Given a territory, returns a structure defining which alphabets (in order of importance) are in common use in the territory
 *
 * Arguments:
 *      territory   - territory to get the common alphabets for.
 *
 * Returns:
 *      A pointer to a TerritoryAlphabets structure (or NULL if the territory is invalid).
 */
const TerritoryAlphabets *getAlphabetsForTerritory(enum Territory territory);


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
 * Convert a zero-terminated UTF16 (containing codes in range 0x0001 - 0xFFFF
 * to a UTF8 string (caller must make sure there is sufficient room in utf8).
 *
 * Arguments:
 *      utf16  - Zero-terminated UTF16 words
 *      utf8   - Target string
 */
void convertUtf16ToUtf8(char *utf8String, const UWORD *utf16String);


/**
 * Convert a zero-terminated UTF8 (or ASCII) string to a UTF16 string.
 * (caller must make sure there is sufficient room in uitf8).
 *
 * Arguments:
 *      utf8   - Zero-terminated UTF8 (or ASCII) string
 *      utf16  - Target string
 *
 * Returns:
 *      0 if successful, negative if an utf8 error is encountered
 *      or a utf8 value is encountered that exceeds 0xFFFF.
 */
int convertUtf8ToUtf16(UWORD *utf16, const char *utf8);


/**
 * Returns the alphabet of given UTF8 (of ASCII) string (based on the
 * first recognisable non-Latin character).
 *
 * Arguments:
 *      utf16  - Zero-terminated UTF16 string.
 *
 * Returns:
 *      ALPHABET_ROMAN if all characters are in ASCII range 0..0xBF.
 *      Otherwise returns the alphabet of the first different character
 *      encountered, or negative (_ALPHABET_MIN) if it isn't recognized.
 */
enum Alphabet recognizeAlphabetUtf16(const UWORD *utf16String);


/**
 * Returns the alphabet of given UTF8 (of ASCII) string (based on the
 * first recognisable non-Latin character).
 *
 * Arguments:
 *      utf8   - Zero-terminated UTF8 (or ASCII) string
 *
 * Returns:
 *      ALPHABET_ROMAN if all characters are in ASCII range 0..0xBF.
 *      Otherwise returns the alphabet of the first different character
 *      encountered, or negative (_ALPHABET_MIN) if it isn't recognized.
 */
enum Alphabet recognizeAlphabetUtf8(const char *utf8);


#endif // NO_SUPPORT_ALPHABETS

#ifdef __cplusplus
}
#endif
#endif
