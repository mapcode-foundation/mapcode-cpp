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
#ifndef __MAPCODE_ALPHABETS_H__
#define __MAPCODE_ALPHABETS_H__


#ifdef __cplusplus
extern "C" {
#endif

/**
 * Alphabets:
 */
#define MAPCODE_ALPHABETS_TOTAL        28

#define MAPCODE_ALPHABET_ROMAN         0
#define MAPCODE_ALPHABET_GREEK         1
#define MAPCODE_ALPHABET_CYRILLIC      2
#define MAPCODE_ALPHABET_HEBREW        3
#define MAPCODE_ALPHABET_DEVANAGARI    4
#define MAPCODE_ALPHABET_MALAYALAM     5
#define MAPCODE_ALPHABET_GEORGIAN      6
#define MAPCODE_ALPHABET_KATAKANA      7
#define MAPCODE_ALPHABET_THAI          8
#define MAPCODE_ALPHABET_LAO           9
#define MAPCODE_ALPHABET_ARMENIAN      10
#define MAPCODE_ALPHABET_BENGALI       11
#define MAPCODE_ALPHABET_GURMUKHI      12
#define MAPCODE_ALPHABET_TIBETAN       13
#define MAPCODE_ALPHABET_ARABIC        14
#define MAPCODE_ALPHABET_KOREAN        15
#define MAPCODE_ALPHABET_BURMESE       16
#define MAPCODE_ALPHABET_KHMER         17
#define MAPCODE_ALPHABET_SINHALESE     18
#define MAPCODE_ALPHABET_THAANA        19
#define MAPCODE_ALPHABET_CHINESE       20
#define MAPCODE_ALPHABET_TIFINAGH      21
#define MAPCODE_ALPHABET_TAMIL         22
#define MAPCODE_ALPHABET_AMHARIC       23
#define MAPCODE_ALPHABET_TELUGU        24
#define MAPCODE_ALPHABET_ODIA          25
#define MAPCODE_ALPHABET_KANNADA       26
#define MAPCODE_ALPHABET_GUJARATI      27

#define MAX_ALPHABETS_PER_TERRITORY    3
typedef struct {
    int count;
    int alphabet[MAX_ALPHABETS_PER_TERRITORY];
} TerritoryAlphabets;

#ifdef __cplusplus
}
#endif

#endif // __MAPCODE_ALPHABETS_H__

