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

#ifndef __MAPCODE_LEGACY_H__
#define __MAPCODE_LEGACY_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "mapcoder.h"

/**
 * List of #defines to support legacy systems.
 */
#define convertTerritoryIsoNameToCode getTerritoryCode
#define coord2mc(results, lat, lon, territory)  encodeLatLonToMapcodes_Deprecated(results, lat, lon,territory, 0)
#define coord2mc1(results, lat, lon, territory) encodeLatLonToSingleMapcode(results, lat, lon, territory, 0)
#define mc2coord decodeMapcodeToLatLon
#define lookslikemapcode compareWithMapcodeFormat
#define text2tc getTerritoryCode
#define tc2text convertTerritoryCodeToIsoName
#define tccontext getCountryOrParentCountry
#define tcparent getParentCountryOf
#define decode_to_roman decodeToRoman
#define encode_to_alphabet encodeToAlphabet
#define MAX_MAPCODE_TERRITORY_CODE MAPCODE_NR_TERRITORIES
#define NR_BOUNDARY_RECS MAPCODE_NR_RECS

#define COMPARE_MAPCODE_MISSING_CHARACTERS  ERR_MAPCODE_INCOMPLETE

#define MAX_LANGUAGES                  _MAPCODE_ALPHABETS_MAX
#define MAPCODE_LANGUAGE_ROMAN         ALPHABET_ROMAN
#define MAPCODE_LANGUAGE_GREEK         ALPHABET_GREEK
#define MAPCODE_LANGUAGE_CYRILLIC      ALPHABET_CYRILLIC
#define MAPCODE_LANGUAGE_HEBREW        ALPHABET_HEBREW
#define MAPCODE_LANGUAGE_HINDI         ALPHABET_DEVANAGARI
#define ALPHABET_HINDI         ALPHABET_DEVANAGARI
#define MAPCODE_LANGUAGE_MALAYALAM     ALPHABET_MALAYALAM
#define MAPCODE_LANGUAGE_GEORGIAN      ALPHABET_GEORGIAN
#define MAPCODE_LANGUAGE_KATAKANA      ALPHABET_KATAKANA
#define MAPCODE_LANGUAGE_THAI          ALPHABET_THAI
#define MAPCODE_LANGUAGE_LAO           ALPHABET_LAO
#define MAPCODE_LANGUAGE_ARMENIAN      ALPHABET_ARMENIAN
#define MAPCODE_LANGUAGE_BENGALI       ALPHABET_BENGALI
#define MAPCODE_LANGUAGE_GURMUKHI      ALPHABET_GURMUKHI
#define MAPCODE_LANGUAGE_TIBETAN       ALPHABET_TIBETAN
#define MAPCODE_LANGUAGE_ARABIC        ALPHABET_ARABIC
// Some alphabets are missing because they were never supported in the legacy library.

#ifdef __cplusplus
}
#endif
#endif // __MAPCODE_LEGACY_H__
