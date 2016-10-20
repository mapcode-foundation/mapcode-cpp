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

/**
 * This application performs a number of tests on the Mapcode C library.
 * It helps to establish that all routines work properly.
 */

#include <stdio.h>
#include <math.h>
#include <time.h>

#include "../mapcodelib/mapcoder.c"
#include "../mapcodelib/mapcode_countrynames_short.h"
#include "test_territories.c"
#include "decode_test.h"

// If your platform does not support pthread.h, either add -DNO_POSIX_THREADS
// to your compiler command-line, or uncomment the following line:
// #define NO_POSIX_THREADS

#ifdef NO_POSIX_THREADS

// Fake implementation of pthread to not use threads at all:
#define pthread_mutex_lock(ignore)      
#define pthread_mutex_unlock(ignore)
#define pthread_mutex_t int
#define PTHREAD_MUTEX_INITIALIZER 0
#define pthread_t int
#define pthread_join(ignore1, ignore2) 0
#define pthread_create(ignore1, ignore2, func, context) func(context)
#define MAX_THREADS 1
#else

#include <pthread.h>

#define MAX_THREADS 16      // Optimal: not too much, approx. nr of cores * 2, better no more than 32.
#endif

#define MAXLINESIZE 1024

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static int nrErrors = 0;

static void found_error(void) {
    pthread_mutex_lock(&mutex);
    ++nrErrors;
    pthread_mutex_unlock(&mutex);
}

// test the alphabet conversion routines
static int alphabet_tests(void) {
    int nrTests = 0;
    int i, j;
    const char *str, *expect;
    static const char *testpairs[] = {
            "mx XX.XX", "mx XX.XX",
            ".123", ".123",
            "49.4V", "49.4V",
            "00.E0", "00.E0",
            "X123.P456", "X123.P456",
            "z789.b012", "Z789.B012",
            "", "",
            "-", "-",
            ".", ".",
            "-123", "-123",
            "-xyz", "-XYZ",
            ".xyz", ".XYZ",
            "12.34", "12.34",
            "56.78", "56.78",
            "90.01", "90.01",
            "OEUoi OIoi#%?-.abcdfghjklmnpqrstvwxyz0123456789euEUABCDFGHJKLMNPQRSTVWXYZ",
            "OEUoi OIOI#%?-.ABCDFGHJKLMNPQRSTVWXYZ0123456789EUEUABCDFGHJ",
            "OEUoi OIoi#%?abcdfghjklmnpqrstvwxyz0123456789euEUABCDFGHJKLMNPQRSTVWXYZ",
            "OEUoi OIOI#%?ABCDFGHJKLMNPQRSTVWXYZ0123456789EUEUABCDFGHJKL",
            "  Oio 12.AU  ", "Oio 12.AU",
            "OMN 112.3EU", "OMN 112.3EU",
            "49.4V", "49.4V",
            "NLD 49.4V-xx123", "NLD 49.4V-XX123",
            "A12.345", "112.3AU",
            "xx.xx", "XX.XX",
            "xx.xxx", "XX.XXX",
            "xxx.xx", "XXX.XX",
            "xx.xxxx", "XX.XXXX",
            "xxx.xxx", "XXX.XXX",
            "xxxx.xx", "XXXX.XX",
            "xxx.xxxx", "XXX.XXXX",
            "xxxx.xxx", "XXXX.XXX",
            "xxxx.xxxx", "XXXX.XXXX",
            "xxxxx.xxxx", "XXXXX.XXXX",
            "pq.rs", "PQ.RS",
            "bc.123", "BC.123",
            "123.xy", "123.XY",
            " nld 12.34E0", "nld 12.34E0",
            "VVX.xxx", "VVX.XXX",
            "x123.xx", "X123.XX",
            "xxx.xxxx", "XXX.XXXX",
            "12xx.xxx", "12XX.XXX",
            "xxxx.xx12", "XXXX.XX12",
            "99zxx.xxxx", "99ZXX.XXXX",
            "xx.xx-P", "XX.XX-P",
            "xx.xxx-pq", "XX.XXX-PQ",
            "xxx.xx-123", "XXX.XX-123",
            "xx.xxxx-pqRS", "XX.XXXX-PQRS",
            "xxx.xxx-PQRSTZVW", "XXX.XXX-PQRSTZVW",
            "xxxx.xx-pqrstZvw", "XXXX.XX-PQRSTZVW",
            "xxx.xxxx-PQrsTZ", "XXX.XXXX-PQRSTZ",
            "xxxx.xxx-09876543", "XXXX.XXX-09876543",
            "xxxx.xxxx-PQRSTZVW", "XXXX.XXXX-PQRSTZVW",
            "xxxxx.xxxx-PQRSTZVW", "XXXXX.XXXX-PQRSTZVW",
            "pq.rs-PQRSTZVW", "PQ.RS-PQRSTZVW",
            "bc.123-PQRSTZVW", "BC.123-PQRSTZVW",
            "123.xy-PQRSTZVW", "123.XY-PQRSTZVW",
            "12.34E0-PQRSTZVW", "12.34E0-PQRSTZVW",
            "VVX.xxx-PQRSTZVW", "VVX.XXX-PQRSTZVW",
            "x123.xx-PQRSTZVW", "X123.XX-PQRSTZVW",
            "xxx.xxxx-PQRSTZVW", "XXX.XXXX-PQRSTZVW",
            "12xx.xxx-PQRSTZVW", "12XX.XXX-PQRSTZVW",
            "xxxx.xx12-PQRSTZVW", "XXXX.XX12-PQRSTZVW",
            "99zxx.xxxx-PQRSTZVW", "99ZXX.XXXX-PQRSTZVW",
            0
    };

    printf("%d alphabets\n", MAPCODE_ALPHABETS_TOTAL);

    for (j = 0; testpairs[j] != 0; j += 2) {
        for (i = 0; i < MAPCODE_ALPHABETS_TOTAL; i++) {
            UWORD enc[64];
            char dec[64];
            // see if alphabets (re)convert as expected
            str = testpairs[j];
            expect = testpairs[j + 1];
            convertToAlphabet(enc, 64, str, i);
            convertToRoman(dec, 60, enc);
            ++nrTests;
            if (strcmp(dec, expect)) {
                found_error();
                printf("*** ERROR *** convertToRoman(convertToAlphabet(\"%s\",%d))=\"%s\"\n", str, i, dec);
            }
        }
    }
    return nrTests;
}


// test the alphabet conversion routines
static int test_mapcode_formats(void) {
    int nrTests = 0;
    int i;
    static const char *testpairs[] = {
            "WLF 01.AE-09V", "WLF 01.AE-09V|22",
            "01.AE", "01.AE|22",
            "CUB 3467.UY", "CUB 3467.UY|42",
            "34.UY", "34.UY|22",
            "mx XX.XX", "MX XX.XX|22",
            "", "", // empty
            "MAP.CODE", "", // vowels
            "XAXX.XXXX", "", // vowels
            "XXAX.XXXX", "", // vowels
            "XXXA.XXXX", "", // vowels
            "XXXAX.XXXX", "", // vowels
            "XXXXA.XXXX", "", // vowels
            "XXXX.AXXX", "", // vowel plus more than one token
            "2A22.2222", "", // vowels
            "22A2.2222", "", // vowels
            "222A.2222", "", // vowels
            "222A2.2222", "", // vowels
            "2222A.2222", "", // vowels
            "2222.A22", "-1102", // vowel plus more than one token
            "2222.A222", "-1102", // vowel plus more than one token
            "2222.2A22", "-1102", // vowel plus more than one token
            "2222.2AAA", "-1103", // 2 vowels plus more tokens
            "A222.2AAA", "-1103", // 2 vowels plus more tokens
            "2222.22A2", "2222.22A2|44", //
            "2222.22AA", "2222.22AA|44", //
            "A222.22AA", "A222.22AA|44", //
            ".123", "", // bad dot
            ".xyz", "", // bad dot
            "x.xyz", "", // bad dot
            "xxx.z-12", "", // bad dot
            "xx.xx.", "", // two dots

            "123", "", // no dot OR incomplete
            "xxx.z", "", // bad dot OR incomplete
            "NLD 49.4V-", "", // incomplete

            "NLD 49.4V", "NLD 49.4V|22",
            "   NLD   49.4V  ", "NLD 49.4V|22",
            "NLD 49.4V-1", "NLD 49.4V-1|22",
            "NLD 49.4V-12", "NLD 49.4V-12|22",
            "NLD 49.4V-123", "NLD 49.4V-123|22",
            "NLD 49.4V-12345678", "NLD 49.4V-12345678|22",

            "NLD 49.4V-123456789", "", // extension too long
            "NLD 49.4V-123456789123456789", "", // extension too long

            "XAX 49.4V", "XAX 49.4V|22",
            "XXA 49.4V", "XXA 49.4V|22",
            "XA 49.4V", "XA 49.4V|22",

            "N 49.4V", "", // bad territory
            "XXXX 49.4V", "", // bad territory
            "XXXXX 49.4V", "", // bad territory
            "-XX 49.4V", "", // bad territory
            "X-XX 49.4V", "", // bad territory
            "XXXX-XX 49.4V", "", // bad territory
            "XX-X 49.4V", "", // bad territory
            "XX-XXXX 49.4V", "", // bad territory

            "12.34", "", // digits only
            "NLD 12.34", "", // digits only
            "AAA 12.34", "", // digits only
            "xx-xx 12.34", "", // digits only
            "12-34 12.34", "", // digits only
            "12-34 12.3X", "12-34 12.3X|22",

            "  TER  XX.XX-XX  ", "TER XX.XX-XX|22",
            "  TER  XXX.XX-XX  ", "TER XXX.XX-XX|32",
            "  TER  XX.XXX-XX  ", "TER XX.XXX-XX|23",
            "  TER  XX.XXXX-XX  ", "TER XX.XXXX-XX|24",
            "  TER  XXX.XXX-XX  ", "TER XXX.XXX-XX|33",
            "  TER  XXXX.XX-XX  ", "TER XXXX.XX-XX|42",
            "  TER  XXX.XXXX-XX  ", "TER XXX.XXXX-XX|34",
            "  TER  XXXX.XXX-XX  ", "TER XXXX.XXX-XX|43",
            "  TER  XXXX.XXXX-XX  ", "TER XXXX.XXXX-XX|44",
            "  TER  XXXXX.XXXX-XX  ", "TER XXXXX.XXXX-XX|54",

            "  TER  XXXXX.XXX-XX  ", "TER XXXXX.XXX-XX|53", // illegal but NOT recognised

            "  TER  XX.XXXXX-XX  ", "",  // too many chars after dot
            "  TER  XXX.XXXXX-XX  ", "",  // too many chars after dot
            "  TER  XXXX.XXXXX-XX  ", "",  // too many chars after dot
            "  TER  XXXXX.XXXXX-XX  ", "",  // too many chars after dot

            "xx-xx.x xx.xx", "",  // dot in territory
            "xx-xx-x xx.xx", "",  // second hyphen in territory
            "xx.xx-x-x", "",  // second hyphen in mapcode
            "xx-xx xx-xx", "",  // no dot in mapcode
            "xx-xx xx-xx.xx", "",  // hyphen before dot (or no dot) in mapcode
            "xx.xx.xx", "",  // second dot in mapcode
            "xx-xx xx.xx.xx", "",  // second dot in mapcode
            "xx-xx xx.xx-xx-xx", "",  // second hyphen in mapcode
            "xx-xx xx.xx x", "",  // debris after mapcode
            "xx-xx xx.xx-x x", "",  // debris after mapcode
            "xx-xx xx.xx-x -", "",  // debris after mapcode
            "xx-xx xx.xx-x .", "",  // debris after mapcode
            "xx-xx xx.xx-x 2", "",  // debris after mapcode
            "xx-xx xx.x#x", "",  // bad char in mapcode
            "xx# xx.xx", "",  // bad char in territory
            "xx-xx -xx.xx", "",  // unexpected hyphen at start of mapcode
            "xx-xx .xx.xx", "",  // unexpected dot at start of mapcode
            "xx-xx #xx.xx", "",  // unexpected char at start of mapcode

            // all possible errors

            ".123", "-1001",  // dot start
            "  .123", "-1001",  // dot start
            "", "-1004",  // empty
            "  ", "-1004",  // empty
            "-xx.xx", "-1005",  // hyphen start
            "  - xx.xx", "-1005",  // hyphen start

            "D xx.xx", "-1010",  // bad territory
            "D.123", "-1011",  // not enough before dot
            "D", "-1014",  // zero
            "D-xxxxx", "-1015",  // hyphen

            "DD", "-1024",  // zero

            "DDDa.DDD", "-1033",  // vowel
            "DDD", "-1034",  // zero

            "DDDD xx.xx", "-1040",  // white
            "DDDDE.xxxx", "-1043",  // vowel
            "DDDD", "-1044",  // zero
            "DDDD-CA xx.xx", "-1045",  // hyphen

            "DDDDD CA xx.xx", "-1050",  // white
            "DDDDDD   xx.xx", "-1052",  // letter
            "DDDDDA   xx.xx", "-1053",  // vowel
            "DDDDD", "-1054",  // zero
            "DDDDD-CA xx.xx", "-1055",  // hyphen

            "DDDDD. xxxx.xx", "-1060",  // white
            "DDDDD..xxxx", "-1061",  // dot
            "DDDDD.", "-999",  // ***PARTIAL***
            "DDDDD.-xxxx.xx", "-1065",  // hyphen

            "DDD.L         ", "-1070",  // white
            "DDD.L.LLL     ", "-1071",  // dot
            "DDD.L", "-999",  // ***PARTIAL***
            "DDD.L-xxxxxxxx", "-1075",  // hyphen

            "DD.DD.CA", "-1081",  // dot

            "DD.DDD.CA", "-1091",  // dot

            "DD.DDDD.CA    ", "-1101",  // dot
            "DD.DDDDD      ", "-1102",  // letter
            "DD.DDDDA      ", "-1103",  // vowel

            "DD.DD-        ", "-1110",  // white
            "DD.DD-.       ", "-1111",  // dot
            "DD.DD-A", "-1113",  // vowel
            "DD.DD-", "-999",  // ***PARTIAL***
            "DD.DD--XXX", "-1115",  // hyphen

            "DD.DD-x.      ", "-1121",  // dot
            "DD.DD-xA", "-1123",  // vowel
            "DD.DD-x-xxx", "-1125",  // hyphen

            "ta.xx     ", "-1131",  // dot
            "ta", "-1134",  // zero

            "DAD-        ", "-1140",  // white
            "DAD-.       ", "-1141",  // dot
            "DAD-", "-1144",  // zero
            "DAD--XXX", "-1145",  // hyphen

            "DAD-X  xx.xx", "-1150",  // white
            "DAD-X.      ", "-1151",  // dot
            "DAD-X", "-1154",  // zero
            "DAD-X-XXX", "-1155",  // hyphen

            "DAD-XX.XX   ", "-1161",  // dot
            "DAD-XX", "-1164",  // zero
            "DAD-XX-XX", "-1165",  // hyphen

            "DAD-XXX.XX   ", "-1171",  // dot
            "DAD-XXXX", "-1172",  // letter
            "DAD-XXXA", "-1173",  // vowel
            "DAD-XXX", "-1174",  // zero
            "DAD-XXX-XX", "-1175",  // hyphen

            "DAD-XX  .XX   ", "-1181",  // dot
            "DAD-XX  ", "-1184",  // zero
            "DAD-XX  -XX", "-1185",  // hyphen

            "DD-DD A      ", "-1190",  // white
            "DD-DD A.     ", "-1191",  // dot
            "DD-DD AA.33  ", "-1193",  // vowel
            "DD-DD A", "-1194",  // zero
            "DD-DD A-XX", "-1195",  // hyphen

            "DD-DD A3     ", "-1200",  // white
            "DD-DD A3A.XX ", "-1203",  // vowel
            "DD-DD A3", "-1204",  // zero
            "DD-DD A3-XX", "-1205",  // hyphen

            "DD-DD A33    ", "-1210",  // white
            "DD-DD A33A.XX", "-1213",  // vowel
            "DD-DD A33", "-1214",  // zero
            "DD-DD A33-XX", "-1215",  // hyphen

            "DD-DD xx.xx .", "-1221",  // dot
            "DD-DD xx.xx x", "-1222",  // letter
            "DD-DD xx.xx a", "-1223",  // vowel
            "DD-DD xx.xx -", "-1225",  // hyphen

            "xx.xx .", "-1221",  // dot
            "xx.xx x", "-1222",  // letter
            "xx.xx a", "-1223",  // vowel
            "xx.xx -", "-1225",  // hyphen

            " xx.xx-DD .", "-1221",  // dot
            " xx.xx-DD x", "-1222",  // letter
            " xx.xx-DD a", "-1223",  // vowel
            " xx.xx-DD -", "-1225",  // hyphen

            "tta.ttt    ", "-1231",  // dot
            "ttat.tt    ", "-1232",  // letter
            "ttaa.ttt   ", "-1233",  // vowel
            "tta", "-1234",  // zero

            NULL, NULL
    };
    int shouldSucceed = 29; // Number of calls to parse() that should be successful.
    int total = 0;
    int succeeded = 0;
    for (i = 0; testpairs[i] != NULL; i += 2) {
        char str[MAX_MAPCODE_RESULT_LEN + 16];
        MapcodeElements mapcodeElements;
        int result = parseMapcodeString(&mapcodeElements, testpairs[i], 1, 0);
        int format = compareWithMapcodeFormat(testpairs[i], 1);

        nrTests++;
        if ((!result && format) || (result && !format)) {
            found_error();
            printf("*** ERROR *** parseMapcodeString=%d, compareWithMapcodeFormat=%d\n", result, format);
        }

        nrTests++;
        ++total;
        if (result == 0) {
            ++succeeded;
            sprintf(str, "%s%s%s%s%s|%d",
                    mapcodeElements.territoryISO,
                    *mapcodeElements.territoryISO ? " " : "",
                    mapcodeElements.properMapcode,
                    *mapcodeElements.precisionExtension ? "-" : "",
                    mapcodeElements.precisionExtension,
                    (mapcodeElements.indexOfDot * 9) + (int) strlen(mapcodeElements.properMapcode) - 1);
            if (strcmp(str, testpairs[i + 1]) != 0) {
                found_error();
                printf("*** ERROR *** parseMapcodeString(\"%s\") succeeded with \"%s\"\n", testpairs[i], str);
            }
        } else {
            sprintf(str, "%d", result);
            if (testpairs[i + 1][0] != 0 && strcmp(str, testpairs[i + 1]) != 0) {
                found_error();
                printf("*** ERROR *** compareWithMapcodeFormat(\"%s\") failed unexpectedly %d\n", testpairs[i], result);
            }
        }
    }
    if (succeeded != shouldSucceed) {
        found_error();
        printf("*** ERROR *** Too few parseMapcodeString() calls succeeded (%d of %d, expected %d)\n", succeeded, total, shouldSucceed);
    }
    return nrTests;
}


// Show progress.
static void show_progress(int at, int max, int nrTests) {
    static clock_t prevTick = 0;

    // No worries, clock() is a very fast call.
    clock_t tick = clock() / (CLOCKS_PER_SEC / 2);
    if (tick != prevTick) {
        prevTick = tick;

        // Use stderr to not pollute logs.
        fprintf(stderr, "\r%0.1f%% (executed %0.1fM tests)\r", (at * 100.0) / max, nrTests / 1000000.0);
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
static int testEncodeAndDecode(const char *str, double y, double x, int localsolutions, int globalsolutions) {
    int nrTests = 0;
    char clean[MAX_MAPCODE_RESULT_LEN];
    const char *p, *s;
    int found = 0;
    int tc = 0;
    int len, i, err, nrresults;
    Mapcodes mapcodes;
    double lat, lon;
    int precision = MAX_PRECISION_DIGITS;

    if (y < -90) {
        y = -90;
    } else if (y > 90) {
        y = 90;
    }

    // if str: determine "precision", territory "tc", and a "clean" copy of str
    if (*str) {
        char territory[MAX_ISOCODE_LEN + 1];
        // find first territory letter in s
        s = str;
        while (*s > 0 && *s <= 32) {
            s++;
        }
        // parse territory, if any
        p = strchr(s, ' ');
        len = p ? (int) (p - s) : 0;
        if (p && len <= MAX_ISOCODE_LEN) {
            // copy and recognise territory
            memcpy(territory, s, len);
            territory[len] = 0;
            tc = getTerritoryCode(territory, 0);
            // make s skip to start of proper mapcode
            s = p;
            while (*s > 0 && *s <= 32) {
                s++;
            }
        } else {
            // assume s is the start of the proper mapcode
            territory[0] = 0;
            tc = getTerritoryCode("AAA", 0);
        }

        // build normalised version of source string in "clean"
        len = (int) strlen(s);
        while (len > 0 && s[len - 1] > 0 && s[len - 1] <= 32) {
            len--;
        }
        i = (int) strlen(territory);
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
            precision = (int) strlen(s + 1);
        } else {
            precision = 0;
        }
    }

    // test if correct nr of local solutions (if requested)
    if (localsolutions) {
        // encode
        nrresults = encodeLatLonToMapcodes(&mapcodes, y, x, tc, precision);

        ++nrTests;
        if (nrresults != localsolutions) {
            found_error();
            printf("*** ERROR *** encode(%0.8f, %0.8f,%d) does not deliver %d local solutions\n",
                   y, x, tc, localsolutions);
            printGeneratedMapcodes("Delivered", &mapcodes);
        }

        // test that EXPECTED solution is there (if requested)
        ++nrTests;
        for (i = 0; i < nrresults; i++) {
            const char *m = mapcodes.mapcode[i];
            if (strstr(m, clean) == m) {
                found = 1;
                break;
            }
        }
        if (!found) {
            found_error();
            printf("*** ERROR *** encode(%0.8f, %0.8f) does not deliver \"%s\"\n", y, x, clean);
            printGeneratedMapcodes("Delivered", &mapcodes);
        }
    }


    // test if correct nr of global solutions (if requested)
    if (globalsolutions > 0) {
        ++nrTests;
        nrresults = encodeLatLonToMapcodes(&mapcodes, y, x, 0, precision);
        if (nrresults != globalsolutions) {
            found_error();
            printf("*** ERROR *** encode(%0.8f, %0.8f) does not deliver %d global solutions\n", y, x, globalsolutions);
            printGeneratedMapcodes("Delivered", &mapcodes);
        }
    }

    // test all global solutions at all precisions...
    for (precision = 0; precision <= 8; precision++) {
        nrresults = encodeLatLonToMapcodes(&mapcodes, y, x, 0, precision);
        for (i = 0; i < nrresults; i++) {
            const char *strResult = mapcodes.mapcode[i];

            // check if every solution decodes
            ++nrTests;
            err = decodeMapcodeToLatLon(&lat, &lon, strResult, 0);
            if (err) {
                found_error();
                printf("*** ERROR *** decode('%s') = no result, expected ~(%0.8f, %0.8f)\n", strResult, y, x);
            } else {
                double dm = distanceInMeters(y, x, lat, lon);
                double maxerror = maxErrorInMeters(precision);
                // check if decode is sufficiently close to the encoded coordinate
                ++nrTests;
                if (dm > maxerror) {
                    found_error();
                    printf("*** ERROR *** decode('%s') = (%0.8f, %0.8f), which is %0.4f cm away (>%0.4f cm) from (%0.8f, %0.8f)\n",
                           strResult, lat, lon,
                           dm * 100.0, maxerror * 100.0, y, x);
                } else {
                    Mapcodes mapcodesTerritory;
                    Mapcodes mapcodesParent;
                    int tc2 = -1;
                    int tcParent = -1;
                    int j;
                    char *e = strchr(strResult, ' ');
                    found = 0;
                    if (e) {
                        *e = 0;
                        tc2 = getTerritoryCode(strResult, 0);
                        tcParent = getParentCountryOf(tc2);
                        *e = ' ';
                    }

                    ++nrTests;

                    // see if the original mapcode was generated
                    {
                        const int nr = encodeLatLonToMapcodes(&mapcodesTerritory, lat, lon, tc2, precision);
                        for (j = 0; j < nr; j++) {
                            if (strcmp(mapcodesTerritory.mapcode[j], strResult) == 0) {
                                found = 1;
                                break;
                            }
                        }
                    }
                    // if not: see if the original mapcode was generated for the parent
                    if (!found && (tcParent >= 0)) {
                        const int nr = encodeLatLonToMapcodes(&mapcodesParent, lat, lon, tcParent, precision);
                        for (j = 0; j < nr; j++) {
                            if (strcmp(strchr(mapcodesParent.mapcode[j], ' '), strchr(strResult, ' ')) == 0) {
                                found = 1;
                                break;
                            }
                        }
                    }

                    if (!found) { // within 7.5 meters, but not reproduced!
                        if (!multipleBordersNearby(lat, lon, tc2)) { // but SHOULD be reproduced!
                            found_error();
                            printf("*** ERROR *** %s does not re-encode (%0.15f,%0.15f) from (%0.15f,%0.15f)\n",
                                   strResult, lat, lon, y, x);
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
    return nrTests;
}

// test strings that are expected to FAIL a decode
static int test_failing_decodes(void) {
    int nrTests = 0;
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
            "W9.SX94",       // reported as an error case
            0
    };
    int i;
    for (i = 0; badcodes[i] != 0; i++) {
        double lat, lon;
        const char *str = badcodes[i];
        int err;

        ++nrTests;
        err = decodeMapcodeToLatLon(&lat, &lon, str, 0);
        if (err >= 0) {
            found_error();
            printf("*** ERROR *** invalid mapcode \"%s\" decodes without error\n", str);
        }
    }
    return nrTests;
}

// perform tests on alphacodes (designed in test_territories.c)
int test_territory(const char *alphaCode, int tc, int isAlias, int needsParent, int tcParent) {
    int nrTests = 0;
    unsigned int i;
    for (i = 0; i <= strlen(alphaCode); i++) {
        char alphacode[8];
        int tn;
        strcpy(alphacode, alphaCode);
        if (!needsParent && (i == 0)) {
            tn = getTerritoryCode(alphacode, 0);
            ++nrTests;
            if (tn != tc) {
                found_error();
                printf("*** ERROR *** getTerritoryCode('%s')=%d but expected %d (%s)\n",
                       alphacode, tn, tc, convertTerritoryCodeToIsoName(tc, 0));
            }
        }
        alphacode[i] = (char) tolower(alphacode[i]);
        tn = getTerritoryCode(alphacode, tcParent);
        ++nrTests;
        if (tn != tc) {
            found_error();
            printf("*** ERROR *** getTerritoryCode('%s',%s)=%d but expected %d\n", alphacode,
                   tcParent ? convertTerritoryCodeToIsoName(tcParent, 0) : "", tn, tc);
        }
    }

    if (tcParent == 0 && !isAlias && (strlen(alphaCode) <= 3 || alphaCode[3] != '-')) {
        char nam[8];
        getTerritoryIsoName(nam, tc, 0);
        ++nrTests;
        if (!strstr(nam, alphaCode)) { // @@@ why strstr
            found_error();
            printf("*** ERROR *** getTerritoryIsoName(%d)=\"%s\" which does not equal or contain \"%s\"\n",
                   tc, nam, alphaCode);
        }
    }
    return nrTests;
}


// test closely around a particular coordinate
static int test_around(double y, double x) {
    int nrTests = 0;
    nrTests += testEncodeAndDecode("", y + 0.00001, x + 0.00001, 0, 0);
    nrTests += testEncodeAndDecode("", y + 0.00001, x, 0, 0);
    nrTests += testEncodeAndDecode("", y + 0.00001, x - 0.00001, 0, 0);

    nrTests += testEncodeAndDecode("", y, x + 0.00001, 0, 0);
    nrTests += testEncodeAndDecode("", y, x, 0, 0);
    nrTests += testEncodeAndDecode("", y, x - 0.00001, 0, 0);

    nrTests += testEncodeAndDecode("", y - 0.00001, x + 0.00001, 0, 0);
    nrTests += testEncodeAndDecode("", y - 0.00001, x, 0, 0);
    nrTests += testEncodeAndDecode("", y - 0.00001, x - 0.00001, 0, 0);
    return nrTests;
}


// This context holds a record to process and a return value (nrTests) per thread.
struct context_test_around {
    int nrTests;
    const mminforec *mminfo;
};


static int join_threads(pthread_t *threads, struct context_test_around *contexts, int total) {
    int i = 0;
    int nrTests = 0;
    for (i = 0; i < total; ++i) {
        if (pthread_join(threads[i], 0)) {
            found_error();
            printf("*** ERROR *** Error joining thread %d of %d\n", i, total);
            return 0;

        }
        nrTests += contexts[i].nrTests;
    }
    return nrTests;
}

// perform testEncodeAndDecode for all elements of encode_test[] (from decode_test.h)
static int encode_decode_tests(void) {
    int nrTests = 0;
    int i = 0;
    int nr = sizeof(encode_test) / sizeof(encode_test_record) - 1;
    printf("%d encodes\n", nr);
    for (i = 0; i < nr; i++) {
        const encode_test_record *t = &encode_test[i];
        show_progress(i, nr, nrTests);
        nrTests += testEncodeAndDecode(t->mapcode, t->latitude, t->longitude, t->nr_local_mapcodes,
                                       t->nr_global_mapcodes);
    }
    return nrTests;
}

static void *execute_test_around(void *context) {
    int nrTests = 0;
    double y, x, midx, midy, thirdx;
    struct context_test_around *c = (struct context_test_around *) context;
    const mminforec *b = c->mminfo;

    midy = (b->miny + b->maxy) / 2000000.0;
    midx = (b->minx + b->maxx) / 2000000.0;
    thirdx = (2 * b->minx + b->maxx) / 3000000.0;
    nrTests += test_around(midy, midx);

    y = (b->miny) / 1000000.0;
    x = (b->minx) / 1000000.0;
    nrTests += test_around(y, x);
    nrTests += test_around(midy, x);
    nrTests += test_around(y, midx);
    nrTests += test_around(y, thirdx);

    x = (b->maxx) / 1000000.0;
    nrTests += test_around(y, x);
    nrTests += test_around(midy, x);

    y = (b->maxy) / 1000000.0;
    x = (b->minx) / 1000000.0;
    nrTests += test_around(y, x);
    nrTests += test_around(y, midx);

    x = (b->maxx) / 1000000.0;
    nrTests += test_around(y, x);
    nrTests += test_around(midy, x);
    c->nrTests = nrTests;
    return 0;
}


// test around all centers and corners of all territory rectangles
static int re_encode_tests(void) {
    int nrTests = 0;
    int ccode = 0;
    int m = 0;
    int nrRecords = lastrec(ccode_earth) + 1;
    int nrThread = 0;

    // Declare threads and contexts.
    pthread_t threads[MAX_THREADS];
    struct context_test_around contexts[MAX_THREADS];

    printf("%d records\n", nrRecords);
    for (ccode = 0; ccode <= ccode_earth; ccode++) {
        show_progress(ccode, ccode_earth, nrTests);
        for (m = firstrec(ccode); m <= lastrec(ccode); m++) {
            const mminforec *b = boundaries(m);

            // Create context for thread.
            contexts[nrThread].nrTests = 0;
            contexts[nrThread].mminfo = b;

            // Execute task on new thread.
            if (pthread_create(&threads[nrThread], 0, execute_test_around, (void *) &contexts[nrThread])) {
                found_error();
                printf("*** ERROR *** Cannot create thread\n");
                return 0;
            }

            // Move to next thread in pool. If out of threads, join them and start over.
            nrThread++;
            if (nrThread >= MAX_THREADS) {
                nrTests += join_threads(threads, contexts, nrThread);
                nrThread = 0;
            }
        }
        nrTests += join_threads(threads, contexts, nrThread);
        nrThread = 0;
    }
    return nrTests;
}

static int distance_tests(void) {
    int nrTests = 0;
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
            ++nrTests;
            if (floor(0.5 + (100000.0 * distance)) != coordpairs[i + 4]) {
                found_error();
                printf("*** ERROR *** distanceInMeters %d failed: %f\n", i, distance);;
            }
        }
    }
    return nrTests;
}


static int test_territory_insides(void) {
    int nrTests = 0;
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

                {0}
        };

        for (i = 0; iTestData[i].territory != 0; i++) {
            int territory = getTerritoryCode(iTestData[i].territory, 0);
            ++nrTests;
            if (multipleBordersNearby(iTestData[i].lat, iTestData[i].lon, territory) != iTestData[i].nearborders) {
                found_error();
                printf("*** ERROR *** multipleBordersNearby(%+18.13f,%+18.13f, \"%s\") not %d\n",
                       iTestData[i].lat, iTestData[i].lon, iTestData[i].territory, iTestData[i].nearborders);
            }
        }
    }
    return nrTests;
}

static int territory_code_tests(void) {
    int nrTests = 0;
    int i;

    static const struct {
        int expectedresult;
        int context;
        const char *inputstring;
    } tcTestData[] = {
            {-1,  0,   ""},
            {-1,  0,   "R"},
            {-1,  0,   "RX"},
            {-1,  0,   "RXX"},
            {497, 0,   "RUS"},
            {-1,  0,   "RUSSIA"},
            {411, 0,   "US"},
            {411, 0,   "USA"},
            {411, 0,   "usa"},
            {-1,  0,   "US-TEST"},
            {411, 0,   "US TEST"},
            {392, 0,   "US-CA"},
            {392, 0,   "US-CA TEST"},
            {392, 0,   "USA-CA"},
            {431, 0,   "RUS-TAM"},
            {-1,  0,   "RUS-TAMX"},
            {431, 0,   "RUS-TAM X"},
            {319, 0,   "AL"}, //
            {483, 497, "AL"}, // 497=rus
            {483, 431, "AL"}, // 431=ru-tam
            {365, 411, "AL"}, // 411=usa
            {365, 392, "AL"}, // 392=us-ca
            {0,   0,   0}
    };

    for (i = 0; tcTestData[i].inputstring != 0; i++) {
        int tc = getTerritoryCode(tcTestData[i].inputstring, tcTestData[i].context);
        ++nrTests;
        if (tc != tcTestData[i].expectedresult) {
            found_error();
            printf("*** ERROR *** getTerritoryCode(\"%s\", %d)=%d, expected %d\n",
                   tcTestData[i].inputstring, tcTestData[i].context,
                   tc, tcTestData[i].expectedresult);
        }
    }
    return nrTests;
}


static int check_incorrect_get_territory_code_test(char *tcAlpha) {
    int tc = getTerritoryCode(tcAlpha, 0);
    if (tc >= 0) {
        found_error();
        printf("*** ERROR *** getTerritoryCode returns '%d' (should be < 0) for territory code '%s'\n", tc, tcAlpha);
    }
    return 1;
}


static int get_territory_robustness_tests(void) {
    int nrTests = 0;
    int i;
    char s1[1];
    char largeString[16000];

    nrTests += check_incorrect_get_territory_code_test("UNKNOWN");
    nrTests += check_incorrect_get_territory_code_test("A");
    nrTests += check_incorrect_get_territory_code_test(" A");
    nrTests += check_incorrect_get_territory_code_test("A ");
    nrTests += check_incorrect_get_territory_code_test(" A ");
    nrTests += check_incorrect_get_territory_code_test("AA");
    nrTests += check_incorrect_get_territory_code_test(" AA");
    nrTests += check_incorrect_get_territory_code_test("AA ");
    nrTests += check_incorrect_get_territory_code_test(" AA ");
    nrTests += check_incorrect_get_territory_code_test("US-");
    nrTests += check_incorrect_get_territory_code_test(" US-");
    nrTests += check_incorrect_get_territory_code_test("US- ");
    nrTests += check_incorrect_get_territory_code_test(" US- ");
    nrTests += check_incorrect_get_territory_code_test(" ");

    s1[0] = 0;
    nrTests += check_incorrect_get_territory_code_test(s1);

    for (i = 0; i < sizeof(largeString) - 1; ++i) {
        largeString[i] = (char) ((i % 223) + 32);
    }
    largeString[sizeof(largeString) - 1] = 0;
    nrTests += check_incorrect_get_territory_code_test(largeString);
    return nrTests;
}


static int check_incorrect_encode_test(double lat, double lon, int treatAsError) {
    int nrResults;
    int nrTests = 0;
    Mapcodes mapcodes;
    ++nrTests;
    nrResults = encodeLatLonToMapcodes(&mapcodes, lat, lon, 0, 0);
    if (nrResults > 0) {
        if (treatAsError) {
            found_error();
        }
        printf("*** %s *** encodeLatLonToMapcodes returns '%d' (should be <= 0) for lat=%f, lon=%f\n",
               treatAsError ? "ERROR" : "WARNING", nrResults, lat, lon);
    }
    return nrTests;
}


static int check_correct_encode_test(double lat, double lon, int treatAsError) {
    Mapcodes mapcodes;
    int nrResults = encodeLatLonToMapcodes(&mapcodes, lat, lon, 0, 0);
    if (nrResults <= 0) {
        if (treatAsError) {
            found_error();
        }
        printf("*** %s *** encodeLatLonToMapcodes returns '%d' (should be > 0) for lat=%f, lon=%f\n",
               treatAsError ? "ERROR" : "WARNING", nrResults, lat, lon);
    }
    return 1;
}


static int encode_robustness_tests(void) {
    int nrTests = 0;
    double d;
    unsigned char *b = (unsigned char *) &d;

    nrTests += check_correct_encode_test(-90.0, 0.0, 1);
    nrTests += check_correct_encode_test(90.0, 0.0, 1);
    nrTests += check_correct_encode_test(-91.0, 0.0, 1);
    nrTests += check_correct_encode_test(91.0, 0.0, 1);

    nrTests += check_correct_encode_test(0.0, -180.0, 1);
    nrTests += check_correct_encode_test(0.0, 180.0, 1);
    nrTests += check_correct_encode_test(1.0, -181.0, 1);
    nrTests += check_correct_encode_test(0.0, 181.0, 1);

    // NAN - See: https://en.wikipedia.org/wiki/Double-precision_floating-point_format
    b[7] = 0x7f;
    b[6] = 0xff;
    b[5] = 0xff;
    b[4] = 0xff;
    b[3] = 0xff;
    b[2] = 0xff;
    b[1] = 0xff;
    b[0] = 0xff;
    nrTests += check_incorrect_encode_test(0.0, d, 0);
    nrTests += check_incorrect_encode_test(d, 0.0, 0);
    nrTests += check_incorrect_encode_test(d, d, 0);

    // Infinity.
    b[7] = 0x7f;
    b[6] = 0xf0;
    b[5] = 0x00;
    b[4] = 0x00;
    b[3] = 0x00;
    b[2] = 0x00;
    b[1] = 0x00;
    b[0] = 0x00;
    nrTests += check_correct_encode_test(d, 0.0, 0);      // Lat may be Inf.
    nrTests += check_incorrect_encode_test(0.0, d, 0);
    nrTests += check_incorrect_encode_test(d, d, 0);

    // -Infinity.
    b[7] = 0xff;
    b[6] = 0xf0;
    b[5] = 0x00;
    b[4] = 0x00;
    b[3] = 0x00;
    b[2] = 0x00;
    b[1] = 0x00;
    b[0] = 0x00;
    nrTests += check_correct_encode_test(d, 0.0, 0);      // Lat may be -Inf.
    nrTests += check_incorrect_encode_test(0.0, d, 0);
    nrTests += check_incorrect_encode_test(d, d, 0);

    // Max double
    b[7] = 0x7f;
    b[6] = 0xef;
    b[5] = 0xff;
    b[4] = 0xff;
    b[3] = 0xff;
    b[2] = 0xff;
    b[1] = 0xff;
    b[0] = 0xff;
    nrTests += check_correct_encode_test(d, 0.0, 0);
    nrTests += check_correct_encode_test(0.0, d, 0);
    nrTests += check_correct_encode_test(d, d, 0);

    d = -d;
    nrTests += check_correct_encode_test(d, 0.0, 0);
    nrTests += check_correct_encode_test(0.0, d, 0);
    nrTests += check_correct_encode_test(d, d, 0);
    return nrTests;
}


static int check_incorrect_decode_test(char *mc, int tc) {
    double lat;
    double lon;
    int rc = decodeMapcodeToLatLon(&lat, &lon, mc, tc);
    if (rc >= 0) {
        found_error();
        printf("*** ERROR *** decodeMapcodeToLatLon returns '%d' (should be non-0) for mapcode='%s'\n", rc, mc);
    }
    return 1;
}


static int check_correct_decode_test(char *mc, int tc) {
    double lat;
    double lon;
    int rc = decodeMapcodeToLatLon(&lat, &lon, mc, tc);
    if (rc < 0) {
        found_error();
        printf("*** ERROR *** decodeMapcodeToLatLon returns '%d' (should be 0) for mapcode='%s'\n", rc, mc);
    }
    return 1;
}


static int decode_robustness_tests(void) {
    int nrTests = 0;
    int i;
    char s1[1];
    char largeString[16000];

    int tc = getTerritoryCode("NLD", 0);
    nrTests += check_incorrect_decode_test("", 0);
    nrTests += check_incorrect_decode_test(" ", 0);
    nrTests += check_incorrect_decode_test("AA", 0);
    nrTests += check_incorrect_decode_test("", tc);
    nrTests += check_incorrect_decode_test(" ", tc);
    nrTests += check_incorrect_decode_test("AA", tc);
    nrTests += check_incorrect_decode_test("XX.XX", 0);
    nrTests += check_correct_decode_test("NLD XX.XX", tc);
    nrTests += check_correct_decode_test("NLD XX.XX", 0);
    nrTests += check_correct_decode_test("MX XX.XX", 0);

    s1[0] = 0;
    nrTests += check_incorrect_decode_test(s1, 0);
    nrTests += check_incorrect_decode_test(s1, tc);

    for (i = 0; i < sizeof(largeString) - 1; ++i) {
        largeString[i] = (char) ((i % 223) + 32);
    }
    largeString[sizeof(largeString) - 1] = 0;
    nrTests += check_incorrect_decode_test(s1, 0);
    nrTests += check_incorrect_decode_test(s1, tc);
    return nrTests;
}


static int check_alphabet_assertion(char *msg, int condition, char *format, int a) {
    if (condition == 0) {
        found_error();
        printf("*** ERROR *** %s, ", msg);
        printf(format, a);
        printf("\n");
    }
    return 1;
}


static int alphabet_robustness_tests(void) {
    int nrTests = 0;
    int i;
    int a;
    char s1[1];
    char largeString1[20000];
    char largeString2[10000];
    char *ps;
    UWORD u1[1];
    UWORD largeUnicodeString1[10000];
    UWORD largeUnicodeString2[20000];
    UWORD *pu;

    s1[0] = 0;
    for (i = 0; i < sizeof(largeString1) - 1; ++i) {
        largeString1[i] = (char) ((i % 223) + 32);
    }
    largeString1[sizeof(largeString1) - 1] = 0;
    for (i = 0; i < sizeof(largeString2) - 1; ++i) {
        largeString1[2] = (char) ((i % 223) + 32);
    }
    largeString2[sizeof(largeString2) - 1] = 0;

    for (a = 0; a < MAPCODE_ALPHABETS_TOTAL; a++) {

        pu = convertToAlphabet(u1, sizeof(u1) / sizeof(u1[0]), "", a);
        nrTests += check_alphabet_assertion("convertToAlphabet cannot return 0", pu != 0, "alphabet=%d", a);
        nrTests += check_alphabet_assertion("convertToAlphabet must return empty string", pu[0] == 0, "alphabet=%d", a);

        ps = convertToRoman(s1, sizeof(s1) / sizeof(s1[0]), u1);
        nrTests += check_alphabet_assertion("convertToRoman cannot return 0", ps != 0, "alphabet=%d", a);
        nrTests += check_alphabet_assertion("convertToRoman must return empty string", ps[0] == 0, "alphabet=%d", a);

        pu = convertToAlphabet(largeUnicodeString1, sizeof(largeUnicodeString1) / sizeof(largeUnicodeString1[0]), largeString1, 0);
        nrTests += check_alphabet_assertion("convertToAlphabet cannot return 0", pu != 0, "alphabet=%d", a);

        ps = convertToRoman(largeString1, sizeof(largeString1) / sizeof(largeString1[0]), pu);
        nrTests += check_alphabet_assertion("convertToRoman cannot return 0", ps != 0, "alphabet=%d", a);
        nrTests += check_alphabet_assertion("convertToRoman must return size", strlen(ps) < (sizeof(largeString1) / sizeof(largeString1[0])), "alphabet=%d", a);

        pu = convertToAlphabet(largeUnicodeString2, sizeof(largeUnicodeString2) / sizeof(largeUnicodeString2[0]), largeString2, 0);
        nrTests += check_alphabet_assertion("convertToAlphabet cannot return 0", pu != 0, "alphabet=%d", a);

        ps = convertToRoman(largeString2, sizeof(largeString2) / sizeof(largeString2[0]), pu);
        nrTests += check_alphabet_assertion("convertToRoman cannot return 0", ps != 0, "alphabet=%d", a);
        nrTests += check_alphabet_assertion("convertToRoman must return size", strlen(ps) < (sizeof(largeString2) / sizeof(largeString2[0])), "alphabet=%d", a);
    }
    return nrTests;
}


static int robustness_tests(void) {
    int nrTests = 0;
    nrTests += get_territory_robustness_tests();
    nrTests += encode_robustness_tests();
    nrTests += decode_robustness_tests();
    nrTests += alphabet_robustness_tests();
    return nrTests;
}

static int alphabet_per_territory_tests(void) {
    int nrTests = 0;
    int i, j;
    for (i = 0; i < MAX_CCODE; i++) {
        ++nrTests;
        if (alphabetsForTerritory[i].count < 1 || alphabetsForTerritory[i].count > MAX_ALPHABETS_PER_TERRITORY) {
            found_error();
            printf("*** ERROR *** Bad alphabetsForTerritory[%d].count: %d\n", i, alphabetsForTerritory[i].count);
        }
        for (j = 0; j < alphabetsForTerritory[i].count; j++) {
            ++nrTests;
            if (alphabetsForTerritory[i].alphabet[j] < 0 ||
                alphabetsForTerritory[i].alphabet[j] >= MAPCODE_ALPHABETS_TOTAL) {
                found_error();
                printf("*** ERROR *** Bad alphabetsForTerritory[%d].alphabet[%d]: %d\n", i, j,
                       alphabetsForTerritory[i].alphabet[j]);
            }
        }
    }
    return nrTests;
}

static int test_territories_csv(void) {
    int nrTests = 0;
    int linesTested = 0;
    const char *csvName = "territories.csv";
    FILE *fp = fopen(csvName, "r");
    if (fp == NULL) {
        found_error();
        printf("*** ERROR *** Can't read file %s\n", csvName);
    } else {
        char line[MAXLINESIZE];
        if (fgets(line, MAXLINESIZE, fp) != NULL) { // skip header line
            while (fgets(line, MAXLINESIZE, fp) != NULL) {
                int csvTerritoryCode;
                char *s = line;
                char *e = strchr(s, ',');
                if (e) {
                    linesTested++;
                    *e = 0;
                    csvTerritoryCode = atoi(s) + 1;
                    s = e + 1;
                    // parse and check aliases
                    e = strchr(s, ',');
                    if (e) {
                        *e = 0;
                        while (*s) {
                            int territoryCode;
                            char *sep = strchr(s, '|');
                            if (sep) {
                                *sep = 0;
                            }
                            territoryCode = getTerritoryCode(s, 0);
                            if (territoryCode != csvTerritoryCode) {
                                found_error();
                                printf("*** ERROR *** Territory string %s returns code %d, expected %d\n", s,
                                       territoryCode, csvTerritoryCode);
                            }
                            if (sep) {
                                s = sep + 1;
                            } else {
                                s = e;
                            }
                        }
                        s++;
                    }
                    // parse and check alphabets
                    e = strchr(s, ',');
                    if (e) {
                        int csvNrAlphabets = 0;
                        const TerritoryAlphabets *territoryAlphabet = getAlphabetsForTerritory(csvTerritoryCode);
                        *e = 0;
                        while (*s) {
                            char *sep = strchr(s, '|');
                            if (sep) {
                                *sep = 0;
                            }
                            csvNrAlphabets++;
                            if ((csvNrAlphabets > territoryAlphabet->count) ||
                                (atoi(s) != territoryAlphabet->alphabet[csvNrAlphabets - 1])) {
                                found_error();
                                printf("*** ERROR *** Mismatch: alphabet %d of territory %d should be %d\n",
                                       csvNrAlphabets, csvTerritoryCode, atoi(s));
                            }
                            if (sep) {
                                s = sep + 1;
                            } else {
                                s = e;
                            }
                        }
                        if (csvNrAlphabets != territoryAlphabet->count) {
                            found_error();
                            printf("*** ERROR *** %d alphabets for territory %d, expected %d\n",
                                   territoryAlphabet->count, csvTerritoryCode, csvNrAlphabets);
                        }
                        s++;
                    }
                    // parse and check names
                    e = strchr(s, 10);
                    if (e) {
                        const char *territoryNames = isofullname[csvTerritoryCode - 1];
                        *e = 0;
                        while (*s) {
                            char *match;
                            char *sep = strchr(s, '|');
                            if (sep) {
                                *sep = 0;
                            }
                            match = strstr(territoryNames, s);
                            if (match == NULL ||
                                (match[strlen(s)] != ' ' && match[strlen(s)] != 0 && match[strlen(s)] != ',' &&
                                 match[strlen(s)] != ')')) {
                                found_error();
                                printf("*** ERROR *** Name \"%s\" not found in \"%s\"\n", s, territoryNames);
                            }
                            if (sep) {
                                s = sep + 1;
                            } else {
                                s = e;
                            }
                        }
                    }
                }
            }
        }

        fclose(fp);
    }
    printf("%d lines tested from %s\n", linesTested, csvName);
    return nrTests;
}

int main(const int argc, const char **argv) {
    int nrTests = 0;
    printf("Mapcode C Library Unit Tests\n");
    printf("Library version %s (data version %s)\n", mapcode_cversion, mapcode_dataversion);
    printf("Using up to %d threads to test in parallel...\n", MAX_THREADS);

    printf("-----------------------------------------------------------\nRobustness tests\n");
    nrTests += robustness_tests();

    printf("-----------------------------------------------------------\nAlphabet tests\n");
    nrTests += alphabet_tests();

    printf("-----------------------------------------------------------\nAlphabet per territory tests\n");
    nrTests += alphabet_per_territory_tests();

    printf("-----------------------------------------------------------\nDistance tests\n");
    nrTests += distance_tests();

    printf("-----------------------------------------------------------\nTerritory tests\n");
    printf("%d territories\n", MAX_CCODE);
    nrTests += test_territories_csv();
    nrTests += test_territories();
    nrTests += territory_code_tests();
    nrTests += test_territory_insides();

    printf("-----------------------------------------------------------\nFormat tests\n");
    nrTests += test_mapcode_formats();
    nrTests += test_failing_decodes();

    printf("-----------------------------------------------------------\nEncode/decode tests\n");
    nrTests += encode_decode_tests();

    printf("-----------------------------------------------------------\nRe-encode tests\n");
    nrTests += re_encode_tests();

    printf("-----------------------------------------------------------\n");
    printf("Done.\nExecuted %d tests, found %d errors\n", nrTests, nrErrors);
    if (nrErrors > 0) {
        printf("UNIT TESTS FAILED!\n");
    } else {
        printf("Unit tests passed\n");
    }
    return nrErrors;
}
