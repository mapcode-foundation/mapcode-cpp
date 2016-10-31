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

#ifndef __INTERNAL_ALPHABET_RECOGNIZER_H__
#define __INTERNAL_ALPHABET_RECOGNIZER_H__

#ifdef __cplusplus
extern "C" {
#endif

// *** GENERATED FILE (dividemaps.cpp), DO NOT CHANGE OR PRETTIFY ***

// Returns alphabet of character, or negative (_ALPHABET_MIN) in not recognized
#define ALPHABET_OF_CHAR(w) ((enum Alphabet) ALPHABET_OF[(w) >> 6])

static const signed char ALPHABET_OF[] = {
    /* 0000 */ ALPHABET_ROMAN       , ALPHABET_ROMAN       , ALPHABET_ROMAN       , ALPHABET_ROMAN,
    /* 0100 */ ALPHABET_ROMAN       , ALPHABET_ROMAN       , ALPHABET_ROMAN       , ALPHABET_ROMAN,
    /* 0200 */ ALPHABET_ROMAN       , ALPHABET_ROMAN       , ALPHABET_ROMAN       , ALPHABET_ROMAN,
    /* 0300 */ ALPHABET_ROMAN       , _ALPHABET_MIN        , ALPHABET_GREEK       , ALPHABET_GREEK,
    /* 0400 */ ALPHABET_CYRILLIC    , ALPHABET_CYRILLIC    , ALPHABET_CYRILLIC    , ALPHABET_CYRILLIC,
    /* 0500 */ ALPHABET_ARMENIAN    , ALPHABET_ARMENIAN    , ALPHABET_ARMENIAN    , ALPHABET_HEBREW,
    /* 0600 */ ALPHABET_ARABIC      , ALPHABET_ARABIC      , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* 0700 */ _ALPHABET_MIN        , _ALPHABET_MIN        , ALPHABET_THAANA      , _ALPHABET_MIN,
    /* 0800 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* 0900 */ ALPHABET_DEVANAGARI  , ALPHABET_DEVANAGARI  , ALPHABET_BENGALI     , ALPHABET_BENGALI,
    /* 0a00 */ ALPHABET_GURMUKHI    , ALPHABET_GURMUKHI    , ALPHABET_GUJARATI    , _ALPHABET_MIN,
    /* 0b00 */ ALPHABET_ODIA        , _ALPHABET_MIN        , ALPHABET_TAMIL       , ALPHABET_TAMIL,
    /* 0c00 */ ALPHABET_TELUGU      , _ALPHABET_MIN        , ALPHABET_KANNADA     , _ALPHABET_MIN,
    /* 0d00 */ ALPHABET_MALAYALAM   , ALPHABET_MALAYALAM   , ALPHABET_SINHALESE   , ALPHABET_SINHALESE,
    /* 0e00 */ ALPHABET_THAI        , ALPHABET_THAI        , ALPHABET_LAO         , ALPHABET_LAO,
    /* 0f00 */ ALPHABET_TIBETAN     , ALPHABET_TIBETAN     , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* 1000 */ ALPHABET_BURMESE     , ALPHABET_BURMESE     , ALPHABET_GEORGIAN    , ALPHABET_GEORGIAN,
    /* 1100 */ ALPHABET_KOREAN      , ALPHABET_KOREAN      , ALPHABET_KOREAN      , ALPHABET_KOREAN,
    /* 1200 */ ALPHABET_AMHARIC     , ALPHABET_AMHARIC     , ALPHABET_AMHARIC     , ALPHABET_AMHARIC,
    /* 1300 */ ALPHABET_AMHARIC     , ALPHABET_AMHARIC     , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* 1400 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* 1500 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* 1600 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* 1700 */ _ALPHABET_MIN        , _ALPHABET_MIN        , ALPHABET_KHMER       , ALPHABET_KHMER,
    /* 1800 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* 1900 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* 1a00 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* 1b00 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* 1c00 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* 1d00 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* 1e00 */ ALPHABET_ROMAN       , ALPHABET_ROMAN       , ALPHABET_ROMAN       , ALPHABET_ROMAN,
    /* 1f00 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* 2000 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* 2100 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* 2200 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* 2300 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* 2400 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* 2500 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* 2600 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* 2700 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* 2800 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* 2900 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* 2a00 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* 2b00 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* 2c00 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* 2d00 */ ALPHABET_TIFINAGH    , ALPHABET_TIFINAGH    , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* 2e00 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* 2f00 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* 3000 */ ALPHABET_KATAKANA    , ALPHABET_KATAKANA    , ALPHABET_KATAKANA    , ALPHABET_KATAKANA,
    /* 3100 */ ALPHABET_CHINESE     , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* 3200 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* 3300 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* 3400 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* 3500 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* 3600 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* 3700 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* 3800 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* 3900 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* 3a00 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* 3b00 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* 3c00 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* 3d00 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* 3e00 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* 3f00 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* 4000 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* 4100 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* 4200 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* 4300 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* 4400 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* 4500 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* 4600 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* 4700 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* 4800 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* 4900 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* 4a00 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* 4b00 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* 4c00 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* 4d00 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* 4e00 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 4f00 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 5000 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 5100 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 5200 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 5300 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 5400 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 5500 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 5600 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 5700 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 5800 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 5900 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 5a00 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 5b00 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 5c00 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 5d00 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 5e00 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 5f00 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 6000 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 6100 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 6200 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 6300 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 6400 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 6500 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_KATAKANA,
    /* 6600 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 6700 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 6800 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 6900 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 6a00 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 6b00 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 6c00 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 6d00 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 6e00 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 6f00 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 7000 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 7100 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 7200 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 7300 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 7400 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 7500 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 7600 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 7700 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 7800 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 7900 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 7a00 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 7b00 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 7c00 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 7d00 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 7e00 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 7f00 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 8000 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 8100 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 8200 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 8300 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 8400 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 8500 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 8600 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 8700 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 8800 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 8900 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 8a00 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 8b00 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 8c00 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 8d00 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 8e00 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 8f00 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 9000 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 9100 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 9200 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 9300 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 9400 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 9500 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 9600 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 9700 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 9800 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 9900 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 9a00 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 9b00 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 9c00 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 9d00 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 9e00 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* 9f00 */ ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE     , ALPHABET_CHINESE,
    /* a000 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* a100 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* a200 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* a300 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* a400 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* a500 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* a600 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* a700 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* a800 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* a900 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* aa00 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* ab00 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* ac00 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* ad00 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* ae00 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* af00 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* b000 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* b100 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* b200 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* b300 */ ALPHABET_KOREAN      , ALPHABET_KOREAN      , ALPHABET_KOREAN      , ALPHABET_KOREAN,
    /* b400 */ ALPHABET_KOREAN      , ALPHABET_KOREAN      , ALPHABET_KOREAN      , ALPHABET_KOREAN,
    /* b500 */ ALPHABET_KOREAN      , ALPHABET_KOREAN      , ALPHABET_KOREAN      , ALPHABET_KOREAN,
    /* b600 */ ALPHABET_KOREAN      , ALPHABET_KOREAN      , ALPHABET_KOREAN      , ALPHABET_KOREAN,
    /* b700 */ ALPHABET_KOREAN      , ALPHABET_KOREAN      , ALPHABET_KOREAN      , ALPHABET_KOREAN,
    /* b800 */ ALPHABET_KOREAN      , ALPHABET_KOREAN      , ALPHABET_KOREAN      , ALPHABET_KOREAN,
    /* b900 */ ALPHABET_KOREAN      , ALPHABET_KOREAN      , ALPHABET_KOREAN      , ALPHABET_KOREAN,
    /* ba00 */ ALPHABET_KOREAN      , ALPHABET_KOREAN      , ALPHABET_KOREAN      , ALPHABET_KOREAN,
    /* bb00 */ ALPHABET_KOREAN      , ALPHABET_KOREAN      , ALPHABET_KOREAN      , ALPHABET_KOREAN,
    /* bc00 */ ALPHABET_KOREAN      , ALPHABET_KOREAN      , ALPHABET_KOREAN      , ALPHABET_KOREAN,
    /* bd00 */ ALPHABET_KOREAN      , ALPHABET_KOREAN      , ALPHABET_KOREAN      , ALPHABET_KOREAN,
    /* be00 */ ALPHABET_KOREAN      , ALPHABET_KOREAN      , ALPHABET_KOREAN      , ALPHABET_KOREAN,
    /* bf00 */ ALPHABET_KOREAN      , ALPHABET_KOREAN      , ALPHABET_KOREAN      , ALPHABET_KOREAN,
    /* c000 */ ALPHABET_KOREAN      , ALPHABET_KOREAN      , ALPHABET_KOREAN      , ALPHABET_KOREAN,
    /* c100 */ ALPHABET_KOREAN      , ALPHABET_KOREAN      , ALPHABET_KOREAN      , ALPHABET_KOREAN,
    /* c200 */ ALPHABET_KOREAN      , ALPHABET_KOREAN      , ALPHABET_KOREAN      , ALPHABET_KOREAN,
    /* c300 */ ALPHABET_KOREAN      , ALPHABET_KOREAN      , ALPHABET_KOREAN      , ALPHABET_KOREAN,
    /* c400 */ ALPHABET_KOREAN      , ALPHABET_KOREAN      , ALPHABET_KOREAN      , ALPHABET_KOREAN,
    /* c500 */ ALPHABET_KOREAN      , ALPHABET_KOREAN      , ALPHABET_KOREAN      , ALPHABET_KOREAN,
    /* c600 */ ALPHABET_KOREAN      , ALPHABET_KOREAN      , ALPHABET_KOREAN      , ALPHABET_KOREAN,
    /* c700 */ ALPHABET_KOREAN      , ALPHABET_KOREAN      , ALPHABET_KOREAN      , ALPHABET_KOREAN,
    /* c800 */ ALPHABET_KOREAN      , ALPHABET_KOREAN      , ALPHABET_KOREAN      , ALPHABET_KOREAN,
    /* c900 */ ALPHABET_KOREAN      , ALPHABET_KOREAN      , ALPHABET_KOREAN      , ALPHABET_KOREAN,
    /* ca00 */ ALPHABET_KOREAN      , ALPHABET_KOREAN      , ALPHABET_KOREAN      , ALPHABET_KOREAN,
    /* cb00 */ ALPHABET_KOREAN      , ALPHABET_KOREAN      , ALPHABET_KOREAN      , ALPHABET_KOREAN,
    /* cc00 */ ALPHABET_KOREAN      , ALPHABET_KOREAN      , ALPHABET_KOREAN      , ALPHABET_KOREAN,
    /* cd00 */ ALPHABET_KOREAN      , ALPHABET_KOREAN      , ALPHABET_KOREAN      , ALPHABET_KOREAN,
    /* ce00 */ ALPHABET_KOREAN      , ALPHABET_KOREAN      , ALPHABET_KOREAN      , ALPHABET_KOREAN,
    /* cf00 */ ALPHABET_KOREAN      , ALPHABET_KOREAN      , ALPHABET_KOREAN      , ALPHABET_KOREAN,
    /* d000 */ ALPHABET_KOREAN      , ALPHABET_KOREAN      , ALPHABET_KOREAN      , ALPHABET_KOREAN,
    /* d100 */ ALPHABET_KOREAN      , ALPHABET_KOREAN      , ALPHABET_KOREAN      , ALPHABET_KOREAN,
    /* d200 */ ALPHABET_KOREAN      , ALPHABET_KOREAN      , ALPHABET_KOREAN      , ALPHABET_KOREAN,
    /* d300 */ ALPHABET_KOREAN      , ALPHABET_KOREAN      , ALPHABET_KOREAN      , ALPHABET_KOREAN,
    /* d400 */ ALPHABET_KOREAN      , ALPHABET_KOREAN      , ALPHABET_KOREAN      , ALPHABET_KOREAN,
    /* d500 */ ALPHABET_KOREAN      , ALPHABET_KOREAN      , ALPHABET_KOREAN      , ALPHABET_KOREAN,
    /* d600 */ ALPHABET_KOREAN      , ALPHABET_KOREAN      , ALPHABET_KOREAN      , ALPHABET_KOREAN,
    /* d700 */ ALPHABET_KOREAN      , ALPHABET_KOREAN      , ALPHABET_KOREAN      , _ALPHABET_MIN,
    /* d800 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* d900 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* da00 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* db00 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* dc00 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* dd00 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* de00 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* df00 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* e000 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* e100 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* e200 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* e300 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* e400 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* e500 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* e600 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* e700 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* e800 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* e900 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* ea00 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* eb00 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* ec00 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* ed00 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* ee00 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* ef00 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* f000 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* f100 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* f200 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* f300 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* f400 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* f500 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* f600 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* f700 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* f800 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* f900 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* fa00 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* fb00 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* fc00 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* fd00 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* fe00 */ ALPHABET_ARABIC      , ALPHABET_ARABIC      , ALPHABET_ARABIC      , ALPHABET_ARABIC,
    /* ff00 */ ALPHABET_ARABIC      , ALPHABET_ARABIC      , ALPHABET_ARABIC      , ALPHABET_ARABIC
}; // 1024 records

#define ROMAN_VERSION_MAX_CHAR 0x3129
static const char *ROMAN_VERSION_OF[] = {
    /* 0000 */ "\0                                !?#$%&'()*+,-./0123456789:;<=>?",
    /* 0040 */ "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[?]^_`abcdefghijklmnopqrstuvwxyz{|}~?",
    /* 0080 */ NULL,
    /* 00c0 */ NULL,
    /* 0100 */ NULL,
    /* 0140 */ NULL,
    /* 0180 */ NULL,
    /* 01c0 */ NULL,
    /* 0200 */ NULL,
    /* 0240 */ NULL,
    /* 0280 */ NULL,
    /* 02c0 */ NULL,
    /* 0300 */ NULL,
    /* 0340 */ NULL,
    /* 0380 */ "????????EU???????ABGDFZHQIKLMNCOJP?STYVXRW??????????????????????",
    /* 03c0 */ NULL,
    /* 0400 */ "????????????????AZBGDEFN??KLMHOJPCTYQXSVW????U?R????????????????",
    /* 0440 */ NULL,
    /* 0480 */ NULL,
    /* 04c0 */ NULL,
    /* 0500 */ "??????????????????????????????????????????????????BCDE??FGHI?J?K",
    /* 0540 */ "LM?N?U?PQ?R??STVWXYZ?OA?????????????????????????????????????????",
    /* 0580 */ NULL,
    /* 05c0 */ "????????????????ABCDFIGHJKLMNPQ?ROSETUVWXYZ?????????????????????",
    /* 0600 */ "???????????????????????????????????????IA?BEVCDFGHJKLMNPQRS?????",
    /* 0640 */ "?TU?WXYOZ???????????????????????????????????????????????????????",
    /* 0680 */ NULL,
    /* 06c0 */ NULL,
    /* 0700 */ NULL,
    /* 0740 */ NULL,
    /* 0780 */ "BCDFGHJKLMNPQRSTVWXYA??E????U????????????????????Z??????????????",
    /* 07c0 */ NULL,
    /* 0800 */ NULL,
    /* 0840 */ NULL,
    /* 0880 */ NULL,
    /* 08c0 */ NULL,
    /* 0900 */ "?????A?????????E?????B?CD?F?G??HJ??KL?MNP?QUZRS?T?V??W??XY??????",
    /* 0940 */ "??????????????????????????????????????0123456789????????????????",
    /* 0980 */ "?????????????????????CDFBGH?AJ?UKLMNPQR?S?T?VWEX??Y??????Z??????",
    /* 09c0 */ "??????????????????????????????????????0123456789????????????????",
    /* 0a00 */ "?????A?????????E?????B?CD?F?G??HJZ?KL?MNP?QU?RS?T?V??W??XY??????",
    /* 0a40 */ "??????????????????????????????????????0123456789????????????????",
    /* 0a80 */ "???????E?U???????????H?B??J?CXRKNDY?L?F?W?MTGZS?P??A?Q?V????????",
    /* 0ac0 */ NULL,
    /* 0b00 */ "?????????????????????BCDF?G?HA?JOKLEMNPQR?S?T?VWX??Y????ZU??????",
    /* 0b40 */ NULL,
    /* 0b80 */ "?????SE?TV????WXY?ZU?B???AC????D????F???G?H???JKLRMQPN??????????",
    /* 0bc0 */ NULL,
    /* 0c00 */ "?????????????????????B?C?DF?G?AHJK?LMNPQR?SETUV?W?XY?Z??????????",
    /* 0c40 */ NULL,
    /* 0c80 */ "???????U??????E???A??BCD?FGHJ???KL?MN?PQR?STVW??X?Y??Z??????????",
    /* 0cc0 */ NULL,
    /* 0d00 */ "?????????U?E??????A??BCD??F?G??HOJ??KLMNP?????Q?RST?VWX?YZ??????",
    /* 0d40 */ "??????????????????????????????????????0123456789????????????????",
    /* 0d80 */ "?????A???E?U??????????????B?C??D??F????G?H??JK?L?M?NP?Q?RSTV?W??",
    /* 0dc0 */ "X??YZ???????????????????????????????????????????????????????????",
    /* 0e00 */ "?BC?D??FGHJ??????K??L?MNP?Q?R????S?T?V?W????UXYZA?E?????????????",
    /* 0e40 */ "????????????????0123456789??????????????????????????????????????",
    /* 0e80 */ "?BC?D??FG?H??J??????K??L?MN?P?Q??RST???V??WX?Y?ZA????????????U??",
    /* 0ec0 */ "???EI???????????????????????????????????????????????????????????",
    /* 0f00 */ "????????????????????????????????0123456789??????????????????????",
    /* 0f40 */ "BCD?FGHJ?K?????NMP?QRLS?A?????TVUWXYE?Z?????????????????????????",
    /* 0f80 */ NULL,
    /* 0fc0 */ NULL,
    /* 1000 */ "BCDFGA?J????KLMYNPQESTUWX?H?ZORV????????????????????????????????",
    /* 1040 */ "0123456789??????????????????????????????????????????????????????",
    /* 1080 */ "????????????????????????????????AB?CE?D?UF?GHOJ?KLM?NPQRSTVW?XYZ",
    /* 10c0 */ NULL,
    /* 1100 */ "B?CD?F?G?H?OV?WXJKA?????????????????????????????????????????????",
    /* 1140 */ "?????????????????????????????????LMNPQER?S???YZ???T?UI??????????",
    /* 1180 */ NULL,
    /* 11c0 */ NULL,
    /* 1200 */ "H???????L??????????????????A??E?T?????V?????S???????????????????",
    /* 1240 */ "Q???R???????????????????????????B???C???????????W?????X?????????",
    /* 1280 */ "???????????????????M????????????????????????K???????????????????",
    /* 12c0 */ "????????U???????O???????????Z???????????J???????D???????????????",
    /* 1300 */ "????????G???????????????????????????????????????????????Y???????",
    /* 1340 */ "??????????F?????N???P????????????????????1234567890?????????????",
    /* 1380 */ NULL,
    /* 13c0 */ NULL,
    /* 1400 */ NULL,
    /* 1440 */ NULL,
    /* 1480 */ NULL,
    /* 14c0 */ NULL,
    /* 1500 */ NULL,
    /* 1540 */ NULL,
    /* 1580 */ NULL,
    /* 15c0 */ NULL,
    /* 1600 */ NULL,
    /* 1640 */ NULL,
    /* 1680 */ NULL,
    /* 16c0 */ NULL,
    /* 1700 */ NULL,
    /* 1740 */ NULL,
    /* 1780 */ "BCDFGE?HJAK?LMN??PQRSTV?W?IX???UY?Z?????????????????????????????",
    /* 17c0 */ "????????????????????????????????0123456789??????????????????????",
    /* 1800 */ NULL,
    /* 1840 */ NULL,
    /* 1880 */ NULL,
    /* 18c0 */ NULL,
    /* 1900 */ NULL,
    /* 1940 */ NULL,
    /* 1980 */ NULL,
    /* 19c0 */ NULL,
    /* 1a00 */ NULL,
    /* 1a40 */ NULL,
    /* 1a80 */ NULL,
    /* 1ac0 */ NULL,
    /* 1b00 */ NULL,
    /* 1b40 */ NULL,
    /* 1b80 */ NULL,
    /* 1bc0 */ NULL,
    /* 1c00 */ NULL,
    /* 1c40 */ NULL,
    /* 1c80 */ NULL,
    /* 1cc0 */ NULL,
    /* 1d00 */ NULL,
    /* 1d40 */ NULL,
    /* 1d80 */ NULL,
    /* 1dc0 */ NULL,
    /* 1e00 */ NULL,
    /* 1e40 */ NULL,
    /* 1e80 */ NULL,
    /* 1ec0 */ NULL,
    /* 1f00 */ NULL,
    /* 1f40 */ NULL,
    /* 1f80 */ NULL,
    /* 1fc0 */ NULL,
    /* 2000 */ NULL,
    /* 2040 */ NULL,
    /* 2080 */ NULL,
    /* 20c0 */ NULL,
    /* 2100 */ NULL,
    /* 2140 */ NULL,
    /* 2180 */ NULL,
    /* 21c0 */ NULL,
    /* 2200 */ NULL,
    /* 2240 */ NULL,
    /* 2280 */ NULL,
    /* 22c0 */ NULL,
    /* 2300 */ NULL,
    /* 2340 */ NULL,
    /* 2380 */ NULL,
    /* 23c0 */ NULL,
    /* 2400 */ NULL,
    /* 2440 */ NULL,
    /* 2480 */ NULL,
    /* 24c0 */ NULL,
    /* 2500 */ NULL,
    /* 2540 */ NULL,
    /* 2580 */ NULL,
    /* 25c0 */ NULL,
    /* 2600 */ NULL,
    /* 2640 */ NULL,
    /* 2680 */ NULL,
    /* 26c0 */ NULL,
    /* 2700 */ NULL,
    /* 2740 */ NULL,
    /* 2780 */ NULL,
    /* 27c0 */ NULL,
    /* 2800 */ NULL,
    /* 2840 */ NULL,
    /* 2880 */ NULL,
    /* 28c0 */ NULL,
    /* 2900 */ NULL,
    /* 2940 */ NULL,
    /* 2980 */ NULL,
    /* 29c0 */ NULL,
    /* 2a00 */ NULL,
    /* 2a40 */ NULL,
    /* 2a80 */ NULL,
    /* 2ac0 */ NULL,
    /* 2b00 */ NULL,
    /* 2b40 */ NULL,
    /* 2b80 */ NULL,
    /* 2bc0 */ NULL,
    /* 2c00 */ NULL,
    /* 2c40 */ NULL,
    /* 2c80 */ NULL,
    /* 2cc0 */ NULL,
    /* 2d00 */ "?????????????????????????????????????????????????B?C???D????FG??",
    /* 2d40 */ "H??JKL?M?A???NPI???EOQR??STVW??X?YUZ????????????????????????????",
    /* 2d80 */ NULL,
    /* 2dc0 */ NULL,
    /* 2e00 */ NULL,
    /* 2e40 */ NULL,
    /* 2e80 */ NULL,
    /* 2ec0 */ NULL,
    /* 2f00 */ NULL,
    /* 2f40 */ NULL,
    /* 2f80 */ NULL,
    /* 2fc0 */ NULL,
    /* 3000 */ NULL,
    /* 3040 */ NULL,
    /* 3080 */ "??????????????????????????????????A?????U?EB?C?D?F?G?H???J??????",
    /* 30c0 */ "?K??????L?M?N?????P??Q??R??S?????TV?????WX???Y????Z?????????????",
    /* 3100 */ "?????BDFCGHJKLMNRSTUEPQWYZ?????????A????XV??????????????????????"
}; // 197 records

#ifdef __cplusplus
}
#endif

#endif // __INTERNAL_ALPHABET_RECOGNIZER_H__
