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

#define UNITTEST_VERSION "2.2"

/**
 * This application performs a number of tests on the Mapcode C library.
 * It helps to establish that all routines work properly.
 */

#include <stdio.h>
#include <math.h>
// #include <time.h>

#include "../mapcodelib/mapcoder.c"
#include "decode_test.h"

extern void test_territories();

// globals to count tests, errors and warnings
int nrTests = 0, nrErrors = 0, nrWarnings = 0;

// test the alphabet conversion routines 
static void alphabet_tests() {
    int i;
    const char *str, *expect;

    printf("%d alphabets\n", MAPCODE_ALPHABETS_TOTAL);

    for (i = 0; i < MAPCODE_ALPHABETS_TOTAL; i++) {
        UWORD enc[64];
        char dec[64];

        // see if convertToAlphabet survives empty string
        nrTests++;
        str = "";
        convertToAlphabet(enc, 64, str, i);
        if (*enc) {
            nrErrors++;
            printf("convertToAlphabet(\"%s\",%d) != \"\"\n", str, i, dec);
        }
        else {
            // see if empty UTF16 converts to empty string
            nrTests++;
            convertToRoman(dec, 64, enc);
            if (*dec) {
                nrErrors++;
                printf("convertToRoman(empty)=\"%s\"\n", "", dec);
            }
        }

        // see if alphabets (re)convert as expected
        str = "OEUoi OIoi#%?-.abcdfghjklmnpqrstvwxyz0123456789ABCDFGHJKLMNPQRSTVWXYZ";
        expect = "OEUoi OIOI#%?-.ABCDFGHJKLMNPQRSTVWXYZ0123456789ABCDFGHJKLMN";
        convertToAlphabet(enc, 64, str, i);
        convertToRoman(dec, 60, enc);
        nrTests++;
        if (strlen(dec) != 59 || strcmp(dec, expect)) {
            nrErrors++;
            printf("convertToRoman(convertToAlphabet(\"%s\",%d))=\"%s\"\n", str, i, dec);
        }

        // see if E/U voweled mapcodes (re)convert as expected
        str = "OMN 112.3EU";
        convertToAlphabet(enc, 64, str, i);
        convertToRoman(dec, 64, enc);
        nrTests++;
        if (strcmp(dec, str) != 0) {
            nrErrors++;
            printf("convertToRoman(convertToAlphabet(\"%s\",%d))=\"%s\"\n", str, i, dec);
        }
        else {
            nrTests++;
            {
                str = "  Oio 112.3AU  ";
                convertToAlphabet(enc, 64, str, i);
                convertToRoman(dec, 64, enc);
                nrTests++;
                if (strcmp(dec, "Oio 112.3AU") != 0) {
                    nrErrors++;
                    printf("convertToRoman(convertToAlphabet(\"%s\",%d))=\"%s\"\n", str, i, dec);
                }
            }
        }


    }
}


// 
static void printGeneratedMapcodes(const char *title, const Mapcodes *mapcodes) {
    int i, nrresults = mapcodes->count;
    printf(" %s: %d results", title, nrresults);
    for (i = 0; i < nrresults; i++) {
        const char *m = mapcodes->mapcode[i];
        printf(" (%s)", m);
    }
    printf("\n");
}

// test encode x,y to M, decode M, re-encode back to M
static void testEncodeAndDecode(const char *str, double y, double x, int localsolutions, int globalsolutions) {
    char clean[MAX_MAPCODE_RESULT_LEN];
    const char *p, *s;
    int found = 0;
    int tc = 0;
    int len, i, err, nrresults;
    Mapcodes mapcodes;
    double lat, lon;
    int precision = MAX_PRECISION_DIGITS;

    if (y < -90) { y = -90; } else if (y > 90) { y = 90; }

    // if str: determine "precision", territory "tc", and a "clean" copy of str
    if (*str) {
        char territory[MAX_ISOCODE_LEN + 1];
        // find first territory letter in s
        s = str;
        while (*s > 0 && *s <= 32) { s++; }
        // parse territory, if any
        p = strchr(s, ' ');
        len = p ? (int) (p - s) : 0;
        if (p && len <= MAX_ISOCODE_LEN) {
            // copy and recognise territory
            memcpy(territory, s, len);
            territory[len] = 0;
            tc = convertTerritoryIsoNameToCode(territory, 0);
            // make s skip to start of proper mapcode
            s = p;
            while (*s > 0 && *s <= 32) { s++; }
        } else {
            // assume s is the start of the proper mapcode
            *territory = 0;
            tc = convertTerritoryIsoNameToCode("AAA", 0);
        }

        // build normalised version of source string in "clean"
        len = strlen(s);
        while (len > 0 && s[len - 1] > 0 && s[len - 1] <= 32) { len--; }
        i = strlen(territory);
        if (i) {
            strcpy(clean, territory);
            strcat(clean, " ");
            i++;
        }
        if (len + i >= MAX_MAPCODE_RESULT_LEN) {
            len = 0;
        }
        memcpy(clean + i, s, len);
        clean[len + i] = 0;
        // determine precision of the source string
        s = strchr(clean, '-');
        if (s) {
            precision = strlen(s + 1);
        } else {
            precision = 0;
        }
    }

    // test if correct nr of local solutions (if requested)
    if (localsolutions) {
        // encode
        nrresults = encodeLatLonToMapcodes(&mapcodes, y, x, tc, precision);

        nrTests++;
        if (nrresults != localsolutions) {
            nrErrors++;
            printf("*** ERROR *** encode(%0.8f , %0.8f,%d) does not deliver %d local solutions\n",
                   y, x, tc, localsolutions);
            printGeneratedMapcodes("Delivered", &mapcodes);
        }

        // test that EXPECTED solution is there (if requested)
        if (str) {
            nrTests++;
            for (i = 0; i < nrresults; i++) {
                const char *m = mapcodes.mapcode[i];
                if (strstr(m, clean) == m) {
                    found = 1;
                    break;
                }
            }
            if (!found) {
                nrErrors++;
                printf("*** ERROR *** encode(%0.8f , %0.8f) does not deliver \"%s\"\n", y, x, clean);
                printGeneratedMapcodes("Delivered", &mapcodes);
            }
        }
    }


    // test if correct nr of global solutions (if requested)
    if (globalsolutions > 0) {
        nrTests++;
        nrresults = encodeLatLonToMapcodes(&mapcodes, y, x, 0, precision);
        if (nrresults != globalsolutions) {
            nrErrors++;
            printf("*** ERROR *** encode(%0.8f , %0.8f) does not deliver %d global solutions\n", y, x, globalsolutions);
            printGeneratedMapcodes("Delivered", &mapcodes);
        }
    }

    // test all global solutions at all precisions...
    for (precision = 0; precision <= 8; precision++) {
        nrresults = encodeLatLonToMapcodes(&mapcodes, y, x, 0, precision);
        for (i = 0; i < nrresults; i++) {
            const char *str = mapcodes.mapcode[i];

            // check if every solution decodes
            nrTests++;
            err = decodeMapcodeToLatLon(&lat, &lon, str, 0);
            if (err) {
                nrErrors++;
                printf("*** ERROR *** decode('%s') = no result, expected ~(%0.8f , %0.8f)\n", str, y, x);
            }
            else {
                double dm = distanceInMeters(y, x, lat, lon);
                double maxerror = maxErrorInMeters(precision);
                // check if decode is sufficiently close to the encoded coordinate
                nrTests++;
                if (dm > maxerror) {
                    nrErrors++;
                    printf("*** ERROR *** decode('%s') = (%0.8f , %0.8f), which is %0.4f cm way (>%0.4f cm) from (%0.8f , %0.8f)\n",
                           str, lat, lon,
                           dm * 100.0, maxerror * 100.0, y, x);
                }
                else {
                    Mapcodes mapcodesTerritory;
                    Mapcodes mapcodesParent;
                    int tc2 = -1, tcParent = -1, j, found = 0;
                    char *e = strchr(str, ' ');
                    if (e) {
                        *e = 0;
                        tc2 = convertTerritoryIsoNameToCode(str, 0);
                        tcParent = getParentCountryOf(tc2);
                        *e = ' ';
                    }

                    nrTests++;

                    // see if the original mapcode was generated
                    {
                        const int nr = encodeLatLonToMapcodes(&mapcodesTerritory, lat, lon, tc2, precision);
                        for (j = 0; j < nr; j++) {
                            if (strcmp(mapcodesTerritory.mapcode[j], str) == 0) {
                                found = 1;
                                break;
                            }
                        }
                    }
                    // if not: see if the original mapcode was generated for the parent
                    if (!found && (tcParent >= 0)) {
                        const int nr = encodeLatLonToMapcodes(&mapcodesParent, lat, lon, tcParent, precision);
                        for (j = 0; j < nr; j++) {
                            if (strcmp(strchr(mapcodesParent.mapcode[j], ' '), strchr(str, ' ')) == 0) {
                                found = 1;
                                break;
                            }
                        }
                    }

                    if (!found) { // within 7.5 meters, but not reproduced!
                        if (!multipleBordersNearby(lat, lon, tc2)) { // but SHOULD be reproduced!
                            nrErrors++;
                            printf("*** ERROR *** %s does not re-encode (%0.15f,%0.15f) from (%0.15f,%0.15f)\n",
                                   str, lat, lon, y, x);
                            printGeneratedMapcodes("Global   ", &mapcodes);
                            printGeneratedMapcodes("Territory", &mapcodesTerritory);
                            if (tcParent >= 0) {
                                printGeneratedMapcodes("Parent   ", &mapcodesParent);
                            }
                        }
                    }
                }
            }
        }
    }
}

// test strings that are expected to FAIL a decode
static test_failing_decodes() {
    static const char *badcodes[] = {

            "",              // empty
            "NLD 00.00",     // all-digits
            "12345.6789",    // all-digits
            "12345.6789-X",  // all-digits
            "GGG XX.XX",     // unknown country
            "GGG-GG XX.XX",  // unknown country
            "NLDX XX.XX",    // unknown/long country
            "NLDNLDNLD XX.XX", // unknown/long country
            "USAUSA-CA XX.XX", // unknown/long country
            "USA-CACA XX.XX",  // unknown/long state
            "US-CACACA XX.XX", // unknown/long state
            "US-US XX00.XX00",     // parent as state
            "US-RU XX00.XX00",     // parent as state
            "CA-CA XX00.XX00",     // state as country
            "US-GG XX.XX",   // unknown state (anywhere)
            "RU-CA XX.XX",   // unknown state (in RU)
            "RUS-CA XX.XX",  // unknown state (in RUS)
            "NLD-CA XX.XX",  // unknown state (NL has none)
            "NLD X.XXX",     // short prefix
            "NLD XXXXXX.XX", // long prefix
            "NLD XXX.X",     // short postfix
            "NLD XXX.XXXXX", // long postfix
            "NLD XXXXX.XXX", // invalid codex 5+3
            "NLD XXXX.XXXX", // non-existing codex in NLD
            "NLD XXXX",      // no dot
            "NLD XXXXX",     // no dot
            "NLD XXX.",      // no postfix
            "NLD .XXX",      // no prefix
            "AAA x234.6789", // too short for AAA
            "x234.6789",     // too short for AAA

            "NLD XXX..XXX",  // 2 dots
            "NLD XXX.XX.X",  // 2 dots

            "NLD XX.XX-Z",   // Z in extension
            "NLD XX.XX-1Z",  // Z in extension
            "NLD XX.XX-X-",  // 2nd -
            "NLD XX.XX-X-X", // 2nd -

            // "NLD XXX.XXX-",  // empty extension ALLOWED!

            "NLD XX.XX-123456789", // extension too long
            "NLD XXX.#XX",   // invalid char
            "NLD XXX.UXX",   // invalid char
            "NLD 123.A45",   // A in invalid position
            "NLD 123.E45",   // E in invalid position
            "NLD 123.U45",   // U in invalid position
            "NLD 123.1UE",   // UE illegal vowel-encode
            "NLD 123.1UU",   // UU illegal
            "NLD x23.1A0",   // A0 with nondigit
            "NLD 1x3.1A0",   // A0 with nondigit
            "NLD 12x.1A0",   // A0 with nondigit
            "NLD 123.xA0",   // A0 with nondigit
            "NLD 123.1U#",   // U#

            "NLD ZZ.ZZ",     // nameless out of range
            "NLD Q000.000",  // grid out of range
            "NLD ZZZ.ZZZ",   // grid out of range
            "NLD SHH.HHH",   // grid out of encompassing
            "NLD L222.222",  // grid out of range (restricted)
            NULL
    };
    int i;
    for (i = 0; badcodes[i] != NULL; i++) {
        double lat, lon;
        const char *str = badcodes[i];
        int err;

        nrTests++;
        err = decodeMapcodeToLatLon(&lat, &lon, str, 0);
        if (err >= 0) {
            nrErrors++;
            printf("*** ERROR *** invalid mapcode \"%s\" decodes without error\n", str);
        }
        //printf("Confirm %d for decode(%s)\n",err,str);
    }
}

// perform testEncodeAndDecode for all elements of encode_test[] (from decode_test.h)
void encode_decode_tests() {
    int i, nr = 0;
    for (i = 0; encode_test[i].mapcode != NULL; i++) {
        nr++;
    }
    printf("%d encodes\n", nr);
    for (i = 0; i < nr; i++) {
        const encode_test_record *t = &encode_test[i];
        if ((i & 255) == 0) { fprintf(stderr, "%0.1f%%\r", i * 100.0 / nr); }
        testEncodeAndDecode(t->mapcode, t->latitude, t->longitude, t->nr_local_mapcodes, t->nr_global_mapcodes);
    }
}

// perform tests on alphacodes (designed in test_territories.c)
void test_territory(const char *alphaCode, int tc, int isAlias, int needsParent, int tcParent) {

    unsigned int i;
    for (i = 0; i <= strlen(alphaCode); i++) {
        char alphacode[8];
        int tn;
        strcpy(alphacode, alphaCode);
        if (!needsParent && (i == 0)) {
            tn = convertTerritoryIsoNameToCode(alphacode, 0);
            nrTests++;
            if (tn != tc) {
                nrErrors++;
                printf("*** ERROR *** convertTerritoryIsoNameToCode('%s')=%d but expected %d (%s)\n",
                       alphacode, tn, tc, convertTerritoryCodeToIsoName(tc, 0));
            }
        }
        alphacode[i] = (char) tolower(alphacode[i]);
        tn = convertTerritoryIsoNameToCode(alphacode, tcParent);
        nrTests++;
        if (tn != tc) {
            nrErrors++;
            printf("*** ERROR *** convertTerritoryIsoNameToCode('%s',%s)=%d but expected %d\n", alphacode,
                   tcParent ? convertTerritoryCodeToIsoName(tcParent, 0) : "", tn, tc);
        }
    }

    if (tcParent == 0 && !isAlias && (strlen(alphaCode) <= 3 || alphaCode[3] != '-')) {
        char nam[8];
        getTerritoryIsoName(nam, tc, 0);
        nrTests++;
        if (!strstr(nam, alphaCode)) { // @@@ why strstr
            nrErrors++;
            printf("*** ERROR *** getTerritoryIsoName(%d)=\"%s\" which does not equal or contain \"%s\"\n",
                   tc, nam, alphaCode);
        }
    }

}


// test closely around a particular coordinate
static void test_around(double y, double x) {
    testEncodeAndDecode("", y + 0.00001, x + 0.00001, 0, 0);
    testEncodeAndDecode("", y + 0.00001, x, 0, 0);
    testEncodeAndDecode("", y + 0.00001, x - 0.00001, 0, 0);

    testEncodeAndDecode("", y, x + 0.00001, 0, 0);
    testEncodeAndDecode("", y, x, 0, 0);
    testEncodeAndDecode("", y, x - 0.00001, 0, 0);

    testEncodeAndDecode("", y - 0.00001, x + 0.00001, 0, 0);
    testEncodeAndDecode("", y - 0.00001, x, 0, 0);
    testEncodeAndDecode("", y - 0.00001, x - 0.00001, 0, 0);
}

// test around all centers and corners of all territory rectangles
static void re_encode_tests() {
    int ccode, m;
    int nrrecords = lastrec(ccode_earth) + 1;
    printf("%d records\n", nrrecords);
    for (ccode = 0; ccode <= ccode_earth; ccode++) {
        for (m = firstrec(ccode); m <= lastrec(ccode); m++) {
            double y, x, midx, midy, thirdx;
            const mminforec *b = boundaries(m);

            fprintf(stderr, "%0.2f%%\r", m * 100.0 / nrrecords);

            midy = (b->miny + b->maxy) / 2000000.0;
            midx = (b->minx + b->maxx) / 2000000.0;
            thirdx = (2 * b->minx + b->maxx) / 3000000.0;
            test_around(midy, midx);

            y = (b->miny) / 1000000.0;
            x = (b->minx) / 1000000.0;
            test_around(y, x);
            test_around(midy, x);
            test_around(y, midx);
            test_around(y, thirdx);

            x = (b->maxx) / 1000000.0;
            test_around(y, x);
            test_around(midy, x);

            y = (b->maxy) / 1000000.0;
            x = (b->minx) / 1000000.0;
            test_around(y, x);
            test_around(y, midx);

            x = (b->maxx) / 1000000.0;
            test_around(y, x);
            test_around(midy, x);
        }
    }
}


void distance_tests() {
    if (strcmp(mapcode_cversion, "2.1.3") >= 0) {
        int i;
        double coordpairs[] = {
                // lat1, lon1, lat2, lon2, expected distance * 100000
                1, 1, 1, 1, 0,
                0, 0, 0, 1, 11131949079,
                89, 0, 89, 1, 194279300,
                3, 0, 3, 1, 11116693130,
                -3, 0, -3, 1, 11116693130,
                -3, -179.5, -3, 179.5, 11116693130,
                -3, 179.5, -3, -179.5, 11116693130,
                3, 8, 3, 9, 11116693130,
                3, -8, 3, -9, 11116693130,
                3, -0.5, 3, 0.5, 11116693130,
                54, 5, 54.000001, 5, 11095,
                54, 5, 54, 5.000001, 6543,
                54, 5, 54.000001, 5.000001, 12880,
                90, 0, 90, 50, 0,
                0.11, 0.22, 0.12, 0.2333, 185011466,
                -1
        };

        for (i = 0; coordpairs[i] != -1; i += 5) {
            const double distance = distanceInMeters(
                    coordpairs[i], coordpairs[i + 1],
                    coordpairs[i + 2], coordpairs[i + 3]);
            nrTests++;
            if (floor(0.5 + (100000.0 * distance)) != coordpairs[i + 4]) {
                nrErrors++;
                printf("*** ERROR *** distanceInMeters %d failed: %f\n", i, distance);;
            }
        }
    }
}


void test_territory_insides() {
    if (strcmp(mapcode_cversion, "2.1.5") >= 0) {
        int i;
        struct {
            const char *territory;
            double lat;
            double lon;
            int nearborders;
        } iTestData[] = {
                {"AAA",    0,               0,                0},
                {"AAA",    0,               999,              0},
                {"AAA",    90,              0,                0},
                {"AAA",    -90,             0,                0},
                {"AAA",    0,               180,              0},
                {"AAA",    0,               -180,             0},
                {"ATA",    -90,             0,                1}, // ATA -90,0 has 2 borders as of data version 2.2
                {"ATA",    -70,             0,                0},

                {"USA",    31,              -70,              0}, // interational waters (not in state)
                {"MEX",    19,              -115,             0}, // interational waters (not in state)
                {"MEX",    18.358525,       -114.722672,      0}, // Isla Clarion, not in a state
                {"MX-ROO", 20,              -87,              0},  // just in ROO
                {"MX-ROO", 20,              -87.3,            0}, // in ROO because in MEX
                {"MEX",    20,              -87.3,            0}, // in ROO because in MEX

                {"IND",    19,              87,               0},

                {"NLD",    52.6,            4.8,              0},
                {"US-WV",  40.18,           -80.87,           0},
                {"USA",    40.18,           -80.87,           0},
                {"US-FL",  24.7,            -82.7,            0},
                {"USA",    24.7,            -82.7,            0},
                {"IN-TG",  16.13,           78.75,            0},
                {"IN-AP",  16.13,           78.75,            0},
                {"IN-MH",  16.13,           78.75,            0},
                {"IN-PY",  16.13,           78.75,            0},
                {"IND",    16.13,           78.75,            0},
                {"USA",    40.7,            -74,              0},

                {"US-NY",  40.7,            -74,              1},
                {"MEX",    20.252060,       -89.779821,       1},
                {"NLD",    52.467314,       4.494037,         1},
                {"MEX",    21.431778909671, -89.779828861356, 1},
                {"MEX",    21.431788272457, -89.779820144176, 1},

                {NULL}
        };

        for (i = 0; iTestData[i].territory != NULL; i++) {
            int territory = convertTerritoryIsoNameToCode(iTestData[i].territory, 0);
            nrTests++;
            if (multipleBordersNearby(iTestData[i].lat, iTestData[i].lon, territory) != iTestData[i].nearborders) {
                nrErrors++;
                printf("*** ERROR *** multipleBordersNearby(%+18.13f,%+18.13f, \"%s\") not %d\n",
                       iTestData[i].lat, iTestData[i].lon, iTestData[i].territory, iTestData[i].nearborders);
            }
        }
    }
}


void main() {
#ifdef XSIDE3    
    const char *mapcode_dataversion = "undefined";
#endif
    printf("Mapcode C Library Unit test %s\n", UNITTEST_VERSION);
    printf("Library version %s (Data version %s)\n", mapcode_cversion, mapcode_dataversion);

    printf("-----------------------------------------------------------\nAlphabet tests\n");
    alphabet_tests();

    printf("-----------------------------------------------------------\nDistance tests\n");
    distance_tests();

    printf("-----------------------------------------------------------\nTerritory tests\n");
    printf("%d territories\n", MAX_CCODE);
    test_territories();
    test_territory_insides();

    printf("-----------------------------------------------------------\nFailing decode tests\n");
    test_failing_decodes();

    printf("-----------------------------------------------------------\nFailing decodes tests\n");
    test_failing_decodes();

    printf("-----------------------------------------------------------\nEncode/Decode tests\n");
    {
        //clock_t c_start = clock();
        encode_decode_tests();
        //fprintf(stderr,"%ld time\n", (clock() - c_start));
    }

    printf("-----------------------------------------------------------\nRe-encode tests\n");
    re_encode_tests();

    printf("-----------------------------------------------------------\n");
    printf("Done.\nExecuted %ld tests, found %ld errors", nrTests, nrErrors);
    if (nrWarnings) {
        printf(", %ld warnings\n", nrWarnings);
    }
    printf("\n");
    getchar();
}
