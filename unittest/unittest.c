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

/**
 * This application performs a number of tests on the Mapcode C library.
 * It helps to establish that all routines work properly.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <ctype.h>

#include "../mapcodelib/mapcoder.h"
#include "../mapcodelib/internal_data.h"

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
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

#define MAXLINESIZE 1024

static const double METERS_PER_DEGREE_LAT = 110946.252133;
static const double METERS_PER_DEGREE_LON = 111319.490793;

static int nrErrors = 0;

static void found_error(void) {
    pthread_mutex_lock(&mutex);
    ++nrErrors;
    pthread_mutex_unlock(&mutex);
}


static int test_mapcode_formats(void) {
    int nrTests = 0;
    static const struct {
        const char *input;              // user input
        enum MapcodeError parseError;   // expected error
        enum MapcodeError decodeError;  // expected error when decoded
    } formattests[] = {
            {"###################",  ERR_INVALID_CHARACTER},
            {"...................",  ERR_UNEXPECTED_DOT},
            {"1111111111111111.11",  ERR_INVALID_MAPCODE_FORMAT},
            {"US-XXXXXXXXXXXXXXXX",  ERR_BAD_TERRITORY_FORMAT},
            {"US-----------------",  ERR_UNEXPECTED_HYPHEN},
            {"-------------------",  ERR_UNEXPECTED_HYPHEN},

            {"cck XX.XX",            ERR_OK, ERR_OK}, // nameless22
            {"cze XX.XXX",           ERR_OK, ERR_OK}, // nameless23
            {"NLD XXX.XX",           ERR_OK, ERR_OK}, // nameless32
            {"VAT 5d.dd",            ERR_OK, ERR_OK}, // Grid22
            {"NLD XX.XXX",           ERR_OK, ERR_OK}, // Grid23
            {"bhr xxx.xx",           ERR_OK, ERR_OK}, // Grid32
            {"FRA XXX.XXX",          ERR_OK, ERR_OK}, // Grid33
            {"irl xx.xxxx",          ERR_OK, ERR_OK}, // Grid24
            {"cub xxxx.xx",          ERR_OK, ERR_OK}, // Grid42
            {"ben xxxx.xxx",         ERR_OK, ERR_OK}, // Grid34
            {"USA xxxx.xxxx",        ERR_OK, ERR_OK}, // Grid44
            {"US-AZ hhh.hh",         ERR_OK, ERR_OK}, // HGrid32
            {"Bel hhh.hhh",          ERR_OK, ERR_OK}, // HGrid33
            {"PAN hh.hhhh",          ERR_OK, ERR_OK}, // HGrid24
            {"GRC hhhh.hh",          ERR_OK, ERR_OK}, // HGrid42
            {"NZL hhhh.hhh",         ERR_OK, ERR_OK}, // HGrid43
            {"KAZ hhh.hhhh",         ERR_OK, ERR_OK}, // HGrid34
            {"RUS xxxx.xxxx",        ERR_OK, ERR_OK}, // HGrid44
            {"CN-SH hhhh.hhhh",      ERR_OK, ERR_OK}, // HGrid44
            {"VAT hhhhh.hhhh",       ERR_OK, ERR_OK}, // HGrid54
            {"hhhhh.hhhh",           ERR_OK, ERR_OK}, // HGrid54
            {"TUV hh.hhh",           ERR_OK, ERR_OK}, // AutoHeader23
            {"LVA L88.ZVR",          ERR_OK, ERR_OK}, // AutoHeader33
            {"WLF XLG.3GP",          ERR_OK, ERR_OK}, // HGrid33 R
            {"VAT j0q3.27r",         ERR_OK, ERR_OK}, // HGrid43 R
            {"PAK hhhh.hhhh",        ERR_OK, ERR_OK}, // HGrid44 R

            {"NLD 49.4V",            ERR_OK, ERR_OK},
            {"NLX 49.4V",            ERR_UNKNOWN_TERRITORY},
            {"49.4V",                ERR_OK, ERR_MISSING_TERRITORY},
            {"BRA 49.4V",            ERR_OK, ERR_MAPCODE_UNDECODABLE}, // type 0
            {"BRA XXXXX.XXX",        ERR_OK, ERR_MAPCODE_UNDECODABLE}, // type 0
            {"NLD XXXX.XXXX",        ERR_OK, ERR_MAPCODE_UNDECODABLE}, // type 0
            {"NLD ZZ.ZZ",            ERR_OK, ERR_MAPCODE_UNDECODABLE}, // type 1 nameless
            {"NLD Q000.000",         ERR_OK, ERR_MAPCODE_UNDECODABLE}, // type 2 grid
            {"NLD L222.222",         ERR_OK, ERR_MAPCODE_UNDECODABLE}, // type 3 restricted
            {"usa A222.22AA",        ERR_OK, ERR_MAPCODE_UNDECODABLE}, // type 4 grid
            {"atf hhh.hhh",          ERR_OK, ERR_MAPCODE_UNDECODABLE}, // type 5 autoh zone
            {"ASM zz.zzh",           ERR_OK, ERR_MAPCODE_UNDECODABLE}, // type 6 autoh out

            {"nld ZNZ.RZG-B",        ERR_OK, ERR_MAPCODE_UNDECODABLE}, // type 0
            {"WLF 01.AE-09V",        ERR_OK, ERR_OK},
            {"LVA LDV.ZVR-B  ",      ERR_OK, ERR_OK}, // AutoHeader
            {"LVA LDV.ZVR-BY  ",     ERR_OK, ERR_EXTENSION_UNDECODABLE},
            {"01.AE",                ERR_OK, ERR_MISSING_TERRITORY},
            {"nld 01.AE",            ERR_OK, ERR_OK},
            {"nld oi.AE",            ERR_OK, ERR_OK},
            {"oi.oi",                ERR_ALL_DIGIT_CODE},
            {"nld oi.OI-xxx",        ERR_ALL_DIGIT_CODE},
            {"CUB 3467.UY",          ERR_OK, ERR_OK},
            {"34.UY",                ERR_OK, ERR_MISSING_TERRITORY},
            {"mx XX.XX",             ERR_OK, ERR_OK},
            {"",                     ERR_DOT_MISSING},
            {"ttat.tt    ",          ERR_INVALID_VOWEL},
            {"ttat-tt tt.tt",        ERR_INVALID_VOWEL},
            {"ttat tt.tt",           ERR_INVALID_VOWEL},
            {"XXAX.XXXX",            ERR_INVALID_VOWEL},
            {"2A22.2222",            ERR_INVALID_VOWEL},
            {"22A2.2222",            ERR_INVALID_VOWEL},
            {"MAP.CODE",             ERR_INVALID_VOWEL},
            {"XAXX.XXXX",            ERR_INVALID_VOWEL},
            {"XXXA.XXXX",            ERR_INVALID_VOWEL},
            {"XXXAX.XXXX",           ERR_INVALID_VOWEL},
            {"XXXXA.XXXX",           ERR_INVALID_VOWEL},
            {"nld XXXX.XXXXA",       ERR_INVALID_VOWEL},
            {"nld XXXX.ALA",         ERR_INVALID_VOWEL},
            {"nld XXXX.LAXA",        ERR_INVALID_VOWEL},
            {"nld XXXX.LLLLA",       ERR_INVALID_VOWEL},
            {"nld XXXX.A2e",         ERR_INVALID_VOWEL},
            {"nld XXXX.2e2e",        ERR_INVALID_VOWEL},
            {"nld XXXX.2222u",       ERR_INVALID_VOWEL},
            {"222A.2222",            ERR_INVALID_VOWEL},
            {"222A2.2222",           ERR_INVALID_VOWEL},
            {"ttt 2222A.2222",       ERR_INVALID_VOWEL},
            {"2222.2AAA",            ERR_INVALID_VOWEL},
            {"A222.2AAA",            ERR_INVALID_VOWEL},
            {"usa 2222.22A2",        ERR_OK, ERR_OK},
            {"usa 2222.22AA",        ERR_OK, ERR_OK},
            {".123",                 ERR_UNEXPECTED_DOT},
            {".xyz",                 ERR_UNEXPECTED_DOT},
            {"x.xyz",                ERR_UNEXPECTED_DOT},
            {"xxx.z-12",             ERR_UNEXPECTED_HYPHEN},
            {"xx.xx.",               ERR_UNEXPECTED_DOT},
            {"xxxx xx.xx",           ERR_BAD_TERRITORY_FORMAT},
            {"xxxxx xx.xx",          ERR_BAD_TERRITORY_FORMAT},
            {"123",                  ERR_DOT_MISSING},
            {"xxx.",                 ERR_MAPCODE_INCOMPLETE},
            {"xxx.z",                ERR_MAPCODE_INCOMPLETE},
            {"NLD 49.4V-",           ERR_MAPCODE_INCOMPLETE},
            {"NLD 49.4V",            ERR_OK, ERR_OK},
            {"   NLD   49.4V  ",     ERR_OK, ERR_OK},
            {"NLD 49.4V-1",          ERR_OK, ERR_OK},
            {"NLD 49.4V-12",         ERR_OK, ERR_OK},
            {"NLD 49.4V-123",        ERR_OK, ERR_OK},
            {"NLD 49.4V-12345678",   ERR_OK, ERR_OK},
            {"NLD 49.4V- ",          ERR_EXTENSION_INVALID_LENGTH},
            {"NLD 49.4V-123456789",  ERR_EXTENSION_INVALID_LENGTH},
            {"49.4V-xxxxxxxxxxxxxx", ERR_EXTENSION_INVALID_LENGTH},
            {"DD.DD-        ",       ERR_EXTENSION_INVALID_LENGTH},
            {"nld DD.DD-",           ERR_MAPCODE_INCOMPLETE},
            {"TAM 49.4V",            ERR_OK, ERR_OK},
            {"BRA 49.4V",            ERR_OK, ERR_MAPCODE_UNDECODABLE}, // type 0
            {"CA 49.4V",             ERR_OK, ERR_OK},
            {"N 49.4V",              ERR_BAD_TERRITORY_FORMAT},
            {"XXXX ",                ERR_BAD_TERRITORY_FORMAT},
            {"XXXXX ",               ERR_BAD_TERRITORY_FORMAT},
            {"XXXX 49.4V",           ERR_BAD_TERRITORY_FORMAT},
            {"XXXXX 49.4V",          ERR_BAD_TERRITORY_FORMAT},
            {"-XX 49.4V",            ERR_UNEXPECTED_HYPHEN},
            {"X-XX 49.4V",           ERR_BAD_TERRITORY_FORMAT},
            {"XXXX-XX 49.4V",        ERR_BAD_TERRITORY_FORMAT},
            {"XX-X 49.4V",           ERR_BAD_TERRITORY_FORMAT},
            {"XX-XXXX 49.4V",        ERR_BAD_TERRITORY_FORMAT},
            {"XX-XXXE 49.4V",        ERR_BAD_TERRITORY_FORMAT},
            {"12.34",                ERR_ALL_DIGIT_CODE},
            {"NLD 12.34",            ERR_ALL_DIGIT_CODE},
            {"AAA 12.34",            ERR_ALL_DIGIT_CODE},
            {"AAA 12.34-XXX",        ERR_ALL_DIGIT_CODE},
            {"123 12.34-123",        ERR_ALL_DIGIT_CODE},
            {"xx-xx 12.34",          ERR_ALL_DIGIT_CODE},
            {"12-34 12.34",          ERR_ALL_DIGIT_CODE},
            {"CN-34 12.3X",          ERR_OK, ERR_OK},
            {"  TAM  XX.XX-XX  ",    ERR_OK, ERR_OK},
            {"  TAM  XXX.XX-XX  ",   ERR_OK, ERR_OK},
            {"  TAM  XX.XXX-XX  ",   ERR_OK, ERR_OK},
            {"  TAM  XX.XXXX-XX  ",  ERR_OK, ERR_OK},
            {"  TAM  XXX.XXX-XX  ",  ERR_OK, ERR_OK},
            {"  gab  XXXX.XX-XX  ",  ERR_OK, ERR_OK},
            {"  kAZ  XXX.XXXX-XX ",  ERR_OK, ERR_OK},
            {"  IND  XXXX.XXX-XX ",  ERR_OK, ERR_OK},
            {" USA  XXXX.XXXX-XX ",  ERR_OK, ERR_OK},
            {" VAT XXXXX.XXXX-XX  ", ERR_OK, ERR_OK},
            {" NLD XXXXX.XXXX-XX  ", ERR_OK, ERR_OK},
            {" USA XXXXX.XXXX-XX  ", ERR_OK, ERR_OK},
            {" XXXXX.XXXX-XX  ",     ERR_OK, ERR_OK},
            {" usa  XXXXX.XXX-XX ",  ERR_OK, ERR_MAPCODE_UNDECODABLE}, // type 0
            {" XXXXX.XXX-XX ",       ERR_OK, ERR_MISSING_TERRITORY},
            {"xx-xx.x xx.xx",        ERR_UNEXPECTED_DOT},
            {"xx-xx-x xx.xx",        ERR_UNEXPECTED_HYPHEN},
            {"xx.xx-x-x",            ERR_UNEXPECTED_HYPHEN},
            {"xx-xx xx-xx",          ERR_UNEXPECTED_HYPHEN},
            {"xx-xx xx-xx.xx",       ERR_UNEXPECTED_HYPHEN},
            {"xx.xx.xx",             ERR_UNEXPECTED_DOT},
            {"xx-xx xx.xx.xx",       ERR_UNEXPECTED_DOT},
            {"xx-xx xx.xx-xx-xx",    ERR_UNEXPECTED_HYPHEN},
            {"xx-xx xx.xx x",        ERR_TRAILING_CHARACTERS},
            {"xx-xx xx.xx-x x",      ERR_TRAILING_CHARACTERS},
            {"xx-xx xx.xx-x -",      ERR_UNEXPECTED_HYPHEN},
            {"xx-xx xx.xx-x .",      ERR_UNEXPECTED_DOT},
            {"xx-xx xx.xx-x 2",      ERR_TRAILING_CHARACTERS},
            {"xx-xx xx.x#x",         ERR_INVALID_CHARACTER},
            {"xx# xx.xx",            ERR_INVALID_CHARACTER},
            {"xx-xx #xx.xx",         ERR_INVALID_CHARACTER},
            {"xx-xx xx.xx-xx#xx",    ERR_INVALID_CHARACTER},
            {"xx-xx -xx.xx",         ERR_UNEXPECTED_HYPHEN},
            {"xx-xx .xx.xx",         ERR_UNEXPECTED_DOT},
            {".123",                 ERR_UNEXPECTED_DOT},
            {"  .123",               ERR_UNEXPECTED_DOT},
            {"",                     ERR_DOT_MISSING},
            {"  ",                   ERR_DOT_MISSING},
            {"-xx.xx",               ERR_UNEXPECTED_HYPHEN},
            {"  - xx.xx",            ERR_UNEXPECTED_HYPHEN},
            {"D xx.xx",              ERR_BAD_TERRITORY_FORMAT},
            {"D.123",                ERR_UNEXPECTED_DOT},
            {"D",                    ERR_DOT_MISSING},
            {"D-xxxxx",              ERR_BAD_TERRITORY_FORMAT},
            {"DD",                   ERR_DOT_MISSING},
            {"DDDa.DDD",             ERR_INVALID_VOWEL},
            {"DDD",                  ERR_DOT_MISSING},
            {"DDDD xx.xx",           ERR_BAD_TERRITORY_FORMAT},
            {"DDDDE.xxxx",           ERR_INVALID_VOWEL},
            {"DDDD",                 ERR_DOT_MISSING},
            {"DDDD-CA xx.xx",        ERR_BAD_TERRITORY_FORMAT},
            {"DDDDD CA xx.xx",       ERR_BAD_TERRITORY_FORMAT},
            {"DDDDDA   xx.xx",       ERR_INVALID_VOWEL},
            {"DDDDD",                ERR_DOT_MISSING},
            {"DDDDD-CA xx.xx",       ERR_BAD_TERRITORY_FORMAT},
            {"DDDDD..xxxx",          ERR_UNEXPECTED_DOT},
            {"DDDDD.",               ERR_MAPCODE_INCOMPLETE},
            {"DDDDD.-xxxx.xx",       ERR_UNEXPECTED_HYPHEN},
            {"DDD.L.LLL     ",       ERR_UNEXPECTED_DOT},
            {"DDD.L",                ERR_MAPCODE_INCOMPLETE},
            {"DDD.L-xxxxxxxx",       ERR_UNEXPECTED_HYPHEN},
            {"DD.DD.CA",             ERR_UNEXPECTED_DOT},
            {"DD.DDD.CA",            ERR_UNEXPECTED_DOT},
            {"DD.DDDD.CA    ",       ERR_UNEXPECTED_DOT},
            {"DD.DDDDA      ",       ERR_INVALID_VOWEL},
            {"DD.DD-.       ",       ERR_UNEXPECTED_DOT},
            {"DD.DD-",               ERR_MAPCODE_INCOMPLETE},
            {"DD.DD--XXX",           ERR_UNEXPECTED_HYPHEN},
            {"DD.DD-x.      ",       ERR_UNEXPECTED_DOT},
            {"DD.DD-A",              ERR_EXTENSION_INVALID_CHARACTER},
            {"DD.DD-xA",             ERR_EXTENSION_INVALID_CHARACTER},
            {"DD.DD-xxxE",           ERR_EXTENSION_INVALID_CHARACTER},
            {"DD.DD-xxxxxu",         ERR_EXTENSION_INVALID_CHARACTER},
            {"DD.DD-x-xxx",          ERR_UNEXPECTED_HYPHEN},
            {"ta.xx     ",           ERR_INVALID_VOWEL},
            {"ta",                   ERR_DOT_MISSING},
            {"DAD-        ",         ERR_BAD_TERRITORY_FORMAT},
            {"DAD-.       ",         ERR_UNEXPECTED_DOT},
            {"DAD-",                 ERR_BAD_TERRITORY_FORMAT},
            {"DAD--XXX",             ERR_UNEXPECTED_HYPHEN},
            {"DAD-X  xx.xx",         ERR_BAD_TERRITORY_FORMAT},
            {"DAD-X.      ",         ERR_UNEXPECTED_DOT},
            {"DAD-X",                ERR_BAD_TERRITORY_FORMAT},
            {"DAD-X-XXX",            ERR_UNEXPECTED_HYPHEN},
            {"DAD-XX.XX   ",         ERR_UNEXPECTED_DOT},
            {"DAD-XX",               ERR_DOT_MISSING},
            {"DAD-XX-XX",            ERR_UNEXPECTED_HYPHEN},
            {"DAD-XXX.XX   ",        ERR_UNEXPECTED_DOT},
            {"DAD-XXXX",             ERR_BAD_TERRITORY_FORMAT},
            {"DAD-XXXA",             ERR_BAD_TERRITORY_FORMAT},
            {"DAD-XXX",              ERR_DOT_MISSING},
            {"DAD-XXX-XX",           ERR_UNEXPECTED_HYPHEN},
            {"DAD-XX  .XX   ",       ERR_UNEXPECTED_DOT},
            {"DAD-XX  ",             ERR_DOT_MISSING},
            {"DAD-XX  -XX",          ERR_UNEXPECTED_HYPHEN},
            {"DD-DD A      ",        ERR_DOT_MISSING},
            {"DD-DD A.     ",        ERR_UNEXPECTED_DOT},
            {"DD-DD AA.33  ",        ERR_INVALID_VOWEL},
            {"DD-DD A",              ERR_DOT_MISSING},
            {"DD-DD A-XX",           ERR_UNEXPECTED_HYPHEN},
            {"DD-DD A3     ",        ERR_DOT_MISSING},
            {"DD-DD A3A.XX ",        ERR_INVALID_VOWEL},
            {"DD-DD A3",             ERR_DOT_MISSING},
            {"DD-DD A3-XX",          ERR_UNEXPECTED_HYPHEN},
            {"DD-DD A33    ",        ERR_DOT_MISSING},
            {"DD-DD A33A.XX",        ERR_INVALID_VOWEL},
            {"DD-DD A33",            ERR_DOT_MISSING},
            {"DD-DD A33-XX",         ERR_UNEXPECTED_HYPHEN},
            {"DD-DD xx.xx .",        ERR_UNEXPECTED_DOT},
            {"DD-DD xx.xx x",        ERR_TRAILING_CHARACTERS},
            {"DD-DD xx.xx a",        ERR_TRAILING_CHARACTERS},
            {"DD-DD xx.xx -x",       ERR_UNEXPECTED_HYPHEN},
            {"xx.xx .xx",            ERR_UNEXPECTED_DOT},
            {"xx.xx x",              ERR_TRAILING_CHARACTERS},
            {"xx.xx a",              ERR_TRAILING_CHARACTERS},
            {"xx.xx -123",           ERR_UNEXPECTED_HYPHEN},
            {" xx.xx-DD .",          ERR_UNEXPECTED_DOT},
            {" xx.xx-DD x",          ERR_TRAILING_CHARACTERS},
            {" xx.xx-DD a",          ERR_TRAILING_CHARACTERS},
            {"xx.xx xxxxxxxxxxxx",   ERR_TRAILING_CHARACTERS},
            {" xx.xx-DD -",          ERR_UNEXPECTED_HYPHEN},
            {"tta.ttt    ",          ERR_INVALID_VOWEL},
            {"ttaa.ttt   ",          ERR_INVALID_VOWEL},
            {"tta",                  ERR_DOT_MISSING},

            {"DDD. ",                ERR_INVALID_MAPCODE_FORMAT}, // 6/0 : white na dot
            {"DDDDD. xxxx.xx",       ERR_INVALID_MAPCODE_FORMAT}, // 6/0 : white na dot
            {"DDD.L         ",       ERR_INVALID_MAPCODE_FORMAT}, // 7.0 : postfix too short
            {"DDDDDD   xx.xx",       ERR_INVALID_MAPCODE_FORMAT}, // 5/2 : 6char ter
            {"DDDDDD.xxx",           ERR_INVALID_MAPCODE_FORMAT}, // 5/2 : 6char mc
            // 10/2 : errors because there are too many letters after a postfix vowel
            {"XXXX.AXXX",            ERR_INVALID_MAPCODE_FORMAT},
            {"nld XXXX.AXX",         ERR_INVALID_MAPCODE_FORMAT},
            {"nld XXXX.XAXX",        ERR_INVALID_MAPCODE_FORMAT},
            {"nld XXXX.AXXA",        ERR_INVALID_MAPCODE_FORMAT},
            {"2222.A22",             ERR_INVALID_MAPCODE_FORMAT},
            {"2222.A222",            ERR_INVALID_MAPCODE_FORMAT},
            {"2222.2A22",            ERR_INVALID_MAPCODE_FORMAT},
            // 10/2 : errors because the postfix has a 5th letter
            {"DD.DDDDD      ",       ERR_INVALID_MAPCODE_FORMAT},
            {"nld XXXX.XXXXX",       ERR_INVALID_MAPCODE_FORMAT},
            {" TAM  XX.XXXXX-XX ",   ERR_INVALID_MAPCODE_FORMAT},
            {" TAM  XXX.XXXXX-XX ",  ERR_INVALID_MAPCODE_FORMAT},
            {" TAM  XXXX.XXXXX-XX ", ERR_INVALID_MAPCODE_FORMAT},
            {" TAM XXXXX.XXXXX-XX ", ERR_INVALID_MAPCODE_FORMAT},

            // Check tabs, spaces and control characters.
            {"NLD 49.YV",            ERR_OK, ERR_OK},
            {" NLD 49.YV",           ERR_OK, ERR_OK},
            {"\tNLD 49.YV",          ERR_OK, ERR_OK},
            {"NLD 49.YV ",           ERR_OK, ERR_OK},
            {"NLD 49.YV\t",          ERR_OK, ERR_OK},
            {"NLD  49.YV",           ERR_OK, ERR_OK},
            {"NLD\t49.YV",           ERR_OK, ERR_OK},
            {"NLD\n49.YV",           ERR_INVALID_CHARACTER},
            {"NLD\r49.YV",           ERR_INVALID_CHARACTER},
            {"NLD\v49.YV",           ERR_INVALID_CHARACTER},
            {"NLD\b49.YV",           ERR_INVALID_CHARACTER},
            {"NLD\a49.YV",           ERR_INVALID_CHARACTER},
            {NULL,                   ERR_OK, ERR_OK}
    };

    int shouldSucceed = 0; // count nr of calls that SHOULD be successful.
    int total = 0;
    int succeeded = 0;
    int i;

    for (i = 0; formattests[i].input != NULL; ++i) {
        MapcodeElements mapcodeElements;
        enum MapcodeError parseError = parseMapcodeString(&mapcodeElements, formattests[i].input, 1, 0);
        enum MapcodeError formatError = compareWithMapcodeFormat(formattests[i].input, 1);
        if (formattests[i].parseError == ERR_OK) {
            shouldSucceed++;
        }

        nrTests++;
        if (parseError != formatError) {
            // there is a special case where parse knows about valid territories
            if (formatError || formattests[i].parseError != ERR_UNKNOWN_TERRITORY) {
                found_error();
                printf("*** ERROR *** \"%s\" : parseMapcodeString=%d, compareWithMapcodeFormat=%d\n",
                       formattests[i].input, parseError, formatError);
            }
        }

        nrTests++;
        if (formattests[i].parseError != parseError) {
            found_error();
            printf("*** ERROR *** compareWithMapcodeFormat(\"%s\") returns %d (%d expected)\n", formattests[i].input,
                   parseError, formattests[i].parseError);
        }

        nrTests++;
        ++total;
        if (parseError == 0) {
            double lat, lon;
            int decodeError = decodeMapcodeToLatLon(&lat, &lon, formattests[i].input, TERRITORY_UNKNOWN);
            ++succeeded;
            if (decodeError != formattests[i].decodeError) {
                found_error();
                printf("*** ERROR *** parseMapcodeString(\"%s\")=%d, expected %d\n", formattests[i].input, decodeError,
                       formattests[i].decodeError);
            }
        }
    }
    if (succeeded != shouldSucceed) {
        found_error();
        printf("*** ERROR *** %d of %d parseMapcodeString() calls succeeded (expected %d)\n", succeeded, total,
               shouldSucceed);
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
    enum Territory tc = TERRITORY_NONE;
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
            // copy and recognize territory
            memcpy(territory, s, (size_t) len);
            territory[len] = 0;
            tc = getTerritoryCode(territory, TERRITORY_NONE);
            // make s skip to start of proper mapcode
            s = p;
            while (*s > 0 && *s <= 32) {
                s++;
            }
        } else {
            // assume s is the start of the proper mapcode
            territory[0] = 0;
            tc = getTerritoryCode("AAA", TERRITORY_NONE);
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
        memcpy(clean + i, s, (size_t) len);
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
        nrresults = encodeLatLonToMapcodes(&mapcodes, y, x, TERRITORY_UNKNOWN, precision);
        if (nrresults != globalsolutions) {
            found_error();
            printf("*** ERROR *** encode(%0.8f, %0.8f) does not deliver %d global solutions\n", y, x, globalsolutions);
            printGeneratedMapcodes("Delivered", &mapcodes);
        }
    }

    // test all global solutions at all precisions...
    for (precision = 0; precision <= 8; precision++) {
        nrresults = encodeLatLonToMapcodes(&mapcodes, y, x, TERRITORY_UNKNOWN, precision);
        for (i = 0; i < nrresults; i++) {
            const char *strResult = mapcodes.mapcode[i];

            // check if every solution decodes
            ++nrTests;
            err = decodeMapcodeToLatLon(&lat, &lon, strResult, TERRITORY_UNKNOWN);
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
                    enum Territory tc2 = TERRITORY_NONE;
                    enum Territory tcParent = TERRITORY_NONE;
                    int j;
                    char *e = strchr(strResult, ' ');
                    found = 0;
                    if (e) {
                        *e = 0;
                        tc2 = getTerritoryCode(strResult, TERRITORY_NONE);
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
                    if (!found && (tcParent > _TERRITORY_MIN)) {
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
                            if (tcParent > _TERRITORY_MIN) {
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
        err = decodeMapcodeToLatLon(&lat, &lon, str, TERRITORY_UNKNOWN);
        if (err >= 0) {
            found_error();
            printf("*** ERROR *** invalid mapcode \"%s\" decodes without error\n", str);
        }
    }
    return nrTests;
}

#include "test_territories.h"

static int
test_territory(const char *alphaCode, enum Territory territory, int isAlias, int needsParent, enum Territory tcParent) {
    int nrTests = 0;
    char nam[MAX_ISOCODE_LEN + 1];
    unsigned int i;
    for (i = 0; i <= strlen(alphaCode); i++) {
        char alphacode[MAX_ISOCODE_LEN + 1];
        int tn;
        strcpy(alphacode, alphaCode);
        if (!needsParent && (i == 0)) {
            tn = getTerritoryCode(alphacode, TERRITORY_NONE);
            ++nrTests;
            if (tn != territory) {
                found_error();
                printf("*** ERROR *** getTerritoryCode('%s')=%d but expected %d (%s)\n",
                       alphacode, tn, territory, getTerritoryIsoName(nam, territory, 0));
            }
        }
        alphacode[i] = (char) tolower(alphacode[i]);
        tn = getTerritoryCode(alphacode, tcParent);
        ++nrTests;
        if (tn != territory) {
            found_error();
            printf("*** ERROR *** getTerritoryCode('%s',%s)=%d but expected %d\n", alphacode,
                   tcParent ? getTerritoryIsoName(nam, tcParent, 0) : "", tn, territory);
        }
    }

    if ((tcParent > _TERRITORY_MIN) && !isAlias) {
        getTerritoryIsoName(nam, territory, 0);
        ++nrTests;
        // every non-alias either equals nam, or is the state in nam
        if ((strcmp(nam, alphaCode) != 0) && (strcmp(nam + 3, alphaCode) != 0)) {
            found_error();
            printf("*** ERROR *** getTerritoryIsoName(%d)=\"%s\" which does not equal or contain \"%s\"\n",
                   territory, nam, alphaCode);
        }
    }
    return nrTests;
}


static int test_territories() {
    int nrTests = 0;
    int nr = sizeof(testTerritories) / sizeof(testTerritories[0]);
    int i;
    for (i = 0; i < nr; ++i) {
        nrTests += test_territory(testTerritories[i].codeISO, testTerritories[i].territory, testTerritories[i].isAlias,
                                  testTerritories[i].needsParent, testTerritories[i].parent);
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
    const TerritoryBoundary *territoryBoundaries;
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
    const TerritoryBoundary *b = c->territoryBoundaries;

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
    enum Territory ccode;
    int m = 0;
    int nrRecords = MAPCODE_BOUNDARY_MAX;
    int nrThread = 0;

    // Declare threads and contexts.
    pthread_t threads[MAX_THREADS];
    struct context_test_around contexts[MAX_THREADS];

    printf("%d records\n", nrRecords);
    for (ccode = _TERRITORY_MIN + 1; ccode < _TERRITORY_MAX; ccode++) {
        int min_boundary = data_start[INDEX_OF_TERRITORY(ccode)];
        int max_boundary = data_start[INDEX_OF_TERRITORY(ccode) + 1];
        show_progress(max_boundary, nrRecords, nrTests);
        // use internal knowledge of mapcoder to test all the territory boundaries
        for (m = min_boundary; m < max_boundary; m++) {
            const TerritoryBoundary *b = territoryBoundary(m);

            // Create context for thread.
            contexts[nrThread].nrTests = 0;
            contexts[nrThread].territoryBoundaries = b;

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

static void check_distance(double d1, double d2) {
    if (fabs(d1 - d2) > 0.00001) {
        found_error();
        printf("*** ERROR *** distanceInMeters failed, %lf != %lf\n", d1, d2);
    }
}


static int distance_tests(void) {
    int nrTests = 0;
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

    ++nrTests;
    check_distance(METERS_PER_DEGREE_LON * 1.5, distanceInMeters(0.0, 0.0, 0.0, 1.5));  // Check if #define is correct.
    ++nrTests;
    check_distance(METERS_PER_DEGREE_LON * 2.0, distanceInMeters(0.0, 0.0, 0.0, 2.0));
    ++nrTests;
    check_distance(METERS_PER_DEGREE_LON * 2.5, distanceInMeters(0.0, 0.0, 0.0, 2.5));

    ++nrTests;
    check_distance(METERS_PER_DEGREE_LON * 1.5, distanceInMeters(0.0, -1.0, 0.0, 0.5)); // Check around 0.
    ++nrTests;
    check_distance(METERS_PER_DEGREE_LON * 2.0, distanceInMeters(0.0, -1.0, 0.0, 1.0));
    ++nrTests;
    check_distance(METERS_PER_DEGREE_LON * 2.5, distanceInMeters(0.0, -1.0, 0.0, 1.5));

    ++nrTests;
    check_distance(METERS_PER_DEGREE_LON * 1.5, distanceInMeters(0.0, 0.5, 0.0, -1.0));
    ++nrTests;
    check_distance(METERS_PER_DEGREE_LON * 2.0, distanceInMeters(0.0, 1.0, 0.0, -1.0));
    ++nrTests;
    check_distance(METERS_PER_DEGREE_LON * 2.5, distanceInMeters(0.0, 1.5, 0.0, -1.0));

    ++nrTests;
    check_distance(METERS_PER_DEGREE_LON * 1.5, distanceInMeters(0.0, 359.0, 0.0, 0.5)); // Check around 360.
    ++nrTests;
    check_distance(METERS_PER_DEGREE_LON * 2.0, distanceInMeters(0.0, 359.0, 0.0, 1.0));
    ++nrTests;
    check_distance(METERS_PER_DEGREE_LON * 2.5, distanceInMeters(0.0, 359.0, 0.0, 1.5));

    ++nrTests;
    check_distance(METERS_PER_DEGREE_LON * 1.5, distanceInMeters(0.0, 0.5, 0.0, 359.0)); // Note that shortest.
    ++nrTests;
    check_distance(METERS_PER_DEGREE_LON * 2.0, distanceInMeters(0.0, 1.0, 0.0, 359.0)); // path needs to be taken!
    ++nrTests;
    check_distance(METERS_PER_DEGREE_LON * 2.5, distanceInMeters(0.0, 1.5, 0.0, 359.0));

    ++nrTests;
    check_distance(METERS_PER_DEGREE_LAT, distanceInMeters(0.5, 0.0, -0.5, 0.0)); // Check constant.
    ++nrTests;
    check_distance(METERS_PER_DEGREE_LAT, distanceInMeters(1.0, 0.0, -0.0, 0.0)); // Check around 0.
    ++nrTests;
    check_distance(METERS_PER_DEGREE_LAT, distanceInMeters(0.0, 0.0, -1.0, 0.0));

    for (i = 0; coordpairs[i] != -1; i += 5) {
        const double distance = distanceInMeters(
                coordpairs[i], coordpairs[i + 1],
                coordpairs[i + 2], coordpairs[i + 3]);
        ++nrTests;
        if (floor(0.5 + (100000.0 * distance)) != coordpairs[i + 4]) {
            found_error();
            printf("*** ERROR *** distanceInMeters %d failed: %f\n", i, distance);
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
            enum Territory territory = getTerritoryCode(iTestData[i].territory, TERRITORY_NONE);
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
        enum Territory expectedresult;
        enum Territory context;
        const char *inputstring;
    } tcTestData[] = {
            {TERRITORY_RU_AL, TERRITORY_RU_TT, "AL"}, // 431=ru-tam
            {TERRITORY_NONE,  TERRITORY_NONE,  ""},
            {TERRITORY_NONE,  TERRITORY_NONE,  "R"},
            {TERRITORY_NONE,  TERRITORY_NONE,  "RX"},
            {TERRITORY_NONE,  TERRITORY_NONE,  "RXX"},
            {TERRITORY_RUS,   TERRITORY_NONE,  "RUS"},
            {TERRITORY_NONE,  TERRITORY_NONE,  "RUSSIA"},
            {TERRITORY_USA,   TERRITORY_NONE,  "US"},
            {TERRITORY_USA,   TERRITORY_NONE,  "USA"},
            {TERRITORY_USA,   TERRITORY_NONE,  "usa"},
            {TERRITORY_NONE,  TERRITORY_NONE,  "US-TEST"},
            {TERRITORY_USA,   TERRITORY_NONE,  "US TEST"},
            {TERRITORY_US_CA, TERRITORY_NONE,  "US-CA"},
            {TERRITORY_US_CA, TERRITORY_NONE,  "Us-CA TEST"},
            {TERRITORY_US_CA, TERRITORY_NONE,  "Usa-CA"},
            {TERRITORY_RU_TT, TERRITORY_NONE,  "RUS-TAM"},
            {TERRITORY_NONE,  TERRITORY_NONE,  "RUS-TAMX"},
            {TERRITORY_RU_TT, TERRITORY_NONE,  "RUS-TAM X"},
            {TERRITORY_BR_AL, TERRITORY_NONE,  "AL"}, //
            {TERRITORY_RU_AL, TERRITORY_RUS,   "AL"}, // 497=rus
            {TERRITORY_RU_AL, TERRITORY_RU_TT, "AL"}, // 431=ru-tam
            {TERRITORY_US_AL, TERRITORY_USA,   "AL"}, // 411=usa
            {TERRITORY_US_AL, TERRITORY_US_CA, "AL"},
            {TERRITORY_NONE,  TERRITORY_NONE,  0}
    };

    for (i = 0; tcTestData[i].inputstring != 0; i++) {
        enum Territory ccode = getTerritoryCode(tcTestData[i].inputstring, tcTestData[i].context);
        ++nrTests;
        if (ccode != tcTestData[i].expectedresult) {
            found_error();
            printf("*** ERROR *** getTerritoryCode(\"%s\", %d)=%d, expected %d\n",
                   tcTestData[i].inputstring, tcTestData[i].context,
                   ccode, tcTestData[i].expectedresult);
        }
    }
    return nrTests;
}


static int check_incorrect_get_territory_code_test(char *tcAlpha) {
    enum Territory ccode = getTerritoryCode(tcAlpha, TERRITORY_NONE);
    if (ccode > _TERRITORY_MIN) {
        found_error();
        printf("*** ERROR *** getTerritoryCode returns '%d' (should be < 0) for territory code '%s'\n", (int) ccode,
               tcAlpha);
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
    nrResults = encodeLatLonToMapcodes(&mapcodes, lat, lon, TERRITORY_UNKNOWN, 0);
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
    int nrResults = encodeLatLonToMapcodes(&mapcodes, lat, lon, TERRITORY_UNKNOWN, 0);
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


static int check_incorrect_decode_test(char *mc, enum Territory tc) {
    double lat;
    double lon;
    int rc = decodeMapcodeToLatLon(&lat, &lon, mc, tc);
    if (rc >= 0) {
        found_error();
        printf("*** ERROR *** decodeMapcodeToLatLon returns '%d' (should be non-0) for mapcode='%s'\n", rc, mc);
    }
    return 1;
}


static int check_correct_decode_test(char *mc, enum Territory tc) {
    double lat;
    double lon;
    int rc = decodeMapcodeToLatLon(&lat, &lon, mc, tc);
    if (rc < 0) {
        found_error();
        printf("*** ERROR *** decodeMapcodeToLatLon returns '%d' (should be 0) for mapcode='%s'\n", rc, mc);
    }
    rc = compareWithMapcodeFormat(mc, 1);
    if (rc < 0) {
        found_error();
        printf("*** ERROR *** decodeMapcodeToLatLon returns '%d' (should be 0) for mapcode='%s'\n", rc, mc);
    }
    return 2;
}


static int decode_robustness_tests(void) {
    int nrTests = 0;
    int i;
    char s1[1];
    char largeString[16000];

    enum Territory tc = getTerritoryCode("NLD", TERRITORY_NONE);
    nrTests += check_incorrect_decode_test("", TERRITORY_NONE);
    nrTests += check_incorrect_decode_test(" ", TERRITORY_NONE);
    nrTests += check_incorrect_decode_test("AA", TERRITORY_NONE);
    nrTests += check_incorrect_decode_test("", tc);
    nrTests += check_incorrect_decode_test(" ", tc);
    nrTests += check_incorrect_decode_test("AA", tc);
    nrTests += check_incorrect_decode_test("XX.XX", TERRITORY_NONE);
    nrTests += check_correct_decode_test("NLD XX.XX", tc);
    nrTests += check_correct_decode_test("NLD 39.UC", tc);
    nrTests += check_correct_decode_test("W9.SX9", tc);
    nrTests += check_correct_decode_test("MEX 49.4V", tc);
    nrTests += check_correct_decode_test("NLD XX.XX", TERRITORY_NONE);
    nrTests += check_correct_decode_test("MX XX.XX", TERRITORY_NONE);

    s1[0] = 0;
    nrTests += check_incorrect_decode_test(s1, TERRITORY_NONE);
    nrTests += check_incorrect_decode_test(s1, tc);

    for (i = 0; i < sizeof(largeString) - 1; ++i) {
        largeString[i] = (char) ((i % 223) + 32);
    }
    largeString[sizeof(largeString) - 1] = 0;
    nrTests += check_incorrect_decode_test(s1, TERRITORY_NONE);
    nrTests += check_incorrect_decode_test(s1, tc);
    return nrTests;
}


static int robustness_tests(void) {
    int nrTests = 0;
    nrTests += get_territory_robustness_tests();
    nrTests += encode_robustness_tests();
    nrTests += decode_robustness_tests();
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
                enum Territory csvTerritoryCode;
                char *s = line;
                char *e = strchr(s, ',');
                if (e) {
                    linesTested++;
                    *e = 0;
                    csvTerritoryCode = TERRITORY_OF_INDEX(atoi(s));
                    s = e + 1;
                    // parse and check aliases
                    e = strchr(s, ',');
                    if (e) {
                        *e = 0;
                        while (*s) {
                            enum Territory territoryCode;
                            char *sep = strchr(s, '|');
                            if (sep) {
                                *sep = 0;
                            }
                            territoryCode = getTerritoryCode(s, TERRITORY_NONE);
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

#ifndef NO_SUPPORT_ALPHABETS
                        int csvNrAlphabets = 0;
                        const TerritoryAlphabets *territoryAlphabet = getAlphabetsForTerritory(csvTerritoryCode);
#endif // NO_SUPPORT_ALPHABETS

                        *e = 0;
                        while (*s) {
                            char *sep = strchr(s, '|');
                            if (sep) {
                                *sep = 0;
                            }

#ifndef NO_SUPPORT_ALPHABETS
                            csvNrAlphabets++;
                            if ((csvNrAlphabets > territoryAlphabet->count) ||
                                (atoi(s) != territoryAlphabet->alphabet[csvNrAlphabets - 1])) {
                                found_error();
                                printf("*** ERROR *** Mismatch: alphabet %d of territory %d should be %d\n",
                                       csvNrAlphabets, csvTerritoryCode, atoi(s));
                            }
#endif // NO_SUPPORT_ALPHABETS

                            if (sep) {
                                s = sep + 1;
                            } else {
                                s = e;
                            }
                        }

#ifndef NO_SUPPORT_ALPHABETS
                        if (csvNrAlphabets != territoryAlphabet->count) {
                            found_error();
                            printf("*** ERROR *** %d alphabets for territory %d, expected %d\n",
                                   territoryAlphabet->count, csvTerritoryCode, csvNrAlphabets);
                        }
#endif // NO_SUPPORT_ALPHABETS

                        s++;
                    }
                    // parse and check names
                    e = strchr(s, 10);
                    if (e) {
                        int i, noMoreNames = 0;
                        *e = 0;
                        for (i = 0; !noMoreNames; i++) {
                            char territoryName[MAX_TERRITORY_FULLNAME_LENGTH + 1];
                            noMoreNames = getFullTerritoryNameEnglish(territoryName, csvTerritoryCode, i);
                            if (!strstr(s, territoryName)) {
                                found_error();
                                printf("*** ERROR *** Name \"%s\" not found in \"%s\"\n", territoryName, s);
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


static int test_single_encodes(void) {
    int nrTests = 0;
    struct {
        double latDeg;
        double lonDeg;
        enum Territory territory;
        const char *result;
    } test_single_encode[] = {
            {52.3, 4.9, TERRITORY_NLD, "NLD GG.LCG-RJQ4"},
            {50,   5,   TERRITORY_BEL, "BEL T71.5V7-N0PT"},
            {32,   5,   TERRITORY_DZA, "DZA CG4.G4F6-KJ00"},
            {0,    0,   TERRITORY_NONE, NULL}
    };
    int i;
    for (i = 0; test_single_encode[i].result != NULL; i++) {
        char result[MAX_MAPCODE_RESULT_LEN];
        encodeLatLonToSingleMapcode(result, test_single_encode[i].latDeg, test_single_encode[i].lonDeg,
                                    test_single_encode[i].territory, 4);
        if (strcmp(result, test_single_encode[i].result) != 0) {
            printf("*** ERROR *** encodeLatLonToSingleMapcode()=%s, expected %s\n",
                   result, test_single_encode[i].result);
        }
        nrTests++;
    }
    return nrTests;
}

/**
 * ALPHABET TESTS.
 */

#ifndef NO_SUPPORT_ALPHABETS

static int check_alphabet_assertion(char *msg, int condition, char *format, int a) {
    if (condition == 0) {
        found_error();
        printf("*** ERROR *** %s, ", msg);
        printf(format, a);
        printf("\n");
    }
    return 1;
}


static int utf8_and_utf16_tests(void) {
    int i = 0;
    UWORD utf16[2] = {0, 0};
    char utf8[4]; // one 16-bit UTF16 code can become at most 3 UTF8 characters
    UWORD newUtf16[4]; // each UTF8 character can become at most 1 UTF16 character
    int err;
    for (i = 0; i < 0xFFFF; i++) { // to test codes 0x0001 up to and including 0xFFFF
        utf16[0]++;
        convertUtf16ToUtf8(utf8, utf16);
        err = convertUtf8ToUtf16(newUtf16, utf8);
        if (err != 0 || newUtf16[1] != 0 || utf16[0] != newUtf16[0]) {
            printf("*** ERROR *** UTF8/UTF16 errors found, first at UTF16 code %x\n", utf16[0]);
            break;
        }
    }
    return i; // nr of tests
}


static int alphabet_tests(void) {
    int nrTests = 0;
    int j;
    const char *str, *expect;
    static const char *alphabet_testpairs[] = {
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

    printf("%d alphabets\n", _ALPHABET_MAX);

    for (j = 0; alphabet_testpairs[j] != 0; j += 2) {
        enum Alphabet i;
        for (i = _ALPHABET_MIN + 1; i < _ALPHABET_MAX; i++) {
            UWORD enc[64];
            char dec[64];
            // see if alphabets (re)convert as expected
            str = alphabet_testpairs[j];
            expect = alphabet_testpairs[j + 1];
            convertToAlphabet(enc, 64, str, i);
            // if any characters, should be recoignisable
            {
                size_t k, n = 0;
                for (k = 0; k < strlen(str); k++) {
                    if (str[k] >= 'A' && str[k] <= 'z') {
                        n++;
                    }
                }
                if (n > 0) {
                    ++nrTests;
                    if (recognizeAlphabetUtf16(enc) != i) {
                        found_error();
                        printf("*** ERROR *** recognizeAlphabetUtf16(convertToAlphabet(\"%s\",%d))=%d\n", str, i, recognizeAlphabetUtf16(enc));
                    }
                }
            }
            convertToRoman(dec, 60, enc);
            ++nrTests;
            if (strcmp(dec, expect)) {
                found_error();
                printf("*** ERROR *** convertToRoman(convertToAlphabet(\"%s\",%d))=\"%s\"\n", str, (int) i, dec);
            }
        }
    }
    return nrTests;
}


static int alphabet_robustness_tests(void) {
    int nrTests = 0;
    int i;
    enum Alphabet a;
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

    for (a = _ALPHABET_MIN + 1; a < _ALPHABET_MAX; a++) {

        pu = convertToAlphabet(u1, sizeof(u1) / sizeof(u1[0]), "", a);
        nrTests += check_alphabet_assertion("convertToAlphabet cannot return 0", pu != 0, "alphabet=%d", a);
        nrTests += check_alphabet_assertion("convertToAlphabet must return empty string", pu[0] == 0, "alphabet=%d", a);

        ps = convertToRoman(s1, sizeof(s1) / sizeof(s1[0]), u1);
        nrTests += check_alphabet_assertion("convertToRoman cannot return 0", ps != 0, "alphabet=%d", a);
        nrTests += check_alphabet_assertion("convertToRoman must return empty string", ps[0] == 0, "alphabet=%d", a);

        pu = convertToAlphabet(largeUnicodeString1, sizeof(largeUnicodeString1) / sizeof(largeUnicodeString1[0]),
                               largeString1, ALPHABET_ROMAN);
        nrTests += check_alphabet_assertion("convertToAlphabet cannot return 0", pu != 0, "alphabet=%d", a);

        ps = convertToRoman(largeString1, sizeof(largeString1) / sizeof(largeString1[0]), pu);
        nrTests += check_alphabet_assertion("convertToRoman cannot return 0", ps != 0, "alphabet=%d", a);
        nrTests += check_alphabet_assertion("convertToRoman must return size",
                                            strlen(ps) < (sizeof(largeString1) / sizeof(largeString1[0])),
                                            "alphabet=%d", a);

        pu = convertToAlphabet(largeUnicodeString2, sizeof(largeUnicodeString2) / sizeof(largeUnicodeString2[0]),
                               largeString2, ALPHABET_ROMAN);
        nrTests += check_alphabet_assertion("convertToAlphabet cannot return 0", pu != 0, "alphabet=%d", a);

        ps = convertToRoman(largeString2, sizeof(largeString2) / sizeof(largeString2[0]), pu);
        nrTests += check_alphabet_assertion("convertToRoman cannot return 0", ps != 0, "alphabet=%d", a);
        nrTests += check_alphabet_assertion("convertToRoman must return size",
                                            strlen(ps) < (sizeof(largeString2) / sizeof(largeString2[0])),
                                            "alphabet=%d", a);
    }
    return nrTests;
}


static int alphabet_per_territory_tests(void) {
    int nrTests = 0;
    int i, j;
    for (i = _TERRITORY_MIN + 1; i < _TERRITORY_MAX; i++) {
        const TerritoryAlphabets *alphabetsForTerritory = getAlphabetsForTerritory((enum Territory) i);
        ++nrTests;
        if (alphabetsForTerritory->count < 1 || alphabetsForTerritory->count > MAX_ALPHABETS_PER_TERRITORY) {
            found_error();
            printf("*** ERROR *** Bad getAlphabetsForTerritory(%d) count: %d\n", i, alphabetsForTerritory->count);
        }
        for (j = 0; j < alphabetsForTerritory->count; j++) {
            ++nrTests;
            if (alphabetsForTerritory->alphabet[j] < 0 ||
                alphabetsForTerritory->alphabet[j] >= _ALPHABET_MAX) {
                found_error();
                printf("*** ERROR *** Bad alphabetsForTerritory[%d].alphabet[%d]: %d\n", i, j,
                       alphabetsForTerritory->alphabet[j]);
            }
        }
    }
    return nrTests;
}

#endif // NO_SUPPORT_ALPHABETS



int full_name_tests(void) {
    int nrTests = 0;
    enum Territory territory;
    int nrNames = 0;
    int maxLength = 0;
    for (territory = _TERRITORY_MIN + 1; territory < _TERRITORY_MAX; ++territory) {
        int alternative = 0;
        char territoryName[2048];
        nrTests++;
        for (alternative = 0;; alternative++) {
            int len;
            nrNames = getFullTerritoryNameEnglish(territoryName, territory, alternative);
            if (nrNames < alternative) {
                break;
            }
            len = (int) strlen(territoryName);
            if (len > MAX_TERRITORY_FULLNAME_LENGTH) {
                found_error();
                printf("*** ERROR *** Bad territoryname, %d characters (limit is %d): %s\n", len, MAX_TERRITORY_FULLNAME_LENGTH, territoryName);
            }
            if (len > maxLength) {
                maxLength = len;
            }

        }

#ifndef NO_SUPPORT_ALPHABETS
        for (alternative = 0;; alternative++) {
            int len;
            nrNames = getFullTerritoryNameLocal(territoryName, territory, alternative, _ALPHABET_MIN);
            if (nrNames<alternative) {
                break;
            }
            len = (int) strlen(territoryName);
            if (len > MAX_TERRITORY_FULLNAME_LENGTH) {
                found_error();
                printf("*** ERROR *** Bad territoryname, %d characters (limit is %d): %s\n", len, MAX_TERRITORY_FULLNAME_LENGTH, territoryName);
            }
            if (len > maxLength) {
                maxLength = len;
            }
        }
#endif // NO_SUPPORT_ALPHABETS

    }
    fprintf(stderr, "%d territory names, max length %d characters\n", nrNames, maxLength);
    return nrTests;
}


int main(const int argc, const char **argv) {
    int nrTests = 0;
    printf("Mapcode C Library Unit Tests\n");
    printf("Library version %s (data version %s)\n", mapcode_cversion, mapcode_dataversion);
#ifdef LIMIT_TO_MICRODEGREES
    printf("Compiler options: LIMIT_TO_MICRODEGREES\n");
#endif
#ifdef NO_SUPPORT_ALPHABETS
    printf("Compiler options: NO_SUPPORT_ALPHABETS\n");
#endif
#ifdef NO_POSIX_THREADS
    printf("Compiler options: NO_POSIX_THREADS\n");
#else
    printf("Using up to %d threads to test in parallel...\n", MAX_THREADS);
#endif

    printf("-----------------------------------------------------------\nRobustness tests\n");
    nrTests += robustness_tests();

#ifndef NO_SUPPORT_ALPHABETS
    printf("-----------------------------------------------------------\nUTF8/UTF16 tests\n");
    nrTests += utf8_and_utf16_tests();

    printf("-----------------------------------------------------------\nAlphabet tests\n");
    nrTests += alphabet_robustness_tests();
    nrTests += alphabet_tests();

    printf("-----------------------------------------------------------\nAlphabet per territory tests\n");
    nrTests += alphabet_per_territory_tests();
#endif

    printf("-----------------------------------------------------------\nDistance tests\n");
    nrTests += distance_tests();

    printf("-----------------------------------------------------------\nDistance tests\n");
    nrTests += full_name_tests();

    printf("-----------------------------------------------------------\nTerritory tests\n");
    nrTests += test_territories_csv();
    nrTests += test_territories();
    nrTests += territory_code_tests();
    nrTests += test_territory_insides();

    printf("-----------------------------------------------------------\nFormat tests\n");
    nrTests += test_mapcode_formats();
    nrTests += test_failing_decodes();

    printf("-----------------------------------------------------------\nEncode/decode tests\n");
    nrTests += test_single_encodes();
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
