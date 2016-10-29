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

#ifndef __INTERNAL_ALPHABET_RECOGNISER_H__
#define __INTERNAL_ALPHABET_RECOGNISER_H__

#ifdef __cplusplus
extern "C" {
#endif

// *** GENERATED FILE (dividemaps.cpp), DO NOT CHANGE OR PRETTIFY ***

#ifdef NO_SUPPORT_ALPHABETS
#error "NO_SUPPORT_ALPHABETS cannot be specified in this file"
#endif

// Returns alphabet of character, or negative (_ALPHABET_MIN) in not recognized
#define recognizeAlphabetOfChar(w) ((w) > 0x313f ? _ALPHABET_MIN : alphabetOf[(w) >> 6])

static enum Alphabet alphabetOf[] = {
    /* 0000 */ ALPHABET_ROMAN       , ALPHABET_ROMAN       , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* 0100 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* 0200 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
    /* 0300 */ _ALPHABET_MIN        , _ALPHABET_MIN        , ALPHABET_GREEK       , ALPHABET_GREEK,
    /* 0400 */ ALPHABET_CYRILLIC    , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
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
    /* 1100 */ ALPHABET_KOREAN      , ALPHABET_KOREAN      , _ALPHABET_MIN        , _ALPHABET_MIN,
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
    /* 1e00 */ _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN        , _ALPHABET_MIN,
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
    /* 3000 */ _ALPHABET_MIN        , _ALPHABET_MIN        , ALPHABET_KATAKANA    , ALPHABET_KATAKANA,
    /* 3100 */ ALPHABET_CHINESE     
}; // 197 records

static const char *romanVersionOf[] = {
    /* 0000 */ "?                                !?#$%&'()*+,-./0123456789:;<=>?",
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

#endif // __INTERNAL_ALPHABET_RECOGNISER_H__
