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

#include <string.h>
#include <stdio.h>

#include "mapcode_legacy.h"


/**
 * Include global legacy buffers. These are not thread-safe!
 */
static Mapcodes GLOBAL_RESULT;
static char GLOBAL_MAKEISO_BUFFER[2 * (MAX_ISOCODE_LEN + 1)];
static char *GLOBAL_MAKEISO_PTR;


int encodeLatLonToMapcodes_Deprecated(
        char **mapcodesAndTerritories,
        double latDeg,
        double lonDeg,
        enum Territory territory,
        int extraDigits) {
    char **v = mapcodesAndTerritories;
    encodeLatLonToMapcodes(&GLOBAL_RESULT, latDeg, lonDeg, territory, extraDigits);
    if (v) {
        int i;
        for (i = 0; i < GLOBAL_RESULT.count; i++) {
            char *s = &GLOBAL_RESULT.mapcode[i][0];
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
    return GLOBAL_RESULT.count;
}


const char *convertTerritoryCodeToIsoName_Deprecated(
        enum Territory territoryContext,
        int useShortName) {
    if (GLOBAL_MAKEISO_PTR == GLOBAL_MAKEISO_BUFFER) {
        GLOBAL_MAKEISO_PTR = GLOBAL_MAKEISO_BUFFER + (MAX_ISOCODE_LEN + 1);
    } else {
        GLOBAL_MAKEISO_PTR = GLOBAL_MAKEISO_BUFFER;
    }
    return (const char *) getTerritoryIsoName(GLOBAL_MAKEISO_PTR, territoryContext, useShortName);
}


/**
 * Include global legacy buffers. These are not thread-safe!
 */
static char GLOBAL_ASCII_BUFFER[MAX_MAPCODE_RESULT_LEN];
static UWORD GLOBAL_UTF16_BUFFER[MAX_MAPCODE_RESULT_LEN];


const char *decodeToRoman_Deprecated(const UWORD *utf16String) {
    return convertToRoman(GLOBAL_ASCII_BUFFER, MAX_MAPCODE_RESULT_LEN, utf16String);
}


const UWORD *encodeToAlphabet_Deprecated(const char *asciiString,
                                         enum Alphabet alphabet) {
    return convertToAlphabet(GLOBAL_UTF16_BUFFER, MAX_MAPCODE_RESULT_LEN, asciiString, alphabet);
}


char *convertToRoman(char *asciiBuffer, int maxLength, const UWORD *unicodeBuffer) {

    MapcodeElements mapcodeElements;
    double lat, lon;
    enum MapcodeError err;

    *asciiBuffer = 0;
    err = decodeMapcodeToLatLonUtf16(&lat, &lon, unicodeBuffer, TERRITORY_UNKNOWN, &mapcodeElements);
    if (err == ERR_MISSING_TERRITORY || err == ERR_MAPCODE_UNDECODABLE || err == ERR_EXTENSION_UNDECODABLE) {
        err = ERR_OK;
    }
    if (!err) {
        char romanized[MAX_MAPCODE_RESULT_LEN];
        sprintf(romanized, "%s%s%s%s%s",
                mapcodeElements.territoryISO,
                *mapcodeElements.territoryISO ? " " : "",
                mapcodeElements.properMapcode,
                *mapcodeElements.precisionExtension ? "-" : "",
                mapcodeElements.precisionExtension);
        if ((int) strlen(romanized) < maxLength) {
            strcpy(asciiBuffer, romanized);
        }
    }
    return asciiBuffer;
}
