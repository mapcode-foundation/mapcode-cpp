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

#include <string.h> // strlen strcpy strcat memcpy memmove strstr strchr memcmp
#include <stdlib.h> // atof
#include <ctype.h>  // toupper
#include <math.h>   // floor fabs

#include "mapcoder.h"
#include "internal_data.h"
#include "internal_iso3166_data.h"
#include "internal_territory_alphabets.h"
#include "internal_territory_names_local.h"
#include "internal_alphabet_recognizer.h"
#include "internal_territory_names_af.h"
#include "internal_territory_names_ar.h"
#include "internal_territory_names_be.h"
#include "internal_territory_names_cn.h"
#include "internal_territory_names_cs.h"
#include "internal_territory_names_da.h"
#include "internal_territory_names_de.h"
#include "internal_territory_names_en.h"
#include "internal_territory_names_es.h"
#include "internal_territory_names_fi.h"
#include "internal_territory_names_fr.h"
#include "internal_territory_names_he.h"
#include "internal_territory_names_hi.h"
#include "internal_territory_names_hr.h"
#include "internal_territory_names_id.h"
#include "internal_territory_names_it.h"
#include "internal_territory_names_ja.h"
#include "internal_territory_names_ko.h"
#include "internal_territory_names_nl.h"
#include "internal_territory_names_no.h"
#include "internal_territory_names_pl.h"
#include "internal_territory_names_pt.h"
#include "internal_territory_names_ru.h"
#include "internal_territory_names_sv.h"
#include "internal_territory_names_sw.h"
#include "internal_territory_names_tr.h"
#include "internal_territory_names_uk.h"


#ifdef DEBUG

#include <stdio.h>


void _TestAssert(int iCondition, const char *cstrFile, int iLine) {
    static int nrAsserts = 0;
    if (!iCondition) {
        fprintf(stderr, "** Assertion failed: file \"%s\", line %d\n", cstrFile, iLine);
        ++nrAsserts;
        if (nrAsserts >= 25) {
            fprintf(stderr, "** Stopped execution after %d assertions!\n", nrAsserts);
            exit(-1);
        }
    }
}


#define ASSERT(condition) _TestAssert((int) (condition), __FILE__, (int) __LINE__)
#else
#define ASSERT(condition)
#endif


// If you do not want to use the fast encoding from internal_territory_search.h, define NO_FAST_ENCODE on the
// command-line of your compiler (or uncomment the following line).
// #define NO_FAST_ENCODE

#ifndef NO_FAST_ENCODE

#include "internal_territory_search.h"

#endif

#define IS_NAMELESS(m)        (TERRITORY_BOUNDARIES[m].flags & 64)
#define IS_RESTRICTED(m)      (TERRITORY_BOUNDARIES[m].flags & 512)
#define IS_SPECIAL_SHAPE(m)   (TERRITORY_BOUNDARIES[m].flags & 1024)
#define REC_TYPE(m)           ((TERRITORY_BOUNDARIES[m].flags >> 7) & 3)
#define SMART_DIV(m)          (TERRITORY_BOUNDARIES[m].flags >> 16)
#define HEADER_LETTER(m)      (ENCODE_CHARS[(TERRITORY_BOUNDARIES[m].flags >> 11) & 31])

#define TOKENSEP   0
#define TOKENDOT   1
#define TOKENCHR   2
#define TOKENVOWEL 3
#define TOKENZERO  4
#define TOKENHYPH  5

#define STATE_GO  31

#define MATH_PI                 3.14159265358979323846
#define MAX_PRECISION_FACTOR    810000      // 30 to the power (MAX_PRECISION_DIGITS/2).

// Radius of Earth.
#define EARTH_RADIUS_X_METERS 6378137
#define EARTH_RADIUS_Y_METERS 6356752

// Circumference of Earth.
#define EARTH_CIRCUMFERENCE_X (EARTH_RADIUS_X_METERS * 2 * MATH_PI)
#define EARTH_CIRCUMFERENCE_Y (EARTH_RADIUS_Y_METERS * 2 * MATH_PI)

#define MICROLAT_TO_FRACTIONS_FACTOR ((double) MAX_PRECISION_FACTOR)
#define MICROLON_TO_FRACTIONS_FACTOR (4.0 * MAX_PRECISION_FACTOR)

#define FLAG_UTF8_STRING      0 // interpret pointer a utf8 characters
#define FLAG_UTF16_STRING     1 // interpret pointer a UWORD* to utf16 characters

// Meters per degree latitude is fixed. For longitude: use factor * cos(midpoint of two degree latitudes).
static const double METERS_PER_DEGREE_LAT = EARTH_CIRCUMFERENCE_Y / 360.0;
static const double METERS_PER_DEGREE_LON = EARTH_CIRCUMFERENCE_X / 360.0;

static const int DEBUG_STOP_AT = -1; // to externally test-restrict internal encoding, do not use!

typedef struct {
    const char *locale;
    const char **territoryFullNames;
} LocaleRegistryItem;

static const LocaleRegistryItem LOCALE_REGISTRY[] = {
        {"AF", TERRITORY_FULL_NAME_AF},
        {"AR", TERRITORY_FULL_NAME_AR},
        {"BE", TERRITORY_FULL_NAME_BE},
        {"CN", TERRITORY_FULL_NAME_CN},
        {"CS", TERRITORY_FULL_NAME_CS},
        {"DA", TERRITORY_FULL_NAME_DA},
        {"DE", TERRITORY_FULL_NAME_DE},
        {"EN", TERRITORY_FULL_NAME_EN},
        {"ES", TERRITORY_FULL_NAME_ES},
        {"FI", TERRITORY_FULL_NAME_FI},
        {"FR", TERRITORY_FULL_NAME_FR},
        {"HE", TERRITORY_FULL_NAME_HE},
        {"HI", TERRITORY_FULL_NAME_HI},
        {"HR", TERRITORY_FULL_NAME_HR},
        {"ID", TERRITORY_FULL_NAME_ID},
        {"IT", TERRITORY_FULL_NAME_IT},
        {"JA", TERRITORY_FULL_NAME_JA},
        {"KO", TERRITORY_FULL_NAME_KO},
        {"NL", TERRITORY_FULL_NAME_NL},
        {"NO", TERRITORY_FULL_NAME_NO},
        {"PT", TERRITORY_FULL_NAME_PT},
        {"PL", TERRITORY_FULL_NAME_PL},
        {"RU", TERRITORY_FULL_NAME_RU},
        {"SV", TERRITORY_FULL_NAME_SV},
        {"SW", TERRITORY_FULL_NAME_SW},
        {"TR", TERRITORY_FULL_NAME_TR},
        {"UK", TERRITORY_FULL_NAME_UK}
};

// important information about the 8 parents
static const char *PARENTS_3 = "USA,IND,CAN,AUS,MEX,BRA,RUS,CHN,";
static const char *PARENTS_2 = "US,IN,CA,AU,MX,BR,RU,CN,";
static const enum Territory PARENT_NR[9] = {
        TERRITORY_NONE,
        TERRITORY_USA,
        TERRITORY_IND,
        TERRITORY_CAN,
        TERRITORY_AUS,
        TERRITORY_MEX,
        TERRITORY_BRA,
        TERRITORY_RUS,
        TERRITORY_CHN
};

// base-31 alphabet, digits (0-9), consonants (10-30), vowels (31-33)
static const char ENCODE_CHARS[34] = {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
        'B', 'C', 'D', 'F', 'G', 'H', 'J', 'K', 'L', 'M',
        'N', 'P', 'Q', 'R', 'S', 'T', 'V', 'W', 'X', 'Y', 'Z',
        'A', 'E', 'U'};


static signed char decodeChar(const char ch) {
    // base-31 value of ascii character (negative for illegal characters)
    // special cases -2, -3, -4 for vowels; o and i interpreted as 0 and 1.
    static const signed char decode_chars[256] = {
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,     // 0
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,     // 16
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,     // 32
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, -1, -1, -1, -1, -1, -1,               // 48
            -1, -2, 10, 11, 12, -3, 13, 14, 15, 1, 16, 17, 18, 19, 20, 0,       // 64
            21, 22, 23, 24, 25, -4, 26, 27, 28, 29, 30, -1, -1, -1, -1, -1,     // 80
            -1, -2, 10, 11, 12, -3, 13, 14, 15, 1, 16, 17, 18, 19, 20, 0,       // 96
            21, 22, 23, 24, 25, -4, 26, 27, 28, 29, 30, -1, -1, -1, -1, -1,     // 112
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,     // 128
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,     // 144
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,     // 160
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,     // 176
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,     // 192
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,     // 208
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,     // 224
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1      // 240
    };
    return decode_chars[(unsigned char) ch];   // ch can be negative, must be fit to range 0-255.
}

///////////////////////////////////////////////////////////////////////////////////////////////
//
//  distanceInMeters
//
///////////////////////////////////////////////////////////////////////////////////////////////

// PUBLIC - returns distance (in meters) between two coordinates (in degrees)
double distanceInMeters(double latDeg1, double lonDeg1, double latDeg2, double lonDeg2) {
    double dx;
    double dy;
    double deltaLonDegrees;
    double deltaLatDegrees;
    int wrapped = lonDeg1 > lonDeg2;
    ASSERT((-90.0 <= latDeg1) && (latDeg1 <= 90.0));
    ASSERT((-90.0 <= latDeg2) && (latDeg2 <= 90.0));
    if (wrapped) {
        deltaLonDegrees = 360.0 - (lonDeg1 - lonDeg2);
    } else {
        deltaLonDegrees = lonDeg2 - lonDeg1;
    }
    if (deltaLonDegrees > 180.0) {
        deltaLonDegrees = 360.0 - deltaLonDegrees;
    }
    deltaLatDegrees = fabs(latDeg1 - latDeg2);
    dy = deltaLatDegrees * METERS_PER_DEGREE_LAT;
    dx = deltaLonDegrees * METERS_PER_DEGREE_LON * cos((latDeg1 + (latDeg2 - latDeg1) / 2.0) * MATH_PI / 180.0);
    return sqrt(dx * dx + dy * dy);
}

///////////////////////////////////////////////////////////////////////////////////////////////
//
//  maxErrorInMeters
//
///////////////////////////////////////////////////////////////////////////////////////////////

// maximum error in meters for a certain nr of high-precision digits
static const double MAX_ERROR_IN_METERS[MAX_PRECISION_DIGITS + 1] = {
        7.49,
        1.39,
        0.251,
        0.0462,
        0.00837,
        0.00154,
        0.000279,
        0.0000514,
        0.0000093
};


// PUBLIC - returns maximum error in meters for a certain nr of high-precision digits
double maxErrorInMeters(int extraDigits) {
    ASSERT(extraDigits >= 0);
    if ((extraDigits < 0) || (extraDigits > MAX_PRECISION_DIGITS)) {
        return 0.0;
    }
    return MAX_ERROR_IN_METERS[extraDigits];
}

///////////////////////////////////////////////////////////////////////////////////////////////
//
//  Point / Point32
//
///////////////////////////////////////////////////////////////////////////////////////////////

typedef struct {
    int latMicroDeg; // latitude in microdegrees
    int lonMicroDeg; // longitude in microdegrees
} Point32;

typedef struct { // Point
    double lat;  // latitude (units depend on situation)
    double lon;  // longitude (units depend on situation)
} Point;


static Point32 convertFractionsToCoord32(const Point *p) {
    Point32 p32;
    p32.latMicroDeg = (int) floor(p->lat / 810000);
    p32.lonMicroDeg = (int) floor(p->lon / 3240000);
    return p32;
}


static Point convertFractionsToDegrees(const Point *p) {
    Point pd;
    pd.lat = p->lat / (810000 * 1000000.0);
    pd.lon = p->lon / (3240000 * 1000000.0);
    return pd;
}


static const unsigned char DOUBLE_NAN[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F};     // NAN (Not a Number)
static const unsigned char DOUBLE_INF[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x7F};     // +Infinity
static const unsigned char DOUBLE_MIN_INF[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0xFF}; // -Infinity

static enum MapcodeError
convertCoordsToMicrosAndFractions(Point32 *coord32, int *fracLat, int *fracLon, double latDeg, double lonDeg) {
    double frac;
    ASSERT(coord32);
    if (memcmp(&lonDeg, DOUBLE_NAN, 8) == 0 || memcmp(&lonDeg, DOUBLE_INF, 8) == 0 ||
        memcmp(&lonDeg, DOUBLE_MIN_INF, 8) == 0 ||
        memcmp(&latDeg, DOUBLE_NAN, 8) == 0) {
        return ERR_BAD_COORDINATE;
    }
    if (latDeg < -90) {
        latDeg = -90;
    } else if (latDeg > 90) {
        latDeg = 90;
    }
    latDeg += 90; // lat now [0..180]
    ASSERT((0.0 <= latDeg) && (latDeg <= 180.0));
    latDeg *= (double) 810000000000;
    frac = floor(latDeg + 0.1);
    coord32->latMicroDeg = (int) (frac / (double) 810000);
    if (fracLat) {
        frac -= ((double) coord32->latMicroDeg * (double) 810000);
        *fracLat = (int) frac;
    }
    coord32->latMicroDeg -= 90000000;

    lonDeg -= (360.0 * floor(lonDeg / 360)); // lon now in [0..360>
    ASSERT((0.0 <= lonDeg) && (lonDeg < 360.0));
    lonDeg *= (double) 3240000000000;
    frac = floor(lonDeg + 0.1);
    coord32->lonMicroDeg = (int) (frac / (double) 3240000);
    if (fracLon) {
        frac -= (double) coord32->lonMicroDeg * (double) 3240000;
        *fracLon = (int) frac;
    }
    if (coord32->lonMicroDeg >= 180000000) {
        coord32->lonMicroDeg -= 360000000;
    }
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////
//
//  TerritoryBoundary (specified in microDegrees)
//
///////////////////////////////////////////////////////////////////////////////////////////////


// returns nonzero if x in the range minx...maxx
static int isInRange(int lonMicroDeg, const int minLonMicroDeg, const int maxLonMicroDeg) {
    if (minLonMicroDeg <= lonMicroDeg && lonMicroDeg < maxLonMicroDeg) {
        return 1;
    }
    if (lonMicroDeg < minLonMicroDeg) {
        lonMicroDeg += 360000000;
    } else {
        lonMicroDeg -= 360000000;
    } // 1.32 fix FIJI edge case
    if (minLonMicroDeg <= lonMicroDeg && lonMicroDeg < maxLonMicroDeg) {
        return 1;
    }
    return 0;
}


// returns true iff given coordinate "coord32" fits inside given TerritoryBoundary
static int fitsInsideBoundaries(const Point32 *coord32, const TerritoryBoundary *b) {
    ASSERT(coord32);
    ASSERT(b);
    return (b->miny <= coord32->latMicroDeg &&
            coord32->latMicroDeg < b->maxy &&
            isInRange(coord32->lonMicroDeg, b->minx, b->maxx));
}


// set target TerritoryBoundary to a source extended with deltalat, deltaLon (in microDegrees)
static TerritoryBoundary *getExtendedBoundaries(TerritoryBoundary *target, const TerritoryBoundary *source,
                                                const int deltaLatMicroDeg, const int deltaLonMicroDeg) {
    ASSERT(target);
    ASSERT(source);
    target->miny = source->miny - deltaLatMicroDeg;
    target->minx = source->minx - deltaLonMicroDeg;
    target->maxy = source->maxy + deltaLatMicroDeg;
    target->maxx = source->maxx + deltaLonMicroDeg;
    return target;
}

///////////////////////////////////////////////////////////////////////////////////////////////
//
//  MapcodeZone
//
///////////////////////////////////////////////////////////////////////////////////////////////


typedef struct {
    // latitudes in "810 billionths", range [-729 E11 .. +720 E11), is well within (-2^47 ... +2^47)
    double fminy;
    double fmaxy;
    // latitudes in "3240 billionths", range [-2916 E13 .. +2916 E13), is well within (-2^49 ... +2^49)
    double fminx;
    double fmaxx;
} MapcodeZone;


static void setFromFractions(MapcodeZone *z,
                             const double y, const double x,
                             const double yDelta, const double xDelta) {
    ASSERT(z);
    z->fminx = x;
    z->fmaxx = x + xDelta;
    if (yDelta < 0) {
        z->fminy = y + 1 + yDelta; // y+yDelta can NOT be represented
        z->fmaxy = y + 1;          // y CAN be represented
    } else {
        z->fminy = y;
        z->fmaxy = y + yDelta;
    }
}


static int isEmpty(const MapcodeZone *z) {
    ASSERT(z);
    return ((z->fmaxx <= z->fminx) || (z->fmaxy <= z->fminy));
}


static Point getMidPointFractions(const MapcodeZone *z) {
    Point p;
    ASSERT(z);
    p.lon = floor((z->fminx + z->fmaxx) / 2);
    p.lat = floor((z->fminy + z->fmaxy) / 2);
    return p;
}


static void zoneCopyFrom(MapcodeZone *target, const MapcodeZone *source) {
    ASSERT(target);
    ASSERT(source);
    target->fminy = source->fminy;
    target->fmaxy = source->fmaxy;
    target->fminx = source->fminx;
    target->fmaxx = source->fmaxx;
}


// determine the non-empty intersection zone z between a given zone and the boundary of territory rectangle m.
// returns nonzero in case such a zone exists
static int restrictZoneTo(MapcodeZone *z, const MapcodeZone *zone, const TerritoryBoundary *b) {
    ASSERT(z);
    ASSERT(zone);
    ASSERT(b);
    z->fminy = zone->fminy;
    z->fmaxy = zone->fmaxy;
    if (z->fminy < b->miny * MICROLAT_TO_FRACTIONS_FACTOR) {
        z->fminy = b->miny * MICROLAT_TO_FRACTIONS_FACTOR;
    }
    if (z->fmaxy > b->maxy * MICROLAT_TO_FRACTIONS_FACTOR) {
        z->fmaxy = b->maxy * MICROLAT_TO_FRACTIONS_FACTOR;
    }
    if (z->fminy < z->fmaxy) {
        double bminx = b->minx * MICROLON_TO_FRACTIONS_FACTOR;
        double bmaxx = b->maxx * MICROLON_TO_FRACTIONS_FACTOR;
        z->fminx = zone->fminx;
        z->fmaxx = zone->fmaxx;
        if (bmaxx < 0 && z->fminx > 0) {
            bminx += (360000000 * MICROLON_TO_FRACTIONS_FACTOR);
            bmaxx += (360000000 * MICROLON_TO_FRACTIONS_FACTOR);
        } else if (bminx > 0 && z->fmaxx < 0) {
            bminx -= (360000000 * MICROLON_TO_FRACTIONS_FACTOR);
            bmaxx -= (360000000 * MICROLON_TO_FRACTIONS_FACTOR);
        }
        if (z->fminx < bminx) {
            z->fminx = bminx;
        }
        if (z->fmaxx > bmaxx) {
            z->fmaxx = bmaxx;
        }
        return (z->fminx < z->fmaxx);
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////
//
//  COPY ROUTINES
//
///////////////////////////////////////////////////////////////////////////////////////////////

// PRIVATE - copy characters into targetString, limited to its size
static char *lengthCopy(char *targetString, const char *sourceString, int nrCharacters, int targetSize) {
    if (nrCharacters >= targetSize) {
        nrCharacters = targetSize - 1;
    }
    memcpy(targetString, sourceString, (size_t) nrCharacters);
    targetString[nrCharacters] = 0;
    return targetString;
}


// PRIVATE - copy as much of sourceString as will fit; returns targetString
static char *safeCopy(char *targetString, const char *sourceString, const int targetSize) {
    int sourceLength = (int) strlen(sourceString);
    return lengthCopy(targetString, sourceString, sourceLength, targetSize);
}


///////////////////////////////////////////////////////////////////////////////////////////////
//
//  Data access
//
///////////////////////////////////////////////////////////////////////////////////////////////

/*** low-level data access ***/

static int firstRec(const enum Territory ccode) {
    ASSERT((_TERRITORY_MIN < ccode) && (ccode < _TERRITORY_MAX));
    return DATA_START[INDEX_OF_TERRITORY(ccode)];
}


static int lastRec(const enum Territory ccode) {
    ASSERT((_TERRITORY_MIN < ccode) && (ccode < _TERRITORY_MAX));
    return DATA_START[INDEX_OF_TERRITORY(ccode) + 1] - 1;
}


// returns parent of ccode (or TERRITORY_NONE)
static enum Territory parentTerritoryOf(const enum Territory ccode) {
    if (ccode <= _TERRITORY_MIN || ccode >= _TERRITORY_MAX) {
        return TERRITORY_NONE;
    }
    return PARENT_NR[(int) PARENT_LETTER[INDEX_OF_TERRITORY(ccode)]];
}


static int coDex(const int m) {
    int c = TERRITORY_BOUNDARIES[m].flags & 31;
    ASSERT((0 <= m) && (m <= MAPCODE_BOUNDARY_MAX));
    return 10 * (c / 5) + ((c % 5) + 1);
}


static int xDivider4(const int miny, const int maxy) {
    // 360 * cos(microdegrees>>19)
    static const int xdivider19[172] = {
            360, 360, 360, 360, 360, 360, 361, 361, 361, 361,
            362, 362, 362, 363, 363, 363, 364, 364, 365, 366,
            366, 367, 367, 368, 369, 370, 370, 371, 372, 373,
            374, 375, 376, 377, 378, 379, 380, 382, 383, 384,
            386, 387, 388, 390, 391, 393, 394, 396, 398, 399,
            401, 403, 405, 407, 409, 411, 413, 415, 417, 420,
            422, 424, 427, 429, 432, 435, 437, 440, 443, 446,
            449, 452, 455, 459, 462, 465, 469, 473, 476, 480,
            484, 488, 492, 496, 501, 505, 510, 515, 520, 525,
            530, 535, 540, 546, 552, 558, 564, 570, 577, 583,
            590, 598, 605, 612, 620, 628, 637, 645, 654, 664,
            673, 683, 693, 704, 715, 726, 738, 751, 763, 777,
            791, 805, 820, 836, 852, 869, 887, 906, 925, 946,
            968, 990, 1014, 1039, 1066, 1094, 1123, 1154, 1187, 1223,
            1260, 1300, 1343, 1389, 1438, 1490, 1547, 1609, 1676, 1749,
            1828, 1916, 2012, 2118, 2237, 2370, 2521, 2691, 2887, 3114,
            3380, 3696, 4077, 4547, 5139, 5910, 6952, 8443, 10747, 14784,
            23681, 59485
    };
    if (miny >= 0) { // both above equator? then miny is closest
        return xdivider19[(miny) >> 19];
    }
    if (maxy >= 0) { // opposite sides? then equator is worst
        return xdivider19[0];
    }
    return xdivider19[(-maxy) >> 19]; // both negative, so maxy is closest to equator
}

/*** mid-level data access ***/

// returns true iff ccode is a subdivision of some other country
static int isSubdivision(const enum Territory ccode) {
    return parentTerritoryOf(ccode) != TERRITORY_NONE;
}


// find first territory rectangle of the same type as m
static int firstNamelessRecord(const int m, const int firstcode) {
    int i = m;
    const int codexm = coDex(m);
    ASSERT((0 <= m) && (m <= MAPCODE_BOUNDARY_MAX));
    ASSERT((0 <= firstcode) && (firstcode <= MAPCODE_BOUNDARY_MAX));
    while (i >= firstcode && coDex(i) == codexm && IS_NAMELESS(i)) {
        i--;
    }
    return (i + 1);
}


// count all territory rectangles of the same type as m
static int countNamelessRecords(const int m, const int firstcode) {
    const int first = firstNamelessRecord(m, firstcode);
    const int codexm = coDex(m);
    int last = m;
    ASSERT((0 <= m) && (m <= MAPCODE_BOUNDARY_MAX));
    ASSERT((0 <= firstcode) && (firstcode <= MAPCODE_BOUNDARY_MAX));
    while (coDex(last) == codexm) {
        last++;
    }
    ASSERT((0 <= last) && (last <= MAPCODE_BOUNDARY_MAX));
    ASSERT(last >= first);
    return (last - first);
}


static int isNearBorderOf(const Point32 *coord32, const TerritoryBoundary *b) {
    int xdiv8 = xDivider4(b->miny, b->maxy) / 4; // should be /8 but there's some extra margin
    TerritoryBoundary tmp;
    ASSERT(coord32);
    ASSERT(b);
    return (fitsInsideBoundaries(coord32, getExtendedBoundaries(&tmp, b, +60, +xdiv8)) &&
            (!fitsInsideBoundaries(coord32, getExtendedBoundaries(&tmp, b, -60, -xdiv8))));
}


static void makeUppercase(char *s) {
    ASSERT(s);
    while (*s) {
        *s = (char) toupper(*s);
        s++;
    }
}


// returns 1 - 8, or negative if error
static int getParentNumber(const char *s, const int len) {
    const char *p = ((len == 2) ? PARENTS_2 : PARENTS_3);
    const char *f;
    char country[4];
    ASSERT(s[0] && s[1]);
    ASSERT((2 <= len) && (len <= 3));
    ASSERT(s && ((int) strlen(s) >= len));
    lengthCopy(country, s, len, 4);
    makeUppercase(country);
    f = strstr(p, country);
    if (!f) {
        return -1;
    }
    return 1 + (int) ((f - p) / (len + 1));
}

///////////////////////////////////////////////////////////////////////////////////////////////
//
//  MAPCODE ALL-DIGIT PACKING/UNPACKING
//
///////////////////////////////////////////////////////////////////////////////////////////////

static void repackIfAllDigits(char *input, const int aonly) {
    char *s = input;
    int alldigits = 1; // assume all digits
    char *e;
    char *dotpos = NULL;
    ASSERT(input);
    for (e = s; *e != 0 && *e != '-'; e++) {
        if (*e < '0' || *e > '9') {
            if (*e == '.' && !dotpos) {
                dotpos = e;
            } else {
                alldigits = 0;
                break;
            }
        }
    }
    e--;
    s = e - 1;
    if (alldigits && dotpos &&
        s > dotpos) // e is last char, s is one before, both are beyond dot, all characters are digits
    {
        if (aonly) // v1.50 - encode only using the letter A
        {
            const int v = ((*input) - '0') * 100 + ((*s) - '0') * 10 + ((*e) - '0');
            *input = 'A';
            *s = ENCODE_CHARS[v / 32];
            *e = ENCODE_CHARS[v % 32];
        } else // encode using A,E,U
        {
            const int v = ((*s) - '0') * 10 + ((*e) - '0');
            *s = ENCODE_CHARS[(v / 34) + 31];
            *e = ENCODE_CHARS[v % 34];
        }
    }
}


// rewrite all-digit codes
// returns 1 if unpacked, 0 if left unchanged, negative if unchanged and an error was detected
static int unpackIfAllDigits(char *input) {
    char *s = input;
    char *dotpos = NULL;
    const int aonly = ((*s == 'A') || (*s == 'a'));
    if (aonly) {
        s++;
    } // v1.50
    for (; *s != 0 && s[2] != 0 && s[2] != '-'; s++) {
        if (*s == '-') {
            break;
        } else if (*s == '.' && !dotpos) {
            dotpos = s;
        } else if ((decodeChar(*s) < 0) || (decodeChar(*s) > 9)) {
            return 0;
        }  // nondigit, so stop
    }

    if (dotpos) {
        if (aonly) // v1.50 encoded only with A's
        {
            const int v = (((s[0] == 'A') || (s[0] == 'a')) ? 31 : decodeChar(s[0])) * 32 +
                          (((s[1] == 'A') || (s[1] == 'a')) ? 31 : decodeChar(s[1]));
            *input = (char) ('0' + (v / 100));
            s[0] = (char) ('0' + ((v / 10) % 10));
            s[1] = (char) ('0' + (v % 10));
            return 1;
        } // v1.50

        if ((*s == 'a') || (*s == 'e') || (*s == 'u') ||
            (*s == 'A') || (*s == 'E') || (*s == 'U')) {
            char *e = s + 1;  // s is vowel, e is lastchar

            int v = 0;
            if (*s == 'e' || *s == 'E') {
                v = 34;
            } else if (*s == 'u' || *s == 'U') {
                v = 68;
            }

            if ((*e == 'a') || (*e == 'A')) {
                v += 31;
            } else if ((*e == 'e') || (*e == 'E')) {
                v += 32;
            } else if ((*e == 'u') || (*e == 'U')) {
                v += 33;
            } else if (decodeChar(*e) < 0) {
                return (int) ERR_INVALID_CHARACTER;
            } else {
                v += decodeChar(*e);
            }

            if (v < 100) {
                *s = ENCODE_CHARS[(unsigned int) v / 10];
                *e = ENCODE_CHARS[(unsigned int) v % 10];
            } else {
                return (int) ERR_INVALID_ENDVOWELS; // mapcodes ends in UE or UU
            }
            return 1;
        }
    }
    return 0; // no vowel just before end
}

///////////////////////////////////////////////////////////////////////////////////////////////
//
//  DECODING
//
///////////////////////////////////////////////////////////////////////////////////////////////

typedef struct {
    // input
    Point32 coord32;
    int fraclat; // latitude fraction of microdegrees, expressed in 1 / 810,000ths
    int fraclon; // longitude fraction of microdegrees, expressed in 1 / 3,240,000ths
    // output
    Mapcodes *mapcodes;
} EncodeRec;


// encode the high-precision extension (0-8 characters)
static void encodeExtension(char *result, const int extrax4, const int extray, const int dividerx4,
                            const int dividery, int extraDigits, const int ydirection,
                            const EncodeRec *enc) // append extra characters to result for more precision
{
    ASSERT(result);
    ASSERT(enc);
    if (extraDigits > 0) { // anything to do?
        char *s = result + strlen(result);
        double factorx = (double) MAX_PRECISION_FACTOR * dividerx4; // perfect integer!
        double factory = (double) MAX_PRECISION_FACTOR * dividery; // perfect integer!
        double valx = ((double) MAX_PRECISION_FACTOR * extrax4) + enc->fraclon; // perfect integer!
        double valy = ((double) MAX_PRECISION_FACTOR * extray) + (ydirection * enc->fraclat); // perfect integer!

        // protect against floating Point errors
        if (valx < 0) {
            valx = 0;
        } else if (valx >= factorx) {
            valx = factorx - 1;
        }
        if (valy < 0) {
            valy = 0;
        } else if (valy >= factory) {
            valy = factory - 1;
        }

        if (extraDigits > MAX_PRECISION_DIGITS) {
            extraDigits = MAX_PRECISION_DIGITS;
        }

        *s++ = '-';

        for (;;) {
            int gx, gy;

            factorx /= 30;
            gx = (int) (valx / factorx);

            factory /= 30;
            gy = (int) (valy / factory);

            *s++ = ENCODE_CHARS[(gy / 5) * 5 + (gx / 6)];
            if (--extraDigits == 0) {
                break;
            }

            *s++ = ENCODE_CHARS[(gy % 5) * 6 + (gx % 6)];
            if (--extraDigits == 0) {
                break;
            }

            valx -= factorx * gx; // for next iteration
            valy -= factory * gy; // for next iteration
        }
        *s = 0; // terminate the result
        ASSERT((int) strlen(s) == extraDigits);
    }
}


// encode 'value' into result[nrchars]
static void encodeBase31(char *result, int value, int nrchars) {
    ASSERT(result);
    ASSERT(nrchars >= 0);
    result[nrchars] = 0; // zero-terminate!
    while (nrchars > 0) {
        nrchars--;
        result[nrchars] = ENCODE_CHARS[value % 31];
        value /= 31;
    }
}


static void encodeTriple(char *result, const int difx, const int dify) {
    ASSERT(result);
    if (dify < 4 * 34) // first 4(x34) rows of 6(x28) wide
    {
        *result = ENCODE_CHARS[((difx / 28) + 6 * (dify / 34))];
        encodeBase31(result + 1, ((difx % 28) * 34 + (dify % 34)), 2);
    } else // bottom row
    {
        *result = ENCODE_CHARS[(difx / 24) + 24];
        encodeBase31(result + 1, (difx % 24) * 40 + (dify - 136), 2);
    }
} // encodeTriple

static int encodeSixWide(int x, int y, int width, int height) {
    int v;
    int D = 6;
    int col = x / 6;
    const int maxcol = (width - 4) / 6;
    if (col >= maxcol) {
        col = maxcol;
        D = width - maxcol * 6;
    }
    v = (height * 6 * col) + (height - 1 - y) * D + (x - col * 6);
    return v;
}

// *** mid-level encode routines ***

// default cell divisions for n characters
static const int X_SIDE[6] = {0, 5, 31, 168, 961, 168 * 31};
static const int Y_SIDE[6] = {0, 6, 31, 176, 961, 176 * 31};
// number of combinations for n characters
static const int NC[6] = {1, 31, 961, 29791, 923521, 28629151};


// returns *result==0 in case of error
static void encodeGrid(char *result, const EncodeRec *enc, const int m, const int extraDigits,
                       const char headerLetter) {
    const TerritoryBoundary *b = TERRITORY_BOUNDARY(m);
    const int orgcodex = coDex(m);
    int codexm;
    ASSERT(result);
    ASSERT(enc);
    ASSERT((0 <= m) && (m <= MAPCODE_BOUNDARY_MAX));
    ASSERT((0 <= extraDigits) && (extraDigits <= MAX_PRECISION_DIGITS));
    codexm = orgcodex;
    if (codexm == 21) {
        codexm = 22;
    } else if (codexm == 14) {
        codexm = 23;
    }

    *result = 0;
    if (headerLetter) {
        result++;
    }

    { // encode
        int divx, divy;
        const int prelen = codexm / 10;
        const int postlen = codexm % 10;

        divy = SMART_DIV(m);
        ASSERT(divy > 0);
        if (divy == 1) {
            divx = X_SIDE[prelen];
            divy = Y_SIDE[prelen];
        } else {
            divx = (NC[prelen] / divy);
        }

        { // grid
            const int ygridsize = (b->maxy - b->miny + divy - 1) / divy;
            const int xgridsize = (b->maxx - b->minx + divx - 1) / divx;
            int rely = enc->coord32.latMicroDeg - b->miny;
            int x = enc->coord32.lonMicroDeg;
            int relx = x - b->minx;

            if (relx < 0) {
                relx += 360000000;
                x += 360000000;
            } else if (relx >= 360000000) // 1.32 fix FIJI edge case
            {
                relx -= 360000000;
                x -= 360000000;
            }

            rely /= ygridsize;
            relx /= xgridsize;

            if (relx >= divx || rely >= divy) {
                return;
            }

            { // prefix
                int v;
                if (divx != divy && prelen > 2) {
                    v = encodeSixWide(relx, rely, divx, divy);
                } else {
                    v = relx * divy + (divy - 1 - rely);
                }
                encodeBase31(result, v, prelen);
            } // prefix

            if (prelen == 4 && divx == 961 && divy == 961) {
                const char t = result[1];
                result[1] = result[2];
                result[2] = t;
            }

            rely = b->miny + (rely * ygridsize);
            relx = b->minx + (relx * xgridsize);

            { // postfix
                const int dividery = ((ygridsize + Y_SIDE[postlen] - 1) / Y_SIDE[postlen]);
                const int dividerx = ((xgridsize + X_SIDE[postlen] - 1) / X_SIDE[postlen]);
                int extrax, extray;

                {
                    char *resultptr = result + prelen;


                    int difx = x - relx;
                    int dify = enc->coord32.latMicroDeg - rely;

                    *resultptr++ = '.';

                    extrax = difx % dividerx;
                    extray = dify % dividery;
                    difx /= dividerx;
                    dify /= dividery;


                    // reverse y-direction
                    dify = Y_SIDE[postlen] - 1 - dify;

                    if (postlen == 3) // encode special
                    {
                        encodeTriple(resultptr, difx, dify);
                    } else {
                        encodeBase31(resultptr, (difx) * Y_SIDE[postlen] + dify, postlen);
                        // swap 4-int codes for readability
                        if (postlen == 4) {
                            char t = resultptr[1];
                            resultptr[1] = resultptr[2];
                            resultptr[2] = t;
                        }
                    }
                }

                if (orgcodex == 14) {
                    result[2] = result[1];
                    result[1] = '.';
                }

                encodeExtension(result, extrax << 2, extray, dividerx << 2, dividery, extraDigits, 1, enc); // grid
                if (headerLetter) {
                    result--;
                    *result = headerLetter;
                }
            } // postfix
        } // grid
    }  // encode
}


// *result==0 in case of error
static void encodeNameless(char *result, const EncodeRec *enc, const enum Territory ccode,
                           const int extraDigits, const int m) {
    // determine how many nameless records there are (A), and which one is this (X)...
    const int A = countNamelessRecords(m, firstRec(ccode));
    const int X = m - firstNamelessRecord(m, firstRec(ccode));
    ASSERT(result);
    ASSERT(enc);
    ASSERT((0 <= m) && (m <= MAPCODE_BOUNDARY_MAX));
    ASSERT((0 <= extraDigits) && (extraDigits <= MAX_PRECISION_DIGITS));
    *result = 0;

    {
        const int p = 31 / A;
        const int r = 31 % A; // the first r items are p+1
        const int codexm = coDex(m);
        const int codexlen = (codexm / 10) + (codexm % 10);
        // determine side of square around centre
        int SIDE;

        int storage_offset;
        const TerritoryBoundary *b;

        int xSIDE, orgSIDE;

        if (codexm != 21 && A <= 31) {
            storage_offset = (X * p + (X < r ? X : r)) * (961 * 961);
        } else if (codexm != 21 && A < 62) {
            if (X < (62 - A)) {
                storage_offset = X * (961 * 961);
            } else {
                storage_offset = (62 - A + ((X - 62 + A) / 2)) * (961 * 961);
                if ((X + A) & 1) {
                    storage_offset += (16 * 961 * 31);
                }
            }
        } else {
            const int BASEPOWER = (codexm == 21) ? 961 * 961 : 961 * 961 * 31;
            int BASEPOWERA = (BASEPOWER / A);
            if (A == 62) {
                BASEPOWERA++;
            } else {
                BASEPOWERA = (961) * (BASEPOWERA / 961);
            }

            storage_offset = X * BASEPOWERA;
        }

        SIDE = SMART_DIV(m);
        ASSERT(SIDE > 0);

        b = TERRITORY_BOUNDARY(m);
        orgSIDE = SIDE;

        {
            int v = storage_offset;

            const int dividerx4 = xDivider4(b->miny, b->maxy); // *** note: dividerx4 is 4 times too large!
            const int xFracture = (enc->fraclon / MAX_PRECISION_FACTOR);
            const int dx = (4 * (enc->coord32.lonMicroDeg - b->minx) + xFracture) / dividerx4; // div with quarters
            const int extrax4 = (enc->coord32.lonMicroDeg - b->minx) * 4 - (dx * dividerx4); // mod with quarters

            const int dividery = 90;
            int dy = (b->maxy - enc->coord32.latMicroDeg) / dividery;
            int extray = (b->maxy - enc->coord32.latMicroDeg) % dividery;

            if (extray == 0 && enc->fraclat > 0) {
                dy--;
                extray += dividery;
            }

            if (IS_SPECIAL_SHAPE(m)) {
                SIDE = 1 + ((b->maxy - b->miny) / 90); // new side, based purely on y-distance
                xSIDE = (orgSIDE * orgSIDE) / SIDE;
                v += encodeSixWide(dx, SIDE - 1 - dy, xSIDE, SIDE);
            } else {
                v += (dx * SIDE + dy);
            }

            encodeBase31(result, v, codexlen + 1); // nameless
            {
                int dotp = codexlen;
                if (codexm == 13) {
                    dotp--;
                }
                memmove(result + dotp, result + dotp - 1, 4);
                result[dotp - 1] = '.';
            }

            if (!IS_SPECIAL_SHAPE(m)) {
                if (codexm == 22 && A < 62 && orgSIDE == 961) {
                    const char t = result[codexlen - 2];
                    result[codexlen - 2] = result[codexlen];
                    result[codexlen] = t;
                }
            }

            encodeExtension(result, extrax4, extray, dividerx4, dividery, extraDigits, -1, enc); // nameless

            return;

        } // in range
    }
}


// encode in m (known to fit)
static void encodeAutoHeader(char *result, const EncodeRec *enc, const int m, const int extraDigits) {
    int i;
    int STORAGE_START = 0;
    int W, H, xdiv, product;
    const TerritoryBoundary *b;

    // search back to first of the group
    int firstindex = m;
    const int codexm = coDex(m);
    ASSERT(result);
    ASSERT(enc);
    ASSERT((1 <= m) && (m <= MAPCODE_BOUNDARY_MAX));
    ASSERT((0 <= extraDigits) && (extraDigits <= MAX_PRECISION_DIGITS));

    while (REC_TYPE(firstindex - 1) > 1 && coDex(firstindex - 1) == codexm) {
        firstindex--;
    }

    i = firstindex;
    for (;;) {
        b = TERRITORY_BOUNDARY(i);
        // determine how many cells
        H = (b->maxy - b->miny + 89) / 90; // multiple of 10m
        xdiv = xDivider4(b->miny, b->maxy);
        W = ((b->maxx - b->minx) * 4 + (xdiv - 1)) / xdiv;

        // round up to multiples of 176*168...
        H = 176 * ((H + 176 - 1) / 176);
        W = 168 * ((W + 168 - 1) / 168);
        product = (W / 168) * (H / 176) * 961 * 31;
        if (REC_TYPE(i) == 2) { // plus pipe
            const int GOODROUNDER = codexm >= 23 ? (961 * 961 * 31) : (961 * 961);
            product = ((STORAGE_START + product + GOODROUNDER - 1) / GOODROUNDER) * GOODROUNDER - STORAGE_START;
        }
        if (i == m) {
            // encode
            const int dividerx = (b->maxx - b->minx + W - 1) / W;
            const int vx = (enc->coord32.lonMicroDeg - b->minx) / dividerx;
            const int extrax = (enc->coord32.lonMicroDeg - b->minx) % dividerx;

            const int dividery = (b->maxy - b->miny + H - 1) / H;
            int vy = (b->maxy - enc->coord32.latMicroDeg) / dividery;
            int extray = (b->maxy - enc->coord32.latMicroDeg) % dividery;

            const int codexlen = (codexm / 10) + (codexm % 10);
            int value = (vx / 168) * (H / 176);

            if (extray == 0 && enc->fraclat > 0) {
                vy--;
                extray += dividery;
            }

            value += (vy / 176);

            // PIPELETTER ENCODE
            encodeBase31(result, (STORAGE_START / (961 * 31)) + value, codexlen - 2);
            result[codexlen - 2] = '.';
            encodeTriple(result + codexlen - 1, vx % 168, vy % 176);

            encodeExtension(result, extrax << 2, extray, dividerx << 2, dividery, extraDigits, -1, enc); // autoheader
            return;
        }
        STORAGE_START += product;
        i++;
    }
}


static void encoderEngine(const enum Territory ccode, const EncodeRec *enc, const int stop_with_one_result,
                          const int extraDigits, const int requiredEncoder, const enum Territory ccode_override) {
    int from;
    int upto;
    ASSERT(enc);
    ASSERT((0 <= extraDigits) && (extraDigits <= MAX_PRECISION_DIGITS));

    if (!enc || (ccode < _TERRITORY_MIN)) {
        return;
    } // bad arguments

    from = firstRec(ccode);
    upto = lastRec(ccode);

    if (!fitsInsideBoundaries(&enc->coord32, TERRITORY_BOUNDARY(upto))) {
        return;
    }

    ///////////////////////////////////////////////////////////
    // look for encoding options
    ///////////////////////////////////////////////////////////
    {
        int i;
        char result[128];
        int result_counter = 0;

        *result = 0;
        for (i = from; i <= upto; i++) {
            if (fitsInsideBoundaries(&enc->coord32, TERRITORY_BOUNDARY(i))) {
                if (IS_NAMELESS(i)) {
                    encodeNameless(result, enc, ccode, extraDigits, i);
                } else if (REC_TYPE(i) > 1) {
                    encodeAutoHeader(result, enc, i, extraDigits);
                } else if ((i == upto) && isSubdivision(ccode)) {
                    // *** do a recursive call for the parent ***
                    encoderEngine(parentTerritoryOf(ccode), enc, stop_with_one_result, extraDigits, requiredEncoder,
                                  ccode);
                    return;
                } else // must be grid
                {
                    // skip IS_RESTRICTED records unless there already is a result
                    if (result_counter || !IS_RESTRICTED(i)) {
                        if (coDex(i) < 54) {
                            char headerletter = (char) ((REC_TYPE(i) == 1) ? HEADER_LETTER(i) : 0);
                            encodeGrid(result, enc, i, extraDigits, headerletter);
                        }
                    }
                }

                // =========== handle result (if any)
                if (*result) {
                    result_counter++;

                    repackIfAllDigits(result, 0);

                    if ((requiredEncoder < 0) || (requiredEncoder == i)) {
                        const enum Territory ccodeFinal = (ccode_override != TERRITORY_NONE ? ccode_override : ccode);
                        if (*result && enc->mapcodes && (enc->mapcodes->count < MAX_NR_OF_MAPCODE_RESULTS)) {
                            char *s = enc->mapcodes->mapcode[enc->mapcodes->count++];
                            if (ccodeFinal == TERRITORY_AAA) { // AAA is never shown with territory
                                strcpy(s, result);
                            } else {
                                getTerritoryIsoName(s, ccodeFinal, 0);
                                strcat(s, " ");
                                strcat(s, result);
                            }
                        }
                        if (requiredEncoder == i) {
                            return;
                        }
                    }
                    if (stop_with_one_result) {
                        return;
                    }
                    *result = 0; // clear for next iteration
                }
            }
        } // for i
    }
}


// pass Point to an array of pointers (at least 42), will be made to Point to result strings...
// returns nr of results;
static int encodeLatLonToMapcodes_internal(Mapcodes *mapcodes,
                                           const double lat, const double lon,
                                           const enum Territory territoryContext, const int stop_with_one_result,
                                           const int requiredEncoder, const int extraDigits) {
    EncodeRec enc;
    enc.mapcodes = mapcodes;
    enc.mapcodes->count = 0;
    ASSERT(mapcodes);
    ASSERT((0 <= extraDigits) && (extraDigits <= MAX_PRECISION_DIGITS));

    if (convertCoordsToMicrosAndFractions(&enc.coord32, &enc.fraclat, &enc.fraclon, lat, lon) < 0) {
        return 0;
    }

    if (territoryContext < _TERRITORY_MIN) // ALL results?
    {

#ifndef NO_FAST_ENCODE
        {
            const int sum = enc.coord32.lonMicroDeg + enc.coord32.latMicroDeg;
            int coord = enc.coord32.lonMicroDeg;
            int i = 0; // pointer into REDIVAR
            for (;;) {
                const int r = REDIVAR[i++];
                if (r >= 0 && r < 1024) { // leaf?
                    int j;
                    for (j = 0; j <= r; j++) {
                        const enum Territory ccode = (j == r ? TERRITORY_AAA : (enum Territory) REDIVAR[i + j]);
                        encoderEngine(ccode, &enc, stop_with_one_result, extraDigits, requiredEncoder, TERRITORY_NONE);
                        if ((stop_with_one_result || (requiredEncoder >= 0)) && (enc.mapcodes->count > 0)) {
                            break;
                        }
                    }
                    break;
                } else {
                    coord = sum - coord;
                    if (coord > r) {
                        i = REDIVAR[i];
                    } else {
                        i++;
                    }
                }
            }
        }
#else
        {
            int i;
            for (i = _TERRITORY_MIN + 1; i < _TERRITORY_MAX; i++) {
                encoderEngine((enum Territory) i, &enc, stop_with_one_result, extraDigits, requiredEncoder, TERRITORY_NONE);
                if ((stop_with_one_result || (requiredEncoder >= 0)) && (enc.mapcodes->count > 0)) {
                    break;
                }
            }
        }
#endif

    } else {
        encoderEngine(territoryContext, &enc, stop_with_one_result, extraDigits, requiredEncoder, TERRITORY_NONE);
    }
    return mapcodes->count;
}

///////////////////////////////////////////////////////////////////////////////////////////////
//
//  DECODING
//
///////////////////////////////////////////////////////////////////////////////////////////////

typedef struct {
    // input
    MapcodeElements mapcodeElements;
    const char *orginput;   // original full input string
    const char *mapcode;    // input mapcode (first character of proper mapcode excluding territory code)
    const char *extension;  // input extension (or empty)
    enum Territory context; // input territory context (or TERRITORY_NONE)
    const char *iso;        // input territory alphacode (context)
    // output
    Point result;           // result
    Point32 coord32;        // result in integer arithmetic (microdegrees)
    MapcodeZone zone;       // result zone (in "DegreeFractions")
} DecodeRec;


// decode the high-precision extension (0-8 characters)
// this routine takes the integer-arithmeteic decoding results (dec->coord32), adds precision, 
// and determines result zone (dec->zone); returns negative in case of error.
static enum MapcodeError decodeExtension(DecodeRec *dec,
                                         int dividerx4, int dividery,
                                         const int lon_offset4,
                                         const int extremeLat32, const int maxLon32) {
    double lat1, lon4;
    const char *extrapostfix = dec->extension;
    int lon32 = 0;
    int lat32 = 0;
    int processor = 1;
    int odd = 0;
    ASSERT(dec);
    if (strlen(extrapostfix) > MAX_PRECISION_DIGITS) {
        return ERR_EXTENSION_INVALID_LENGTH;
    }
    while (*extrapostfix) {
        int column1, row1, column2, row2;
        const int c1 = decodeChar(*extrapostfix++);
        if (c1 < 0 || c1 == 30) {
            return ERR_EXTENSION_INVALID_CHARACTER;
        } // illegal extension character
        row1 = (c1 / 5);
        column1 = (c1 % 5);
        if (*extrapostfix) {
            const int c2 = decodeChar(*extrapostfix++);
            if (c2 < 0 || c2 == 30) {
                return ERR_EXTENSION_INVALID_CHARACTER;
            } // illegal extension character
            row2 = (c2 / 6);
            column2 = (c2 % 6);
        } else {
            odd = 1;
            row2 = 0;
            column2 = 0;
        }

        processor *= 30;
        lon32 = lon32 * 30 + column1 * 6 + column2;
        lat32 = lat32 * 30 + row1 * 5 + row2;
    }

    while (processor < MAX_PRECISION_FACTOR) {
        dividerx4 *= 30;
        dividery *= 30;
        processor *= 30;
    }

    lon4 = (dec->coord32.lonMicroDeg * 4 * (double) MAX_PRECISION_FACTOR) + ((lon32 * (double) dividerx4)) +
           (lon_offset4 * (double) MAX_PRECISION_FACTOR);
    lat1 = (dec->coord32.latMicroDeg * (double) MAX_PRECISION_FACTOR) + ((lat32 * (double) dividery));

    // determine the range of coordinates that are encoded to this mapcode
    if (odd) {
        setFromFractions(&dec->zone, lat1, lon4, 5 * dividery, 6 * dividerx4);
    } else {
        setFromFractions(&dec->zone, lat1, lon4, dividery, dividerx4);
    }

    // restrict the coordinate range to the extremes that were provided
    if (dec->zone.fmaxx > maxLon32 * MICROLON_TO_FRACTIONS_FACTOR) {
        dec->zone.fmaxx = maxLon32 * MICROLON_TO_FRACTIONS_FACTOR;
    }
    if (dividery >= 0) {
        if (dec->zone.fmaxy > extremeLat32 * MICROLAT_TO_FRACTIONS_FACTOR) {
            dec->zone.fmaxy = extremeLat32 * MICROLAT_TO_FRACTIONS_FACTOR;
        }
    } else {
        if (dec->zone.fminy < extremeLat32 * MICROLAT_TO_FRACTIONS_FACTOR) {
            dec->zone.fminy = extremeLat32 * MICROLAT_TO_FRACTIONS_FACTOR;
        }
    }
    if (isEmpty(&dec->zone)) {
        return ERR_EXTENSION_UNDECODABLE;
    }
    return ERR_OK;
}


// decode 'code' until either a dot or an end-of-string is encountered
static int decodeBase31(const char *code) {
    int value = 0;
    ASSERT(code);
    while (*code != '.' && *code != 0) {
        value = value * 31 + decodeChar(*code++);
    }
    return value;
}


static void decodeTriple(const char *result, int *difx, int *dify) {
    // decode the first character
    const int c1 = decodeChar(*result++);
    ASSERT(result);
    ASSERT(difx);
    ASSERT(dify);
    if (c1 < 24) {
        int m = decodeBase31(result);
        *difx = (c1 % 6) * 28 + (m / 34);
        *dify = (c1 / 6) * 34 + (m % 34);
    } else // bottom row
    {
        int x = decodeBase31(result);
        *dify = (x % 40) + 136;
        *difx = (x / 40) + 24 * (c1 - 24);
    }
} // decodeTriple

static void decodeSixWide(const int v, const int width, const int height,
                          int *x, int *y) {
    int w;
    int D = 6;
    int col = v / (height * 6);
    const int maxcol = (width - 4) / 6;
    ASSERT(x);
    ASSERT(y);
    if (col >= maxcol) {
        col = maxcol;
        D = width - maxcol * 6;
    }
    w = v - (col * height * 6);

    *x = col * 6 + (w % D);
    *y = height - 1 - (w / D);
}

// *** mid-level encode routines ***

// decodes dec->mapcode in context of territory rectangle m; returns negative if error
static enum MapcodeError decodeGrid(DecodeRec *dec, const int m, const int hasHeaderLetter) {
    const char *input = (hasHeaderLetter ? dec->mapcode + 1 : dec->mapcode);
    const int codexlen = (int) (strlen(input) - 1);
    int prelen = (int) (strchr(input, '.') - input);
    char result[MAX_PROPER_MAPCODE_ASCII_LEN + 1];
    ASSERT(dec);

    if (codexlen > MAX_PROPER_MAPCODE_ASCII_LEN) {
        return ERR_BAD_MAPCODE_LENGTH;
    }
    if (prelen > 5) {
        return ERR_UNEXPECTED_DOT;
    }

    strcpy(result, input);
    if (prelen == 1 && codexlen == 5) {
        result[1] = result[2];
        result[2] = '.';
        prelen++;
    }

    {
        const int postlen = codexlen - prelen;

        int divx, divy;

        divy = SMART_DIV(m);
        ASSERT(divy > 0);
        if (divy == 1) {
            divx = X_SIDE[prelen];
            divy = Y_SIDE[prelen];
        } else {
            divx = (NC[prelen] / divy);
        }

        if (prelen == 4 && divx == 961 && divy == 961) {
            char t = result[1];
            result[1] = result[2];
            result[2] = t;
        }

        {
            int relx, rely;
            int v = decodeBase31(result);

            if (divx != divy && prelen > 2) {
                // special grid, useful when prefix is 3 or more, and not a nice 961x961
                decodeSixWide(v, divx, divy, &relx, &rely);
            } else {
                relx = (v / divy);
                rely = divy - 1 - (v % divy);
            }

            if (relx < 0 || rely < 0 || relx >= divx || rely >= divy) {
                return ERR_MAPCODE_UNDECODABLE; // type 4 "usa A222.22AA"
            }


            {
                const TerritoryBoundary *b = TERRITORY_BOUNDARY(m);
                const int ygridsize = (b->maxy - b->miny + divy - 1) / divy; // microdegrees per cell
                const int xgridsize = (b->maxx - b->minx + divx - 1) / divx; // microdegrees per cell

                // encode relative to THE CORNER of this cell
                rely = b->miny + (rely * ygridsize);
                relx = b->minx + (relx * xgridsize);

                {
                    const int xp = X_SIDE[postlen];
                    const int dividerx = ((xgridsize + xp - 1) / xp);
                    const int yp = Y_SIDE[postlen];
                    const int dividery = ((ygridsize + yp - 1) / yp);
                    // decoderelative

                    {
                        char *r = result + prelen + 1;
                        int difx, dify;

                        if (postlen == 3) // decode special
                        {
                            decodeTriple(r, &difx, &dify);
                        } else {
                            if (postlen == 4) {
                                char t = r[1];
                                r[1] = r[2];
                                r[2] = t;
                            } // swap
                            v = decodeBase31(r);
                            difx = (v / yp);
                            dify = (v % yp);
                            if (postlen == 4) {
                                char t = r[1];
                                r[1] = r[2];
                                r[2] = t;
                            } // swap back
                        }

                        // reverse y-direction
                        dify = yp - 1 - dify;

                        dec->coord32.lonMicroDeg = relx + (difx * dividerx);
                        dec->coord32.latMicroDeg = rely + (dify * dividery);
                        if (!fitsInsideBoundaries(&dec->coord32, TERRITORY_BOUNDARY(m))) {
                            return ERR_MAPCODE_UNDECODABLE; // type 2 "NLD Q000.000"
                        }

                        {
                            const int decodeMaxx = ((relx + xgridsize) < b->maxx) ? (relx + xgridsize) : b->maxx;
                            const int decodeMaxy = ((rely + ygridsize) < b->maxy) ? (rely + ygridsize) : b->maxy;
                            return decodeExtension(dec, dividerx << 2, dividery, 0, decodeMaxy, decodeMaxx); // grid
                        }
                    } // decoderelative
                }
            }
        }
    }
}


// decodes dec->mapcode in context of territory rectangle m, territory dec->context
// Returns negative in case of error
static enum MapcodeError decodeNameless(DecodeRec *dec, int m) {
    int A, F;
    char input[8];
    const int codexm = coDex(m);
    const int codexlen = (int) (strlen(dec->mapcode) - 1);
    ASSERT(dec);
    ASSERT((0 <= m) && (m <= MAPCODE_BOUNDARY_MAX));
    if (codexlen != 4 && codexlen != 5) {
        return ERR_BAD_MAPCODE_LENGTH;
    } // solve bad args

    // copy without dot
    {
        const int dc = (codexm != 22) ? 2 : 3;
        strcpy(input, dec->mapcode);
        strcpy(input + dc, dec->mapcode + dc + 1);
    }

    A = countNamelessRecords(m, firstRec(dec->context));
    F = firstNamelessRecord(m, firstRec(dec->context));

    {
        const int p = 31 / A;
        const int r = 31 % A;
        int v = 0;
        int SIDE;
        int swapletters = 0;
        int xSIDE;
        int X;
        const TerritoryBoundary *b;

        // make copy of input, so we can swap around letters during the decoding
        char result[32];
        strcpy(result, input);

        // now determine X = index of first area, and SIDE
        if (codexm != 21 && A <= 31) {
            const int offset = decodeChar(*result);

            if (offset < r * (p + 1)) {
                X = offset / (p + 1);
            } else {
                swapletters = ((p == 1) && (codexm == 22));
                X = r + (offset - (r * (p + 1))) / p;
            }
        } else if (codexm != 21 && A < 62) {

            X = decodeChar(*result);
            if (X < (62 - A)) {
                swapletters = (codexm == 22);
            } else {
                X = X + (X - (62 - A));
            }
        } else // code==21 || A>=62
        {
            const int BASEPOWER = (codexm == 21) ? 961 * 961 : 961 * 961 * 31;
            int BASEPOWERA = (BASEPOWER / A);

            if (A == 62) {
                BASEPOWERA++;
            } else {
                BASEPOWERA = 961 * (BASEPOWERA / 961);
            }

            v = decodeBase31(result);
            X = (v / BASEPOWERA);
            v %= BASEPOWERA;
        }


        if (swapletters) {
            if (!IS_SPECIAL_SHAPE(F + X)) {
                const char t = result[codexlen - 3];
                result[codexlen - 3] = result[codexlen - 2];
                result[codexlen - 2] = t;
            }
        }

        if (codexm != 21 && A <= 31) {
            v = decodeBase31(result);
            if (X > 0) {
                v -= (X * p + (X < r ? X : r)) * (961 * 961);
            }
        } else if (codexm != 21 && A < 62) {
            v = decodeBase31(result + 1);
            if (X >= (62 - A)) {
                if (v >= (16 * 961 * 31)) {
                    v -= (16 * 961 * 31);
                    X++;
                }
            }
        }

        m = (F + X);

        SIDE = SMART_DIV(m);
        ASSERT(SIDE > 0);
        xSIDE = SIDE;

        b = TERRITORY_BOUNDARY(m);

        // decode
        {
            int dx, dy;

            if (IS_SPECIAL_SHAPE(m)) {
                xSIDE *= SIDE;
                SIDE = 1 + ((b->maxy - b->miny) / 90); // side purely on y range
                xSIDE = xSIDE / SIDE;

                decodeSixWide(v, xSIDE, SIDE, &dx, &dy);
                dy = SIDE - 1 - dy;
            } else {
                dy = v % SIDE;
                dx = v / SIDE;
            }


            if (dx >= xSIDE) {
                return ERR_MAPCODE_UNDECODABLE; // type 1 "NLD ZZ.ZZ"
            }

            {
                const int dividerx4 = xDivider4(b->miny, b->maxy); // *** note: dividerx4 is 4 times too large!
                const int dividery = 90;

                // *** note: FIRST multiply, then divide... more precise, larger rects
                dec->coord32.lonMicroDeg = b->minx + ((dx * dividerx4) / 4);
                dec->coord32.latMicroDeg = b->maxy - (dy * dividery);

                return decodeExtension(dec, dividerx4, -dividery, ((dx * dividerx4) % 4), b->miny, b->maxx); // nameless
            }
        }
    }
}


// decodes dec->mapcode in context of territory rectangle m or one of its mates
static enum MapcodeError decodeAutoHeader(DecodeRec *dec, int m) {
    const char *input = dec->mapcode;
    const int codexm = coDex(m);
    const char *dot = strchr(input, '.');
    int STORAGE_START = 0;
    int value;
    ASSERT(dec);
    ASSERT((0 <= m) && (m <= MAPCODE_BOUNDARY_MAX));
    if (dot == NULL) {
        return ERR_DOT_MISSING;
    }

    value = decodeBase31(input); // decode top
    value *= (961 * 31);

    for (; coDex(m) == codexm && REC_TYPE(m) > 1; m++) {
        const TerritoryBoundary *b = TERRITORY_BOUNDARY(m);
        // determine how many cells
        int H = (b->maxy - b->miny + 89) / 90; // multiple of 10m
        const int xdiv = xDivider4(b->miny, b->maxy);
        int W = ((b->maxx - b->minx) * 4 + (xdiv - 1)) / xdiv;
        int product;

        // decode
        H = 176 * ((H + 176 - 1) / 176);
        W = 168 * ((W + 168 - 1) / 168);
        product = (W / 168) * (H / 176) * 961 * 31;

        if (REC_TYPE(m) == 2) {
            const int GOODROUNDER = codexm >= 23 ? (961 * 961 * 31) : (961 * 961);
            product = ((STORAGE_START + product + GOODROUNDER - 1) / GOODROUNDER) * GOODROUNDER - STORAGE_START;
        }

        if (value >= STORAGE_START && value < STORAGE_START + product) {
            const int dividerx = (b->maxx - b->minx + W - 1) / W;
            const int dividery = (b->maxy - b->miny + H - 1) / H;

            value -= STORAGE_START;
            value /= (961 * 31);

            {
                int difx, dify;
                decodeTriple(dot + 1, &difx, &dify); // decode bottom 3 chars
                {
                    const int vx = (value / (H / 176)) * 168 + difx; // is vx/168
                    const int vy = (value % (H / 176)) * 176 + dify; // is vy/176

                    dec->coord32.latMicroDeg = b->maxy - vy * dividery;
                    dec->coord32.lonMicroDeg = b->minx + vx * dividerx;

                    if ((dec->coord32.lonMicroDeg < b->minx) || (dec->coord32.lonMicroDeg >= b->maxx) ||
                        (dec->coord32.latMicroDeg < b->miny) ||
                        (dec->coord32.latMicroDeg > b->maxy)) // *** CAREFUL! do this test BEFORE adding remainder...
                    {
                        return ERR_MAPCODE_UNDECODABLE; // type 5 "atf hhh.hhh"
                    }
                }
            }

            return decodeExtension(dec, dividerx << 2, -dividery, 0, b->miny, b->maxx); // autoheader decode
        }
        STORAGE_START += product;
    } // for j
    return ERR_MAPCODE_UNDECODABLE; // type 6 "ASM zz.zzh"
}

/**
 * ROMAN / ABJAD
 *
 */


// Returns romanised version of character, or question mark in not recognized
static unsigned char getRomanVersionOf(UWORD w) {
    if (w > ROMAN_VERSION_MAX_CHAR || ROMAN_VERSION_OF[w >> 6] == NULL) {
        return '?';
    }
    return (unsigned char) ROMAN_VERSION_OF[w >> 6][w & 63];
}


static void convertFromAbjad(char *s) {
    int len, dot, form, c;
    char *postfix = strchr(s, '-');
    dot = (int) (strchr(s, '.') - s);
    if (dot < 2 || dot > 5) {
        return;
    }
    if (postfix) {
        *postfix = 0;
    }

    unpackIfAllDigits(s);

    len = (int) strlen(s);
    form = (dot >= 2 && dot <= 5 ? dot * 10 + (len - dot - 1) : 0);

    if (form == 23) {
        c = decodeChar(s[3]) * 8 + (decodeChar(s[4]) - 18);
        if (c >= 0 && c < 31) {
//          s[0] = s[0];
//          s[1] = s[1];
//          s[2] = '.';
            s[3] = ENCODE_CHARS[c];
            s[4] = s[5];
            s[5] = 0;
        }
    } else if (form == 24) {
        c = decodeChar(s[3]) * 8 + (decodeChar(s[4]) - 18);
        if (c >= 0 && c < 63) {
//          s[0] = s[0];
//          s[1] = s[1];
//          s[2] = '.';
            s[3] = '.';
            s[4] = s[5];
            s[5] = s[6];
            s[6] = 0;
            if (c >= 32) {
                s[2] = ENCODE_CHARS[c - 32];
            } else {
                s[3] = ENCODE_CHARS[c];
            }
        }
    } else if (form == 34) {
        c = (decodeChar(s[2]) * 10) + (decodeChar(s[5]) - 7);
        if (c >= 0 && c < 93) {
//          s[0] = s[0];
//          s[1] = s[1];
            s[2] = '.';
//          s[3] = '.';
//          s[4] = s[4];
            s[5] = s[6];
            s[6] = s[7];
            s[7] = 0;

            if (c < 31) {
                s[3] = ENCODE_CHARS[c];
            } else if (c < 62) {
                s[2] = ENCODE_CHARS[c - 31];
            } else {
                s[2] = ENCODE_CHARS[c - 62];
                s[3] = s[4];
                s[4] = '.';
            }
        }
    } else if (form == 35) {
        c = (decodeChar(s[2]) * 8) + (decodeChar(s[6]) - 18);
        if (c >= 0 && c < 63) {
//          s[0] = s[0];
//          s[1] = s[1];
//          s[3] = '.';
//          s[4] = s[4];
//          s[5] = s[5];
            s[6] = s[7];
            s[7] = s[8];
            s[8] = 0;
            if (c >= 32) {
                s[2] = ENCODE_CHARS[c - 32];
                s[3] = s[4];
                s[4] = '.';
            } else {
                s[2] = ENCODE_CHARS[c];
            }
        }
    } else if (form == 45) {
        c = (decodeChar(s[2]) * 100) + (decodeChar(s[5]) * 10) + (decodeChar(s[8]) - 39);
        if (c >= 0 && c < 961) {
//          s[0] = s[0];
//          s[1] = s[1];
            s[2] = ENCODE_CHARS[c / 31];
//          s[3] = s[3];
//          s[4] = '.';
            s[5] = s[6];
            s[6] = s[7];
            s[7] = s[9];
            s[8] = ENCODE_CHARS[c % 31];
            s[9] = 0;
        }
    } else if (form == 55) {
        c = (decodeChar(s[2]) * 100) + (decodeChar(s[6]) * 10) + (decodeChar(s[9]) - 39);
        if (c >= 0 && c < 961) {
//          s[0] = s[0];
//          s[1] = s[1];
            s[2] = ENCODE_CHARS[c / 31];
//          s[3] = s[3];
//          s[4] = s[4];
//          s[5] = '.';
            s[6] = s[7];
            s[7] = s[8];
            s[8] = s[10];
            s[9] = ENCODE_CHARS[c % 31];
            s[10] = 0;
        }
    }
    repackIfAllDigits(s, 0);
    if (postfix) {
        len = (int) strlen(s);
        *postfix = '-';
        memmove(s + len, postfix, strlen(postfix) + 1);
    }
}


/**
 * Returns the alphabet of given UTF8 (of ASCII) string (based on the
 * first recognizable non-Latin character).
 *
 * Arguments:
 *      utf8   - Zero-terminated UTF8 (or ASCII) string
 *
 * Returns:
 *      ALPHABET_ROMAN if all characters are in ASCII range 0..0xBF.
 *      otherwise returns the alphabet of the first different character
 *      encountered, or negative (_ALPHABET_MIN) if it isn't recognized.
 */
static enum Alphabet recognizeAlphabetUtf8(const char *utf8) {
    ASSERT(utf8);
    while (*utf8 != 0) {
        int c = (unsigned char) *utf8++;
        if (c >= 0xC0) {
            enum Alphabet alphabet;
            int c2 = (unsigned char) *utf8++;
            if (c2 < 0x80) {
                return _ALPHABET_MIN; // utf8 error!
            }
            c = ((c - 0xC0) << 6) + (c2 & 63);
            if (c >= 0x800) {
                int c3 = (unsigned char) *utf8++;
                c = ((c - 0x800) << 6) + (c3 & 63);
                if (c3 < 0x80 || c > 0xFFFF) {
                    return _ALPHABET_MIN; // utf8 error!
                }
            }
            alphabet = ALPHABET_OF_CHAR((UWORD) c);
            if (alphabet != ALPHABET_ROMAN) {
                return alphabet;
            }
        }
    }
    return ALPHABET_ROMAN;
}


///////////////////////////////////////////////////////////////////////////////////////////////
//
//  compareWithMapcodeFormat & parseMapcode
//
///////////////////////////////////////////////////////////////////////////////////////////////


// 32=busyextension 64=end territory 128(256)=end of clean mapcode(with extension) 512=end of extension 
static const int STATE_MACHINE[27][6] = {
        // SPACE                       DOT                 DETTER                      VOWEL                            ZERO                      HYPHEN
        // 0 start === looking for very first detter
        {0,                            ERR_UNEXPECTED_DOT, 1,                          1,                               ERR_DOT_MISSING,          ERR_UNEXPECTED_HYPHEN},
        // 1 L/P === det:LL vowel:TA
        {ERR_BAD_TERRITORY_FORMAT,     ERR_UNEXPECTED_DOT, 2,                          23,                              ERR_DOT_MISSING,          ERR_BAD_TERRITORY_FORMAT},
        // 2 LL/PP === white: TT waitprefix | dot: PP. | det:LLL/PPP | vowel:TTA | hyphen:TT-
        {18 |
         64,                           6,                  3,                          24,                              ERR_DOT_MISSING,          14},
        // 3 LLL/PPP === white: TTT prefix | dot: PPP. mapcode | det: PPPP | hyphen: TTT-
        {18 |
         64,                           6,                  4,                          ERR_INVALID_VOWEL,               ERR_DOT_MISSING,          14},
        // 4 PPPP === dot: PPPP. | det: PPPPP
        {ERR_BAD_TERRITORY_FORMAT,     6,                  5,                          ERR_INVALID_VOWEL,               ERR_DOT_MISSING,          ERR_BAD_TERRITORY_FORMAT},
        // 5 PPPPP === must get dot now! Dot:PPPPP.
        {ERR_BAD_TERRITORY_FORMAT,     6,                  ERR_INVALID_MAPCODE_FORMAT, ERR_INVALID_VOWEL,               ERR_DOT_MISSING,          ERR_BAD_TERRITORY_FORMAT},
        // 6 prefix. === get first postfix! det: prefix.L | vowel: prefix.A
        {ERR_INVALID_MAPCODE_FORMAT,   ERR_UNEXPECTED_DOT, 7,                          25,                              ERR_MAPCODE_INCOMPLETE,   ERR_UNEXPECTED_HYPHEN},
        // 7 prefix.L === get 2nd postfix! det: prefix.LL | vowel: prefix.LA
        {ERR_INVALID_MAPCODE_FORMAT,   ERR_UNEXPECTED_DOT, 8,                          25,                              ERR_MAPCODE_INCOMPLETE,   ERR_UNEXPECTED_HYPHEN},
        // 8 prefix.LL === get 3d postfix! white:trail | det: prefix.LLL | vowel: prefix.LLA | zero:done | hyphen: mc-
        {22 | 128,                     ERR_UNEXPECTED_DOT, 9,                          25,                      STATE_GO |
                                                                                                                128,
                                                                                                                     11 |
                                                                                                                     256},
        // 9 prefix.LLL === white:trail | zero:done | hyphen:mc-
        {22 |
         128,                          ERR_UNEXPECTED_DOT, 10,                         25,                      STATE_GO |
                                                                                                                128, 11 |
                                                                                                                     256},
        //10 prefix.LLLL === white:trail | zero:done | hyphen:mc- | det/vowel = postfix full
        {22 |
         128,                          ERR_UNEXPECTED_DOT, 13,                         13,                      STATE_GO |
                                                                                                                128, 11 |
                                                                                                                     256},

        //11 mc- === MUST get first precision detter
        {ERR_EXTENSION_INVALID_LENGTH, ERR_UNEXPECTED_DOT, 12,                         ERR_EXTENSION_INVALID_CHARACTER, ERR_MAPCODE_INCOMPLETE,   ERR_UNEXPECTED_HYPHEN},
        //12 mc-L* === Keep reading precision detters | white=trail | zero=done
        {22 | 512,                     ERR_UNEXPECTED_DOT, 12 | 32,                    ERR_EXTENSION_INVALID_CHARACTER,
                                                                                                                STATE_GO |
                                                                                                                512, ERR_UNEXPECTED_HYPHEN},

        //13 prefix.LLLLL === 
        {22 |
         128,                          ERR_UNEXPECTED_DOT, ERR_INVALID_MAPCODE_FORMAT, ERR_INVALID_VOWEL,       STATE_GO |
                                                                                                                128, 11 |
                                                                                                                     256},

        //14 TC- === get first state letter
        {ERR_BAD_TERRITORY_FORMAT,     ERR_UNEXPECTED_DOT, 15,                         15,                              ERR_BAD_TERRITORY_FORMAT, ERR_UNEXPECTED_HYPHEN},
        //15 TC-S === get 2nd state letter
        {ERR_BAD_TERRITORY_FORMAT,     ERR_UNEXPECTED_DOT, 16,                         16,                              ERR_BAD_TERRITORY_FORMAT, ERR_UNEXPECTED_HYPHEN},
        //16 TC-SS === white:waitprefix | det/vow:TC-SSS 
        {18 |
         64,                           ERR_UNEXPECTED_DOT, 17,                         17,                              ERR_DOT_MISSING,          ERR_UNEXPECTED_HYPHEN},
        //17 TC-SSS === white:waitprefix
        {18 |
         64,                           ERR_UNEXPECTED_DOT, ERR_BAD_TERRITORY_FORMAT,   ERR_BAD_TERRITORY_FORMAT,        ERR_DOT_MISSING,          ERR_UNEXPECTED_HYPHEN},

        //18 TC waitprefix === skip more whitespace, MUST get 1st prefix letter/vowel
        {18,                           ERR_UNEXPECTED_DOT, 19,                         19,                              ERR_DOT_MISSING,          ERR_UNEXPECTED_HYPHEN},
        //19 TC P === get second prefix detter
        {ERR_DOT_MISSING,              ERR_UNEXPECTED_DOT, 20,                         ERR_INVALID_VOWEL,               ERR_DOT_MISSING,          ERR_UNEXPECTED_HYPHEN},
        //20 TC PP === dot:prefix. | det:TC PPP
        {ERR_DOT_MISSING,              6,                  21,                         ERR_INVALID_VOWEL,               ERR_DOT_MISSING,          ERR_UNEXPECTED_HYPHEN},
        //21 TC PPP === dot:prefix. | det:PPPP
        {ERR_DOT_MISSING,              6,                  4,                          ERR_INVALID_VOWEL,               ERR_DOT_MISSING,          ERR_UNEXPECTED_HYPHEN},

        //22 trailing === skip whitespace until end of string
        {22,                           ERR_UNEXPECTED_DOT, ERR_TRAILING_CHARACTERS,    ERR_TRAILING_CHARACTERS, STATE_GO,                         ERR_UNEXPECTED_HYPHEN},

        //23 TA === white:waitprefix | det: TAT | vowel:TAA | hyphen:TC-
        {18 |
         64,                           ERR_INVALID_VOWEL,  24,                         24,                              ERR_DOT_MISSING,          14},
        //24 TTA/TAT/TAA === space:TC waitprefix | hyphen:TC-
        {18 |
         64,                           ERR_INVALID_VOWEL,  ERR_INVALID_VOWEL,          ERR_INVALID_VOWEL,               ERR_DOT_MISSING,          14},

        //25 prefix.[L*]A === white:trail | det/vow:full mc | zero:done | hyphen:mc-
        {22 |
         128,                          ERR_UNEXPECTED_DOT, 26,                         26,                      STATE_GO |
                                                                                                                128, 11 |
                                                                                                                     256},
        //26 prefix.[L*]AL === white:trail | zero:done | hyphen:mc-
        {22 |
         128,                          ERR_UNEXPECTED_DOT, ERR_INVALID_VOWEL,          ERR_INVALID_VOWEL,       STATE_GO |
                                                                                                                128, 11 |
                                                                                                                     256},
};


// Returns 0 if ok, negative in case of error (where -999 represents "may BECOME a valid mapcode if more characters are added)
static enum MapcodeError parseMapcodeString(MapcodeElements *mapcodeElements, const char *string, int interpretAsUtf16,
                                            enum Territory territory) {
    const UWORD *utf16 = (const UWORD *) string;
    int isAbjad = 0;
    const unsigned char *utf8 = (unsigned char *) string;
    int extensionLength = 0;
    char *cleanPtr = NULL;
    int nondigits = 0, vowels = 0;
    int state = 0;
    ASSERT(string);
    if (mapcodeElements) {
        *mapcodeElements->precisionExtension = 0;
        *mapcodeElements->territoryISO = 0;
        cleanPtr = mapcodeElements->properMapcode;
    }
    for (;;) {
        int newstate, token;
        unsigned char cx;
        // handle utf16
        if (interpretAsUtf16) {
            const enum Alphabet alphabet = ALPHABET_OF_CHAR(*utf16);
            if (alphabet == ALPHABET_GREEK || alphabet == ALPHABET_HEBREW ||
                alphabet == ALPHABET_ARABIC || alphabet == ALPHABET_KOREAN) {
                isAbjad = 1;
            }
            cx = getRomanVersionOf(*utf16++);
        } else {
            cx = *utf8++;
        }
        // recognize token: decode returns -2=a -3=e -4=0, 0..9 for digit or "o" or "i", 10..31 for char, -1 for illegal char
        if (cx == '.') {
            token = TOKENDOT;
            if (mapcodeElements) {
                mapcodeElements->indexOfDot = (int) (cleanPtr - mapcodeElements->properMapcode);
            }
            if (mapcodeElements) {
                *cleanPtr++ = cx;
            }
        } else if (cx == '-') {
            token = TOKENHYPH;
            if (mapcodeElements) {
                *cleanPtr++ = cx;
            }
        } else if (cx == 0) {
            token = TOKENZERO;
        } else if ((cx == ' ') || (cx == '\t')) {
            token = TOKENSEP;
        } else {
            signed char c;
            if (cx >= 0xC0) { // utf8 character
                unsigned char c2 = *utf8++;
                int w = ((cx - 0xC0) << 6) + (c2 & 63);
                if (c2 < 0x80) {
                    return ERR_INVALID_CHARACTER; // utf8 error
                }
                if (w >= 0x800) {
                    int c3 = (int) *utf8++;
                    w = ((w - 0x800) << 6) + (c3 & 63);
                    if (c3 < 0x80 || w > 0xFFFF) {
                        return ERR_INVALID_CHARACTER; // utf8 error
                    }
                }
                {
                    const enum Alphabet alphabet = ALPHABET_OF_CHAR(w);
                    if (alphabet == ALPHABET_GREEK || alphabet == ALPHABET_HEBREW ||
                        alphabet == ALPHABET_ARABIC || alphabet == ALPHABET_KOREAN) {
                        isAbjad = 1;
                    }
                }
                cx = getRomanVersionOf((UWORD) w);
            }
            c = decodeChar(cx);
            if (c < 0) { // vowel or illegal?                
                if (c == -1) { // illegal?
                    return ERR_INVALID_CHARACTER;
                }
                token = TOKENVOWEL;
                vowels++;
                if (mapcodeElements) {
                    *cleanPtr++ = (char) toupper(cx);
                }
            } else if (c < 10) { // digit
                token = TOKENCHR; // digit
                if (mapcodeElements) {
                    *cleanPtr++ = (char) toupper(cx);
                }
            } else { // character B-Z
                token = TOKENCHR;
                if (!extensionLength) {
                    nondigits++;
                }
                if (mapcodeElements) {
                    *cleanPtr++ = (char) toupper(cx);
                }
            }
        }
        newstate = STATE_MACHINE[state][token];
        if (newstate >= 32) {
            if (newstate >= 512) { // end of extension
                if (mapcodeElements) {
                    *cleanPtr = 0;
                    cleanPtr = mapcodeElements->precisionExtension;
                }
            } else if (newstate >= 128) {
                if (newstate >= 256) { // start of extension
                    extensionLength = 1;
                    cleanPtr--; // get rid of hyphen
                }
                // end of proper mapcode
                if (mapcodeElements) {
                    *cleanPtr = 0;
                    cleanPtr = mapcodeElements->precisionExtension;
                }
            } else if (newstate >= 64) { // end of territory
                nondigits = vowels = 0;
                if (mapcodeElements) {
                    int len = (int) (cleanPtr - mapcodeElements->properMapcode);
                    ASSERT(len < MAX_ISOCODE_ASCII_LEN);
                    lengthCopy(mapcodeElements->territoryISO, mapcodeElements->properMapcode, len, MAX_ISOCODE_ASCII_LEN + 1);
                    cleanPtr = mapcodeElements->properMapcode;
                }
            } else { // add to extension
                if (++extensionLength > MAX_PRECISION_DIGITS) {
                    return ERR_EXTENSION_INVALID_LENGTH;
                }
            }
            newstate &= 31;
        }

        if (newstate < 0) {
            return (enum MapcodeError) newstate;
        } else if (newstate == STATE_GO) {
            if (vowels > 3 || (nondigits == 1 && vowels > 1) || (nondigits > 1 && vowels > 0)) {
                return ERR_INVALID_VOWEL;
            } else if (nondigits == 0 && vowels == 0) {
                return ERR_ALL_DIGIT_CODE;
            }
            if (mapcodeElements) {
                if (*mapcodeElements->properMapcode == 'A') {
                    unpackIfAllDigits(mapcodeElements->properMapcode);
                    repackIfAllDigits(mapcodeElements->properMapcode, 0);
                }
                if (isAbjad) {
                    convertFromAbjad(mapcodeElements->properMapcode);
                    mapcodeElements->indexOfDot = (int) (strchr(mapcodeElements->properMapcode, '.') -
                                                         mapcodeElements->properMapcode);
                }
                if (*mapcodeElements->territoryISO) {
                    mapcodeElements->territoryCode = getTerritoryCode(mapcodeElements->territoryISO, territory);
                    if (mapcodeElements->territoryCode < _TERRITORY_MIN) {
                        return ERR_UNKNOWN_TERRITORY;
                    }
                } else {
                    mapcodeElements->territoryCode = territory;
                }
                if ((mapcodeElements->territoryCode == TERRITORY_MEX) && (strlen(mapcodeElements->properMapcode) < 8)) {
                    // special case: short MEX codes are handled in the state (which ALSO has iso code MEX)
                    mapcodeElements->territoryCode = TERRITORY_MX_MX;
                }
            }
            return ERR_OK;
        }
        state = newstate;
    }
    ASSERT(0);
}


enum MapcodeError compareWithMapcodeFormatUtf8(const char *utf8String) {
    ASSERT(utf8String);
    return parseMapcodeString(NULL, utf8String, FLAG_UTF8_STRING, TERRITORY_NONE);
}


enum MapcodeError compareWithMapcodeFormatUtf16(const UWORD *Utf16String) {
    ASSERT(Utf16String);
    return parseMapcodeString(NULL, (const char *) Utf16String, FLAG_UTF16_STRING, TERRITORY_NONE);
}


// returns nonzero if error
static enum MapcodeError decoderEngine(DecodeRec *dec, int parseFlags) {
    enum Territory ccode;
    enum MapcodeError err;
    int codex;
    int from;
    int upto;
    int i;
    char *s;
    int wasAllDigits = 0;
    ASSERT(dec);

    err = parseMapcodeString(&dec->mapcodeElements, dec->orginput, parseFlags, dec->context);
    if (err) { // clear all parsed fields in case of error
        dec->mapcodeElements.territoryISO[0] = 0;
        dec->mapcodeElements.properMapcode[0] = 0;
        dec->mapcodeElements.precisionExtension[0] = 0;
        return err;
    }

    ccode = dec->mapcodeElements.territoryCode;
    dec->context = ccode;
    dec->mapcode = dec->mapcodeElements.properMapcode;
    dec->extension = dec->mapcodeElements.precisionExtension;
    codex = dec->mapcodeElements.indexOfDot * 9 + (int) strlen(dec->mapcodeElements.properMapcode) - 1;
    s = dec->mapcodeElements.properMapcode;

    if (strchr(s, 'A') || strchr(s, 'E') || strchr(s, 'U')) {
        if (unpackIfAllDigits(s) <= 0) {
            return ERR_INVALID_VOWEL;
        }
        wasAllDigits = 1;
    }

    if (codex > 54) {
        ASSERT(codex == 55);
        return ERR_MAPCODE_UNDECODABLE;
    } else if (codex == 54) {
        // international mapcodes must be in international context
        ccode = TERRITORY_AAA;
    } else if (ccode < _TERRITORY_MIN) {
        return ERR_MISSING_TERRITORY;
    } else if (isSubdivision(ccode)) {
        // int mapcodes must be interpreted in the parent of a subdivision
        enum Territory parent = parentTerritoryOf(ccode);
        if ((codex == 44) || ((codex == 34 || codex == 43) && (parent == TERRITORY_IND || parent == TERRITORY_MEX))) {
            ccode = parent;
        }
    }

    from = firstRec(ccode);
    upto = lastRec(ccode);

    // try all ccode rectangles to decode s (pointing to first character of proper mapcode), assume not decodable
    err = ERR_MAPCODE_UNDECODABLE;
    for (i = from; i <= upto; i++) {
        const int codexi = coDex(i);
        const int r = REC_TYPE(i);
        if (r == 0) {
            if (IS_NAMELESS(i)) {
                if (((codexi == 21) && (codex == 22)) ||
                    ((codexi == 22) && (codex == 32)) ||
                    ((codexi == 13) && (codex == 23))) {
                    err = decodeNameless(dec, i);
                    break;
                }
            } else {
                if ((codexi == codex) || ((codex == 22) && (codexi == 21))) {
                    err = decodeGrid(dec, i, 0);

                    // first of all, make sure the zone fits the country
                    restrictZoneTo(&dec->zone, &dec->zone, TERRITORY_BOUNDARY(upto));

                    if ((err == ERR_OK) && IS_RESTRICTED(i)) {
                        int nrZoneOverlaps = 0;
                        int j;

                        // *** make sure decode fits somewhere ***
                        dec->result = getMidPointFractions(&dec->zone);
                        dec->coord32 = convertFractionsToCoord32(&dec->result);
                        for (j = i - 1; j >= from; j--) { // look in previous rects
                            if (!IS_RESTRICTED(j)) {
                                if (fitsInsideBoundaries(&dec->coord32, TERRITORY_BOUNDARY(j))) {
                                    nrZoneOverlaps = 1;
                                    break;
                                }
                            }
                        }

                        if (!nrZoneOverlaps) {
                            MapcodeZone zfound;
                            TerritoryBoundary prevu;
                            for (j = from; j < i; j++) { // try all smaller rectangles j
                                if (!IS_RESTRICTED(j)) {
                                    MapcodeZone z;
                                    if (restrictZoneTo(&z, &dec->zone, TERRITORY_BOUNDARY(j))) {
                                        nrZoneOverlaps++;
                                        if (nrZoneOverlaps == 1) {
                                            // first fit! remember...
                                            zoneCopyFrom(&zfound, &z);
                                            ASSERT(j <= MAPCODE_BOUNDARY_MAX);
                                            memcpy(&prevu, TERRITORY_BOUNDARY(j), sizeof(TerritoryBoundary));
                                        } else { // nrZoneOverlaps >= 2
                                            // more than one hit
                                            break; // give up
                                        }
                                    }
                                } // IS_RESTRICTED
                            } // for j

                            // if several sub-areas intersect, just return the whole zone
                            // (the center of which may NOT re-encode to the same mapcode!)
                            if (nrZoneOverlaps == 1) { // found exactly ONE intersection?
                                zoneCopyFrom(&dec->zone, &zfound);
                            }
                        }

                        if (!nrZoneOverlaps) {
                            err = ERR_MAPCODE_UNDECODABLE; // type 3 "NLD L222.222"
                        }
                    }  // *** make sure decode fits somewhere ***
                    break;
                }
            }
        } else if (r == 1) {
            if (codex == codexi + 10 && HEADER_LETTER(i) == *s) {
                err = decodeGrid(dec, i, 1);
                break;
            }
        } else { //r>1
            if (((codex == 23) && (codexi == 22)) ||
                ((codex == 33) && (codexi == 23))) {
                err = decodeAutoHeader(dec, i);
                break;
            }
        }
    } // for

    if (!err) {
        restrictZoneTo(&dec->zone, &dec->zone, TERRITORY_BOUNDARY(lastRec(ccode)));

        if (isEmpty(&dec->zone)) {
            err = ERR_MAPCODE_UNDECODABLE; // type 0 "BRA xx.xx"
        }
    }

    if (err) {
        dec->result.lat = dec->result.lon = 0;
        return err;
    }

    dec->result = getMidPointFractions(&dec->zone);
    dec->result = convertFractionsToDegrees(&dec->result);

    // normalise between =180 and 180
    if (dec->result.lat < -90.0) {
        dec->result.lat = -90.0;
    }
    if (dec->result.lat > 90.0) {
        dec->result.lat = 90.0;
    }
    if (dec->result.lon < -180.0) {
        dec->result.lon += 360.0;
    }
    if (dec->result.lon >= 180.0) {
        dec->result.lon -= 360.0;
    }

    if (wasAllDigits) {
        repackIfAllDigits(dec->mapcodeElements.properMapcode, 0);
    }
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////
//
//  Alphabet support
//
///////////////////////////////////////////////////////////////////////////////////////////////

// WARNING - these alphabets have NOT yet been released as standard! use at your own risk! check www.mapcode.com for details.
static const UWORD ASCII_TO_UTF16[_ALPHABET_MAX][36] = { // A-Z equivalents for ascii characters A to Z, 0-9
        //  A       B       C       D       E       F       G       H       I       J       K       L       M       N       O       P       Q       R       S       T       U       V       W       X       Y       Z       0       1       2       3       4       5       6       7       8       9
        {0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f, 0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005a, 0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039}, // roman
        {0x0391, 0x0392, 0x039e, 0x0394, 0x0388, 0x0395, 0x0393, 0x0397, 0x0399, 0x03a0, 0x039a, 0x039b, 0x039c, 0x039d, 0x039f, 0x03a1, 0x0398, 0x03a8, 0x03a3, 0x03a4, 0x0389, 0x03a6, 0x03a9, 0x03a7, 0x03a5, 0x0396, 0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039}, // greek
        {0x0410, 0x0412, 0x0421, 0x0414, 0x0415, 0x0416, 0x0413, 0x041d, 0x0049, 0x041f, 0x041a, 0x041b, 0x041c, 0x0417, 0x041e, 0x0420, 0x0424, 0x042f, 0x0426, 0x0422, 0x042d, 0x0427, 0x0428, 0x0425, 0x0423, 0x0411, 0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039}, // cyrillic
        {0x05d0, 0x05d1, 0x05d2, 0x05d3, 0x05e3, 0x05d4, 0x05d6, 0x05d7, 0x05d5, 0x05d8, 0x05d9, 0x05da, 0x05db, 0x05dc, 0x05e1, 0x05dd, 0x05de, 0x05e0, 0x05e2, 0x05e4, 0x05e5, 0x05e6, 0x05e7, 0x05e8, 0x05e9, 0x05ea, 0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039}, // hebrew
        {0x0905, 0x0915, 0x0917, 0x0918, 0x090f, 0x091a, 0x091c, 0x091f, 0x0049, 0x0920, 0x0923, 0x0924, 0x0926, 0x0927, 0x004f, 0x0928, 0x092a, 0x092d, 0x092e, 0x0930, 0x092b, 0x0932, 0x0935, 0x0938, 0x0939, 0x092c, 0x0966, 0x0967, 0x0968, 0x0969, 0x096a, 0x096b, 0x096c, 0x096d, 0x096e, 0x096f}, // Devanagari
        {0x0d12, 0x0d15, 0x0d16, 0x0d17, 0x0d0b, 0x0d1a, 0x0d1c, 0x0d1f, 0x0049, 0x0d21, 0x0d24, 0x0d25, 0x0d26, 0x0d27, 0x0d20, 0x0d28, 0x0d2e, 0x0d30, 0x0d31, 0x0d32, 0x0d09, 0x0d34, 0x0d35, 0x0d36, 0x0d38, 0x0d39, 0x0d66, 0x0d67, 0x0d68, 0x0d69, 0x0d6a, 0x0d6b, 0x0d6c, 0x0d6d, 0x0d6e, 0x0d6f}, // Malayalam
        {0x10a0, 0x10a1, 0x10a3, 0x10a6, 0x10a4, 0x10a9, 0x10ab, 0x10ac, 0x0049, 0x10ae, 0x10b0, 0x10b1, 0x10b2, 0x10b4, 0x10ad, 0x10b5, 0x10b6, 0x10b7, 0x10b8, 0x10b9, 0x10a8, 0x10ba, 0x10bb, 0x10bd, 0x10be, 0x10bf, 0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039}, // Georgian
        {0x30a2, 0x30ab, 0x30ad, 0x30af, 0x30aa, 0x30b1, 0x30b3, 0x30b5, 0x0049, 0x30b9, 0x30c1, 0x30c8, 0x30ca, 0x30cc, 0x004f, 0x30d2, 0x30d5, 0x30d8, 0x30db, 0x30e1, 0x30a8, 0x30e2, 0x30e8, 0x30e9, 0x30ed, 0x30f2, 0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039}, // Katakana
        {0x0e30, 0x0e01, 0x0e02, 0x0e04, 0x0e32, 0x0e07, 0x0e08, 0x0e09, 0x0049, 0x0e0a, 0x0e11, 0x0e14, 0x0e16, 0x0e17, 0x004f, 0x0e18, 0x0e1a, 0x0e1c, 0x0e21, 0x0e23, 0x0e2c, 0x0e25, 0x0e27, 0x0e2d, 0x0e2e, 0x0e2f, 0x0e50, 0x0e51, 0x0e52, 0x0e53, 0x0e54, 0x0e55, 0x0e56, 0x0e57, 0x0e58, 0x0e59}, // Thai
        {0x0eb0, 0x0e81, 0x0e82, 0x0e84, 0x0ec3, 0x0e87, 0x0e88, 0x0e8a, 0x0ec4, 0x0e8d, 0x0e94, 0x0e97, 0x0e99, 0x0e9a, 0x004f, 0x0e9c, 0x0e9e, 0x0ea1, 0x0ea2, 0x0ea3, 0x0ebd, 0x0ea7, 0x0eaa, 0x0eab, 0x0ead, 0x0eaf, 0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039}, // Laos
        {0x0556, 0x0532, 0x0533, 0x0534, 0x0535, 0x0538, 0x0539, 0x053a, 0x053b, 0x053d, 0x053f, 0x0540, 0x0541, 0x0543, 0x0555, 0x0547, 0x0548, 0x054a, 0x054d, 0x054e, 0x0545, 0x054f, 0x0550, 0x0551, 0x0552, 0x0553, 0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039}, // armenian
        {0x099c, 0x0998, 0x0995, 0x0996, 0x09ae, 0x0997, 0x0999, 0x099a, 0x0049, 0x099d, 0x09a0, 0x09a1, 0x09a2, 0x09a3, 0x004f, 0x09a4, 0x09a5, 0x09a6, 0x09a8, 0x09aa, 0x099f, 0x09ac, 0x09ad, 0x09af, 0x09b2, 0x09b9, 0x09e6, 0x09e7, 0x09e8, 0x09e9, 0x09ea, 0x09eb, 0x09ec, 0x09ed, 0x09ee, 0x09ef}, // Bengali/Assamese
        {0x0a05, 0x0a15, 0x0a17, 0x0a18, 0x0a0f, 0x0a1a, 0x0a1c, 0x0a1f, 0x0049, 0x0a20, 0x0a23, 0x0a24, 0x0a26, 0x0a27, 0x004f, 0x0a28, 0x0a2a, 0x0a2d, 0x0a2e, 0x0a30, 0x0a2b, 0x0a32, 0x0a35, 0x0a38, 0x0a39, 0x0a21, 0x0a66, 0x0a67, 0x0a68, 0x0a69, 0x0a6a, 0x0a6b, 0x0a6c, 0x0a6d, 0x0a6e, 0x0a6f}, // Gurmukhi
        {0x0f58, 0x0f40, 0x0f41, 0x0f42, 0x0f64, 0x0f44, 0x0f45, 0x0f46, 0x0049, 0x0f47, 0x0f49, 0x0f55, 0x0f50, 0x0f4f, 0x004f, 0x0f51, 0x0f53, 0x0f54, 0x0f56, 0x0f5e, 0x0f60, 0x0f5f, 0x0f61, 0x0f62, 0x0f63, 0x0f66, 0x0f20, 0x0f21, 0x0f22, 0x0f23, 0x0f24, 0x0f25, 0x0f26, 0x0f27, 0x0f28, 0x0f29}, // Tibetan
        {0x0628, 0x062a, 0x062d, 0x062e, 0x062B, 0x062f, 0x0630, 0x0631, 0x0627, 0x0632, 0x0633, 0x0634, 0x0635, 0x0636, 0x0647, 0x0637, 0x0638, 0x0639, 0x063a, 0x0641, 0x0642, 0x062C, 0x0644, 0x0645, 0x0646, 0x0648, 0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039}, // Arabic
        {0x1112, 0x1100, 0x1102, 0x1103, 0x1166, 0x1105, 0x1107, 0x1109, 0x1175, 0x1110, 0x1111, 0x1161, 0x1162, 0x1163, 0x110b, 0x1164, 0x1165, 0x1167, 0x1169, 0x1172, 0x1174, 0x110c, 0x110e, 0x110f, 0x116d, 0x116e, 0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039}, // Korean // 0xc601, 0xc77c, 0xc774, 0xc0bc, 0xc0ac, 0xc624, 0xc721, 0xce60, 0xd314, 0xad6c (vocal digits)
        {0x1005, 0x1000, 0x1001, 0x1002, 0x1013, 0x1003, 0x1004, 0x101a, 0x0049, 0x1007, 0x100c, 0x100d, 0x100e, 0x1010, 0x101d, 0x1011, 0x1012, 0x101e, 0x1014, 0x1015, 0x1016, 0x101f, 0x1017, 0x1018, 0x100f, 0x101c, 0x1040, 0x1041, 0x1042, 0x1043, 0x1044, 0x1045, 0x1046, 0x1047, 0x1048, 0x1049}, // Burmese
        {0x1789, 0x1780, 0x1781, 0x1782, 0x1785, 0x1783, 0x1784, 0x1787, 0x179a, 0x1788, 0x178a, 0x178c, 0x178d, 0x178e, 0x004f, 0x1791, 0x1792, 0x1793, 0x1794, 0x1795, 0x179f, 0x1796, 0x1798, 0x179b, 0x17a0, 0x17a2, 0x17e0, 0x17e1, 0x17e2, 0x17e3, 0x17e4, 0x17e5, 0x17e6, 0x17e7, 0x17e8, 0x17e9}, // Khmer
        {0x0d85, 0x0d9a, 0x0d9c, 0x0d9f, 0x0d89, 0x0da2, 0x0da7, 0x0da9, 0x0049, 0x0dac, 0x0dad, 0x0daf, 0x0db1, 0x0db3, 0x004f, 0x0db4, 0x0db6, 0x0db8, 0x0db9, 0x0dba, 0x0d8b, 0x0dbb, 0x0dbd, 0x0dc0, 0x0dc3, 0x0dc4, 0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039}, // Sinhalese
        {0x0794, 0x0780, 0x0781, 0x0782, 0x0797, 0x0783, 0x0784, 0x0785, 0x0049, 0x0786, 0x0787, 0x0788, 0x0789, 0x078a, 0x004f, 0x078b, 0x078c, 0x078d, 0x078e, 0x078f, 0x079c, 0x0790, 0x0791, 0x0792, 0x0793, 0x07b1, 0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039}, // Thaana
        {0x3123, 0x3105, 0x3108, 0x3106, 0x3114, 0x3107, 0x3109, 0x310a, 0x0049, 0x310b, 0x310c, 0x310d, 0x310e, 0x310f, 0x004f, 0x3115, 0x3116, 0x3110, 0x3111, 0x3112, 0x3113, 0x3129, 0x3117, 0x3128, 0x3118, 0x3119, 0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039}, // Chinese
        {0x2D49, 0x2D31, 0x2D33, 0x2D37, 0x2D53, 0x2D3C, 0x2D3D, 0x2D40, 0x2D4F, 0x2D43, 0x2D44, 0x2D45, 0x2D47, 0x2D4D, 0x2D54, 0x2D4E, 0x2D55, 0x2D56, 0x2D59, 0x2D5A, 0x2D62, 0x2D5B, 0x2D5C, 0x2D5F, 0x2D61, 0x2D63, 0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039}, // Tifinagh (BERBER)
        {0x0b99, 0x0b95, 0x0b9a, 0x0b9f, 0x0b86, 0x0ba4, 0x0ba8, 0x0baa, 0x0049, 0x0bae, 0x0baf, 0x0bb0, 0x0bb2, 0x0bb5, 0x004f, 0x0bb4, 0x0bb3, 0x0bb1, 0x0b85, 0x0b88, 0x0b93, 0x0b89, 0x0b8e, 0x0b8f, 0x0b90, 0x0b92, 0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039}, // Tamil (digits 0xBE6-0xBEF)
        {0x121B, 0x1260, 0x1264, 0x12F0, 0x121E, 0x134A, 0x1308, 0x1200, 0x0049, 0x12E8, 0x12AC, 0x1208, 0x1293, 0x1350, 0x12D0, 0x1354, 0x1240, 0x1244, 0x122C, 0x1220, 0x12C8, 0x1226, 0x1270, 0x1276, 0x1338, 0x12DC, 0x1372, 0x1369, 0x136a, 0x136b, 0x136c, 0x136d, 0x136e, 0x136f, 0x1370, 0x1371}, // Amharic (digits 1372|1369-1371)
        {0x0C1E, 0x0C15, 0x0C17, 0x0C19, 0x0C2B, 0x0C1A, 0x0C1C, 0x0C1F, 0x0049, 0x0C20, 0x0C21, 0x0C23, 0x0C24, 0x0C25, 0x004f, 0x0C26, 0x0C27, 0x0C28, 0x0C2A, 0x0C2C, 0x0C2D, 0x0C2E, 0x0C30, 0x0C32, 0x0C33, 0x0C35, 0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039}, // Telugu
        {0x0B1D, 0x0B15, 0x0B16, 0x0B17, 0x0B23, 0x0B18, 0x0B1A, 0x0B1C, 0x0049, 0x0B1F, 0x0B21, 0x0B22, 0x0B24, 0x0B25, 0x0B20, 0x0B26, 0x0B27, 0x0B28, 0x0B2A, 0x0B2C, 0x0B39, 0x0B2E, 0x0B2F, 0x0B30, 0x0B33, 0x0B38, 0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039}, // Odia
        {0x0C92, 0x0C95, 0x0C96, 0x0C97, 0x0C8E, 0x0C99, 0x0C9A, 0x0C9B, 0x0049, 0x0C9C, 0x0CA0, 0x0CA1, 0x0CA3, 0x0CA4, 0x004f, 0x0CA6, 0x0CA7, 0x0CA8, 0x0CAA, 0x0CAB, 0x0C87, 0x0CAC, 0x0CAD, 0x0CB0, 0x0CB2, 0x0CB5, 0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039}, // Kannada
        {0x0AB3, 0x0A97, 0x0A9C, 0x0AA1, 0x0A87, 0x0AA6, 0x0AAC, 0x0A95, 0x0049, 0x0A9A, 0x0A9F, 0x0AA4, 0x0AAA, 0x0AA0, 0x004f, 0x0AB0, 0x0AB5, 0x0A9E, 0x0AAE, 0x0AAB, 0x0A89, 0x0AB7, 0x0AA8, 0x0A9D, 0x0AA2, 0x0AAD, 0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039}, // Gujarati
};

///////////////////////////////////////////////////////////////////////////////////////////////
//
//  ABJAD ROUTINES
//
///////////////////////////////////////////////////////////////////////////////////////////////



/// PRIVATE convert a mapcode to an ABJAD-format (never more than 2 non-digits in a row)
static char *convertToAbjad(char *targetAsciiString, const char *sourceAsciiString, int maxLength) {
    int form, i, dot, inarow, len;
    const char *rest;
    ASSERT(targetAsciiString);
    ASSERT(sourceAsciiString);
    len = (int) strlen(sourceAsciiString);
    rest = strchr(sourceAsciiString, '-');
    if (rest != NULL) {
        len = ((int) (rest - sourceAsciiString));
    }
    if (len >= maxLength) {
        len = maxLength - 1;
    }
    while (len > 0 && sourceAsciiString[len - 1] == ' ') {
        len--;
    }

    lengthCopy(targetAsciiString, sourceAsciiString, len, maxLength);
    unpackIfAllDigits(targetAsciiString);

    len = (int) strlen(targetAsciiString);
    dot = (int) (strchr(targetAsciiString, '.') - targetAsciiString);

    form = dot * 10 + (len - dot - 1);

    // see if >2 non-digits in a row
    inarow = 0;
    for (i = 0; i < len; i++) {
        char c = targetAsciiString[i];
        if (c != 46) {
            inarow++;
            if (decodeChar(c) <= 9) {
                inarow = 0;
            } else if (inarow > 2) {
                break;
            }
        }
    }
    if (dot < 2 || dot > 5 || (inarow < 3 &&
                               (form == 22 || form == 32 || form == 33 || form == 42 || form == 43 || form == 44 ||
                                form == 54))) {
        // no need to do anything, return input unchanged
        return safeCopy(targetAsciiString, sourceAsciiString, maxLength);
    } else if (form >= 22 && form <= 54) {
        char c1, c2, c3 = '?';
        int c = decodeChar(targetAsciiString[2]);
        if (c < 0) {
            c = decodeChar(targetAsciiString[3]);
        }

        if (form >= 44) {
            c = (c * 31) + (decodeChar(targetAsciiString[len - 1]) + 39);
            c1 = ENCODE_CHARS[c / 100];
            c2 = ENCODE_CHARS[(c % 100) / 10];
            c3 = ENCODE_CHARS[c % 10];
        } else if (len == 7) {
            if (form == 24) {
                c += 7;
            } else if (form == 33) {
                c += 38;
            } else if (form == 42) {
                c += 69;
            }
            c1 = ENCODE_CHARS[c / 10];
            c2 = ENCODE_CHARS[c % 10];
        } else {
            c1 = ENCODE_CHARS[2 + (c / 8)];
            c2 = ENCODE_CHARS[2 + (c % 8)];
        }

        if (form == 22) // s0 s1 . s3 s4 -> s0 s1 . C1 C2 s4
        {
            targetAsciiString[6] = 0;
            targetAsciiString[5] = targetAsciiString[4];
            targetAsciiString[4] = c2;
            targetAsciiString[3] = c1;
//          targetAsciiString[2] = '.';
//          targetAsciiString[1] = targetAsciiString[1];
//          targetAsciiString[0] = targetAsciiString[0];
        } else if (form == 23) { // s0 s1 . s3 s4 s5 -> s0 s1 . C1 C2 s4 s5
            targetAsciiString[7] = 0;
            targetAsciiString[6] = targetAsciiString[5];
            targetAsciiString[5] = targetAsciiString[4];
            targetAsciiString[4] = c2;
            targetAsciiString[3] = c1;
//          targetAsciiString[2] = '.';
//          targetAsciiString[1] = targetAsciiString[1];
//          targetAsciiString[0] = targetAsciiString[0];
        } else if (form == 32) { // s0 s1 s2 . s4 s5 -> s0 s1 . C* C2 s4 s5
            targetAsciiString[7] = 0;
            targetAsciiString[6] = targetAsciiString[5];
            targetAsciiString[5] = targetAsciiString[4];
            targetAsciiString[4] = c2;
            targetAsciiString[3] = (char) (c1 + 4);
            targetAsciiString[2] = '.';
//          targetAsciiString[1] = targetAsciiString[1];
//          targetAsciiString[0] = targetAsciiString[0];
        } else if (form == 24 || form == 33 || form == 42) {
            // s0 s1 . s3 s4 s5 s6 -> s0 s1 C1 . s4 C2 s5 s6
            // s0 s1 s2 . s4 s5 s6 -> s0 s1 C1 . s4 C2 s5 s6
            // s0 s1 s2 s3 . s5 s6 -> s0 s1 C1 . s3 C2 s5 s6
            targetAsciiString[8] = 0;
            targetAsciiString[7] = targetAsciiString[6];
            targetAsciiString[6] = targetAsciiString[5];
            targetAsciiString[5] = c2;
            targetAsciiString[4] = targetAsciiString[(form == 42 ? 3 : 4)];
            targetAsciiString[3] = '.';
            targetAsciiString[2] = c1;
//          targetAsciiString[1] = targetAsciiString[1];
//          targetAsciiString[0] = targetAsciiString[0];
        } else if (form == 34) {  // s0 s1 s2 . s4 s5 s6 s7 -> s0 s1 C1 . s4 s5 C2 S6 S7
            targetAsciiString[9] = 0;
            targetAsciiString[8] = targetAsciiString[7];
            targetAsciiString[7] = targetAsciiString[6];
            targetAsciiString[6] = c2;
//          targetAsciiString[5] = targetAsciiString[5];
//          targetAsciiString[4] = targetAsciiString[4];
//          targetAsciiString[3] = '.';
            targetAsciiString[2] = c1;
//          targetAsciiString[1] = targetAsciiString[1];
//          targetAsciiString[0] = targetAsciiString[0];
        } else if (form == 43) { // s0 s1 s2 s3 . s5 s6 s7 -> s0 s1 C* . s3 s5 C2 S6 S7
            targetAsciiString[9] = 0;
            targetAsciiString[8] = targetAsciiString[7];
            targetAsciiString[7] = targetAsciiString[6];
            targetAsciiString[6] = c2;
//          targetAsciiString[5] = targetAsciiString[5];
            targetAsciiString[4] = targetAsciiString[3];
            targetAsciiString[3] = '.';
            targetAsciiString[2] = (char) (c1 + 4);
//          targetAsciiString[1] = targetAsciiString[1];
//          targetAsciiString[0] = targetAsciiString[0];
        } else if (form == 44) {
            targetAsciiString[10] = 0;
            targetAsciiString[9] = targetAsciiString[7];
            targetAsciiString[8] = c3;
            targetAsciiString[7] = targetAsciiString[6];
            targetAsciiString[6] = targetAsciiString[5];
            targetAsciiString[5] = c2;
//          targetAsciiString[4] = '.';
//          targetAsciiString[3] = targetAsciiString[3];
            targetAsciiString[2] = c1;
//          targetAsciiString[1] = targetAsciiString[1];
//          targetAsciiString[0] = targetAsciiString[0];
        } else if (form == 54) {
            targetAsciiString[11] = 0;
            targetAsciiString[10] = targetAsciiString[8];
            targetAsciiString[9] = c3;
            targetAsciiString[8] = targetAsciiString[7];
            targetAsciiString[7] = targetAsciiString[6];
            targetAsciiString[6] = c2;
//          targetAsciiString[5] = '.';
//          targetAsciiString[4] = targetAsciiString[4];
//          targetAsciiString[3] = targetAsciiString[3];
            targetAsciiString[2] = c1;
//          targetAsciiString[1] = targetAsciiString[1];
//          targetAsciiString[0] = targetAsciiString[0];        
        }
    }
    repackIfAllDigits(targetAsciiString, 0);
    if (rest) {
        int totalLen = (int) strlen(targetAsciiString);
        int needed = (int) strlen(rest);
        int tocopy = maxLength - totalLen - 1;
        if (tocopy > needed) {
            tocopy = needed;
        }
        if (tocopy > 0) {
            memcpy(targetAsciiString + totalLen, rest, (size_t) tocopy);
            targetAsciiString[totalLen + tocopy] = 0;
        }
    }
    return targetAsciiString;
}


static UWORD *encodeUtf16(UWORD *utf16String, const int maxLength, const char *asciiString,
                          const enum Alphabet alphabet) // convert mapcode string alphabet
{
    UWORD *w = utf16String;
    const UWORD *e = w + maxLength - 1;
    const char *r = asciiString;
    ASSERT(utf16String);
    ASSERT(asciiString);
    while (*r != 0 && w < e) {
        char c = *r++;
        if ((c >= 'a') && (c <= 'z')) {
            c += ('A' - 'a');
        }
        if ((c < ' ') || (c > 'Z')) { // not in any valid range?
            *w++ = (UWORD) c; // leave untranslated
        } else if ((c >= '0') && (c <= '9')) { // digit?
            *w++ = ASCII_TO_UTF16[alphabet][26 + (int) c - '0'];
        } else if (c < 'A') { // valid but not a letter (e.g. a dot, a space...)
            *w++ = (UWORD) c; // leave untranslated
        } else {
            *w++ = ASCII_TO_UTF16[alphabet][c - 'A'];
        }
    }
    *w = 0;
    return utf16String;
}


// PUBLIC - convert as much as will fit of mapcode into utf16String
UWORD *convertToAlphabet(UWORD *utf16String, int maxLength, const char *asciiString,
                         enum Alphabet alphabet) // 0=roman, 2=cyrillic
{
    UWORD *startbuf = utf16String;
    UWORD *lastspot = &utf16String[maxLength - 1];
    ASSERT(utf16String);
    ASSERT(asciiString);
    if (maxLength > 0) {
        char targetAsciiString[MAX_MAPCODE_RESULT_ASCII_LEN] = "";
        char abjadString[MAX_MAPCODE_RESULT_ASCII_LEN] = "";

        // skip leading spaces
        while (*asciiString > 0 && *asciiString <= 32) {
            asciiString++;
        }

        // straight-copy everything up to and including first space
        {
            const char *e = strchr(asciiString, ' ');
            if (e) {
                while (asciiString <= e) {
                    if (utf16String == lastspot) { // buffer fully filled?
                        // zero-terminate and return
                        *utf16String = 0;
                        return startbuf;
                    }
                    *utf16String++ = (UWORD) *asciiString++;
                }
                while (*asciiString == ' ') {
                    asciiString++;
                }
            }
        }

        if (alphabet == ALPHABET_GREEK || alphabet == ALPHABET_HEBREW ||
            alphabet == ALPHABET_ARABIC || alphabet == ALPHABET_KOREAN) {
            asciiString = convertToAbjad(abjadString, asciiString, MAX_MAPCODE_RESULT_ASCII_LEN);
        }

        // re-pack E/U-voweled mapcodes when necessary:
        if (alphabet == ALPHABET_GREEK) { // alphabet has fewer characters than Roman!
            if (strchr(asciiString, 'E') || strchr(asciiString, 'U') ||
                strchr(asciiString, 'e') || strchr(asciiString, 'u')) {
                // copy trimmed mapcode into temporary buffer targetAsciiString
                int len = (int) strlen(asciiString);
                if (len < MAX_MAPCODE_RESULT_ASCII_LEN) {
                    while (len > 0 && asciiString[len - 1] > 0 && asciiString[len - 1] <= 32) {
                        len--;
                    }
                    lengthCopy(targetAsciiString, asciiString, len, maxLength);
                    // re-pack into A-voweled mapcode
                    unpackIfAllDigits(targetAsciiString);
                    repackIfAllDigits(targetAsciiString, 1);
                    asciiString = targetAsciiString;
                }
            }
        }

        encodeUtf16(utf16String, 1 + (int) (lastspot - utf16String), asciiString, alphabet);
    }
    return startbuf;
}


/**
 * Convert a zero-terminated UTF16 to a UTF8 string
 */
char *convertUtf16ToUtf8(char *utf8, const UWORD *utf16) {
    ASSERT(utf16);
    ASSERT(utf8);
    while (*utf16) {
        UWORD c = *utf16++;
        if (c < 0x80) {
            *utf8++ = (char) c;
        } else if (c < 0x800) {
            *utf8++ = (char) (0xC0 + (c >> 6));
            *utf8++ = (char) (0x80 + (c & 63));
        } else {
            *utf8++ = (char) (0xE0 + (c >> 12));
            *utf8++ = (char) (0x80 + ((c >> 6) & 63));
            *utf8++ = (char) (0x80 + (c & 63));
        }
    }
    *utf8 = 0;
    return utf8;
}

// Caller must make sure utf8String can hold at least MAX_MAPCODE_RESULT_LEN characters (including 0-terminator).
UWORD *convertMapcodeToAlphabetUtf16(UWORD *utf16String, const char *mapcodeString, enum Alphabet alphabet) {
    ASSERT(utf16String);
    ASSERT(mapcodeString);
    ASSERT(alphabet > _ALPHABET_MIN && alphabet < _ALPHABET_MAX);
    *utf16String = 0;
    if (strlen(mapcodeString) < MAX_MAPCODE_RESULT_ASCII_LEN) {
        convertToAlphabet(utf16String, MAX_MAPCODE_RESULT_UTF16_LEN, mapcodeString, alphabet);
    }
    return utf16String;
}


char *convertMapcodeToAlphabetUtf8(char *utf8String, const char *mapcodeString, enum Alphabet alphabet) {
    UWORD utf16[MAX_MAPCODE_RESULT_UTF16_LEN + 1];
    return convertUtf16ToUtf8(utf8String, convertMapcodeToAlphabetUtf16(utf16, mapcodeString, alphabet));
}


///////////////////////////////////////////////////////////////////////////////////////////////
//
//  PUBLIC INTERFACE
//
///////////////////////////////////////////////////////////////////////////////////////////////

// PUBLIC - returns name of territory in (sufficiently large!) result string. 
// useShortNames: 0=full 1=short
// returns empty string in case of error
char *getTerritoryIsoName(char *territoryISO, enum Territory territory, int useShortName) {
    ASSERT(territoryISO);
    ASSERT(useShortName == 0 || useShortName == 1);
    if (territory <= _TERRITORY_MIN || territory >= _TERRITORY_MAX) {
        *territoryISO = 0;
    } else {
        const char *alphaCode = ISO3166_ALPHA[INDEX_OF_TERRITORY(territory)];
        const char *hyphen = strchr(alphaCode, '-');
        if (useShortName && hyphen != NULL) {
            strcpy(territoryISO, hyphen + 1);
        } else {
            strcpy(territoryISO, alphaCode);
        }
    }
    return territoryISO;
}


// PUBLIC - returns negative if territory is not a code that has a parent country
enum Territory getParentCountryOf(enum Territory territory) {
    return parentTerritoryOf(territory);
}


// PUBLIC - returns territory if it is a country, or parent country if territory is a state.
// returns megative if territory is invalid.
enum Territory getCountryOrParentCountry(enum Territory territory) {
    const enum Territory tp = getParentCountryOf(territory);
    if (tp != TERRITORY_NONE) {
        return tp;
    }
    return territory;
}


// PUBLIC - returns nonzero if coordinate is near more than one territory border
int multipleBordersNearby(double latDeg, double lonDeg, enum Territory territory) {
    const enum Territory ccode = territory;
    if ((ccode > _TERRITORY_MIN) && (ccode != TERRITORY_AAA)) { // valid territory, not earth
        const enum Territory parentTerritoryCode = getParentCountryOf(territory);
        if (parentTerritoryCode != TERRITORY_NONE) {
            // there is a parent! check its borders as well...
            if (multipleBordersNearby(latDeg, lonDeg, parentTerritoryCode)) {
                return 1;
            }
        }
        {
            int m;
            int nrFound = 0;
            const int from = firstRec(ccode);
            const int upto = lastRec(ccode);
            Point32 coord32;
            convertCoordsToMicrosAndFractions(&coord32, NULL, NULL, latDeg, lonDeg);
            for (m = upto; m >= from; m--) {
                if (!IS_RESTRICTED(m)) {
                    if (isNearBorderOf(&coord32, TERRITORY_BOUNDARY(m))) {
                        nrFound++;
                        if (nrFound > 1) {
                            return 1;
                        }
                    }
                }
            }
        }
    }
    return 0;
}


static int compareAlphaCode(const void *e1, const void *e2) {
    const AlphaRec *a1 = (const AlphaRec *) e1;
    const AlphaRec *a2 = (const AlphaRec *) e2;
    ASSERT(e1);
    ASSERT(e2);
    return strcmp(a1->alphaCode, a2->alphaCode);
} // cmp

static enum Territory findMatch(const int parentNumber, const char *territoryISO) {
    // build an uppercase search term
    char codeISO[MAX_ISOCODE_ASCII_LEN + 1];
    const char *r = territoryISO;
    int len = 0;
    ASSERT(territoryISO);

    if (parentNumber < 0) {
        return TERRITORY_NONE;
    }
    if (parentNumber > 0) {
        codeISO[0] = PARENTS_2[3 * parentNumber - 3];
        codeISO[1] = PARENTS_2[3 * parentNumber - 2];
        codeISO[2] = '-';
        len = 3;
    }
    while ((len < MAX_ISOCODE_ASCII_LEN) && (*r > 32)) {
        codeISO[len++] = *r++;
    }
    if (*r > 32) {
        return TERRITORY_NONE;
    }
    codeISO[len] = 0;
    makeUppercase(codeISO);
    { // binary-search the result
        const AlphaRec *p;
        AlphaRec t;
        t.alphaCode = codeISO;

        p = (const AlphaRec *) bsearch(&t, ALPHA_SEARCH, NR_TERRITORY_RECS, sizeof(AlphaRec), compareAlphaCode);
        if (p) {
            if (strcmp(t.alphaCode, p->alphaCode) == 0) { // only interested in PERFECT match
                return p->territory;
            } // match
        } // found
    } //
    return TERRITORY_NONE;
}


// PUBLIC - returns territory of territoryISO (or negative if not found).
// optionalTerritoryContext: pass to handle ambiguities (pass TERRITORY_NONE if unknown).
enum Territory getTerritoryCode(const char *territoryISO, enum Territory optionalTerritoryContext) {
    if (territoryISO == NULL) {
        return TERRITORY_NONE;
    }
    ASSERT(territoryISO);
    while (*territoryISO > 0 && *territoryISO <= 32) {
        territoryISO++;
    } // skip leading whitespace

    if (territoryISO[0] && territoryISO[1]) {
        if (territoryISO[2] == '-') {
            return findMatch(getParentNumber(territoryISO, 2), territoryISO + 3);
        } else if (territoryISO[2] && territoryISO[3] == '-') {
            return findMatch(getParentNumber(territoryISO, 3), territoryISO + 4);
        } else {
            enum Territory b;
            int parentNumber = 0;
            if (optionalTerritoryContext > _TERRITORY_MIN) {
                parentNumber = PARENT_NUMBER[INDEX_OF_TERRITORY(getCountryOrParentCountry(optionalTerritoryContext))];
            }
            b = findMatch(parentNumber, territoryISO);
            if (b != TERRITORY_NONE) {
                return b;
            }
        }
        return findMatch(0, territoryISO);
    } // else, fail:
    return TERRITORY_NONE;
}


// PUBLIC - decode string into lat,lon; returns negative in case of error
enum MapcodeError decodeMapcodeToLatLonUtf8(double *latDeg, double *lonDeg,
                                            const char *mapcode, enum Territory territory,
                                            MapcodeElements *mapcodeElements) {
    if ((latDeg == NULL) || (lonDeg == NULL) || (mapcode == NULL)) {
        return ERR_BAD_ARGUMENTS;
    } else {
        enum MapcodeError ret;
        DecodeRec dec = {
                {"", TERRITORY_NONE, "", 0, ""},
                0,
                0,
                0,
                TERRITORY_NONE,
                0,
                {0.0, 0.0},
                {0, 0},
                {0.0, 0.0, 0.0, 0.0}
        };
        dec.orginput = mapcode;
        dec.context = territory;

        ret = decoderEngine(&dec, 0);
        *latDeg = dec.result.lat;
        *lonDeg = dec.result.lon;

        if (mapcodeElements) {
            memcpy(mapcodeElements, &dec.mapcodeElements, sizeof(MapcodeElements));
        }
        return ret;
    }
}


// PUBLIC - decode string into lat,lon; returns negative in case of error
enum MapcodeError decodeMapcodeToLatLonUtf16(double *latDeg, double *lonDeg,
                                             const UWORD *mapcode, enum Territory territory,
                                             MapcodeElements *mapcodeElements) {
    if ((latDeg == NULL) || (lonDeg == NULL) || (mapcode == NULL)) {
        return ERR_BAD_ARGUMENTS;
    } else {
        enum MapcodeError ret;
        DecodeRec dec = {
                {"", TERRITORY_NONE, "", 0, ""},
                0,
                0,
                0,
                TERRITORY_NONE,
                0,
                {0.0, 0.0},
                {0, 0},
                {0.0, 0.0, 0.0, 0.0}
        };
        dec.orginput = (const char *) mapcode;
        dec.context = territory;

        ret = decoderEngine(&dec, FLAG_UTF16_STRING);
        *latDeg = dec.result.lat;
        *lonDeg = dec.result.lon;

        if (mapcodeElements) {
            memcpy(mapcodeElements, &dec.mapcodeElements, sizeof(MapcodeElements));
        }
        return ret;
    }
}


// PUBLIC - encode lat,lon for territory to a mapcode with extraDigits accuracy
int
encodeLatLonToSingleMapcode(char *mapcode, double latDeg, double lonDeg, enum Territory territory, int extraDigits) {
    Mapcodes rlocal;
    int ret;
    ASSERT(mapcode);
    if (extraDigits < 0) {
        return 0;
    }
    if (extraDigits > MAX_PRECISION_DIGITS) {
        extraDigits = MAX_PRECISION_DIGITS;
    }
    if (territory <= TERRITORY_UNKNOWN) {
        return 0;
    }
    ret = encodeLatLonToMapcodes_internal(&rlocal, latDeg, lonDeg, territory, 1, DEBUG_STOP_AT, extraDigits);
    *mapcode = 0;
    if (ret <= 0) { // no solutions?
        return ret;
    }
    // prefix territory unless international
    strcpy(mapcode, rlocal.mapcode[0]);
    return 1;
}


// PUBLIC - encode lat,lon for (optional) territory to mapcodes with extraDigits accuracy
int
encodeLatLonToMapcodes(Mapcodes *mapcodes, double latDeg, double lonDeg, enum Territory territory, int extraDigits) {
    ASSERT(mapcodes);
    if (extraDigits < 0) {
        return 0;
    }
    if (extraDigits > MAX_PRECISION_DIGITS) {
        extraDigits = MAX_PRECISION_DIGITS;
    }
    return encodeLatLonToMapcodes_internal(mapcodes, latDeg, lonDeg, territory, 0, DEBUG_STOP_AT, extraDigits);
}

///////////////////////////////////////////////////////////////////////////////////////////////
//
//  ALPHABET / UTF ROUTINES
//
///////////////////////////////////////////////////////////////////////////////////////////////

// PUBLIC - returns most common alphabets for territory, NULL if error
const TerritoryAlphabets *getAlphabetsForTerritory(enum Territory territory) {
    if (territory > _TERRITORY_MIN && territory < _TERRITORY_MAX) {
        return &ALPHABETS_FOR_TERRITORY[INDEX_OF_TERRITORY(territory)];
    }
    return NULL;
}


///////////////////////////////////////////////////////////////////////////////////////////////
//
//  FULL TERRITORY NAMES
//
///////////////////////////////////////////////////////////////////////////////////////////////

static int
getFullTerritoryName_internal(char *territoryName, enum Territory territory, int alternative, const char *locale,
                              enum Alphabet alphabet) {

    const char *territoryNamesPiped;
    const char *pipePtr;
    const char **territoryNamesList = NULL;

    ASSERT(territoryName);
    ASSERT((_TERRITORY_MIN < territory) && (territory < _TERRITORY_MAX));
    ASSERT((alphabet == _ALPHABET_MIN) || ((_ALPHABET_MIN < alphabet) && (alphabet < _ALPHABET_MAX)));

    // Defensive bail out if incorrect arguments.
    if (!territoryName || (alternative < 0) || (territory <= _TERRITORY_MIN) || (territory >= _TERRITORY_MAX)) {
        if (territoryName) {
            *territoryName = 0;
        }
        return 0;
    }

    // Check locale.
    if (locale == NULL) {

        // Use local names if locale is null.
        territoryNamesList = TERRITORY_FULL_NAME_LOCAL;
    } else {

        // Try and get correct list.
        int i;
        int upTo = (int) strlen(locale);
        char localeUpper[4] = "";           // Default locale is empty (which implies 'fallback').
        char *sep = strchr(locale, '_');    // Official separator is '_' (as in "en_US").
        if (!sep) {
            sep = strchr(locale, '-');      // But we also allow '-' (often used as well).
        }
        if (sep) {
            upTo = (int) (sep - locale);
        }
        lengthCopy(localeUpper, locale, upTo, sizeof(localeUpper));
        makeUppercase(localeUpper);

        territoryNamesList = NULL;
        for (i = 0; i < (int) (sizeof(LOCALE_REGISTRY) / sizeof(LOCALE_REGISTRY[0])); ++i) {
            if (!strcmp(LOCALE_REGISTRY[i].locale, localeUpper)) {
                territoryNamesList = LOCALE_REGISTRY[i].territoryFullNames;
                break;
            }
        }
    }

    // Use English if locale is invalid (or was empty = fallback).
    if (territoryNamesList == NULL || territoryNamesList[0] == NULL) {
        territoryNamesList = DEFAULT_TERRITORY_FULL_NAME;
    }

    *territoryName = 0;
    territoryNamesPiped = territoryNamesList[INDEX_OF_TERRITORY(territory)];
    for (;;) {
        pipePtr = strchr(territoryNamesPiped, '|');

        if ((_ALPHABET_MIN < alphabet) && (alphabet < _ALPHABET_MAX)) {

            // Alphabet was specified.
            if (pipePtr) {
                ASSERT((pipePtr - territoryNamesPiped) <= MAX_TERRITORY_FULLNAME_UTF8_LEN);
                lengthCopy(territoryName, territoryNamesPiped, (int) (pipePtr - territoryNamesPiped),
                           MAX_TERRITORY_FULLNAME_UTF8_LEN);
            } else {
                ASSERT(strlen(territoryNamesPiped) <= MAX_TERRITORY_FULLNAME_UTF8_LEN);
                strcpy(territoryName, territoryNamesPiped);
            }
            if (alphabet != recognizeAlphabetUtf8(territoryName)) { // filter out
                if (!pipePtr) { // this is the last string!
                    return 0;
                }
                territoryNamesPiped = pipePtr + 1;
                continue;
            }
        }

        if (!pipePtr) { // this is the last string!
            if (alternative > 0) { // not what we want?
                return 0;
            }
            ASSERT(strlen(territoryNamesPiped) <= MAX_TERRITORY_FULLNAME_UTF8_LEN);
            strcpy(territoryName, territoryNamesPiped); // no bracket, return it all
            return 1;
        } else {
            if (!alternative) { // what we want?
                break;
            }
            alternative--;
            territoryNamesPiped = pipePtr + 1;
        }
    }
    lengthCopy(territoryName, territoryNamesPiped, (int) (pipePtr - territoryNamesPiped), MAX_TERRITORY_FULLNAME_UTF8_LEN);
    return 1;
}


int getFullTerritoryNameEnglish(char *territoryName, enum Territory territory, int alternative) {
    ASSERT(territoryName);
    ASSERT((_TERRITORY_MIN < territory) && (territory < _TERRITORY_MAX));
    return getFullTerritoryNameInLocaleUtf8(territoryName, territory, alternative, "en_US");
}


int getFullTerritoryNameInLocaleUtf8(char *territoryName, enum Territory territory, int alternative,
                                     const char *locale) {
    ASSERT(territoryName);
    ASSERT(((_TERRITORY_MIN < territory) && (territory < _TERRITORY_MAX)) || (territory == TERRITORY_UNKNOWN));
    return getFullTerritoryName_internal(territoryName, territory, alternative, locale, _ALPHABET_MIN);
}


int getFullTerritoryNameInLocaleInAlphabetUtf8(char *territoryName, enum Territory territory, int alternative,
                                               const char *locale, enum Alphabet alphabet) {
    ASSERT(territoryName);
    ASSERT((_TERRITORY_MIN < territory) && (territory < _TERRITORY_MAX));
    if ((alphabet <= _ALPHABET_MIN) || (alphabet >= _ALPHABET_MAX)) {
        *territoryName = 0;
        return 0;
    }
    return getFullTerritoryName_internal(territoryName, territory, alternative, locale, alphabet);
}


int getFullTerritoryNameLocalUtf8(char *territoryName, enum Territory territory, int alternative) {
    ASSERT(territoryName);
    ASSERT((_TERRITORY_MIN < territory) && (territory < _TERRITORY_MAX));
    return getFullTerritoryName_internal(territoryName, territory, alternative, NULL, _ALPHABET_MIN);
}


int getFullTerritoryNameLocalInAlphabetUtf8(char *territoryName, enum Territory territory, int alternative,
                                            enum Alphabet alphabet) {
    ASSERT(territoryName);
    ASSERT((_TERRITORY_MIN < territory) && (territory < _TERRITORY_MAX));
    if ((alphabet <= _ALPHABET_MIN) || (alphabet >= _ALPHABET_MAX)) {
        *territoryName = 0;
        return 0;
    }
    return getFullTerritoryName_internal(territoryName, territory, alternative, NULL, alphabet);
}


