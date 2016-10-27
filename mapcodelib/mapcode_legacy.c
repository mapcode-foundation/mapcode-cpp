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

#include <string.h>

#include "mapcode_legacy.h"
#include "mapcoder.h"

// Legacy buffers: NOT threadsafe!
static char legacy_asciiBuffer[MAX_MAPCODE_RESULT_LEN];
static UWORD legacy_utf16Buffer[MAX_MAPCODE_RESULT_LEN];
static Mapcodes rglobal;
static char makeiso_bufbytes[2 * (MAX_ISOCODE_LEN + 1)];
static char *makeiso_buf;


int encodeLatLonToMapcodes_Deprecated(char **mapcodesAndTerritories,
                                      double latDeg,
                                      double lonDeg,
                                      enum Territory territory,
                                      int extraDigits) {
    char **v = mapcodesAndTerritories;
    encodeLatLonToMapcodes(&rglobal, latDeg, lonDeg, territory, extraDigits);
    if (v) {
        int i;
        for (i = 0; i < rglobal.count; i++) {
            char *s = &rglobal.mapcode[i][0];
            char *p = strchr(s, ' ');
            if (p == NULL) {
                v[i * 2 + 1] = (char *) "AAA";
                v[i * 2] = s;
            } else {
                *p++ = 0;
                v[i * 2 + 1] = s;
                v[i * 2] = p;
            }
        }
    }
    return rglobal.count;
}


const char *convertTerritoryCodeToIsoName_Deprecated(enum Territory territoryContext,
                                                     int useShortName) {
    if (makeiso_buf == makeiso_bufbytes) {
        makeiso_buf = makeiso_bufbytes + (MAX_ISOCODE_LEN + 1);
    } else {
        makeiso_buf = makeiso_bufbytes;
    }
    return (const char *) getTerritoryIsoName(makeiso_buf, territoryContext, useShortName);
}


const char *decodeToRoman_Deprecated(const UWORD *utf16String) {
    return convertToRoman(legacy_asciiBuffer, MAX_MAPCODE_RESULT_LEN, utf16String);
}


const UWORD *encodeToAlphabet_Deprecated(const char *asciiString,
                                         enum Alphabet alphabet) {
    return convertToAlphabet(legacy_utf16Buffer, MAX_MAPCODE_RESULT_LEN, asciiString, alphabet);
}