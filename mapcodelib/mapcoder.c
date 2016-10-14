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

#include <string.h> // strlen strcpy strcat memcpy memmove strstr strchr memcmp
#include <stdlib.h> // atof
#include <ctype.h>  // toupper
#include <math.h>   // floor
#include "mapcoder.h"
#include "basics.h"
#include "mapcode_fastalpha.h"

#define FAST_ENCODE
#ifdef FAST_ENCODE

#include "mapcode_fast_encode.h"

#endif

///////////////////////////////////////////////////////////////////////////////////////////////
//
//  distanceInMeters
//
///////////////////////////////////////////////////////////////////////////////////////////////

// PUBLIC - returns distance (in meters) between two coordinates (in degrees)
double distanceInMeters(double latDeg1, double lonDeg1, double latDeg2, double lonDeg2) {
    // Radius of Earth.
#define EARTH_RADIUS_X_METERS 6378137
#define EARTH_RADIUS_Y_METERS 6356752

    // Circumference of Earth.
#define EARTH_CIRCUMFERENCE_X (EARTH_RADIUS_X_METERS * 2 * _PI)
#define EARTH_CIRCUMFERENCE_Y (EARTH_RADIUS_Y_METERS * 2 * _PI)

    // Meters per degree latitude is fixed. For longitude: use factor * cos(midpoint of two degree latitudes).
#define METERS_PER_DEGREE_LAT (EARTH_CIRCUMFERENCE_Y / 360.0)
#define METERS_PER_DEGREE_LON (EARTH_CIRCUMFERENCE_X / 360.0)

    // PI
#define _PI 3.14159265358979323846

    if (lonDeg1 < 0 && lonDeg2 > 1) {
        lonDeg1 += 360;
    }
    if (lonDeg2 < 0 && lonDeg1 > 1) {
        lonDeg2 += 360;
    }
    {
        const double dy = (latDeg2 - latDeg1) * METERS_PER_DEGREE_LAT;
        const double dx = (lonDeg2 - lonDeg1) * METERS_PER_DEGREE_LON * cos((latDeg1 + latDeg2) * _PI / 360.0);
        return sqrt(dx * dx + dy * dy);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////
//
//  maxErrorInMeters
//
///////////////////////////////////////////////////////////////////////////////////////////////

// maximum error in meters for a certain nr of high-precision digits
static const double maxErrorInMetersForDigits[MAX_PRECISION_DIGITS + 1] = {
        7.49,
        1.39,
        0.251,
        0.0462,
        0.00837,
        0.00154,
        0.00028,
        0.000052,
        0.0000093
};

// PUBLIC - returns maximum error in meters for a certain nr of high-precision digits
double maxErrorInMeters(int extraDigits) {
    if ((extraDigits < 0) || (extraDigits > MAX_PRECISION_DIGITS)) {
        return (double) 0;
    }
    return maxErrorInMetersForDigits[extraDigits];
}

///////////////////////////////////////////////////////////////////////////////////////////////
//
//  point / point32
//
///////////////////////////////////////////////////////////////////////////////////////////////

typedef struct {
    int lat; // latitude in microdegrees
    int lon; // longitude in microdegrees
} point32;

typedef struct { // point
    double lat;  // latitude (units depend on situation)
    double lon;  // longitude (units depend on situation)
} point;

static point32 convertFractionsToCoord32(const point *p) {
    point32 p32;
    p32.lat = (int) floor(p->lat / 810000);
    p32.lon = (int) floor(p->lon / 3240000);
    return p32;
}

static point convertFractionsToDegrees(const point *p) {
    point pd;
    pd.lat = p->lat / (810000 * 1000000.0);
    pd.lon = p->lon / (3240000 * 1000000.0);
    return pd;
}

static void convertCoordsToMicrosAndFractions(point32 *coord32, int *fraclat, int *fraclon, double lat, double lon) {
    double frac;
    if (lat < -90) {
        lat = -90;
    } else if (lat > 90) {
        lat = 90;
    }
    lat += 90; // lat now [0..180]
    lat *= (double) 810000000000;
    frac = floor(lat + 0.1);
    coord32->lat = (int) (frac / (double) 810000);
    if (fraclat) {
        frac -= ((double) coord32->lat * (double) 810000);
        *fraclat = (int) frac;
    }
    coord32->lat -= 90000000;

    lon -= (360.0 * floor(lon / 360)); // lon now in [0..360>
    lon *= (double) 3240000000000;
    frac = floor(lon + 0.1);
    coord32->lon = (int) (frac / (double) 3240000);
    if (fraclon) {
        frac -= (double) coord32->lon * (double) 3240000;
        *fraclon = (int) frac;
    }
    if (coord32->lon >= 180000000) {
        coord32->lon -= 360000000;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////
//
//  Boundaries (specified in microDegrees)
//
///////////////////////////////////////////////////////////////////////////////////////////////

#define Boundaries mminforec

// returns nonzero if x in the range minx...maxx
static int isInRange(int x, const int minx, const int maxx) {
    if (minx <= x && x < maxx) {
        return 1;
    }
    if (x < minx) {
        x += 360000000;
    } else {
        x -= 360000000;
    } // 1.32 fix FIJI edge case
    if (minx <= x && x < maxx) {
        return 1;
    }
    return 0;
}

// returns true iff given coordinate "coord32" fits inside given Boundaries
static int fitsInsideBoundaries(const point32 *coord32, const Boundaries *b) {
    return (b->miny <= coord32->lat && coord32->lat < b->maxy && isInRange(coord32->lon, b->minx, b->maxx));
}

// set target Boundaries to a source extended with deltalat, deltaLon (in microDegrees)
static Boundaries *getExtendedBoundaries(Boundaries *target, const Boundaries *source,
                                         const int deltaLat, const int deltaLon) {
    target->miny = source->miny - deltaLat;
    target->minx = source->minx - deltaLon;
    target->maxy = source->maxy + deltaLat;
    target->maxx = source->maxx + deltaLon;
    return target;
}

///////////////////////////////////////////////////////////////////////////////////////////////
//
//  MapcodeZone
//
///////////////////////////////////////////////////////////////////////////////////////////////

#define MICROLAT_TO_FRACTIONS_FACTOR ((double)MAX_PRECISION_FACTOR)
#define MICROLON_TO_FRACTIONS_FACTOR (4.0 * MAX_PRECISION_FACTOR)
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
    return ((z->fmaxx <= z->fminx) || (z->fmaxy <= z->fminy));
}

static point getMidPointFractions(const MapcodeZone *z) {
    point p;
    p.lon = floor((z->fminx + z->fmaxx) / 2);
    p.lat = floor((z->fminy + z->fmaxy) / 2);
    return p;
}

static void zoneCopyFrom(MapcodeZone *target, const MapcodeZone *source) {
    target->fminy = source->fminy;
    target->fmaxy = source->fmaxy;
    target->fminx = source->fminx;
    target->fmaxx = source->fmaxx;
}

// determine the non-empty intersection zone z between a given zone and the boundaries of territory rectangle m.
// returns nonzero in case such a zone exists
static int restrictZoneTo(MapcodeZone *z, const MapcodeZone *zone, const Boundaries *b) {
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
//  Data access
//
///////////////////////////////////////////////////////////////////////////////////////////////

/*** low-level data access ***/

static int firstrec(const int ccode) {
    return data_start[ccode];
}

static int lastrec(const int ccode) {
    return data_start[ccode + 1] - 1;
}

#define ParentLetter(ccode) ((int)parentletter[ccode])

// returns parent of ccode, or -1
static int ParentTerritoryOf(const int ccode) {
    return parentnr[ParentLetter(ccode)];
}

static int coDex(const int m) {
    int c = mminfo[m].flags & 31;
    return 10 * (c / 5) + ((c % 5) + 1);
}

#define isNameless(m)        ((mminfo[m].flags & 64))
#define recType(m)           ((mminfo[m].flags>>7) & 3)
#define isRestricted(m)      (mminfo[m].flags & 512)
#define isSpecialShape22(m)  (mminfo[m].flags & 1024)
#define headerLetter(m)      (encode_chars[(mminfo[m].flags>>11)&31])
#define smartDiv(m)          (mminfo[m].flags>>16)
#define boundaries(m)        (&mminfo[m])

static int xDivider4(const int miny, const int maxy) {
    if (miny >= 0) { // both above equator? then miny is closest
        return xdivider19[(miny) >> 19];
    }
    if (maxy >= 0) { // opposite sides? then equator is worst
        return xdivider19[0];
    }
    return xdivider19[(-maxy) >> 19]; // both negative, so maxy is closest to equator
}

// Legacy: NOT threadsafe
static int debugStopAt = -1; // to externally test-restrict internal encoding, do not use!

#define decodeChar(c) decode_chars[(unsigned char)c] // force c to be in range of the index, between 0 and 255

/*** mid-level data access ***/

// returns true iff ccode is a subdivision of some other country
static int isSubdivision(const int ccode) {
    return (ParentTerritoryOf(ccode) >= 0);
}

// find first territory rectangle of the same type as m
static int firstNamelessRecord(const int m, const int firstcode) {
    int i = m;
    const int codexm = coDex(m);
    while (i >= firstcode && coDex(i) == codexm && isNameless(i)) {
        i--;
    }
    return (i + 1);
}

// count all territory rectangles of the same type as m
static int countNamelessRecords(const int m, const int firstcode) {
    const int first = firstNamelessRecord(m, firstcode);
    const int codexm = coDex(m);
    int last = m;
    while (coDex(last) == codexm) {
        last++;
    }
    return (last - first);
}

static int isNearBorderOf(const point32 *coord32, const Boundaries *b) {
    int xdiv8 = xDivider4(b->miny, b->maxy) / 4; // should be /8 but there's some extra margin
    Boundaries tmp;
    return (fitsInsideBoundaries(coord32, getExtendedBoundaries(&tmp, b, +60, +xdiv8)) &&
            (!fitsInsideBoundaries(coord32, getExtendedBoundaries(&tmp, b, -60, -xdiv8))));
}

static const char *get_entity_iso3(char *entity_iso3_result, const int ccode) {
    if (ccode < 0 || ccode >= MAX_MAPCODE_TERRITORY_CODE) {
        return "AAA";
    } // solve bad args
    memcpy(entity_iso3_result, entity_iso + ccode * 4, 3);
    entity_iso3_result[3] = 0;
    return entity_iso3_result;
}

static void makeupper(char *s) {
    while (*s) {
        *s = (char) toupper(*s);
        s++;
    }
}

// returns 1 - 8, or negative if error
static int getParentcode(const char *s, const int len) {
    const char *p = (len == 2 ? parents2 : parents3);
    const char *f;
    char country[4];
    if (s[0] == 0 || s[1] == 0) {
        return -27;
    } // solve bad args
    if (len != 2 && len != 3) {
        return -923;
    } // solve bad args
    memcpy(country, s, len);
    country[len] = 0;
    makeupper(country);
    f = strstr(p, country);
    if (f == NULL) {
        return -23; // unknown country
    }
    return 1 + (int) ((f - p) / (len + 1));
}

///////////////////////////////////////////////////////////////////////////////////////////////
//
//  MAPCODE ALL-DIGIT PACKING/UNPACKING
//
///////////////////////////////////////////////////////////////////////////////////////////////

static void repack_if_alldigits(char *input, const int aonly) {
    char *s = input;
    int alldigits = 1; // assume all digits
    char *e;
    char *dotpos = NULL;

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
            *s = encode_chars[v / 32];
            *e = encode_chars[v % 32];
        } else // encode using A,E,U
        {
            const int v = ((*s) - '0') * 10 + ((*e) - '0');
            *s = encode_chars[(v / 34) + 31];
            *e = encode_chars[v % 34];
        }
    }
}

// rewrite all-digit codes
// returns 1 if unpacked, 0 if left unchanged, negative if unchanged and an error was detected
static int unpack_if_alldigits(char *input) {
    char *s = input;
    char *dotpos = NULL;
    const int aonly = ((*s == 'A') || (*s == 'a'));
    if (aonly) {
        s++;
    } //*** v1.50
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
                return -9; // invalid last character!
            } else {
                v += decodeChar(*e);
            }

            if (v < 100) {
                *s = encode_chars[(unsigned int) v / 10];
                *e = encode_chars[(unsigned int) v % 10];
            } else {
                return -31; // overflow (ending in UE or UU)
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
    point32 coord32;
    int fraclat; // latitude fraction of microdegrees, expressed in 1 / 810,000ths
    int fraclon; // longitude fraction of microdegrees, expressed in 1 / 3,240,000ths
    // output
    Mapcodes *mapcodes;
} encodeRec;

// encode the high-precision extension (0-8 characters)
static void encodeExtension(char *result, const int extrax4, const int extray, const int dividerx4,
                            const int dividery, int extraDigits, const int ydirection,
                            const encodeRec *enc) // append extra characters to result for more precision
{
    if (extraDigits > 0) { // anything to do?
        char *s = result + strlen(result);
        double factorx = (double) MAX_PRECISION_FACTOR * dividerx4; // perfect integer!
        double factory = (double) MAX_PRECISION_FACTOR * dividery; // perfect integer!
        double valx = ((double) MAX_PRECISION_FACTOR * extrax4) + enc->fraclon; // perfect integer!
        double valy = ((double) MAX_PRECISION_FACTOR * extray) + (ydirection * enc->fraclat); // perfect integer!

        // protect against floating point errors
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

            *s++ = encode_chars[(gy / 5) * 5 + (gx / 6)];
            if (--extraDigits == 0) {
                break;
            }

            *s++ = encode_chars[(gy % 5) * 6 + (gx % 6)];
            if (--extraDigits == 0) {
                break;
            }

            valx -= factorx * gx; // for next iteration
            valy -= factory * gy; // for next iteration
        }
        *s = 0; // terminate the result
    }
}

// encode 'value' into result[nrchars]
static void encodeBase31(char *result, int value, int nrchars) {
    result[nrchars] = 0; // zero-terminate!
    while (nrchars-- > 0) {
        result[nrchars] = encode_chars[value % 31];
        value /= 31;
    }
}

static void encode_triple(char *result, const int difx, const int dify) {
    if (dify < 4 * 34) // first 4(x34) rows of 6(x28) wide
    {
        *result = encode_chars[((difx / 28) + 6 * (dify / 34))];
        encodeBase31(result + 1, ((difx % 28) * 34 + (dify % 34)), 2);
    } else // bottom row
    {
        *result = encode_chars[(difx / 24) + 24];
        encodeBase31(result + 1, (difx % 24) * 40 + (dify - 136), 2);
    }
} // encode_triple

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

// returns *result==0 in case of error
static void encodeGrid(char *result, const encodeRec *enc, const int m, const int extraDigits,
                       const char headerLetter) {
    const Boundaries *b = boundaries(m);

    const int orgcodex = coDex(m);
    int codexm = orgcodex;
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

        divy = smartDiv(m);
        if (divy == 1) {
            divx = xside[prelen];
            divy = yside[prelen];
        } else {
            divx = (nc[prelen] / divy);
        }

        { // grid
            const int ygridsize = (b->maxy - b->miny + divy - 1) / divy;
            const int xgridsize = (b->maxx - b->minx + divx - 1) / divx;
            int rely = enc->coord32.lat - b->miny;
            int x = enc->coord32.lon;
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
                const int dividery = ((ygridsize + yside[postlen] - 1) / yside[postlen]);
                const int dividerx = ((xgridsize + xside[postlen] - 1) / xside[postlen]);
                int extrax, extray;

                {
                    char *resultptr = result + prelen;


                    int difx = x - relx;
                    int dify = enc->coord32.lat - rely;

                    *resultptr++ = '.';

                    extrax = difx % dividerx;
                    extray = dify % dividery;
                    difx /= dividerx;
                    dify /= dividery;


                    // reverse y-direction
                    dify = yside[postlen] - 1 - dify;

                    if (postlen == 3) // encode special
                    {
                        encode_triple(resultptr, difx, dify);
                    } else {
                        encodeBase31(resultptr, (difx) * yside[postlen] + dify, postlen);
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
static void encodeNameless(char *result, const encodeRec *enc, const int input_ctry,
                           const int extraDigits, const int m) {
    // determine how many nameless records there are (A), and which one is this (X)...
    const int A = countNamelessRecords(m, firstrec(input_ctry));
    const int X = m - firstNamelessRecord(m, firstrec(input_ctry));

    *result = 0;

    {
        const int p = 31 / A;
        const int r = 31 % A; // the first r items are p+1
        const int codexm = coDex(m);
        const int codexlen = (codexm / 10) + (codexm % 10);
        // determine side of square around centre
        int SIDE;

        int storage_offset;
        const Boundaries *b;

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

        SIDE = smartDiv(m);

        b = boundaries(m);
        orgSIDE = SIDE;

        {
            int v = storage_offset;

            const int dividerx4 = xDivider4(b->miny, b->maxy); // *** note: dividerx4 is 4 times too large!
            const int xFracture = (enc->fraclon / MAX_PRECISION_FACTOR);
            const int dx = (4 * (enc->coord32.lon - b->minx) + xFracture) / dividerx4; // div with quarters
            const int extrax4 = (enc->coord32.lon - b->minx) * 4 - (dx * dividerx4); // mod with quarters

            const int dividery = 90;
            int dy = (b->maxy - enc->coord32.lat) / dividery;
            int extray = (b->maxy - enc->coord32.lat) % dividery;

            if (extray == 0 && enc->fraclat > 0) {
                dy--;
                extray += dividery;
            }

            if (isSpecialShape22(m)) {
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

            if (!isSpecialShape22(m)) {
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
static void encodeAutoHeader(char *result, const encodeRec *enc, const int m, const int extraDigits) {
    int i;
    int STORAGE_START = 0;
    int W, H, xdiv, product;
    const Boundaries *b;

    // search back to first of the group
    int firstindex = m;
    const int codexm = coDex(m);
    while (recType(firstindex - 1) > 1 && coDex(firstindex - 1) == codexm) {
        firstindex--;
    }

    i = firstindex;
    for (;;) {
        b = boundaries(i);
        // determine how many cells
        H = (b->maxy - b->miny + 89) / 90; // multiple of 10m
        xdiv = xDivider4(b->miny, b->maxy);
        W = ((b->maxx - b->minx) * 4 + (xdiv - 1)) / xdiv;

        // round up to multiples of 176*168...
        H = 176 * ((H + 176 - 1) / 176);
        W = 168 * ((W + 168 - 1) / 168);
        product = (W / 168) * (H / 176) * 961 * 31;
        if (recType(i) == 2) { // plus pipe
            const int GOODROUNDER = codexm >= 23 ? (961 * 961 * 31) : (961 * 961);
            product = ((STORAGE_START + product + GOODROUNDER - 1) / GOODROUNDER) * GOODROUNDER - STORAGE_START;
        }
        if (i == m) {
            // encode
            const int dividerx = (b->maxx - b->minx + W - 1) / W;
            const int vx = (enc->coord32.lon - b->minx) / dividerx;
            const int extrax = (enc->coord32.lon - b->minx) % dividerx;

            const int dividery = (b->maxy - b->miny + H - 1) / H;
            int vy = (b->maxy - enc->coord32.lat) / dividery;
            int extray = (b->maxy - enc->coord32.lat) % dividery;

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
            encode_triple(result + codexlen - 1, vx % 168, vy % 176);

            encodeExtension(result, extrax << 2, extray, dividerx << 2, dividery, extraDigits, -1, enc); // autoheader
            return;
        }
        STORAGE_START += product;
        i++;
    }
}

static void encoderEngine(const int ccode, const encodeRec *enc, const int stop_with_one_result,
                          const int extraDigits, const int requiredEncoder, const int ccode_override) {
    int from, upto;

    if ((enc == NULL) || (ccode < 0) || (ccode > ccode_earth)) {
        return;
    } // bad arguments

    from = firstrec(ccode);
    upto = lastrec(ccode);

    if (!fitsInsideBoundaries(&enc->coord32, boundaries(upto))) {
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
            if (fitsInsideBoundaries(&enc->coord32, boundaries(i))) {
                if (isNameless(i)) {
                    encodeNameless(result, enc, ccode, extraDigits, i);
                } else if (recType(i) > 1) {
                    encodeAutoHeader(result, enc, i, extraDigits);
                } else if ((i == upto) && isSubdivision(ccode)) {
                    // *** do a recursive call for the parent ***
                    encoderEngine(ParentTerritoryOf(ccode), enc, stop_with_one_result, extraDigits, requiredEncoder,
                                  ccode);
                    return; /**/
                } else // must be grid
                {
                    // skip isRestricted records unless there already is a result
                    if (result_counter || !isRestricted(i)) {
                        if (coDex(i) < 54) {
                            char headerletter = (char) ((recType(i) == 1) ? headerLetter(i) : 0);
                            encodeGrid(result, enc, i, extraDigits, headerletter);
                        }
                    }
                }

                // =========== handle result (if any)
                if (*result) {
                    result_counter++;

                    repack_if_alldigits(result, 0);

                    if ((requiredEncoder < 0) || (requiredEncoder == i)) {
                        const int cc = (ccode_override >= 0 ? ccode_override : ccode);
                        if (*result && enc->mapcodes && (enc->mapcodes->count < MAX_NR_OF_MAPCODE_RESULTS)) {
                            char *s = enc->mapcodes->mapcode[enc->mapcodes->count++];
                            if (cc == ccode_earth) {
                                strcpy(s, result);
                            } else {
                                getTerritoryIsoName(s, cc + 1, 0);
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

// pass point to an array of pointers (at least 42), will be made to point to result strings...
// returns nr of results;
static int encodeLatLonToMapcodes_internal(char **v, Mapcodes *mapcodes,
                                           const double lat, const double lon,
                                           const int tc, const int stop_with_one_result,
                                           const int requiredEncoder, const int extraDigits) {
    encodeRec enc;
    enc.mapcodes = mapcodes;
    enc.mapcodes->count = 0;

    convertCoordsToMicrosAndFractions(&enc.coord32, &enc.fraclat, &enc.fraclon, lat, lon);

    if (tc <= 0) // ALL results?
    {
#ifdef FAST_ENCODE
        const int sum = enc.coord32.lon + enc.coord32.lat;
        int coord = enc.coord32.lon;
        int i = 0; // pointer into redivar
        for (;;) {
            const int r = redivar[i++];
            if (r >= 0 && r < 1024) { // leaf?
                int j;
                for (j = 0; j <= r; j++) {
                    const int ctry = (j == r ? ccode_earth : redivar[i + j]);
                    encoderEngine(ctry, &enc, stop_with_one_result, extraDigits, requiredEncoder, -1);
                    if ((stop_with_one_result || (requiredEncoder >= 0)) && (enc.mapcodes->count > 0)) {
                        break;
                    }
                }
                break;
            } else {
                coord = sum - coord;
                if (coord > r) {
                    i = redivar[i];
                } else {
                    i++;
                }
            }
        }
#else
        int i;
        for(i=0;i<MAX_MAPCODE_TERRITORY_CODE;i++) {
          encoderEngine(i,&enc,stop_with_one_result,extraDigits, requiredEncoder, -1);
          if ((stop_with_one_result || (requiredEncoder >= 0)) && (enc.mapcodes->count > 0)) { break; }
        }
#endif
    } else {
        encoderEngine((tc - 1), &enc, stop_with_one_result, extraDigits, requiredEncoder, -1);
    }

    if (v) {
        int i;
        for (i = 0; i < enc.mapcodes->count; i++) {
            char *s = &enc.mapcodes->mapcode[i][0];
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

    return enc.mapcodes->count;
}

///////////////////////////////////////////////////////////////////////////////////////////////
//
//  DECODING
//
///////////////////////////////////////////////////////////////////////////////////////////////

typedef struct {
    // input
    const char *orginput;   // original full input string
    char minput[MAX_MAPCODE_RESULT_LEN]; // room to manipulate clean copy of input
    const char *mapcode;    // input mapcode (first character of proper mapcode excluding territory code)
    const char *extension;  // input extension (or empty)
    int context;            // input territory context (or negative)
    const char *iso;        // input territory alphacode (context)
    // output
    point result;           // result
    point32 coord32;        // result in integer arithmetic (microdegrees)
    MapcodeZone zone;       // result zone (in "DegreeFractions")
} decodeRec;

// decode the high-precision extension (0-8 characters)
// this routine takes the integer-arithmeteic decoding results (dec->coord32), adds precision, 
// and determines result zone (dec->zone); returns negative in case of error.
static int decodeExtension(decodeRec *dec,
                           int dividerx4, int dividery,
                           const int lon_offset4,
                           const int extremeLat32, const int maxLon32) {
    double lat1, lon4;
    const char *extrapostfix = dec->extension;
    int lon32 = 0;
    int lat32 = 0;
    int processor = 1;
    int odd = 0;
    if (strlen(extrapostfix) > MAX_PRECISION_DIGITS) {
        return -79; // too many digits
    }
    while (*extrapostfix) {
        int column1, row1, column2, row2;
        const int c1 = decodeChar(*extrapostfix++);
        if (c1 < 0 || c1 == 30) {
            return -1;
        } // illegal extension character
        row1 = (c1 / 5);
        column1 = (c1 % 5);
        if (*extrapostfix) {
            const int c2 = decodeChar(*extrapostfix++);
            if (c2 < 0 || c2 == 30) {
                return -1;
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

    lon4 = (dec->coord32.lon * 4 * (double) MAX_PRECISION_FACTOR) + ((lon32 * (double) dividerx4)) +
           (lon_offset4 * (double) MAX_PRECISION_FACTOR);
    lat1 = (dec->coord32.lat * (double) MAX_PRECISION_FACTOR) + ((lat32 * (double) dividery));

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
    return isEmpty(&dec->zone) ? -45 : 0;
}

// decode 'code' until either a dot or an end-of-string is encountered
static int decodeBase31(const char *code) {
    int value = 0;
    while (*code != '.' && *code != 0) {
        value = value * 31 + decodeChar(*code++);
    }
    return value;
}

static void decode_triple(const char *result, int *difx, int *dify) {
    // decode the first character
    const int c1 = decodeChar(*result++);
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
} // decode_triple

static void decodeSixWide(const int v, const int width, const int height,
                          int *x, int *y) {
    int w;
    int D = 6;
    int col = v / (height * 6);
    const int maxcol = (width - 4) / 6;
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
static int decodeGrid(decodeRec *dec, const int m, const int hasHeaderLetter) {
    const char *input = (hasHeaderLetter ? dec->mapcode + 1 : dec->mapcode);
    const int codexlen = (int) (strlen(input) - 1);
    int prelen = (int) (strchr(input, '.') - input);
    char result[MAX_PROPER_MAPCODE_LEN + 1];

    if (codexlen > MAX_PROPER_MAPCODE_LEN) {
        return -109;
    }
    if (prelen > 5) {
        return -119;
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

        divy = smartDiv(m);
        if (divy == 1) {
            divx = xside[prelen];
            divy = yside[prelen];
        } else {
            divx = (nc[prelen] / divy);
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
                return -111; // EVER?
            }


            {
                const Boundaries *b = boundaries(m);
                const int ygridsize = (b->maxy - b->miny + divy - 1) / divy; // microdegrees per cell
                const int xgridsize = (b->maxx - b->minx + divx - 1) / divx; // microdegrees per cell

                // encode relative to THE CORNER of this cell
                rely = b->miny + (rely * ygridsize);
                relx = b->minx + (relx * xgridsize);

                {
                    const int xp = xside[postlen];
                    const int dividerx = ((xgridsize + xp - 1) / xp);
                    const int yp = yside[postlen];
                    const int dividery = ((ygridsize + yp - 1) / yp);
                    // decoderelative

                    {
                        char *r = result + prelen + 1;
                        int difx, dify;

                        if (postlen == 3) // decode special
                        {
                            decode_triple(r, &difx, &dify);
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

                        dec->coord32.lon = relx + (difx * dividerx);
                        dec->coord32.lat = rely + (dify * dividery);
                        if (!fitsInsideBoundaries(&dec->coord32, boundaries(m))) {
                            return -912;
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
static int decodeNameless(decodeRec *dec, int m) {
    int A, F;
    char input[8];
    const int codexm = coDex(m);
    const int codexlen = (int) (strlen(dec->mapcode) - 1);
    if (codexlen != 4 && codexlen != 5) {
        return -2;
    } // solve bad args

    // copy without dot
    {
        const int dc = (codexm != 22) ? 2 : 3;
        strcpy(input, dec->mapcode);
        strcpy(input + dc, dec->mapcode + dc + 1);
    }

    A = countNamelessRecords(m, firstrec(dec->context));
    F = firstNamelessRecord(m, firstrec(dec->context));

    {
        const int p = 31 / A;
        const int r = 31 % A;
        int v = 0;
        int SIDE;
        int swapletters = 0;
        int xSIDE;
        int X;
        const Boundaries *b;

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
            if (!isSpecialShape22(F + X)) {
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

        xSIDE = SIDE = smartDiv(m);

        b = boundaries(m);

        // decode
        {
            int dx, dy;

            if (isSpecialShape22(m)) {
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
                return -123;
            }

            {
                const int dividerx4 = xDivider4(b->miny, b->maxy); // *** note: dividerx4 is 4 times too large!
                const int dividery = 90;

                // *** note: FIRST multiply, then divide... more precise, larger rects
                dec->coord32.lon = b->minx + ((dx * dividerx4) / 4);
                dec->coord32.lat = b->maxy - (dy * dividery);

                return decodeExtension(dec, dividerx4, -dividery, ((dx * dividerx4) % 4), b->miny, b->maxx); // nameless
            }
        }
    }
}

// decodes dec->mapcode in context of territory rectangle m or one of its mates
static int decodeAutoHeader(decodeRec *dec, int m) {
    const char *input = dec->mapcode;
    const int codexm = coDex(m);
    const char *dot = strchr(input, '.');

    int STORAGE_START = 0;
    int value;

    if (dot == NULL) {
        return -201;
    }

    value = decodeBase31(input); // decode top
    value *= (961 * 31);

    for (; coDex(m) == codexm && recType(m) > 1; m++) {
        const Boundaries *b = boundaries(m);
        // determine how many cells
        int H = (b->maxy - b->miny + 89) / 90; // multiple of 10m
        const int xdiv = xDivider4(b->miny, b->maxy);
        int W = ((b->maxx - b->minx) * 4 + (xdiv - 1)) / xdiv;
        int product;

        // decode
        H = 176 * ((H + 176 - 1) / 176);
        W = 168 * ((W + 168 - 1) / 168);
        product = (W / 168) * (H / 176) * 961 * 31;

        if (recType(m) == 2) {
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
                decode_triple(dot + 1, &difx, &dify); // decode bottom 3 chars
                {
                    const int vx = (value / (H / 176)) * 168 + difx; // is vx/168
                    const int vy = (value % (H / 176)) * 176 + dify; // is vy/176

                    dec->coord32.lat = b->maxy - vy * dividery;
                    dec->coord32.lon = b->minx + vx * dividerx;

                    if ((dec->coord32.lon < b->minx) || (dec->coord32.lon >= b->maxx) ||
                        (dec->coord32.lat < b->miny) ||
                        (dec->coord32.lat > b->maxy)) // *** CAREFUL! do this test BEFORE adding remainder...
                    {
                        return -122; // invalid code
                    }
                }
            }

            return decodeExtension(dec, dividerx << 2, -dividery, 0, b->miny, b->maxx); // autoheader decode
        }
        STORAGE_START += product;
    } // for j
    return -1;
}

// returns nonzero if error
static int decoderEngine(decodeRec *dec) {

    int ccode;
    int err = -817;
    int codex;
    char *s;

    {
        int hasvowels = 0;
        int hasletters = 0;
        const char *dot = NULL;
        int prelen = 0;
        int len;
        char *w;
        // skip whitesace
        s = (char *) dec->orginput;
        while (*s <= 32 && *s > 0) {
            s++;
        }
        // remove trail and overhead
        len = (int) strlen(s);
        if (len > MAX_MAPCODE_RESULT_LEN - 1) {
            len = MAX_MAPCODE_RESULT_LEN - 1;
        }
        while (len > 0 && s[len - 1] <= 32 && s[len - 1] >= 0) {
            len--;
        }
        // copy into dec->minput;
        memcpy(w = dec->minput, s, len);
        w[len] = 0;
        // split off iso
        s = strchr(w, ' ');
        if (s) {
            *s++ = 0;
            while (*s > 0 && *s <= 32) {
                s++;
            }
            ccode = getTerritoryCode(w, dec->context - 1) - 1;
        } else {
            ccode = dec->context - 1;
            s = w;
        }
        if (ccode == ccode_mex && len < 8) {
            ccode = getTerritoryCode("5MX", -1) - 1;
        } // special case for mexico country vs state
        if ((*s == 'u') || (*s == 'U')) {
            strcpy(s, s + 1);
            repack_if_alldigits(s, 1);
        }
        dec->context = ccode;
        dec->mapcode = s;
        dec->extension = NULL;
        // make upper, handle i and o, split off high precision characters if any
        for (w = s; *w != 0; w++) {
            // uppercase
            if (*w >= 'a' && *w <= 'z') {
                *w += ('A' - 'a');
            }
            // analyse
            if (*w >= 'A' && *w <= 'Z') {
                if (*w == 'O') {
                    *w = '0';
                } else if (*w == 'I') {
                    *w = '1';
                } else if ((*w == 'A') || (*w == 'E') || (*w == 'U')) {
                    hasvowels = 1;
                } else if (dec->extension == NULL) {
                    hasletters = 1;
                }
            } else if (*w == '.') {
                if (dot) {
                    return -18;
                } // already had a dot
                prelen = (int) ((dot = w) - s);
            } else if (*w == '-') {
                if (dec->extension != NULL) {
                    return -17; // already had a hyphen
                }
                dec->extension = w + 1;
                *w = 0;
            } else if (decodeChar(*w) < 0) { // invalid char?
                return -4;
            }
        }
        if (!dot) {
            return -27;
        }
        if (dec->extension == NULL) {
            dec->extension = "";
        }

        codex = prelen * 10 + (int) strlen(dot) - 1;

        if (hasvowels) {
            if (unpack_if_alldigits(s) <= 0) {
                return -77;
            }
        } else if (!hasletters) {
            return -78;
        }

        if (codex == 54) {
            // international mapcodes must be in international context
            ccode = ccode_earth;
        } else if (isSubdivision(ccode)) {
            // int mapcodes must be interpreted in the parent of a subdivision
            int parent = ParentTerritoryOf(ccode);
            if ((codex == 44) || ((codex == 34 || codex == 43) && (parent == ccode_ind || parent == ccode_mex))) {
                ccode = parent;
            }
        }

    }

    {
        const int from = firstrec(ccode);
        const int upto = lastrec(ccode);
        int i;

        // try all ccode rectangles to decode s (pointing to first character of proper mapcode)
        for (i = from; i <= upto; i++) {
            const int codexi = coDex(i);
            const int r = recType(i);
            if (r == 0) {
                if (isNameless(i)) {
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
                        restrictZoneTo(&dec->zone, &dec->zone, boundaries(upto));

                        if ((err == 0) && isRestricted(i)) {
                            int nrZoneOverlaps = 0;
                            int j;

                            // *** make sure decode fits somewhere ***
                            dec->result = getMidPointFractions(&dec->zone);
                            dec->coord32 = convertFractionsToCoord32(&dec->result);
                            for (j = i - 1; j >= from; j--) { // look in previous rects
                                if (!isRestricted(j)) {
                                    if (fitsInsideBoundaries(&dec->coord32, boundaries(j))) {
                                        nrZoneOverlaps = 1;
                                        break;
                                    }
                                }
                            }

                            if (!nrZoneOverlaps) {
                                MapcodeZone zfound;
                                Boundaries prevu;
                                for (j = from; j < i; j++) { // try all smaller rectangles j
                                    if (!isRestricted(j)) {
                                        MapcodeZone z;
                                        if (restrictZoneTo(&z, &dec->zone, boundaries(j))) {
                                            nrZoneOverlaps++;
                                            if (nrZoneOverlaps == 1) {
                                                // first fit! remember...
                                                zoneCopyFrom(&zfound, &z);
                                                memcpy(&prevu, boundaries(j), sizeof(Boundaries));
                                            } else { // nrZoneOverlaps >= 2
                                                // more than one hit
                                                break; // give up
                                            }
                                        }
                                    } // isRestricted
                                } // for j

                                // if several sub-areas intersect, just return the whole zone
                                // (the center of which may NOT re-encode to the same mapcode!)
                                if (nrZoneOverlaps == 1) { // found exactly ONE intersection?
                                    zoneCopyFrom(&dec->zone, &zfound);
                                }
                            }

                            if (!nrZoneOverlaps) {
                                err = -1234;
                            }
                        }  // *** make sure decode fits somewhere ***
                        break;
                    }
                }
            } else if (r == 1) {
                if (codex == codexi + 10 && headerLetter(i) == *s) {
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
    }

    restrictZoneTo(&dec->zone, &dec->zone, boundaries(lastrec(ccode)));

    if (isEmpty(&dec->zone)) {
        err = -2222;
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

    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////
//
//  Alphabet support
//
///////////////////////////////////////////////////////////////////////////////////////////////

#ifdef SUPPORT_FOREIGN_ALPHABETS

// WARNING - these alphabets have NOT yet been released as standard! use at your own risk! check www.mapcode.com for details.
static UWORD asc2lan[MAPCODE_ALPHABETS_TOTAL][36] = // A-Z equivalents for ascii characters A to Z, 0-9
        {
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

static struct {
    UWORD min;
    UWORD max;
    const char *convert;
} unicode2asc[] =
        {
                {0x0041, 0x005a, "ABCDEFGHIJKLMNOPQRSTUVWXYZ"}, // Roman
                {0x0388, 0x03a9, "EU???????ABGDFZHQIKLMNCOJP?STYVXRW"}, // Greek                                  
                {0x0410, 0x042f, "AZBGDEFN??KLMHOJPCTYQXSVW????U?R"}, // Cyrillic								                 
                {0x05d0, 0x05ea, "ABCDFIGHJKLMNPQ?ROSETUVWXYZ"}, // Hebrew
                {0x0905, 0x0939, "A?????????E?????B?CD?F?G??HJZ?KL?MNP?QUZRS?T?V??W??XY"}, // Devanagari
                {0x0d07, 0x0d39, "??U?E??????A??BCD??F?G??HOJ??KLMNP?????Q?RST?VWX?YZ"}, // Malayalam
                {0x10a0, 0x10bf, "AB?CE?D?UF?GHOJ?KLM?NPQRSTVW?XYZ"}, // Georgian
                {0x30a2, 0x30f2, "A?????U?EB?C?D?F?G?H???J???????K??????L?M?N?????P??Q??R??S?????TV?????WX???Y????Z"}, // Katakana
                {0x0e01, 0x0e32, "BC?D??FGHJ??????K??L?MNP?Q?R????S?T?V?W????UXYZA?E"}, // Thai
                {0x0e81, 0x0ec6, "BC?D??FG?H??J??????K??L?MN?P?Q??RST???V??WX?Y?ZA????????????U?????EI??"}, // Lao
                {0x0532, 0x0556, "BCDE??FGHI?J?KLM?N?U?PQ?R??STVWXYZ?OA"}, // Armenian
                {0x0995, 0x09b9, "CDFBGH?AJ?UKLMNPQR?S?T?VWEX??Y??????Z"}, // Bengali/Assamese
                {0x0a05, 0x0a39, "A?????????E?????B?CD?F?G??HJZ?KL?MNP?QU?RS?T?V??W??XY"}, // Gurmukhi
                {0x0f40, 0x0f66, "BCD?FGHJ?K?????NMP?QRLS?A?????TVUWXYE?Z"}, // Tibetan
                {0x0627, 0x0648, "IA?BEVCDFGHJKLMNPQRS??????TU?WXYOZ"}, // Arabic

                {0x0966, 0x096f, ""}, // Devanagari digits
                {0x0d66, 0x0d6f, ""}, // Malayalam digits
                {0x0e50, 0x0e59, ""}, // Thai digits
                {0x09e6, 0x09ef, ""}, // Bengali digits
                {0x0a66, 0x0a6f, ""}, // Gurmukhi digits
                {0x0f20, 0x0f29, ""}, // Tibetan digits
                {0x1040, 0x1049, ""}, // Burmese digits
                {0x17e0, 0x17e9, ""}, // Khmer digits
                {0x0be6, 0x0bef, ""}, // Tamil digits
                {0x1369, 0x1372, "1234567890"}, // Amharic digits [1-9][0]

                {0x1100, 0x1175, "B?CD?F?G?H?OV?WXJKA??????????????????????????????????????????????????????????????????????????????LMNPQER?S???YZ???T?UI"}, // Korean
                {0x1000, 0x101f, "BCDFGA?J????KLMYNPQESTUWX?H?ZORV"}, // Burmese
                {0x1780, 0x17a2, "BCDFGE?HJAK?LMN??PQRSTV?W?IX???UY?Z"}, // Khmer
                {0x0d85, 0x0dc5, "A???E?U??????????????B?C??D??F????G?H??JK?L?M?NP?Q?RSTV?W??X??YZ?"}, // Sinhalese
                {0x0780, 0x07b1, "BCDFGHJKLMNPQRSTVWXYA??E????U????????????????????Z"}, // Thaana
                {0x3105, 0x3129, "BDFCGHJKLMNRSTUEPQWYZ?????????A????XV"}, // Chinese                
                {0x2d31, 0x2d63, "B?C???D????FG??H??JKL?M?A???NPI???EOQR??STVW??X?YUZ"}, // Tifinagh
                {0x0b85, 0x0bb5, "SE?TV????WXY?ZU?B???AC????D????F???G?H???JKLRMQPN"}, // Tamil
                {0x1200, 0x1354, "H???????L??????????????????A??E?T?????V?????S???????????????????Q???R???????????????????????????B???C???????????W?????X????????????????????????????M????????????I???????????K???????????????????????????U???????O???????????Z???????????J???????D???????????????????????G???????????????????????????????????????????????Y?????????????????F?????N???P"}, // Amharic
                {0x0c15, 0x0c35, "B?C?DF?G?AHJK?LMNPQR?SETUV?W?XY?Z"}, // Telugu
                {0x0b15, 0x0b39, "BCDF?G?HA?JOKLEMNPQR?S?T?VWX??Y????ZU"}, // Odia
                {0x0c85, 0x0cb5, "??U??????E???A??BCD?FGHJ???KL?MN?PQR?STVW??X?Y??Z"}, // Kannada
                {0x0a87, 0x0ab7, "E?U???????????H?B??J?CXRKNDY?L?F?W?MTGZS?P??A?Q?V"}, // Gujarati

                // lowercase variants: greek, georgisch
                {0x03AD, 0x03c9, "EU??ABGDFZHQIKLMNCOJP?STYVXRW"}, // Greek lowercase
                {0x10d0, 0x10ef, "AB?CE?D?UF?GHOJ?KLMINPQRSTVW?XYZ"}, // Georgisch lowercase
                {0x0562, 0x0586, "BCDE??FGHI?J?KLM?N?U?PQ?R??STVWXYZ?OA"}, // Armenian lowercase
                {0,      0, NULL}
        };

// Abjad forward declarations
static int isAbjadScript(const UWORD *s);

static char *convertToAbjad(char *str, const char *source, int maxlen);

static void convertFromAbjad(char *s);

char *convertToRoman(char *asciibuf, int maxlen, const UWORD *s) {
    char *w = asciibuf;
    const char *e = w + maxlen - 1;
    int is_abjad = isAbjadScript(s);
    while (*s > 0 && *s <= 32) {
        s++;
    } // skip lead
    for (; *s != 0 && w < e; s++) {
        if (*s >= 1 && *s <= 127) { // normal ascii
            *w++ = (char) (*s);
        } else {
            int i, found = 0;
            for (i = 0; unicode2asc[i].min != 0; i++) {
                if (*s >= unicode2asc[i].min && *s <= unicode2asc[i].max) {
                    const char *cv = unicode2asc[i].convert;
                    if (*cv == 0) {
                        cv = "0123456789";
                    }
                    *w++ = cv[*s - unicode2asc[i].min];
                    found = 1;
                    break;
                }
            }
            if (!found) {
                *w++ = '?';
                break;
            }
        }
    }
    // trim
    while (w > asciibuf && w[-1] > 0 && w[-1] <= 32) {
        w--;
    }
    *w = 0;
    // skip past last space (if any)
    w = strrchr(asciibuf, ' ');
    if (w) {
        w++;
    } else {
        w = asciibuf;
    }
    if (*w == 'A') {
        unpack_if_alldigits(w);
        repack_if_alldigits(w, 0);
    }
    if (is_abjad) {
        convertFromAbjad(w);
    }
    return asciibuf;
}


static UWORD *encode_utf16(UWORD *unibuf, const int maxlen, const char *mapcode,
                           const int language) // convert mapcode to language (0=roman 1=greek 2=cyrillic 3=hebrew)
{
    UWORD *w = unibuf;
    const UWORD *e = w + maxlen - 1;
    const char *r = mapcode;
    while (*r != 0 && w < e) {
        char c = *r++;
        if ((c >= 'a') && (c <= 'z')) {
            c += ('A' - 'a');
        }
        if ((c < ' ') || (c > 'Z')) { // not in any valid range?
            *w++ = (UWORD) c; // leave untranslated
        } else if ((c >= '0') && (c <= '9')) { // digit?
            *w++ = asc2lan[language][26 + (int) c - '0'];
        } else if (c < 'A') { // valid but not a letter (e.g. a dot, a space...)
            *w++ = (UWORD) c; // leave untranslated
        } else {
            *w++ = asc2lan[language][c - 'A'];
        }
    }
    *w = 0;
    return unibuf;
}

// PUBLIC - convert as much as will fit of mapcode into unibuf
UWORD *convertToAlphabet(UWORD *unibuf, int maxlength, const char *mapcode, int alphabet) // 0=roman, 2=cyrillic
{
    UWORD *startbuf = unibuf;
    UWORD *lastspot = &unibuf[maxlength - 1];
    if (maxlength > 0) {
#define USIZE 256
        char u[USIZE];

        // skip leading spaces
        while (*mapcode > 0 && *mapcode <= 32) {
            mapcode++;
        }

        // straight-copy everything up to and including first space
        {
            const char *e = strchr(mapcode, ' ');
            if (e) {
                while (mapcode <= e) {
                    if (unibuf == lastspot) { // buffer fully filled?
                        // zero-terminate and return
                        *unibuf = 0;
                        return startbuf;
                    }
                    *unibuf++ = (UWORD) *mapcode++;
                }
            }
        }

        if (alphabet == 1 || alphabet == 3 || alphabet == 14 || alphabet == 15) { // greek hebrew arabic korean
            mapcode = convertToAbjad(u, mapcode, USIZE);
        }

        // re-pack E/U-voweled mapcodes when necessary:
        if (alphabet == 1) { // alphabet has no letter E (greek!)
            if (strchr(mapcode, 'E') || strchr(mapcode, 'U') ||
                strchr(mapcode, 'e') || strchr(mapcode, 'u')) {
                // copy trimmed mapcode into temporary buffer u
                int len = (int) strlen(mapcode);
                if (len < MAX_MAPCODE_RESULT_LEN) {
                    while (len > 0 && mapcode[len - 1] > 0 && mapcode[len - 1] <= 32) {
                        len--;
                    }
                    memcpy(u, mapcode, len);
                    u[len] = 0;
                    // re-pack into A-voweled mapcode
                    unpack_if_alldigits(u);
                    repack_if_alldigits(u, 1);
                    mapcode = u;
                }
            }
        }

        encode_utf16(unibuf, 1 + (int) (lastspot - unibuf), mapcode, alphabet);
    }
    return startbuf;
}

#endif

///////////////////////////////////////////////////////////////////////////////////////////////
//
//  compareWithMapcodeFormat
//
///////////////////////////////////////////////////////////////////////////////////////////////

#define TOKENSEP   0
#define TOKENDOT   1
#define TOKENCHR   2
#define TOKENVOWEL 3
#define TOKENZERO  4
#define TOKENHYPH  5
#define ERR -1
#define Prt -9 // partial
#define GO  99

static signed char fullmc_statemachine[23][6] = {
        //                    WHI  DOT  DET VOW   ZER  HYP
        /* 0 start        */ {0,   ERR, 1,  1,    ERR, ERR}, // looking for very first detter
        /* 1 gotL         */
                             {ERR, ERR, 2,  2,    ERR, ERR}, // got one detter, MUST get another one
        /* 2 gotLL        */
                             {18,  6,   3,  3,    ERR, 14}, // GOT2: white: got territory + start prefix | dot: 2.X mapcode | det:3letter | hyphen: 2-state
        /* 3 gotLLL       */
                             {18,  6,   4,   ERR, ERR, 14}, // white: got territory + start prefix | dot: 3.X mapcode | det:4letterprefix | hyphen: 3-state
        /* 4 gotprefix4   */
                             {ERR, 6,   5,   ERR, ERR, ERR}, // dot: 4.X mapcode | det: got 5th prefix letter
        /* 5 gotprefix5   */
                             {ERR, 6,   ERR, ERR, ERR, ERR}, // got 5char so MUST get dot!
        /* 6 prefix.      */
                             {ERR, ERR, 7,  7,    Prt, ERR}, // MUST get first letter after dot
        /* 7 prefix.L     */
                             {ERR, ERR, 8,  8,    Prt, ERR}, // MUST get second letter after dot
        /* 8 prefix.LL    */
                             {22,  ERR, 9,  9,    GO,  11}, // get 3d letter after dot | X.2- | X.2 done!
        /* 9 prefix.LLL   */
                             {22,  ERR, 10, 10,   GO,  11}, // get 4th letter after dot | X.3- | X.3 done!
        /*10 prefix.LLLL  */
                             {22,  ERR, ERR, ERR, GO,  11}, // X.4- | x.4 done!

        /*11 mc-          */
                             {ERR, ERR, 12,  ERR, Prt, ERR}, // MUST get first precision letter
        /*12 mc-L         */
                             {22,  ERR, 13,  ERR, GO,  ERR}, // Get 2nd precision letter | done X.Y-1
        /*13 mc-LL*       */
                             {22,  ERR, 13,  ERR, GO,  ERR}, // *** keep reading precision detters *** until whitespace or done

        /*14 ctry-        */
                             {ERR, ERR, 15, 15,   ERR, ERR}, // MUST get first state letter
        /*15 ctry-L       */
                             {ERR, ERR, 16, 16,   ERR, ERR}, // MUST get 2nd state letter
        /*16 ctry-LL      */
                             {18,  ERR, 17, 17,   ERR, ERR}, // white: got CCC-SS and get prefix | got 3d letter
        /*17 ctry-LLL     */
                             {18,  ERR, ERR, ERR, ERR, ERR}, // got CCC-SSS so MUST get whitespace and then get prefix

        /*18 startprefix  */
                             {18,  ERR, 19, 19,   ERR, ERR}, // skip more whitespace, MUST get 1st prefix letter
        /*19 gotprefix1   */
                             {ERR, ERR, 20,  ERR, ERR, ERR}, // MUST get second prefix letter
        /*20 gotprefix2   */
                             {ERR, 6,   21,  ERR, ERR, ERR}, // dot: 2.X mapcode | det: 3d perfix letter
        /*21 gotprefix3   */
                             {ERR, 6,   4,   ERR, ERR, ERR}, // dot: 3.x mapcode | det: got 4th prefix letter

        /*22 whitespace   */
                             {22,  ERR, ERR, ERR, GO,  ERR}  // whitespace until end of string
};


// pass fullcode=1 to recognise territory and mapcode, pass fullcode=0 to only recognise proper mapcode (without optional territory)
// returns 0 if ok, negative in case of error (where -999 represents "may BECOME a valid mapcode if more characters are added)
int compareWithMapcodeFormat(const char *s, int fullcode) {
    int nondigits = 0, vowels = 0;
    int state = (fullcode ? 0 : 18); // initial state
    for (;; s++) {
        int newstate, token;
        // recognise token: decode returns -2=a -3=e -4=0, 0..9 for digit or "o" or "i", 10..31 for char, -1 for illegal char
        if (*s == '.') {
            token = TOKENDOT;
        } else if (*s == '-') {
            token = TOKENHYPH;
        } else if (*s == 0) {
            token = TOKENZERO;
        } else if ((*s == ' ') || (*s == '\t')) {
            token = TOKENSEP;
        } else {
            const signed char c = decode_chars[(unsigned char) *s];
            if (c < 0) { // vowel or illegal?
                token = TOKENVOWEL;
                vowels++; // assume vowel (-2,-3,-4)
                if (c == -1) { // illegal?
                    return -4;
                }
            } else if (c < 10) { // digit
                token = TOKENCHR; // digit
            } else { // charcter B-Z
                token = TOKENCHR;
                if (state != 11 && state != 12 && state != 13) {
                    nondigits++;
                }
            }
        }
        newstate = fullmc_statemachine[state][token];
        if (newstate == ERR) {
            return -(1000 + 10 * state + token);
        } else if (newstate == GO) {
            return (nondigits ? (vowels > 0 ? -6 : 0) : (vowels > 0 && vowels <= 2 ? 0 : -5));
        } else if (newstate == Prt) {
            return -999;
        } else if (newstate == 18) {
            nondigits = vowels = 0;
        }
        state = newstate;
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////
//
//  PUBLIC INTERFACE
//
///////////////////////////////////////////////////////////////////////////////////////////////

// PUBLIC - returns name of territoryCode in (sufficiently large!) result string. 
// formats: 0=full 1=short
// returns empty string in case of error
char *getTerritoryIsoName(char *result, int territoryCode, int format) {
    if ((territoryCode < 1) || (territoryCode > MAX_MAPCODE_TERRITORY_CODE)) {
        *result = 0;
    } else {
        const int p = ParentLetter(territoryCode - 1);
        char iso3[4];
        const char *ei = get_entity_iso3(iso3, territoryCode - 1);
        if (*ei >= '0' && *ei <= '9') {
            ei++;
        }
        if (format == 0 && p) {
            memcpy(result, &parents2[p * 3 - 3], 2);
            result[2] = '-';
            strcpy(result + 3, ei);
        } else {
            strcpy(result, ei);
        }
    }
    return result;
}

// PUBLIC - returns negative if territoryCode tc is not a code that has a parent country
int getParentCountryOf(int tc) {
    const int parentccode = ParentTerritoryOf(tc - 1); // returns parent ccode or -1
    if (parentccode >= 0) {
        return parentccode + 1;
    }
    return -1;
}

// PUBLIC - returns tc if territoryCode tc is a country, or parent country if tc is a state.
// returns megative if tc is invalid.
int getCountryOrParentCountry(int tc) {
    if (tc > 0 && tc < MAX_MAPCODE_TERRITORY_CODE) {
        const int tp = getParentCountryOf(tc);
        if (tp > 0) {
            return tp;
        }
        return tc;
    }
    return -1;
}

// PUBLIC - returns nonzero if coordinate is near more than one territory border
int multipleBordersNearby(double lat, double lon, int territoryCode) {
    const int ccode = territoryCode - 1;
    if ((ccode >= 0) && (ccode < ccode_earth)) { // valid territory, not earth
        const int parentTerritoryCode = getParentCountryOf(territoryCode);
        if (parentTerritoryCode >= 0) {
            // there is a parent! check its borders as well...
            if (multipleBordersNearby(lat, lon, parentTerritoryCode)) {
                return 1;
            }
        }
        {
            int m;
            int nrFound = 0;
            const int from = firstrec(ccode);
            const int upto = lastrec(ccode);
            point32 coord32;
            convertCoordsToMicrosAndFractions(&coord32, NULL, NULL, lat, lon);
            for (m = upto; m >= from; m--) {
                if (!isRestricted(m)) {
                    if (isNearBorderOf(&coord32, boundaries(m))) {
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

static int cmp_alphacode(const void *e1, const void *e2) {
    const alphaRec *a1 = (const alphaRec *) e1;
    const alphaRec *a2 = (const alphaRec *) e2;
    return strcmp(a1->alphaCode, a2->alphaCode);
} // cmp

static int binfindmatch(const int parentcode, const char *str) {
    // build a 4-letter uppercase search term
    char alphaCode[5];
    const char *r = str;
    int len = 0;

    if (parentcode < 0) {
        return -1;
    }
    if (parentcode > 0) {
        alphaCode[len++] = (char) ('0' + parentcode);
    }
    while ((len < 4) && (*r > 32)) {
        alphaCode[len++] = *r++;
    }
    if (*r > 32) {
        return -1;
    }
    alphaCode[len] = 0;
    makeupper(alphaCode);
    { // binary-search the result
        const alphaRec *p;
        alphaRec t;
        t.alphaCode = alphaCode;
        t.ccode = parentcode;

        p = (const alphaRec *) bsearch(&t, alphaSearch, NRTERREC, sizeof(alphaRec), cmp_alphacode);
        if (p) {
            if (strcmp(t.alphaCode, p->alphaCode) == 0) { // only interested in PERFECT match
                return p->ccode + 1;
            } // match
        } // found
    } //
    return -1;
}

// PUBLIC - returns territoryCode of string (or negative if not found).
// optional_tc: context territoryCode to handle ambiguities (pass <=0 if unknown).
int getTerritoryCode(const char *string, int optional_tc) {
    if (string == NULL) {
        return -1;
    }
    while (*string > 0 && *string <= 32) {
        string++;
    } // skip leading whitespace

    if (string[0] && string[1]) {
        const int ccode = optional_tc - 1;
        if (string[2] == '-') {
            return binfindmatch(getParentcode(string, 2), string + 3);
        } else if (string[2] && string[3] == '-') {
            return binfindmatch(getParentcode(string, 3), string + 4);
        } else {
            const int parentcode =
                    ccode < 0 ? 0 : ((parentnumber[ccode] > 0) ? parentnumber[ccode] : parentnumber[ParentTerritoryOf(
                            ccode)]);
            const int b = binfindmatch(parentcode, string);
            if (b > 0) {
                return b;
            } //
        } //
        return binfindmatch(0, string);
    } // else, fail:
    return -1;
}

// PUBLIC - decode string into lat,lon; returns negative in case of error
int decodeMapcodeToLatLon(double *lat, double *lon, const char *input,
                          int context_tc) // context_tc is used to disambiguate ambiguous short mapcode inputs; pass 0 or negative if not available
{
    if ((lat == NULL) || (lon == NULL) || (input == NULL)) {
        return -100;
    } else {
        int ret;
        decodeRec dec;
        dec.orginput = input;
        dec.context = context_tc;

        ret = decoderEngine(&dec);
        *lat = dec.result.lat;
        *lon = dec.result.lon;
        return ret;
    }
}

// PUBLIC - encode lat,lon for TerritoryCode tc to a mapcode with extraDigits accuracy
int encodeLatLonToSingleMapcode(char *result, double lat, double lon, int tc, int extraDigits) {
    char *v[2];
    Mapcodes rlocal;
    int ret;
    if (tc <= 0) {
        return 0;
    }
    ret = encodeLatLonToMapcodes_internal(v, &rlocal, lat, lon, tc, 1, debugStopAt, extraDigits);
    *result = 0;
    if (ret <= 0) { // no solutions?
        return -1;
    }
    // prefix territory unless international
    if (strcmp(v[1], "AAA") != 0) {
        strcpy(result, v[1]);
        strcat(result, " ");
    }
    strcat(result, v[0]);
    return 1;
}

// PUBLIC - encode lat,lon for (optional) TerritoryCode tc to mapcodes with extraDigits accuracy
int encodeLatLonToMapcodes(Mapcodes *results, double lat, double lon, int territoryCode, int extraDigits) {
    return encodeLatLonToMapcodes_internal(NULL, results, lat, lon, territoryCode, 0, debugStopAt, extraDigits);
}

///////////////////////////////////////////////////////////////////////////////////////////////
//
//  LEGACY ROUTINES (NOT THREADSAFE)
//
///////////////////////////////////////////////////////////////////////////////////////////////

// Legacy: NOT threadsafe
Mapcodes rglobal;

int encodeLatLonToMapcodes_Deprecated(char **v, double lat, double lon, int territoryCode, int extraDigits) {
    return encodeLatLonToMapcodes_internal(v, &rglobal, lat, lon, territoryCode, 0, debugStopAt, extraDigits);
}

// Legacy: NOT threadsafe
static char makeiso_bufbytes[16];
static char *makeiso_buf;

const char *convertTerritoryCodeToIsoName(int tc, int format) {
    if (makeiso_buf == makeiso_bufbytes) {
        makeiso_buf = makeiso_bufbytes + 8;
    } else {
        makeiso_buf = makeiso_bufbytes;
    }
    return (const char *) getTerritoryIsoName(makeiso_buf, tc, format);
}

#ifdef SUPPORT_FOREIGN_ALPHABETS

// Legacy: NOT threadsafe
static char asciibuf[MAX_MAPCODE_RESULT_LEN];

const char *decodeToRoman(const UWORD *s) {
    return convertToRoman(asciibuf, MAX_MAPCODE_RESULT_LEN, s);
}

// Legacy: NOT threadsafe
static UWORD unibuf[MAX_MAPCODE_RESULT_LEN];

const UWORD *encodeToAlphabet(const char *mapcode, int alphabet) // 0=roman, 2=cyrillic
{
    return convertToAlphabet(unibuf, MAX_MAPCODE_RESULT_LEN, mapcode, alphabet);
}

#endif

///////////////////////////////////////////////////////////////////////////////////////////////
//
//  ABJAD ROUTINES
//
///////////////////////////////////////////////////////////////////////////////////////////////

static int isAbjadScript(const UWORD *s) {
    for (; *s != 0; s++) {
        UWORD c = *s;
        if (c >= 0x0628 && c <= 0x0649) {
            return 1;
        } // arabic
        if (c >= 0x05d0 && c <= 0x05ea) {
            return 1;
        } // hebrew
        if (c >= 0x388 && c <= 0x3c9) {
            return 1;
        } // greek uppercase and lowecase
        if ((c >= 0x1100 && c <= 0x1174) || (c >= 0xad6c && c <= 0xd314)) {
            return 1;
        } // korean
    }
    return 0;
}


/// PRIVATE convert a mapcode to an ABJAD-format (never more than 2 non-digits in a row)
static char *convertToAbjad(char *str, const char *source, int maxlen) {
    int form, i, dot, inarow;
    int len = (int) strlen(source);
    const char *rest = strchr(source, '-');
    if (rest != NULL) {
        len = ((int) (rest - source));
    }
    if (len >= maxlen) {
        len = maxlen - 1;
    }
    while (len > 0 && source[len - 1] == ' ') {
        len--;
    }

    // copy source into str
    memcpy(str, source, len);
    str[len] = 0;
    unpack_if_alldigits(str);

    len = (int) strlen(str);
    dot = (int) (strchr(str, '.') - str);

    form = dot * 10 + (len - dot - 1);

    // see if >2 non-digits in a row
    inarow = 0;
    for (i = 0; i < len; i++) {
        int c = (int) str[i];
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
        len = (int) strlen(source);
        if (len >= maxlen) {
            len = maxlen - 1;
        }
        memcpy(str, source, len);
        str[len] = 0;
        return str;
    } else if (form >= 22 && form <= 54) {
        char c1, c2, c3 = '?';
        int c = decodeChar(str[2]);
        if (c < 0) {
            c = decodeChar(str[3]);
        }

        if (form >= 44) {
            c = (c * 31) + (decodeChar(str[len - 1]) + 39);
            c1 = encode_chars[c / 100];
            c2 = encode_chars[(c % 100) / 10];
            c3 = encode_chars[c % 10];
        } else if (len == 7) {
            if (form == 24) {
                c += 7;
            } else if (form == 33) {
                c += 38;
            } else if (form == 42) {
                c += 69;
            }
            c1 = encode_chars[c / 10];
            c2 = encode_chars[c % 10];
        } else {
            c1 = encode_chars[2 + (c / 8)];
            c2 = encode_chars[2 + (c % 8)];
        }

        if (form == 22) // s0 s1 . s3 s4 -> s0 s1 . C1 C2 s4
        {
            str[6] = 0;
            str[5] = str[4];
            str[4] = c2;
            str[3] = c1;
//      str[2] = '.';
//      str[1] = str[1];
//      str[0] = str[0];
        } else if (form == 23) { // s0 s1 . s3 s4 s5 -> s0 s1 . C1 C2 s4 s5
            str[7] = 0;
            str[6] = str[5];
            str[5] = str[4];
            str[4] = c2;
            str[3] = c1;
//      str[2] = '.';
//      str[1] = str[1];
//      str[0] = str[0];
        } else if (form == 32) { // s0 s1 s2 . s4 s5 -> s0 s1 . C* C2 s4 s5
            str[7] = 0;
            str[6] = str[5];
            str[5] = str[4];
            str[4] = c2;
            str[3] = (char) (c1 + 4);
            str[2] = '.';
//      str[1] = str[1];
//      str[0] = str[0];
        } else if (form == 24 || form == 33 || form == 42) {
            // s0 s1 . s3 s4 s5 s6 -> s0 s1 C1 . s4 C2 s5 s6
            // s0 s1 s2 . s4 s5 s6 -> s0 s1 C1 . s4 C2 s5 s6
            // s0 s1 s2 s3 . s5 s6 -> s0 s1 C1 . s3 C2 s5 s6
            str[8] = 0;
            str[7] = str[6];
            str[6] = str[5];
            str[5] = c2;
            str[4] = str[(form == 42 ? 3 : 4)];
            str[3] = '.';
            str[2] = c1;
//      str[1] = str[1];
//      str[0] = str[0];
        } else if (form == 34) {  // s0 s1 s2 . s4 s5 s6 s7 -> s0 s1 C1 . s4 s5 C2 S6 S7
            str[9] = 0;
            str[8] = str[7];
            str[7] = str[6];
            str[6] = c2;
//      str[5] = str[5];
//      str[4] = str[4];
//      str[3] = '.';
            str[2] = c1;
//      str[1] = str[1];
//      str[0] = str[0];
        } else if (form == 43) { // s0 s1 s2 s3 . s5 s6 s7 -> s0 s1 C* . s3 s5 C2 S6 S7
            str[9] = 0;
            str[8] = str[7];
            str[7] = str[6];
            str[6] = c2;
//      str[5] = str[5];
            str[4] = str[3];
            str[3] = '.';
            str[2] = (char) (c1 + 4);
//      str[1] = str[1];
//      str[0] = str[0];
        } else if (form == 44) {
            str[10] = 0;
            str[9] = str[7];
            str[8] = c3;
            str[7] = str[6];
            str[6] = str[5];
            str[5] = c2;
//      str[4] = '.';
//      str[3] = str[3];
            str[2] = c1;
//      str[1] = str[1];
//      str[0] = str[0];
        } else if (form == 54) {
            str[11] = 0;
            str[10] = str[8];
            str[9] = c3;
            str[8] = str[7];
            str[7] = str[6];
            str[6] = c2;
//      str[5] = '.';
//      str[4] = str[4];
//      str[3] = str[3];
            str[2] = c1;
//      str[1] = str[1];
//      str[0] = str[0];        
        }
    }
    repack_if_alldigits(str, 0);
    if (rest) {
        int totalLen = (int) strlen(str);
        int needed = (int) strlen(rest);
        int tocopy = maxlen - totalLen - 1;
        if (tocopy > needed) {
            tocopy = needed;
        }
        if (tocopy > 0) {
            memcpy(str + totalLen, rest, tocopy);
            str[totalLen + tocopy] = 0;
        }
    }
    return str;
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

    unpack_if_alldigits(s);

    len = (int) strlen(s);
    form = (dot >= 2 && dot <= 5 ? dot * 10 + (len - dot - 1) : 0);

    if (form == 23) {
        c = decodeChar(s[3]) * 8 + (decodeChar(s[4]) - 18);
        if (c >= 0 && c < 31) {
//          s[0] = s[0];
//          s[1] = s[1];
//          s[2] = '.';
            s[3] = encode_chars[c];
            s[4] = s[5];
            s[5] = 0;
        }
    } else if (form == 24) {
        c = decodeChar(s[3]) * 8 + (decodeChar(s[4]) - 18);
        if (c >= 0 && c < 63) {
//          s[0] = s[0];
//          s[1] = s[1];
//        	s[2] = '.';
            s[3] = '.';
            s[4] = s[5];
            s[5] = s[6];
            s[6] = 0;
            if (c >= 32) {
                s[2] = encode_chars[c - 32];
            } else {
                s[3] = encode_chars[c];
            }
        }
    } else if (form == 34) {
        c = (decodeChar(s[2]) * 10) + (decodeChar(s[5]) - 7);
        if (c >= 0 && c < 93) {
//          s[0] = s[0];
//          s[1] = s[1];
            s[2] = '.';
//        	s[3] = '.';
//        	s[4] = s[4];
            s[5] = s[6];
            s[6] = s[7];
            s[7] = 0;

            if (c < 31) {
                s[3] = encode_chars[c];
            } else if (c < 62) {
                s[2] = encode_chars[c - 31];
            } else {
                s[2] = encode_chars[c - 62];
                s[3] = s[4];
                s[4] = '.';
            }
        }
    } else if (form == 35) {
        c = (decodeChar(s[2]) * 8) + (decodeChar(s[6]) - 18);
        if (c >= 0 && c < 63) {
//          s[0] = s[0];
//          s[1] = s[1];
//        	s[3] = '.';
//        	s[4] = s[4];
//        	s[5] = s[5];
            s[6] = s[7];
            s[7] = s[8];
            s[8] = 0;
            if (c >= 32) {
                s[2] = encode_chars[c - 32];
                s[3] = s[4];
                s[4] = '.';
            } else {
                s[2] = encode_chars[c];
            }
        }
    } else if (form == 45) {
        c = (decodeChar(s[2]) * 100) + (decodeChar(s[5]) * 10) + (decodeChar(s[8]) - 39);
        if (c >= 0 && c < 961) {
//          s[0] = s[0];
//          s[1] = s[1];
            s[2] = encode_chars[c / 31];
//        	s[3] = s[3];
//        	s[4] = '.';
            s[5] = s[6];
            s[6] = s[7];
            s[7] = s[9];
            s[8] = encode_chars[c % 31];
            s[9] = 0;
        }
    } else if (form == 55) {
        c = (decodeChar(s[2]) * 100) + (decodeChar(s[6]) * 10) + (decodeChar(s[9]) - 39);
        if (c >= 0 && c < 961) {
//          s[0] = s[0];
//          s[1] = s[1];
            s[2] = encode_chars[c / 31];
//        	s[3] = s[3];
//        	s[4] = s[4];
//          s[5] = '.';
            s[6] = s[7];
            s[7] = s[8];
            s[8] = s[10];
            s[9] = encode_chars[c % 31];
            s[10] = 0;
        }
    }
    repack_if_alldigits(s, 0);
    if (postfix) {
        len = (int) strlen(s);
        *postfix = '-';
        memmove(s + len, postfix, strlen(postfix) + 1);
    }
}


const TerritoryAlphabets *getAlphabetsForTerritory(int territoryCode) {
    const int ccode = territoryCode - 1;
    if ((ccode >= 0) && (ccode <= ccode_earth)) { // valid territory, not earth
        return &alphabetsForTerritory[ccode];
    }
    return NULL;
}

