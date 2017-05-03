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
#ifndef __MAPCODE_ALPHABETS_H__
#define __MAPCODE_ALPHABETS_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Mapcodes are suppored in a variety of alphabets, using UTF16. The following
 * enum specifies the alphabets (or scripts, as they are also called).
 * The 'default' alphabet is Roman, which is always supported. Other
 * alphabets may not be supported by every application that accepts
 * mapcodes.
 */
enum Alphabet {
    _ALPHABET_MIN = -1,
    ALPHABET_ROMAN,
    ALPHABET_GREEK,
    ALPHABET_CYRILLIC,
    ALPHABET_HEBREW,
    ALPHABET_DEVANAGARI,
    ALPHABET_MALAYALAM,
    ALPHABET_GEORGIAN,
    ALPHABET_KATAKANA,
    ALPHABET_THAI,
    ALPHABET_LAO,
    ALPHABET_ARMENIAN,
    ALPHABET_BENGALI,
    ALPHABET_GURMUKHI,
    ALPHABET_TIBETAN,
    ALPHABET_ARABIC,
    ALPHABET_KOREAN,
    ALPHABET_BURMESE,
    ALPHABET_KHMER,
    ALPHABET_SINHALESE,
    ALPHABET_THAANA,
    ALPHABET_CHINESE,
    ALPHABET_TIFINAGH,
    ALPHABET_TAMIL,
    ALPHABET_AMHARIC,
    ALPHABET_TELUGU,
    ALPHABET_ODIA,
    ALPHABET_KANNADA,
    ALPHABET_GUJARATI,
    _ALPHABET_MAX
};

#ifdef __cplusplus
}
#endif

#endif // __MAPCODE_ALPHABETS_H__

