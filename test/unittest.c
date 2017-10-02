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
#define pthread_join(ignore1, ignore2) (!ignore1)
#define pthread_create(ignore1, ignore2, func, context) func(context)
#define MAX_THREADS 1
#else

#include <pthread.h>

#define MAX_THREADS 16      // Optimal: not too much, approx. nr of cores * 2, better no more than 32.
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

#define REF(x) if (x) {} else {}

static int nrErrors = 0;


static void foundError(void) {
    pthread_mutex_lock(&mutex);
    ++nrErrors;
    pthread_mutex_unlock(&mutex);
}


static char *myConvertToRoman(char *asciiBuffer, const UWORD *utf16String) {
    MapcodeElements mapcodeElements;
    double lat, lon;
    *asciiBuffer = 0;
    decodeMapcodeToLatLonUtf16(&lat, &lon, utf16String, TERRITORY_FRA, &mapcodeElements);
    sprintf(asciiBuffer, "%s%s%s%s%s",
            mapcodeElements.territoryISO,
            *mapcodeElements.territoryISO ? " " : "",
            mapcodeElements.properMapcode,
            *mapcodeElements.precisionExtension ? "-" : "",
            mapcodeElements.precisionExtension);
    return asciiBuffer;
}


static enum MapcodeError myParseMapcodeString(
        MapcodeElements *mapcodeElements,
        const char *string,
        enum Territory territory) {
    double lat, lon;
    enum MapcodeError err = decodeMapcodeToLatLonUtf8(&lat, &lon, string, territory, mapcodeElements);
    // filter out post-parsing errors
    if (err == ERR_MISSING_TERRITORY || err == ERR_MAPCODE_UNDECODABLE || err == ERR_EXTENSION_UNDECODABLE) {
        return ERR_OK;
    }
    return err;
}


static int testMapcodeFormats(void) {
    int nrTests = 0;
    static const struct {
        const char *input;              // user input
        enum MapcodeError parseError;   // expected error
        enum MapcodeError decodeError;  // expected error when decoded
    } formattests[] = {
            {"nld bc.xy-x1y",        ERR_OK,                          ERR_OK},
            {"pan a1.2e-b2c",        ERR_OK,                          ERR_OK},
            {"###################",  ERR_INVALID_CHARACTER,           ERR_OK},
            {"...................",  ERR_UNEXPECTED_DOT,              ERR_OK},
            {"1111111111111111.11",  ERR_INVALID_MAPCODE_FORMAT,      ERR_OK},
            {"US-XXXXXXXXXXXXXXXX",  ERR_BAD_TERRITORY_FORMAT,        ERR_OK},
            {"US-----------------",  ERR_UNEXPECTED_HYPHEN,           ERR_OK},
            {"-------------------",  ERR_UNEXPECTED_HYPHEN,           ERR_OK},

            {"cck XX.XX",            ERR_OK,                          ERR_OK}, // nameless22
            {"cze XX.XXX",           ERR_OK,                          ERR_OK}, // nameless23
            {"NLD XXX.XX",           ERR_OK,                          ERR_OK}, // nameless32
            {"VAT 5d.dd",            ERR_OK,                          ERR_OK}, // Grid22
            {"NLD XX.XXX",           ERR_OK,                          ERR_OK}, // Grid23
            {"bhr xxx.xx",           ERR_OK,                          ERR_OK}, // Grid32
            {"FRA XXX.XXX",          ERR_OK,                          ERR_OK}, // Grid33
            {"irl xx.xxxx",          ERR_OK,                          ERR_OK}, // Grid24
            {"cub xxxx.xx",          ERR_OK,                          ERR_OK}, // Grid42
            {"ben xxxx.xxx",         ERR_OK,                          ERR_OK}, // Grid34
            {"USA xxxx.xxxx",        ERR_OK,                          ERR_OK}, // Grid44
            {"US-AZ hhh.hh",         ERR_OK,                          ERR_OK}, // HGrid32
            {"Bel hhh.hhh",          ERR_OK,                          ERR_OK}, // HGrid33
            {"PAN hh.hhhh",          ERR_OK,                          ERR_OK}, // HGrid24
            {"GRC hhhh.hh",          ERR_OK,                          ERR_OK}, // HGrid42
            {"NZL hhhh.hhh",         ERR_OK,                          ERR_OK}, // HGrid43
            {"KAZ hhh.hhhh",         ERR_OK,                          ERR_OK}, // HGrid34
            {"RUS xxxx.xxxx",        ERR_OK,                          ERR_OK}, // HGrid44
            {"CN-SH hhhh.hhhh",      ERR_OK,                          ERR_OK}, // HGrid44
            {"VAT hhhhh.hhhh",       ERR_OK,                          ERR_OK}, // HGrid54
            {"hhhhh.hhhh",           ERR_OK,                          ERR_OK}, // HGrid54
            {"TUV hh.hhh",           ERR_OK,                          ERR_OK}, // AutoHeader23
            {"LVA L88.ZVR",          ERR_OK,                          ERR_OK}, // AutoHeader33
            {"WLF XLG.3GP",          ERR_OK,                          ERR_OK}, // HGrid33 R
            {"VAT j0q3.27r",         ERR_OK,                          ERR_OK}, // HGrid43 R
            {"PAK hhhh.hhhh",        ERR_OK,                          ERR_OK}, // HGrid44 R

            {"NLD 49.4V",            ERR_OK,                          ERR_OK},
            {"NLX 49.4V",            ERR_UNKNOWN_TERRITORY,           ERR_OK},
            {"49.4V",                ERR_OK,                          ERR_MISSING_TERRITORY},
            {"BRA 49.4V",            ERR_OK,                          ERR_MAPCODE_UNDECODABLE}, // type 0
            {"BRA XXXXX.XXX",        ERR_OK,                          ERR_MAPCODE_UNDECODABLE}, // type 0
            {"NLD XXXX.XXXX",        ERR_OK,                          ERR_MAPCODE_UNDECODABLE}, // type 0
            {"NLD ZZ.ZZ",            ERR_OK,                          ERR_MAPCODE_UNDECODABLE}, // type 1 nameless
            {"NLD Q000.000",         ERR_OK,                          ERR_MAPCODE_UNDECODABLE}, // type 2 grid
            {"NLD L222.222",         ERR_OK,                          ERR_MAPCODE_UNDECODABLE}, // type 3 restricted
            {"usa A222.22AA",        ERR_OK,                          ERR_MAPCODE_UNDECODABLE}, // type 4 grid
            {"atf hhh.hhh",          ERR_OK,                          ERR_MAPCODE_UNDECODABLE}, // type 5 autoh zone
            {"ASM zz.zzh",           ERR_OK,                          ERR_MAPCODE_UNDECODABLE}, // type 6 autoh out

            {"nld ZNZ.RZG-B",        ERR_OK,                          ERR_MAPCODE_UNDECODABLE}, // type 0
            {"WLF 01.AE-09V",        ERR_OK,                          ERR_OK},
            {"LVA LDV.ZVR-B  ",      ERR_OK,                          ERR_OK}, // AutoHeader
            {"LVA LDV.ZVR-BY  ",     ERR_OK,                          ERR_EXTENSION_UNDECODABLE},
            {"01.AE",                ERR_OK,                          ERR_MISSING_TERRITORY},
            {"nld 01.AE",            ERR_OK,                          ERR_OK},
            {"nld oi.AE",            ERR_OK,                          ERR_OK},
            {"oi.oi",                ERR_ALL_DIGIT_CODE,              ERR_OK},
            {"nld oi.OI-xxx",        ERR_ALL_DIGIT_CODE,              ERR_OK},
            {"CUB 3467.UY",          ERR_OK,                          ERR_OK},
            {"34.UY",                ERR_OK,                          ERR_MISSING_TERRITORY},
            {"mx XX.XX",             ERR_OK,                          ERR_OK},
            {"",                     ERR_DOT_MISSING,                 ERR_OK},
            {"ttat.tt    ",          ERR_INVALID_VOWEL,               ERR_OK},
            {"ttat-tt tt.tt",        ERR_INVALID_VOWEL,               ERR_OK},
            {"ttat tt.tt",           ERR_INVALID_VOWEL,               ERR_OK},
            {"XXAX.XXXX",            ERR_INVALID_VOWEL,               ERR_OK},
            {"2A22.2222",            ERR_INVALID_VOWEL,               ERR_OK},
            {"22A2.2222",            ERR_INVALID_VOWEL,               ERR_OK},
            {"MAP.CODE",             ERR_INVALID_VOWEL,               ERR_OK},
            {"XAXX.XXXX",            ERR_INVALID_VOWEL,               ERR_OK},
            {"XXXA.XXXX",            ERR_INVALID_VOWEL,               ERR_OK},
            {"XXXAX.XXXX",           ERR_INVALID_VOWEL,               ERR_OK},
            {"XXXXA.XXXX",           ERR_INVALID_VOWEL,               ERR_OK},
            {"nld XXXX.XXXXA",       ERR_INVALID_VOWEL,               ERR_OK},
            {"nld XXXX.ALA",         ERR_INVALID_VOWEL,               ERR_OK},
            {"nld XXXX.LAXA",        ERR_INVALID_VOWEL,               ERR_OK},
            {"nld XXXX.LLLLA",       ERR_INVALID_VOWEL,               ERR_OK},
            {"nld XXXX.A2e",         ERR_INVALID_VOWEL,               ERR_OK},
            {"nld XXXX.2e2e",        ERR_INVALID_VOWEL,               ERR_OK},
            {"nld XXXX.2222u",       ERR_INVALID_VOWEL,               ERR_OK},
            {"222A.2222",            ERR_INVALID_VOWEL,               ERR_OK},
            {"222A2.2222",           ERR_INVALID_VOWEL,               ERR_OK},
            {"ttt 2222A.2222",       ERR_INVALID_VOWEL,               ERR_OK},
            {"2222.2AAA",            ERR_INVALID_VOWEL,               ERR_OK},
            {"A222.2AAA",            ERR_INVALID_VOWEL,               ERR_OK},
            {"usa 2222.22A2",        ERR_OK,                          ERR_OK},
            {"usa 2222.22AA",        ERR_OK,                          ERR_OK},
            {".123",                 ERR_UNEXPECTED_DOT,              ERR_OK},
            {".xyz",                 ERR_UNEXPECTED_DOT,              ERR_OK},
            {"x.xyz",                ERR_UNEXPECTED_DOT,              ERR_OK},
            {"xxx.z-12",             ERR_UNEXPECTED_HYPHEN,           ERR_OK},
            {"xx.xx.",               ERR_UNEXPECTED_DOT,              ERR_OK},
            {"xxxx xx.xx",           ERR_BAD_TERRITORY_FORMAT,        ERR_OK},
            {"xxxxx xx.xx",          ERR_BAD_TERRITORY_FORMAT,        ERR_OK},
            {"123",                  ERR_DOT_MISSING,                 ERR_OK},
            {"xxx.",                 ERR_MAPCODE_INCOMPLETE,          ERR_OK},
            {"xxx.z",                ERR_MAPCODE_INCOMPLETE,          ERR_OK},
            {"NLD 49.4V-",           ERR_MAPCODE_INCOMPLETE,          ERR_OK},
            {"NLD 49.4V",            ERR_OK,                          ERR_OK},
            {"   NLD   49.4V  ",     ERR_OK,                          ERR_OK},
            {"NLD 49.4V-1",          ERR_OK,                          ERR_OK},
            {"NLD 49.4V-12",         ERR_OK,                          ERR_OK},
            {"NLD 49.4V-123",        ERR_OK,                          ERR_OK},
            {"NLD 49.4V-12345678",   ERR_OK,                          ERR_OK},
            {"NLD 49.4V- ",          ERR_EXTENSION_INVALID_LENGTH,    ERR_OK},
            {"NLD 49.4V-123456789",  ERR_EXTENSION_INVALID_LENGTH,    ERR_OK},
            {"49.4V-xxxxxxxxxxxxxx", ERR_EXTENSION_INVALID_LENGTH,    ERR_OK},
            {"DD.DD-        ",       ERR_EXTENSION_INVALID_LENGTH,    ERR_OK},
            {"nld DD.DD-",           ERR_MAPCODE_INCOMPLETE,          ERR_OK},
            {"TAM 49.4V",            ERR_OK,                          ERR_OK},
            {"BRA 49.4V",            ERR_OK,                          ERR_MAPCODE_UNDECODABLE}, // type 0
            {"CA 49.4V",             ERR_OK,                          ERR_OK},
            {"N 49.4V",              ERR_BAD_TERRITORY_FORMAT,        ERR_OK},
            {"XXXX ",                ERR_BAD_TERRITORY_FORMAT,        ERR_OK},
            {"XXXXX ",               ERR_BAD_TERRITORY_FORMAT,        ERR_OK},
            {"XXXX 49.4V",           ERR_BAD_TERRITORY_FORMAT,        ERR_OK},
            {"XXXXX 49.4V",          ERR_BAD_TERRITORY_FORMAT,        ERR_OK},
            {"-XX 49.4V",            ERR_UNEXPECTED_HYPHEN,           ERR_OK},
            {"X-XX 49.4V",           ERR_BAD_TERRITORY_FORMAT,        ERR_OK},
            {"XXXX-XX 49.4V",        ERR_BAD_TERRITORY_FORMAT,        ERR_OK},
            {"XX-X 49.4V",           ERR_BAD_TERRITORY_FORMAT,        ERR_OK},
            {"XX-XXXX 49.4V",        ERR_BAD_TERRITORY_FORMAT,        ERR_OK},
            {"XX-XXXE 49.4V",        ERR_BAD_TERRITORY_FORMAT,        ERR_OK},
            {"12.34",                ERR_ALL_DIGIT_CODE,              ERR_OK},
            {"NLD 12.34",            ERR_ALL_DIGIT_CODE,              ERR_OK},
            {"AAA 12.34",            ERR_ALL_DIGIT_CODE,              ERR_OK},
            {"AAA 12.34-XXX",        ERR_ALL_DIGIT_CODE,              ERR_OK},
            {"123 12.34-123",        ERR_ALL_DIGIT_CODE,              ERR_OK},
            {"xx-xx 12.34",          ERR_ALL_DIGIT_CODE,              ERR_OK},
            {"12-34 12.34",          ERR_ALL_DIGIT_CODE,              ERR_OK},
            {"CN-34 12.3X",          ERR_OK,                          ERR_OK},
            {"  TAM  XX.XX-XX  ",    ERR_OK,                          ERR_OK},
            {"  TAM  XXX.XX-XX  ",   ERR_OK,                          ERR_OK},
            {"  TAM  XX.XXX-XX  ",   ERR_OK,                          ERR_OK},
            {"  TAM  XX.XXXX-XX  ",  ERR_OK,                          ERR_OK},
            {"  TAM  XXX.XXX-XX  ",  ERR_OK,                          ERR_OK},
            {"  gab  XXXX.XX-XX  ",  ERR_OK,                          ERR_OK},
            {"  kAZ  XXX.XXXX-XX ",  ERR_OK,                          ERR_OK},
            {"  IND  XXXX.XXX-XX ",  ERR_OK,                          ERR_OK},
            {" USA  XXXX.XXXX-XX ",  ERR_OK,                          ERR_OK},
            {" VAT XXXXX.XXXX-XX  ", ERR_OK,                          ERR_OK},
            {" NLD XXXXX.XXXX-XX  ", ERR_OK,                          ERR_OK},
            {" USA XXXXX.XXXX-XX  ", ERR_OK,                          ERR_OK},
            {" XXXXX.XXXX-XX  ",     ERR_OK,                          ERR_OK},
            {" usa  XXXXX.XXX-XX ",  ERR_OK,                          ERR_MAPCODE_UNDECODABLE}, // type 0
            {" XXXXX.XXX-XX ",       ERR_OK,                          ERR_MISSING_TERRITORY},
            {"xx-xx.x xx.xx",        ERR_UNEXPECTED_DOT,              ERR_OK},
            {"xx-xx-x xx.xx",        ERR_UNEXPECTED_HYPHEN,           ERR_OK},
            {"xx.xx-x-x",            ERR_UNEXPECTED_HYPHEN,           ERR_OK},
            {"xx-xx xx-xx",          ERR_UNEXPECTED_HYPHEN,           ERR_OK},
            {"xx-xx xx-xx.xx",       ERR_UNEXPECTED_HYPHEN,           ERR_OK},
            {"xx.xx.xx",             ERR_UNEXPECTED_DOT,              ERR_OK},
            {"xx-xx xx.xx.xx",       ERR_UNEXPECTED_DOT,              ERR_OK},
            {"xx-xx xx.xx-xx-xx",    ERR_UNEXPECTED_HYPHEN,           ERR_OK},
            {"xx-xx xx.xx x",        ERR_TRAILING_CHARACTERS,         ERR_OK},
            {"xx-xx xx.xx-x x",      ERR_TRAILING_CHARACTERS,         ERR_OK},
            {"xx-xx xx.xx-x -",      ERR_UNEXPECTED_HYPHEN,           ERR_OK},
            {"xx-xx xx.xx-x .",      ERR_UNEXPECTED_DOT,              ERR_OK},
            {"xx-xx xx.xx-x 2",      ERR_TRAILING_CHARACTERS,         ERR_OK},
            {"xx-xx xx.x#x",         ERR_INVALID_CHARACTER,           ERR_OK},
            {"xx# xx.xx",            ERR_INVALID_CHARACTER,           ERR_OK},
            {"xx-xx #xx.xx",         ERR_INVALID_CHARACTER,           ERR_OK},
            {"xx-xx xx.xx-xx#xx",    ERR_INVALID_CHARACTER,           ERR_OK},
            {"xx-xx -xx.xx",         ERR_UNEXPECTED_HYPHEN,           ERR_OK},
            {"xx-xx .xx.xx",         ERR_UNEXPECTED_DOT,              ERR_OK},
            {".123",                 ERR_UNEXPECTED_DOT,              ERR_OK},
            {"  .123",               ERR_UNEXPECTED_DOT,              ERR_OK},
            {"",                     ERR_DOT_MISSING,                 ERR_OK},
            {"  ",                   ERR_DOT_MISSING,                 ERR_OK},
            {"-xx.xx",               ERR_UNEXPECTED_HYPHEN,           ERR_OK},
            {"  - xx.xx",            ERR_UNEXPECTED_HYPHEN,           ERR_OK},
            {"D xx.xx",              ERR_BAD_TERRITORY_FORMAT,        ERR_OK},
            {"D.123",                ERR_UNEXPECTED_DOT,              ERR_OK},
            {"D",                    ERR_DOT_MISSING,                 ERR_OK},
            {"D-xxxxx",              ERR_BAD_TERRITORY_FORMAT,        ERR_OK},
            {"DD",                   ERR_DOT_MISSING,                 ERR_OK},
            {"DDDa.DDD",             ERR_INVALID_VOWEL,               ERR_OK},
            {"DDD",                  ERR_DOT_MISSING,                 ERR_OK},
            {"DDDD xx.xx",           ERR_BAD_TERRITORY_FORMAT,        ERR_OK},
            {"DDDDE.xxxx",           ERR_INVALID_VOWEL,               ERR_OK},
            {"DDDD",                 ERR_DOT_MISSING,                 ERR_OK},
            {"DDDD-CA xx.xx",        ERR_BAD_TERRITORY_FORMAT,        ERR_OK},
            {"DDDDD CA xx.xx",       ERR_BAD_TERRITORY_FORMAT,        ERR_OK},
            {"DDDDDA   xx.xx",       ERR_INVALID_VOWEL,               ERR_OK},
            {"DDDDD",                ERR_DOT_MISSING,                 ERR_OK},
            {"DDDDD-CA xx.xx",       ERR_BAD_TERRITORY_FORMAT,        ERR_OK},
            {"DDDDD..xxxx",          ERR_UNEXPECTED_DOT,              ERR_OK},
            {"DDDDD.",               ERR_MAPCODE_INCOMPLETE,          ERR_OK},
            {"DDDDD.-xxxx.xx",       ERR_UNEXPECTED_HYPHEN,           ERR_OK},
            {"DDD.L.LLL     ",       ERR_UNEXPECTED_DOT,              ERR_OK},
            {"DDD.L",                ERR_MAPCODE_INCOMPLETE,          ERR_OK},
            {"DDD.L-xxxxxxxx",       ERR_UNEXPECTED_HYPHEN,           ERR_OK},
            {"DD.DD.CA",             ERR_UNEXPECTED_DOT,              ERR_OK},
            {"DD.DDD.CA",            ERR_UNEXPECTED_DOT,              ERR_OK},
            {"DD.DDDD.CA    ",       ERR_UNEXPECTED_DOT,              ERR_OK},
            {"DD.DDDDA      ",       ERR_INVALID_VOWEL,               ERR_OK},
            {"DD.DD-.       ",       ERR_UNEXPECTED_DOT,              ERR_OK},
            {"DD.DD-",               ERR_MAPCODE_INCOMPLETE,          ERR_OK},
            {"DD.DD--XXX",           ERR_UNEXPECTED_HYPHEN,           ERR_OK},
            {"DD.DD-x.      ",       ERR_UNEXPECTED_DOT,              ERR_OK},
            {"DD.DD-A",              ERR_EXTENSION_INVALID_CHARACTER, ERR_OK},
            {"DD.DD-xA",             ERR_EXTENSION_INVALID_CHARACTER, ERR_OK},
            {"DD.DD-xxxE",           ERR_EXTENSION_INVALID_CHARACTER, ERR_OK},
            {"DD.DD-xxxxxu",         ERR_EXTENSION_INVALID_CHARACTER, ERR_OK},
            {"DD.DD-x-xxx",          ERR_UNEXPECTED_HYPHEN,           ERR_OK},
            {"ta.xx     ",           ERR_INVALID_VOWEL,               ERR_OK},
            {"ta",                   ERR_DOT_MISSING,                 ERR_OK},
            {"DAD-        ",         ERR_BAD_TERRITORY_FORMAT,        ERR_OK},
            {"DAD-.       ",         ERR_UNEXPECTED_DOT,              ERR_OK},
            {"DAD-",                 ERR_BAD_TERRITORY_FORMAT,        ERR_OK},
            {"DAD--XXX",             ERR_UNEXPECTED_HYPHEN,           ERR_OK},
            {"DAD-X  xx.xx",         ERR_BAD_TERRITORY_FORMAT,        ERR_OK},
            {"DAD-X.      ",         ERR_UNEXPECTED_DOT,              ERR_OK},
            {"DAD-X",                ERR_BAD_TERRITORY_FORMAT,        ERR_OK},
            {"DAD-X-XXX",            ERR_UNEXPECTED_HYPHEN,           ERR_OK},
            {"DAD-XX.XX   ",         ERR_UNEXPECTED_DOT,              ERR_OK},
            {"DAD-XX",               ERR_DOT_MISSING,                 ERR_OK},
            {"DAD-XX-XX",            ERR_UNEXPECTED_HYPHEN,           ERR_OK},
            {"DAD-XXX.XX   ",        ERR_UNEXPECTED_DOT,              ERR_OK},
            {"DAD-XXXX",             ERR_BAD_TERRITORY_FORMAT,        ERR_OK},
            {"DAD-XXXA",             ERR_BAD_TERRITORY_FORMAT,        ERR_OK},
            {"DAD-XXX",              ERR_DOT_MISSING,                 ERR_OK},
            {"DAD-XXX-XX",           ERR_UNEXPECTED_HYPHEN,           ERR_OK},
            {"DAD-XX  .XX   ",       ERR_UNEXPECTED_DOT,              ERR_OK},
            {"DAD-XX  ",             ERR_DOT_MISSING,                 ERR_OK},
            {"DAD-XX  -XX",          ERR_UNEXPECTED_HYPHEN,           ERR_OK},
            {"DD-DD A      ",        ERR_DOT_MISSING,                 ERR_OK},
            {"DD-DD A.     ",        ERR_UNEXPECTED_DOT,              ERR_OK},
            {"DD-DD AA.33  ",        ERR_INVALID_VOWEL,               ERR_OK},
            {"DD-DD A",              ERR_DOT_MISSING,                 ERR_OK},
            {"DD-DD A-XX",           ERR_UNEXPECTED_HYPHEN,           ERR_OK},
            {"DD-DD A3     ",        ERR_DOT_MISSING,                 ERR_OK},
            {"DD-DD A3A.XX ",        ERR_INVALID_VOWEL,               ERR_OK},
            {"DD-DD A3",             ERR_DOT_MISSING,                 ERR_OK},
            {"DD-DD A3-XX",          ERR_UNEXPECTED_HYPHEN,           ERR_OK},
            {"DD-DD A33    ",        ERR_DOT_MISSING,                 ERR_OK},
            {"DD-DD A33A.XX",        ERR_INVALID_VOWEL,               ERR_OK},
            {"DD-DD A33",            ERR_DOT_MISSING,                 ERR_OK},
            {"DD-DD A33-XX",         ERR_UNEXPECTED_HYPHEN,           ERR_OK},
            {"DD-DD xx.xx .",        ERR_UNEXPECTED_DOT,              ERR_OK},
            {"DD-DD xx.xx x",        ERR_TRAILING_CHARACTERS,         ERR_OK},
            {"DD-DD xx.xx a",        ERR_TRAILING_CHARACTERS,         ERR_OK},
            {"DD-DD xx.xx -x",       ERR_UNEXPECTED_HYPHEN,           ERR_OK},
            {"xx.xx .xx",            ERR_UNEXPECTED_DOT,              ERR_OK},
            {"xx.xx x",              ERR_TRAILING_CHARACTERS,         ERR_OK},
            {"xx.xx a",              ERR_TRAILING_CHARACTERS,         ERR_OK},
            {"xx.xx -123",           ERR_UNEXPECTED_HYPHEN,           ERR_OK},
            {" xx.xx-DD .",          ERR_UNEXPECTED_DOT,              ERR_OK},
            {" xx.xx-DD x",          ERR_TRAILING_CHARACTERS,         ERR_OK},
            {" xx.xx-DD a",          ERR_TRAILING_CHARACTERS,         ERR_OK},
            {"xx.xx xxxxxxxxxxxx",   ERR_TRAILING_CHARACTERS,         ERR_OK},
            {" xx.xx-DD -",          ERR_UNEXPECTED_HYPHEN,           ERR_OK},
            {"tta.ttt    ",          ERR_INVALID_VOWEL,               ERR_OK},
            {"ttaa.ttt   ",          ERR_INVALID_VOWEL,               ERR_OK},
            {"tta",                  ERR_DOT_MISSING,                 ERR_OK},

            {"DDD. ",                ERR_INVALID_MAPCODE_FORMAT,      ERR_OK}, // 6/0 : white na dot
            {"DDDDD. xxxx.xx",       ERR_INVALID_MAPCODE_FORMAT,      ERR_OK}, // 6/0 : white na dot
            {"DDD.L         ",       ERR_INVALID_MAPCODE_FORMAT,      ERR_OK}, // 7.0 : postfix too short
            {"DDDDDD   xx.xx",       ERR_INVALID_MAPCODE_FORMAT,      ERR_OK}, // 5/2 : 6char ter
            {"DDDDDD.xxx",           ERR_INVALID_MAPCODE_FORMAT,      ERR_OK}, // 5/2 : 6char mc
            {"XXXX.XXXXX",           ERR_OK,                          ERR_MISSING_TERRITORY},   // 4/5
            {"XXXXX.XXXXX",          ERR_OK,                          ERR_MAPCODE_UNDECODABLE}, // 5/5

            // errors because there are too many letters after a postfix vowel
            {"XXXX.AXXX",            ERR_INVALID_VOWEL,               ERR_OK},
            {"nld XXXX.AXX",         ERR_INVALID_VOWEL,               ERR_OK},
            {"nld XXXX.XAXX",        ERR_INVALID_VOWEL,               ERR_OK},
            {"nld XXXX.AXXA",        ERR_INVALID_VOWEL,               ERR_OK},
            {"2222.A22",             ERR_INVALID_VOWEL,               ERR_OK},
            {"2222.A222",            ERR_INVALID_VOWEL,               ERR_OK},
            {"2222.2A22",            ERR_INVALID_VOWEL,               ERR_OK},
            // 5th letter
            {"nld DD.DDDDD  ",       ERR_OK,                          ERR_MAPCODE_UNDECODABLE},
            {"nld XXXX.XXXXX",       ERR_OK,                          ERR_MAPCODE_UNDECODABLE},
            {"TAM XX.XXXXX-XX",      ERR_OK,                          ERR_MAPCODE_UNDECODABLE},
            {"TAM XXX.XXXXX-XX",     ERR_OK,                          ERR_MAPCODE_UNDECODABLE},
            {"TAM XXXX.XXXXX-X",     ERR_OK,                          ERR_MAPCODE_UNDECODABLE},
            {"TAM XXXXX.XXXXX-X",    ERR_OK,                          ERR_MAPCODE_UNDECODABLE},
            {"40822.schol",          ERR_OK,                          ERR_MAPCODE_UNDECODABLE},
            {"AAA 40822.schol",      ERR_OK,                          ERR_MAPCODE_UNDECODABLE},

            // errors because the postfix has a 6th letter
            {"DD.DDDDDD      ",      ERR_INVALID_MAPCODE_FORMAT,      ERR_OK},
            {"nld XXXX.XXXXXX",      ERR_INVALID_MAPCODE_FORMAT,      ERR_OK},
            {" TAM  XX.XXXXXX-XX ",  ERR_INVALID_MAPCODE_FORMAT,      ERR_OK},
            {" TAM  XXX.XXXXXX-XX ", ERR_INVALID_MAPCODE_FORMAT,      ERR_OK},
            {" TAM  XXXX.XXXXXX-X ", ERR_INVALID_MAPCODE_FORMAT,      ERR_OK},
            {" TAM XXXXX.XXXXXX-X ", ERR_INVALID_MAPCODE_FORMAT,      ERR_OK},

            // Check tabs, spaces and control characters.
            {"NLD 49.YV",            ERR_OK,                          ERR_OK},
            {" NLD 49.YV",           ERR_OK,                          ERR_OK},
            {"\tNLD 49.YV",          ERR_OK,                          ERR_OK},
            {"NLD 49.YV ",           ERR_OK,                          ERR_OK},
            {"NLD 49.YV\t",          ERR_OK,                          ERR_OK},
            {"NLD  49.YV",           ERR_OK,                          ERR_OK},
            {"NLD\t49.YV",           ERR_OK,                          ERR_OK},
            {"NLD\n49.YV",           ERR_INVALID_CHARACTER,           ERR_OK},
            {"NLD\r49.YV",           ERR_INVALID_CHARACTER,           ERR_OK},
            {"NLD\v49.YV",           ERR_INVALID_CHARACTER,           ERR_OK},
            {"NLD\b49.YV",           ERR_INVALID_CHARACTER,           ERR_OK},
            {"NLD\a49.YV",           ERR_INVALID_CHARACTER,           ERR_OK},
            {NULL,                   ERR_OK,                          ERR_OK}
    };

    int shouldSucceed = 0; // count nr of calls that SHOULD be successful.
    int total = 0;
    int succeeded = 0;
    int i;

    for (i = 0; formattests[i].input != NULL; ++i) {
        MapcodeElements mapcodeElements;
        enum MapcodeError parseError = myParseMapcodeString(&mapcodeElements, formattests[i].input, TERRITORY_UNKNOWN);
        enum MapcodeError formatError = compareWithMapcodeFormatUtf8(formattests[i].input);
        if (formattests[i].parseError == ERR_OK) {
            shouldSucceed++;
        }

        nrTests++;
        if (parseError != formatError) {
            // there is a special case where parse knows about valid territories
            if (formatError || formattests[i].parseError != ERR_UNKNOWN_TERRITORY) {
                foundError();
                printf("*** ERROR *** \"%s\" : myParseMapcodeString=%d, compareWithMapcodeFormatUtf8=%d\n",
                       formattests[i].input, parseError, formatError);
            }
        }

        nrTests++;
        if (formattests[i].parseError != parseError) {
            foundError();
            printf("*** ERROR *** compareWithMapcodeFormatUtf8(\"%s\") returns %d (%d expected)\n",
                   formattests[i].input,
                   parseError, formattests[i].parseError);
        }

        nrTests++;
        ++total;
        if (parseError == 0) {
            double lat, lon;
            int decodeError = decodeMapcodeToLatLonUtf8(&lat, &lon, formattests[i].input, TERRITORY_UNKNOWN, NULL);
            ++succeeded;
            if (decodeError != formattests[i].decodeError) {
                foundError();
                printf("*** ERROR *** myParseMapcodeString(\"%s\")=%d, expected %d\n", formattests[i].input,
                       decodeError,
                       formattests[i].decodeError);
            }
        }
    }
    if (succeeded != shouldSucceed) {
        foundError();
        printf("*** ERROR *** %d of %d myParseMapcodeString() calls succeeded (expected %d)\n", succeeded, total,
               shouldSucceed);
    }
    return nrTests;
}


static int testAlphabetParser(void) {
    int nrTests = 0;
    static const struct {
        const char *userInput;
        const char *expected;
    } parseTests[] = {
            {"nld bc.xy-x1y",   "NLD BC.XY-X1Y (1113;2)"},
            {"nld Α0.12",       "NLD 00.E0 (1113;2)"},       // A-encoded greek
            {"DNK  РФ.ХХ",      "DNK PQ.XX (1114;2)"},       // greek
            {"GRC HP.NO-1Х2ХХ", "GRC HP.NO-1X2XX (1150;2)"},
            {"PRT 31.E2-b2c",   "PRT 31.E2-B2C (1136;2)"},
            {"GBR  רר.56ר",     "GBR XX.XX (1167;2)"},       // hebrew abjad
            {"BEL  طظ.56ط ",    "BEL PQ.XP (1107;2)"},       // arab abjad
            {"FRA  ヒフ.ラヲ",      "FRA PQ.XZ (1198;2)"},
            {"CA XX.XX",        "CA XX.XX (1392;2)"}
    };
    static const struct {
        const char *input;
        enum Alphabet alphabet;
        const char *expected;
    } convertTests[] = {
            {"nld bc.XY-p2q", ALPHABET_ROMAN,      "nld BC.XY-P2Q"},
            {"DNK PQ.XX",     ALPHABET_DEVANAGARI, "DNK नप.सस"},
            {"GBR XX.XX",     ALPHABET_HEBREW,     "GBR רר.56ר"},
            {"BEL PQ.XP",     ALPHABET_ARABIC,     "BEL طظ.56ط"},
            {"nld 00.E0",     ALPHABET_GREEK,      "nld \xCE\x91\x30.12"}
    };
    int i;
    for (i = 0; i < (int) (sizeof(convertTests) / sizeof(convertTests[0])); i++) {
        char utf8[MAX_MAPCODE_RESULT_UTF8_LEN + 1];
        convertMapcodeToAlphabetUtf8(utf8, convertTests[i].input, convertTests[i].alphabet);
        nrTests++;
        if (strcmp(utf8, convertTests[i].expected) != 0) {
            foundError();
            printf("*** ERROR *** convertMapcodeToAlphabetUtf8(\"%s\",%d) returned \"%s\", (expected %s)\n",
                   convertTests[i].input, convertTests[i].alphabet, utf8, convertTests[i].expected);
        }
    }

    for (i = 0; i < (int) (sizeof(parseTests) / sizeof(parseTests[0])); i++) {
        char romanized1[MAX_MAPCODE_RESULT_ASCII_LEN + 1];
        MapcodeElements mapcodeElements;
        enum MapcodeError parseError = myParseMapcodeString(&mapcodeElements, parseTests[i].userInput,
                                                            TERRITORY_UNKNOWN);
        nrTests++;
        if (parseError) {
            foundError();
            printf("*** ERROR *** myParseMapcodeString(\"%s\") failed with error %d (expected %s)\n",
                   parseTests[i].userInput, (int) parseError, parseTests[i].expected);
        } else {
            nrTests++;
            sprintf(romanized1, "%s %s%s%s (%d;%d)",
                    mapcodeElements.territoryISO,
                    mapcodeElements.properMapcode,
                    *mapcodeElements.precisionExtension ? "-" : "",
                    mapcodeElements.precisionExtension,
                    mapcodeElements.territoryCode,
                    mapcodeElements.indexOfDot);
            if (strcmp(romanized1, parseTests[i].expected) != 0) {
                foundError();
                printf("*** ERROR *** myParseMapcodeString(\"%s\") = \"%s\", (expected \"%s\")\n",
                       parseTests[i].userInput,
                       romanized1, parseTests[i].expected);
            } else {
                int err1, err2;
                double lat1, lon1, lat2, lon2;
                char romanized2[MAX_MAPCODE_RESULT_ASCII_LEN + 1];
                sprintf(romanized2, "%s %s%s%s",
                        mapcodeElements.territoryISO,
                        mapcodeElements.properMapcode,
                        *mapcodeElements.precisionExtension ? "-" : "",
                        mapcodeElements.precisionExtension);
                err1 = decodeMapcodeToLatLonUtf8(&lat1, &lon1, parseTests[i].userInput, TERRITORY_UNKNOWN, NULL);
                err2 = decodeMapcodeToLatLonUtf8(&lat2, &lon2, romanized2, TERRITORY_UNKNOWN, NULL);
                ++nrTests;
                if (err1 || err2) {
                    foundError();
                    printf("*** ERROR *** decoding \"%s\" returns %d, decoding \"%s\" returns %d\n",
                           parseTests[i].userInput, err1, romanized2, err2);
                }
                if (lat1 != lat2 || lon1 != lon2) {
                    foundError();
                    printf("*** ERROR *** decoding \"%s\" returns (%f,%f), decoding \"%s\" returns (%f,%f)\n",
                           parseTests[i].userInput, lat1, lon1, romanized2, lat2, lon2);
                }
            }
        }
    }
    return nrTests;
}


// Show progress.
static void showTestProgress(int at, int max, int nrTests) {
    static clock_t prevTick = 0;

    // No worries, clock() is a very fast call.
    clock_t tick = clock() / (CLOCKS_PER_SEC / 2);
    if (tick != prevTick) {
        prevTick = tick;

        // Use stderr to not pollute logs.
        fprintf(stderr, "\r%0.1f%% (executed %0.1fM tests)\r", (at * 100.0) / max, nrTests / 1000000.0);
    }
}


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
    char clean[MAX_MAPCODE_RESULT_ASCII_LEN + 1];
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
        char territory[MAX_ISOCODE_ASCII_LEN + 1];
        // find first territory letter in s
        s = str;
        while (*s > 0 && *s <= 32) {
            s++;
        }
        // parse territory, if any
        p = strchr(s, ' ');
        len = p ? (int) (p - s) : 0;
        if (p && len <= MAX_ISOCODE_ASCII_LEN) {
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
        if (len + i >= MAX_MAPCODE_RESULT_ASCII_LEN) {
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
            foundError();
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
            foundError();
            printf("*** ERROR *** encode(%0.8f, %0.8f) does not deliver \"%s\"\n", y, x, clean);
            printGeneratedMapcodes("Delivered", &mapcodes);
        }
    }


    // test if correct nr of global solutions (if requested)
    if (globalsolutions > 0) {
        ++nrTests;
        nrresults = encodeLatLonToMapcodes(&mapcodes, y, x, TERRITORY_UNKNOWN, precision);
        if (nrresults != globalsolutions) {
            foundError();
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
            err = decodeMapcodeToLatLonUtf8(&lat, &lon, strResult, TERRITORY_UNKNOWN, NULL);
            if (err) {
                foundError();
                printf("*** ERROR *** decode('%s') = no result, expected ~(%0.8f, %0.8f)\n", strResult, y, x);
            } else {
                double dm = distanceInMeters(y, x, lat, lon);
                double maxerror = maxErrorInMeters(precision);
                // check if decode is sufficiently close to the encoded coordinate
                ++nrTests;
                if (dm > maxerror) {
                    foundError();
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
                            foundError();
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
static int testFailingDecodes(void) {
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
        err = decodeMapcodeToLatLonUtf8(&lat, &lon, str, TERRITORY_UNKNOWN, NULL);
        if (err >= 0) {
            foundError();
            printf("*** ERROR *** invalid mapcode \"%s\" decodes without error\n", str);
        }
    }
    return nrTests;
}


#include "test_territories.h"


static int testTerritory(const char *alphaCode, enum Territory territory,
                         int isAlias, int needsParent, enum Territory tcParent) {
    int nrTests = 0;
    char nam[MAX_ISOCODE_ASCII_LEN + 1];
    unsigned int i;
    for (i = 0; i <= strlen(alphaCode); i++) {
        char alphacode[MAX_ISOCODE_ASCII_LEN + 1];
        int tn;
        strcpy(alphacode, alphaCode);
        if (!needsParent && (i == 0)) {
            tn = getTerritoryCode(alphacode, TERRITORY_NONE);
            ++nrTests;
            if (tn != territory) {
                foundError();
                printf("*** ERROR *** getTerritoryCode('%s')=%d but expected %d (%s)\n",
                       alphacode, tn, territory, getTerritoryIsoName(nam, territory, 0));
            }
        }
        alphacode[i] = (char) tolower(alphacode[i]);
        tn = getTerritoryCode(alphacode, tcParent);
        ++nrTests;
        if (tn != territory) {
            foundError();
            printf("*** ERROR *** getTerritoryCode('%s',%s)=%d but expected %d\n", alphacode,
                   tcParent ? getTerritoryIsoName(nam, tcParent, 0) : "", tn, territory);
        }
    }

    if ((tcParent > _TERRITORY_MIN) && !isAlias) {
        getTerritoryIsoName(nam, territory, 0);
        ++nrTests;
        // every non-alias either equals nam, or is the state in nam
        if ((strcmp(nam, alphaCode) != 0) && (strcmp(nam + 3, alphaCode) != 0)) {
            foundError();
            printf("*** ERROR *** getTerritoryIsoName(%d)=\"%s\" which does not equal or contain \"%s\"\n",
                   territory, nam, alphaCode);
        }
    }
    return nrTests;
}


static int testTerritories() {
    char nam[MAX_ISOCODE_ASCII_LEN + 1];
    int nrTests = 0;
    int nr = sizeof(TEST_TERRITORIES) / sizeof(TEST_TERRITORIES[0]);
    int i;
    for (i = 0; i < nr; ++i) {
        nrTests += testTerritory(TEST_TERRITORIES[i].codeISO, TEST_TERRITORIES[i].territory,
                                 TEST_TERRITORIES[i].isAlias,
                                 TEST_TERRITORIES[i].needsParent, TEST_TERRITORIES[i].parent);
    }
    // test extremes
    ++nrTests;
    if (*getTerritoryIsoName(nam, TERRITORY_NONE, 0) || *getTerritoryIsoName(nam, TERRITORY_UNKNOWN, 0) ||
        *getTerritoryIsoName(nam, _TERRITORY_MIN, 0) || *getTerritoryIsoName(nam, _TERRITORY_MAX, 0)) {
        foundError();
        printf("*** ERROR *** getTerritoryIsoName returned nonempty for bad arguments\n");
    }
    ++nrTests;
    if (getTerritoryCode(NULL, TERRITORY_VAT) != TERRITORY_NONE) {
        foundError();
        printf("*** ERROR *** unexpected getTerritoryCode return for bad arguments\n");
    }
    // test some short values
    ++nrTests;
    if (strcmp(getTerritoryIsoName(nam, TERRITORY_US_CA, 1), "CA") ||
        strcmp(getTerritoryIsoName(nam, TERRITORY_IN_DD, 1), "DD") ||
        strcmp(getTerritoryIsoName(nam, TERRITORY_NLD, 1), "NLD") ||
        strcmp(getTerritoryIsoName(nam, TERRITORY_USA, 1), "USA")) {
        foundError();
        printf("*** ERROR *** getTerritoryIsoName returned bad short versions\n");
    }

    return nrTests;
}


// test closely around a particular coordinate
static int testAround(double y, double x) {
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
struct ContextTestAround {
    int nrTests;
    const TerritoryBoundary *territoryBoundaries;
};


static int joinThreads(pthread_t *threads, struct ContextTestAround *contexts, int total) {
    int i = 0;
    int nrTests = 0;
    for (i = 0; i < total; ++i) {
        if (pthread_join(threads[i], 0)) {
            foundError();
            printf("*** ERROR *** Error joining thread %d of %d\n", i, total);
            return 0;

        }
        nrTests += contexts[i].nrTests;
    }
    return nrTests;
}


// perform testEncodeAndDecode for all elements of encode_test[] (from decode_test.h)
static int testEncodeDecode(void) {
    int nrTests = 0;
    int i = 0;
    int nr = sizeof(ENCODE_TEST) / sizeof(ENCODE_TEST[0]) - 1;
    printf("%d encodes\n", nr);
    for (i = 0; i < nr; i++) {
        const EncodeTestRecord *t = &ENCODE_TEST[i];
        showTestProgress(i, nr, nrTests);
        nrTests += testEncodeAndDecode(t->mapcode, t->latitude, t->longitude,
                                       t->nrLocalMapcodes, t->nrGlobalMapcodes);
    }
    return nrTests;
}


static void *executeTestAround(void *context) {
    int nrTests = 0;
    double y, x, midx, midy, thirdx;
    struct ContextTestAround *c = (struct ContextTestAround *) context;
    const TerritoryBoundary *b = c->territoryBoundaries;

    midy = (b->miny + b->maxy) / 2000000.0;
    midx = (b->minx + b->maxx) / 2000000.0;
    thirdx = (2 * b->minx + b->maxx) / 3000000.0;
    nrTests += testAround(midy, midx);

    y = (b->miny) / 1000000.0;
    x = (b->minx) / 1000000.0;
    nrTests += testAround(y, x);
    nrTests += testAround(midy, x);
    nrTests += testAround(y, midx);
    nrTests += testAround(y, thirdx);

    x = (b->maxx) / 1000000.0;
    nrTests += testAround(y, x);
    nrTests += testAround(midy, x);

    y = (b->maxy) / 1000000.0;
    x = (b->minx) / 1000000.0;
    nrTests += testAround(y, x);
    nrTests += testAround(y, midx);

    x = (b->maxx) / 1000000.0;
    nrTests += testAround(y, x);
    nrTests += testAround(midy, x);
    c->nrTests = nrTests;
    return 0;
}


// test around all centers and corners of all territory rectangles
static int testReEncode(void) {
    int nrTests = 0;
    enum Territory ccode;
    int m = 0;
    int nrRecords = MAPCODE_BOUNDARY_MAX;
    int nrThread = 0;

    // Declare threads and contexts.
    pthread_t threads[MAX_THREADS];
    struct ContextTestAround contexts[MAX_THREADS];

    printf("%d records\n", nrRecords);
    for (ccode = _TERRITORY_MIN + 1; ccode < _TERRITORY_MAX; ccode++) {
        int min_boundary = DATA_START[INDEX_OF_TERRITORY(ccode)];
        int max_boundary = DATA_START[INDEX_OF_TERRITORY(ccode) + 1];
        showTestProgress(max_boundary, nrRecords, nrTests);
        // use internal knowledge of mapcoder to test all the territory boundaries
        for (m = min_boundary; m < max_boundary; m++) {
            const TerritoryBoundary *b = TERRITORY_BOUNDARY(m);

            // Create context for thread.
            contexts[nrThread].nrTests = 0;
            contexts[nrThread].territoryBoundaries = b;

            // Execute task on new thread.
            if (pthread_create(&threads[nrThread], 0, executeTestAround, (void *) &contexts[nrThread])) {
                foundError();
                printf("*** ERROR *** Cannot create thread\n");
                return 0;
            }

            // Move to next thread in pool. If out of threads, join them and start over.
            nrThread++;
            if (nrThread >= MAX_THREADS) {
                nrTests += joinThreads(threads, contexts, nrThread);
                nrThread = 0;
            }
        }
        nrTests += joinThreads(threads, contexts, nrThread);
        nrThread = 0;
    }
    return nrTests;
}


static void testDistance(double d1, double d2) {
    if (fabs(d1 - d2) > 0.00001) {
        foundError();
        printf("*** ERROR *** distanceInMeters failed, %lf != %lf\n", d1, d2);
    }
}


static int testDistances(void) {
    static const double METERS_PER_DEGREE_LAT = 110946.252133;
    static const double METERS_PER_DEGREE_LON = 111319.490793;
    int nrTests = 0;
    int i;
    static const double coordpairs[] = {
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
            0.11, 0.22, 0.12, 0.2333, 185011466
    };

    // check bad values    
    ++nrTests;
    if (maxErrorInMeters(99) != 0.0) {
        foundError();
        printf("*** ERROR *** maxErrorInMeters(99) = %f (expected 0.0)\n", maxErrorInMeters(99));
    }
    // check expected values    
    ++nrTests;
    testDistance(METERS_PER_DEGREE_LON * 1.5, distanceInMeters(0.0, 0.0, 0.0, 1.5));  // Check if #define is correct.
    ++nrTests;
    testDistance(METERS_PER_DEGREE_LON * 2.0, distanceInMeters(0.0, 0.0, 0.0, 2.0));
    ++nrTests;
    testDistance(METERS_PER_DEGREE_LON * 2.5, distanceInMeters(0.0, 0.0, 0.0, 2.5));

    ++nrTests;
    testDistance(METERS_PER_DEGREE_LON * 1.5, distanceInMeters(0.0, -1.0, 0.0, 0.5)); // Check around 0.
    ++nrTests;
    testDistance(METERS_PER_DEGREE_LON * 2.0, distanceInMeters(0.0, -1.0, 0.0, 1.0));
    ++nrTests;
    testDistance(METERS_PER_DEGREE_LON * 2.5, distanceInMeters(0.0, -1.0, 0.0, 1.5));

    ++nrTests;
    testDistance(METERS_PER_DEGREE_LON * 1.5, distanceInMeters(0.0, 0.5, 0.0, -1.0));
    ++nrTests;
    testDistance(METERS_PER_DEGREE_LON * 2.0, distanceInMeters(0.0, 1.0, 0.0, -1.0));
    ++nrTests;
    testDistance(METERS_PER_DEGREE_LON * 2.5, distanceInMeters(0.0, 1.5, 0.0, -1.0));

    ++nrTests;
    testDistance(METERS_PER_DEGREE_LON * 1.5, distanceInMeters(0.0, 359.0, 0.0, 0.5)); // Check around 360.
    ++nrTests;
    testDistance(METERS_PER_DEGREE_LON * 2.0, distanceInMeters(0.0, 359.0, 0.0, 1.0));
    ++nrTests;
    testDistance(METERS_PER_DEGREE_LON * 2.5, distanceInMeters(0.0, 359.0, 0.0, 1.5));

    ++nrTests;
    testDistance(METERS_PER_DEGREE_LON * 1.5, distanceInMeters(0.0, 0.5, 0.0, 359.0)); // Note that shortest.
    ++nrTests;
    testDistance(METERS_PER_DEGREE_LON * 2.0, distanceInMeters(0.0, 1.0, 0.0, 359.0)); // path needs to be taken!
    ++nrTests;
    testDistance(METERS_PER_DEGREE_LON * 2.5, distanceInMeters(0.0, 1.5, 0.0, 359.0));

    ++nrTests;
    testDistance(METERS_PER_DEGREE_LAT, distanceInMeters(0.5, 0.0, -0.5, 0.0)); // Check constant.
    ++nrTests;
    testDistance(METERS_PER_DEGREE_LAT, distanceInMeters(1.0, 0.0, -0.0, 0.0)); // Check around 0.
    ++nrTests;
    testDistance(METERS_PER_DEGREE_LAT, distanceInMeters(0.0, 0.0, -1.0, 0.0));

    for (i = 0; i < (int) (sizeof(coordpairs) / sizeof(coordpairs[0])); i += 5) {
        const double distance = distanceInMeters(
                coordpairs[i], coordpairs[i + 1],
                coordpairs[i + 2], coordpairs[i + 3]);
        ++nrTests;
        if (floor(0.5 + (100000.0 * distance)) != coordpairs[i + 4]) {
            foundError();
            printf("*** ERROR *** distanceInMeters %d failed: %f\n", i, distance);
        }
    }
    return nrTests;
}


static int testTerritoryInsides(void) {
    int nrTests = 0;
    if (strcmp(MAPCODE_C_VERSION, "2.1.5") >= 0) {
        int i;
        const struct {
            const char *territory;
            double lat;
            double lon;
            int nearborders;
        } TEST_DATA[] = {
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
                {"MEX",    21.431788272457, -89.779820144176, 1}
        };

        for (i = 0; i < (int) (sizeof(TEST_DATA) / sizeof(TEST_DATA[0])); i++) {
            enum Territory territory = getTerritoryCode(TEST_DATA[i].territory, TERRITORY_NONE);
            ++nrTests;
            if (multipleBordersNearby(TEST_DATA[i].lat, TEST_DATA[i].lon, territory) != TEST_DATA[i].nearborders) {
                foundError();
                printf("*** ERROR *** multipleBordersNearby(%+18.13f,%+18.13f, \"%s\") not %d\n",
                       TEST_DATA[i].lat, TEST_DATA[i].lon, TEST_DATA[i].territory, TEST_DATA[i].nearborders);
            }
        }
    }
    return nrTests;
}


static int testTerritoryCode(void) {
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
            {TERRITORY_BR_AL, TERRITORY_NONE,  "AL"},
            {TERRITORY_RU_AL, TERRITORY_RUS,   "AL"}, // 497=rus
            {TERRITORY_RU_AL, TERRITORY_RU_TT, "AL"}, // 431=ru-tam
            {TERRITORY_US_AL, TERRITORY_USA,   "AL"}, // 411=usa
            {TERRITORY_US_AL, TERRITORY_US_CA, "AL"},
            {TERRITORY_NONE,  TERRITORY_NONE,  0}
    };

    static const struct {
        enum Territory input;
        enum Territory output;
    } ptcTestData[] = {
            {TERRITORY_NONE,    TERRITORY_NONE},
            {_TERRITORY_MIN,    TERRITORY_NONE},
            {TERRITORY_VAT,     TERRITORY_NONE},
            {TERRITORY_MX_DIF,  TERRITORY_MEX},
            {TERRITORY_MX_CHH,  TERRITORY_MEX},
            {TERRITORY_GRL,     TERRITORY_NONE},
            {TERRITORY_IN_DD,   TERRITORY_IND},
            {TERRITORY_AU_VIC,  TERRITORY_AUS},
            {TERRITORY_BR_DF,   TERRITORY_BRA},
            {TERRITORY_US_AL,   TERRITORY_USA},
            {TERRITORY_CA_NU,   TERRITORY_CAN},
            {TERRITORY_RU_LIP,  TERRITORY_RUS},
            {TERRITORY_CN_HA,   TERRITORY_CHN},
            {TERRITORY_AAA,     TERRITORY_NONE},
            {_TERRITORY_MAX,    TERRITORY_NONE},
            {TERRITORY_UNKNOWN, TERRITORY_NONE}
    };

    for (i = 0; i < (int) (sizeof(ptcTestData) / sizeof(ptcTestData[0])); i++) {
        enum Territory result = getParentCountryOf(ptcTestData[i].input);
        if (result != ptcTestData[i].output) {
            foundError();
            printf("*** ERROR *** getParentCountryOf(%d) returned unexpected %d\n", ptcTestData[i].input, result);
        }
    }

    for (i = 0; tcTestData[i].inputstring != 0; i++) {
        enum Territory ccode = getTerritoryCode(tcTestData[i].inputstring, tcTestData[i].context);
        ++nrTests;
        if (ccode != tcTestData[i].expectedresult) {
            foundError();
            printf("*** ERROR *** getTerritoryCode(\"%s\", %d)=%d, expected %d\n",
                   tcTestData[i].inputstring, tcTestData[i].context,
                   ccode, tcTestData[i].expectedresult);
        }
    }
    return nrTests;
}


static int testIncorrectGetTerritoryCode(char *tcAlpha) {
    enum Territory ccode = getTerritoryCode(tcAlpha, TERRITORY_NONE);
    if (ccode > _TERRITORY_MIN) {
        foundError();
        printf("*** ERROR *** getTerritoryCode returns '%d' (should be < 0) for territory code '%s'\n", (int) ccode,
               tcAlpha);
    }
    return 1;
}


static int testGetTerritoryCode(void) {
    int nrTests = 0;
    int i;
    char s1[1];
    char largeString[16000];

    nrTests += testIncorrectGetTerritoryCode("UNKNOWN");
    nrTests += testIncorrectGetTerritoryCode("A");
    nrTests += testIncorrectGetTerritoryCode(" A");
    nrTests += testIncorrectGetTerritoryCode("A ");
    nrTests += testIncorrectGetTerritoryCode(" A ");
    nrTests += testIncorrectGetTerritoryCode("AA");
    nrTests += testIncorrectGetTerritoryCode(" AA");
    nrTests += testIncorrectGetTerritoryCode("AA ");
    nrTests += testIncorrectGetTerritoryCode(" AA ");
    nrTests += testIncorrectGetTerritoryCode("US-");
    nrTests += testIncorrectGetTerritoryCode(" US-");
    nrTests += testIncorrectGetTerritoryCode("US- ");
    nrTests += testIncorrectGetTerritoryCode(" US- ");
    nrTests += testIncorrectGetTerritoryCode(" ");

    s1[0] = 0;
    nrTests += testIncorrectGetTerritoryCode(s1);

    for (i = 0; i < (int) sizeof(largeString) - 1; ++i) {
        largeString[i] = (char) ((i % 223) + 32);
    }
    largeString[sizeof(largeString) - 1] = 0;
    nrTests += testIncorrectGetTerritoryCode(largeString);
    return nrTests;
}


static int testIncorrectEncode(double lat, double lon, int treatAsError) {
    int nrResults;
    int nrTests = 0;
    Mapcodes mapcodes;
    ++nrTests;
    nrResults = encodeLatLonToMapcodes(&mapcodes, lat, lon, TERRITORY_UNKNOWN, 0);
    if (nrResults > 0) {
        if (treatAsError) {
            foundError();
        }
        printf("*** %s *** encodeLatLonToMapcodes returns '%d' (should be <= 0) for lat=%f, lon=%f\n",
               treatAsError ? "ERROR" : "WARNING", nrResults, lat, lon);
    }
    return nrTests;
}


static int testCorrectEncode(double lat, double lon, int treatAsError) {
    Mapcodes mapcodes;
    int nrResults = encodeLatLonToMapcodes(&mapcodes, lat, lon, TERRITORY_UNKNOWN, 0);
    if (nrResults <= 0) {
        if (treatAsError) {
            foundError();
        }
        printf("*** %s *** encodeLatLonToMapcodes returns '%d' (should be > 0) for lat=%f, lon=%f\n",
               treatAsError ? "ERROR" : "WARNING", nrResults, lat, lon);
    }
    return 1;
}


static int testEncodeRobustness(void) {
    int nrTests = 0;
    double d;
    unsigned char *b = (unsigned char *) &d;

    nrTests += testCorrectEncode(-90.0, 0.0, 1);
    nrTests += testCorrectEncode(90.0, 0.0, 1);
    nrTests += testCorrectEncode(-91.0, 0.0, 1);
    nrTests += testCorrectEncode(91.0, 0.0, 1);

    nrTests += testCorrectEncode(0.0, -180.0, 1);
    nrTests += testCorrectEncode(0.0, 180.0, 1);
    nrTests += testCorrectEncode(1.0, -181.0, 1);
    nrTests += testCorrectEncode(0.0, 181.0, 1);

    // NAN - See: https://en.wikipedia.org/wiki/Double-precision_floating-point_format
    b[7] = 0x7f;
    b[6] = 0xff;
    b[5] = 0xff;
    b[4] = 0xff;
    b[3] = 0xff;
    b[2] = 0xff;
    b[1] = 0xff;
    b[0] = 0xff;
    nrTests += testIncorrectEncode(0.0, d, 0);
    nrTests += testIncorrectEncode(d, 0.0, 0);
    nrTests += testIncorrectEncode(d, d, 0);

    // Infinity.
    b[7] = 0x7f;
    b[6] = 0xf0;
    b[5] = 0x00;
    b[4] = 0x00;
    b[3] = 0x00;
    b[2] = 0x00;
    b[1] = 0x00;
    b[0] = 0x00;
    nrTests += testCorrectEncode(d, 0.0, 0);      // Lat may be Inf.
    nrTests += testIncorrectEncode(0.0, d, 0);
    nrTests += testIncorrectEncode(d, d, 0);

    // -Infinity.
    b[7] = 0xff;
    b[6] = 0xf0;
    b[5] = 0x00;
    b[4] = 0x00;
    b[3] = 0x00;
    b[2] = 0x00;
    b[1] = 0x00;
    b[0] = 0x00;
    nrTests += testCorrectEncode(d, 0.0, 0);      // Lat may be -Inf.
    nrTests += testIncorrectEncode(0.0, d, 0);
    nrTests += testIncorrectEncode(d, d, 0);

    // Max double
    b[7] = 0x7f;
    b[6] = 0xef;
    b[5] = 0xff;
    b[4] = 0xff;
    b[3] = 0xff;
    b[2] = 0xff;
    b[1] = 0xff;
    b[0] = 0xff;
    nrTests += testCorrectEncode(d, 0.0, 0);
    nrTests += testCorrectEncode(0.0, d, 0);
    nrTests += testCorrectEncode(d, d, 0);

    d = -d;
    nrTests += testCorrectEncode(d, 0.0, 0);
    nrTests += testCorrectEncode(0.0, d, 0);
    nrTests += testCorrectEncode(d, d, 0);
    return nrTests;
}


static int testIncorrectDecode(char *mc, enum Territory tc) {
    double lat;
    double lon;
    int rc = decodeMapcodeToLatLonUtf8(&lat, &lon, mc, tc, NULL);
    if (rc >= 0) {
        foundError();
        printf("*** ERROR *** decodeMapcodeToLatLonUtf8 returns '%d' (should be non-0) for mapcode='%s'\n", rc, mc);
    }
    return 1;
}


static int testCorrectDecode(char *mc, enum Territory tc) {
    double lat1;
    double lon1;
    double lat2;
    double lon2;
    UWORD utf16[MAX_CLEAN_MAPCODE_ASCII_LEN + 1];
    int rc = decodeMapcodeToLatLonUtf8(&lat1, &lon1, mc, tc, NULL);
    if (rc < 0) {
        foundError();
        printf("*** ERROR *** decodeMapcodeToLatLonUtf8 returns '%d' (should be 0) for mapcode='%s'\n", rc, mc);
    }
    rc = compareWithMapcodeFormatUtf8(mc);
    if (rc < 0) {
        foundError();
        printf("*** ERROR *** decodeMapcodeToLatLonUtf8 returns '%d' (should be 0) for mapcode='%s'\n", rc, mc);
    }

    convertMapcodeToAlphabetUtf16(utf16, mc, ALPHABET_ARABIC);
    rc = decodeMapcodeToLatLonUtf16(&lat2, &lon2, utf16, tc, NULL);
    if (rc < 0) {
        foundError();
        printf("*** ERROR *** decodeMapcodeToLatLonUtf16 returns '%d' (should be 0) for mapcode='%s'\n", rc, mc);
    }
    if ((fabs(lat1 - lat2) > 0.000001) || (fabs(lon1 - lon2) > 0.000001)) {
        foundError();
        printf("*** ERROR *** decodeMapcodeToLatLonUtf16 returns (%lf, %lf) (should be (%lf, %lf)) for mapcode='%s'\n",
               lat2, lon2, lat1, lon1, mc);
    }
    rc = compareWithMapcodeFormatUtf16(utf16);
    if (rc < 0) {
        foundError();
        printf("*** ERROR *** decodeMapcodeToLatLonUtf16 returns '%d' (should be 0) for mapcode='%s'\n", rc, mc);
    }
    return 2;
}


static int testDecodeRobustness(void) {
    int nrTests = 0;
    int i;
    char s1[1];
    char largeString[16000];

    enum Territory tc = getTerritoryCode("NLD", TERRITORY_NONE);
    nrTests += testIncorrectDecode("", TERRITORY_NONE);
    nrTests += testIncorrectDecode(" ", TERRITORY_NONE);
    nrTests += testIncorrectDecode("AA", TERRITORY_NONE);
    nrTests += testIncorrectDecode("", tc);
    nrTests += testIncorrectDecode(" ", tc);
    nrTests += testIncorrectDecode("AA", tc);
    nrTests += testIncorrectDecode("XX.XX", TERRITORY_NONE);
    nrTests += testCorrectDecode("NLD XX.XX", tc);
    nrTests += testCorrectDecode("NLD 39.UC", tc);
    nrTests += testCorrectDecode("W9.SX9", tc);
    nrTests += testCorrectDecode("MEX 49.4V", tc);
    nrTests += testCorrectDecode("NLD XX.XX", TERRITORY_NONE);
    nrTests += testCorrectDecode("MX XX.XX", TERRITORY_NONE);

    s1[0] = 0;
    nrTests += testIncorrectDecode(s1, TERRITORY_NONE);
    nrTests += testIncorrectDecode(s1, tc);

    for (i = 0; i < (int) sizeof(largeString) - 1; ++i) {
        largeString[i] = (char) ((i % 223) + 32);
    }
    largeString[sizeof(largeString) - 1] = 0;
    nrTests += testIncorrectDecode(s1, TERRITORY_NONE);
    nrTests += testIncorrectDecode(s1, tc);
    return nrTests;
}


static int testEnvironment(void) {
    int nrTests = 0;
    int sizeOfWord = sizeof(UWORD);
    char *s = "1234567890";
    long distance = (strstr(s, "0") - s);

    int testSigned = (int) ((char) -1);
    printf("sizeof(char)=%ld, sizeof(UWORD)=%ld, sizeof(int)=%ld, sizeof(long int)=%ld char=%s\n",
           sizeof(char), sizeof(UWORD), sizeof(int), sizeof(long int), (testSigned == -1) ? "signed" : "unsigned");

    // Check size of UWORD.
    nrTests++;
    if (sizeOfWord != 2) {
        foundError();
        printf("*** ERROR *** Incompatible system, UWORD is not 2 bytes, but %d\n", (int) sizeof(UWORD));
    }

    // Check char* math.
    nrTests++;
    if (distance != 9) {
        foundError();
        printf("*** ERROR *** Incompatible system, char* math does not work as expected, distance=%ld\n", distance);
    }
    return nrTests;
}


static int testRobustness(void) {
    int nrTests = 0;
    nrTests += testGetTerritoryCode();
    nrTests += testEncodeRobustness();
    nrTests += testDecodeRobustness();
    return nrTests;
}


static int testTerritoriesCsv(void) {
    int nrTests = 0;
    int linesTested = 0;
    const char *csvName = "territories.csv";
#define MAXLINESIZE 512 // worst-case line length in the file
    FILE *fp = fopen(csvName, "r");
    if (fp == NULL) {
        foundError();
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
                                foundError();
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
                                foundError();
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
                            foundError();
                            printf("*** ERROR *** %d alphabets for territory %d, expected %d\n",
                                   territoryAlphabet->count, csvTerritoryCode, csvNrAlphabets);
                        }
                        s++;
                    }
                    // parse and check names
                    e = strchr(s, 10);
                    if (e) {
                        *e = 0;
                        {
                            int i, noMoreNames = 0;
                            for (i = 0; !noMoreNames; i++) {
                                char territoryName[MAX_TERRITORY_FULLNAME_UTF8_LEN + 1];
                                noMoreNames = getFullTerritoryNameInLocaleUtf8(territoryName, csvTerritoryCode, i, "en_US");
                                if (!strstr(s, territoryName)) {
                                    foundError();
                                    printf("*** ERROR *** Name \"%s\" not found in \"%s\"\n", territoryName, s);
                                }
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


static int testSingleEncodes(void) {
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
        char result[MAX_MAPCODE_RESULT_ASCII_LEN + 1];
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


static int testSelectedEncodes(void) {
    int nrTests = 0;
    char mapcode[MAX_MAPCODE_RESULT_ASCII_LEN];
    double lat = 52.158993;
    double lon = 4.492346;
    int i = 0;
    int total = 0;
    do {
        total = encodeLatLonToSelectedMapcode(mapcode, lat, lon, TERRITORY_NONE, 0, i);
        if (total != 4) {
            foundError();
            printf("*** ERROR *** testSelectedEncodes, expected %d alternatives, but got %d", 4, total);
        }
        switch (i) {
        case 0:
            if (strcmp(mapcode, "NLD QJM.0G") != 0) {
                foundError();
                printf("*** ERROR *** testSelectedEncodes, expected '%s', but got '%s', alternative %d\n", "NLD QJM.0G", mapcode, i);
            }
            break;
        case 1:
            if (strcmp(mapcode, "NLD CZQ.15C") != 0) {
                foundError();
                printf("*** ERROR *** testSelectedEncodes, expected '%s', but got '%s', alternative %d\n", "NLD CZQ.15C", mapcode, i);
            }
            break;
        case 2:
            if (strcmp(mapcode, "NLD N39J.MZN") != 0) {
                foundError();
                printf("*** ERROR *** testSelectedEncodes, expected '%s', but got '%s', alternative %d\n", "NLD N39J.MZN", mapcode, i);
            }
            break;
        case 3:
            if (strcmp(mapcode, "VHVN4.TZ9S") != 0) {
                foundError();
                printf("*** ERROR *** testSelectedEncodes, expected '%s', but got '%s', alternative %d\n", "VHVN4.TZ9S", mapcode, i);
            }
            break;
        default:
            foundError();
            printf("*** ERROR *** testSelectedEncodes, expected %d alternatives, but got %d", 4, i);
            break;
        }
        ++i;
    } while (i < total);
    ++nrTests;
    return nrTests;
}


static int testGetFullTerritoryNameLocal(const char *expectedName, enum Territory territory, int alternative) {
    int nrTests = 0;
    int expectedCode = (*expectedName ? 1 : 0);
    char gotName[MAX_TERRITORY_FULLNAME_UTF8_LEN + 1];
    int gotCode = getFullTerritoryNameLocalUtf8(gotName, territory, alternative);
    ++nrTests;
    if (expectedCode && strcmp(expectedName, gotName)) {
        char s[MAX_ISOCODE_ASCII_LEN + 1];
        foundError();
        printf("*** ERROR *** getFullTerritoryNameLocal error, expected name '%s', but got '%s' for territory %s, alternative %d\n",
               expectedName, gotName, getTerritoryIsoName(s, territory, 0), alternative);
    }
    ++nrTests;
    if ((expectedCode && !gotCode) || (!expectedCode && gotCode)) {
        char s[MAX_ISOCODE_ASCII_LEN + 1];
        foundError();
        printf("*** ERROR *** getFullTerritoryNameLocal error, expected return code %d, but got %d (%s) for territory %s, alternative %d\n",
               expectedCode, gotCode, gotName, getTerritoryIsoName(s, territory, 0), alternative);
    }
    return nrTests;
}


static int testGetFullTerritoryNameInLocale(const char *expectedName,
                                            enum Territory territory, int alternative,
                                            const char *locale) {
    int nrTests = 0;
    int expectedCode = (*expectedName ? 1 : 0);
    char gotName[MAX_TERRITORY_FULLNAME_UTF8_LEN + 1];
    int gotCode = getFullTerritoryNameInLocaleUtf8(gotName, territory, alternative, locale);
    ++nrTests;
    if (expectedCode && strcmp(expectedName, gotName)) {
        char s[MAX_ISOCODE_ASCII_LEN + 1];
        foundError();
        printf("*** ERROR *** getFullTerritoryNameInLocale error, expected name '%s', but got '%s' for territory %s, alternative %d, locale %s\n",
               expectedName, gotName, getTerritoryIsoName(s, territory, 0), alternative, locale ? locale : "NULL");
    }
    ++nrTests;
    if ((expectedCode && !gotCode) || (!expectedCode && gotCode)) {
        char s[MAX_ISOCODE_ASCII_LEN + 1];
        foundError();
        printf("*** ERROR *** getFullTerritoryNameInLocale error, expected return code %d, but got %d (%s) for territory %s, alternative %d, locale %s\n",
               expectedCode, gotCode, gotName, getTerritoryIsoName(s, territory, 0), alternative, locale ? locale : "NULL");
    }
    return nrTests;
}


static int testGetFullTerritoryNameLocalInAlphabet(const char *expectedName, enum Territory territory,
                                                   int alternative, enum Alphabet alphabet) {
    int nrTests = 0;
    int expectedCode = (*expectedName ? 1 : 0);
    char gotName[MAX_TERRITORY_FULLNAME_UTF8_LEN + 1];
    int gotCode = getFullTerritoryNameLocalInAlphabetUtf8(gotName, territory, alternative, alphabet);
    ++nrTests;
    if (expectedCode && strcmp(expectedName, gotName)) {
        char s[MAX_ISOCODE_ASCII_LEN + 1];
        foundError();
        printf("*** ERROR *** getFullTerritoryNameLocalInAlphabet error, expected name '%s', but got '%s' for territory %s, alternative %d\n",
               expectedName, gotName, getTerritoryIsoName(s, territory, 0), alternative);
    }
    ++nrTests;
    if ((expectedCode && !gotCode) || (!expectedCode && gotCode)) {
        char s[MAX_ISOCODE_ASCII_LEN + 1];
        foundError();
        printf("*** ERROR *** getFullTerritoryNameLocalInAlphabet error, expected return code %d, but got %d (%s) for territory %s, alternative %d\n",
               expectedCode, gotCode, gotName, getTerritoryIsoName(s, territory, 0), alternative);
    }
    return nrTests;
}


int testGetFullTerritoryName(void) {
    int nrTests = 0;
    enum Territory territory;
    int nrNames = 0;
    int maxLength = 0;
    int alternative;
    int i;
    const TerritoryAlphabets *territoryAlphabets;
    char territoryName[MAX_TERRITORY_FULLNAME_UTF8_LEN + 1024]; // large so we can test overflow
    static const char *locales_to_test[] = {
            "AF", "AR", "BE", "CN", "CS", "DA", "DE", "EN", "FI", "ES", "FR", "HE", "HI",
            "HR", "ID", "IT", "JA", "KO", "NL", "NO", "PL", "PT", "RU", "SV", "SW", "TR", "UK"};

    nrTests += testGetFullTerritoryNameInLocale("Sancta Sedes", TERRITORY_VAT, 0, NULL);    // Local name.
    nrTests += testGetFullTerritoryNameInLocale("Vatican", TERRITORY_VAT, 0, "");           // Wrong locale -> fallback language.
    nrTests += testGetFullTerritoryNameInLocale("Vatican", TERRITORY_VAT, 0, "E");
    nrTests += testGetFullTerritoryNameInLocale("Vatican", TERRITORY_VAT, 0, "EN");
    nrTests += testGetFullTerritoryNameInLocale("Vatican", TERRITORY_VAT, 0, "??");
    nrTests += testGetFullTerritoryNameInLocale("Vatican", TERRITORY_VAT, 0, "-");
    nrTests += testGetFullTerritoryNameInLocale("Vatican", TERRITORY_VAT, 0, "_");
    nrTests += testGetFullTerritoryNameInLocale("Vatican", TERRITORY_VAT, 0, "_EN");

    for (i = 0; i < (int) (sizeof(locales_to_test) / sizeof(locales_to_test[0])); i++) {
        const char *locale = locales_to_test[i];
        int nrInLocale = 0;
        for (territory = _TERRITORY_MIN + 1; territory < _TERRITORY_MAX; ++territory) {
            char expectedLocalName[MAX_TERRITORY_FULLNAME_UTF8_LEN + 1];
            getFullTerritoryNameInLocaleUtf8(expectedLocalName, territory, 0, 0);

            nrTests += testGetFullTerritoryNameInLocale(expectedLocalName, territory, 0, NULL);
            nrTests += testGetFullTerritoryNameInLocale("", territory, -1, locale);
            nrTests += testGetFullTerritoryNameInLocale("", territory, 999, locale);
            for (alternative = 0;; alternative++) {
                int len;
                if (!getFullTerritoryNameInLocaleUtf8(territoryName, territory, alternative, locale)) {
                    break;
                }
                ++nrInLocale;
                ++nrNames;
                len = (int) strlen(territoryName);
                nrTests++;
                if (len < 1 || len > MAX_TERRITORY_FULLNAME_UTF8_LEN) {
                    char s[MAX_ISOCODE_ASCII_LEN + 1];
                    foundError();
                    printf("*** ERROR *** Bad %s territoryname %s, %d characters (limit is %d): %s\n",
                           locale, getTerritoryIsoName(s, territory, 0), len,
                           MAX_TERRITORY_FULLNAME_UTF8_LEN, territoryName);
                }
                if (len > maxLength) {
                    maxLength = len;
                }
            }
        }
    }

    // check that every territory has a local name in all the common alphabets
    for (territory = _TERRITORY_MIN + 1; territory < _TERRITORY_MAX; ++territory) {
        territoryAlphabets = getAlphabetsForTerritory(territory);
        for (i = 0; i < territoryAlphabets->count; i++) {
            nrTests++;
            if (!getFullTerritoryNameLocalInAlphabetUtf8(territoryName, territory, 0,
                                                         territoryAlphabets->alphabet[i])) {
                char s[MAX_ISOCODE_ASCII_LEN + 1];
                foundError();
                printf("*** ERROR *** getFullTerritoryNameLocal territory %s has NO name in common alphabet (%d)\n",
                       getTerritoryIsoName(s, territory, 0), territoryAlphabets->alphabet[0]);
            }
        }

        // check that the x-th local name is in the x-th common alphabet
        nrTests++;
        territoryAlphabets = getAlphabetsForTerritory(territory);
        for (i = 0; i < territoryAlphabets->count; i++) {
            char territoryNameLocal[MAX_TERRITORY_FULLNAME_UTF8_LEN + 1];
            if (!getFullTerritoryNameLocalInAlphabetUtf8(territoryName, territory, 0, territoryAlphabets->alphabet[i])) {
                char s[MAX_ISOCODE_ASCII_LEN + 1];
                foundError();
                printf("*** ERROR *** getFullTerritoryNameLocal Territory %s has NO name in common alphabet %d\n",
                       getTerritoryIsoName(s, territory, 0), territoryAlphabets->alphabet[i]);
            }
            if (!getFullTerritoryNameLocalUtf8(territoryNameLocal, territory, i) ||
                strcmp(territoryName, territoryNameLocal)) {
                foundError();
                printf("*** ERROR *** getFullTerritoryNameLocal Territory name %s (%d) does not match local name %s\n",
                       territoryName, i, territoryNameLocal);
            }
        }
    }
    printf("%d territory names, max length %d characters\n", nrNames, maxLength);

    nrTests += testGetFullTerritoryNameInLocale("Netherlands", TERRITORY_NLD, 0, "en_US");
    nrTests += testGetFullTerritoryNameInLocale("The Netherlands", TERRITORY_NLD, 1, "en_US");
    nrTests += testGetFullTerritoryNameInLocale("", TERRITORY_NLD, 2, "en_US");
    nrTests += testGetFullTerritoryNameInLocale("Russia", TERRITORY_RUS, 0, "en_US");
    nrTests += testGetFullTerritoryNameInLocale("Russian Federation", TERRITORY_RUS, 1, "en_US");
    nrTests += testGetFullTerritoryNameInLocale("", TERRITORY_RUS, 2, "en_US");
    nrTests += testGetFullTerritoryNameInLocale("Kazakhstan", TERRITORY_KAZ, 0, "en_US");
    nrTests += testGetFullTerritoryNameInLocale("", TERRITORY_KAZ, 1, "en_US");
    nrTests += testGetFullTerritoryNameInLocale("Turkmenistan", TERRITORY_TKM, 0, "en_US");
    nrTests += testGetFullTerritoryNameInLocale("", TERRITORY_TKM, 1, "en_US");
    nrTests += testGetFullTerritoryNameInLocale("", TERRITORY_TKM, 2, "en_US");
    nrTests += testGetFullTerritoryNameInLocale("", TERRITORY_TKM, 999, "en_US");

#ifdef MAPCODE_SUPPORT_LANGUAGE_NL
    nrTests += testGetFullTerritoryNameInLocale("Nederland", TERRITORY_NLD, 0, "nl");
    nrTests += testGetFullTerritoryNameInLocale("", TERRITORY_NLD, 1, "nl");
    nrTests += testGetFullTerritoryNameInLocale("Vaticaanstad", TERRITORY_VAT, 0, "nl");
    nrTests += testGetFullTerritoryNameInLocale("Heilige Stoel", TERRITORY_VAT, 1, "nl");
    nrTests += testGetFullTerritoryNameInLocale("", TERRITORY_VAT, 2, "nl");
    nrTests += testGetFullTerritoryNameInLocale("", TERRITORY_VAT, 3, "nl");
#else
    nrTests += testGetFullTerritoryNameInLocale("Netherlands", TERRITORY_NLD, 0, "nl");
#endif

#ifdef MAPCODE_SUPPORT_LANGUAGE_FR
    nrTests += testGetFullTerritoryNameInLocale("Pays-Bas", TERRITORY_NLD, 0, "fr");
    nrTests += testGetFullTerritoryNameInLocale("", TERRITORY_NLD, 1, "fr");
    nrTests += testGetFullTerritoryNameInLocale("Vatican", TERRITORY_VAT, 0, "fr");
    nrTests += testGetFullTerritoryNameInLocale("Cit\xC3\xA9 du Vatican", TERRITORY_VAT, 1, "fr");
    nrTests += testGetFullTerritoryNameInLocale("Saint-Si\xC3\xA8ge", TERRITORY_VAT, 2, "fr");
    nrTests += testGetFullTerritoryNameInLocale("", TERRITORY_VAT, 3, "fr");
#else
    nrTests += testGetFullTerritoryNameInLocale("Netherlands", TERRITORY_NLD, 0, "fr");
#endif

    nrTests += testGetFullTerritoryNameLocal("Nederland", TERRITORY_NLD, 0);
    nrTests += testGetFullTerritoryNameLocal("", TERRITORY_NLD, 1);
    nrTests += testGetFullTerritoryNameLocalInAlphabet("Nederland", TERRITORY_NLD, 0, ALPHABET_ROMAN);
    nrTests += testGetFullTerritoryNameLocalInAlphabet("", TERRITORY_NLD, 1, ALPHABET_ROMAN);
    nrTests += testGetFullTerritoryNameLocalInAlphabet("", TERRITORY_NLD, 0, ALPHABET_GREEK);
    nrTests += testGetFullTerritoryNameLocalInAlphabet("", TERRITORY_NLD, 0, _ALPHABET_MIN);
    nrTests += testGetFullTerritoryNameLocalInAlphabet("", TERRITORY_NLD, 0, _ALPHABET_MAX);
    nrTests += testGetFullTerritoryNameLocal("Росси́я", TERRITORY_RUS, 0);
    nrTests += testGetFullTerritoryNameLocal("", TERRITORY_RUS, 1);
    nrTests += testGetFullTerritoryNameLocalInAlphabet("Росси́я", TERRITORY_RUS, 0, ALPHABET_CYRILLIC);
    nrTests += testGetFullTerritoryNameLocalInAlphabet("", TERRITORY_RUS, 0, ALPHABET_ROMAN);
    nrTests += testGetFullTerritoryNameLocalInAlphabet("", TERRITORY_RUS, 0, ALPHABET_GREEK);
    nrTests += testGetFullTerritoryNameLocalInAlphabet("", TERRITORY_RUS, 0, _ALPHABET_MIN);
    nrTests += testGetFullTerritoryNameLocalInAlphabet("", TERRITORY_RUS, 0, _ALPHABET_MAX);
    nrTests += testGetFullTerritoryNameLocal("Қазақстан", TERRITORY_KAZ, 0);
    nrTests += testGetFullTerritoryNameLocal("Qazaqstan", TERRITORY_KAZ, 1);
    nrTests += testGetFullTerritoryNameLocal("Kazakhstan", TERRITORY_KAZ, 2);
    nrTests += testGetFullTerritoryNameLocal("", TERRITORY_KAZ, 3);
    nrTests += testGetFullTerritoryNameLocalInAlphabet("Қазақстан", TERRITORY_KAZ, 0, ALPHABET_CYRILLIC);
    nrTests += testGetFullTerritoryNameLocalInAlphabet("", TERRITORY_KAZ, 1, ALPHABET_CYRILLIC);
    nrTests += testGetFullTerritoryNameLocalInAlphabet("Qazaqstan", TERRITORY_KAZ, 0, ALPHABET_ROMAN);
    nrTests += testGetFullTerritoryNameLocalInAlphabet("Kazakhstan", TERRITORY_KAZ, 1, ALPHABET_ROMAN);
    nrTests += testGetFullTerritoryNameLocalInAlphabet("", TERRITORY_KAZ, 2, ALPHABET_ROMAN);
    nrTests += testGetFullTerritoryNameLocalInAlphabet("", TERRITORY_KAZ, 0, ALPHABET_GREEK);
    nrTests += testGetFullTerritoryNameLocalInAlphabet("", TERRITORY_KAZ, 0, _ALPHABET_MIN);
    nrTests += testGetFullTerritoryNameLocalInAlphabet("", TERRITORY_KAZ, 0, _ALPHABET_MAX);
    nrTests += testGetFullTerritoryNameLocal("Түркменистан", TERRITORY_TKM, 0);
    nrTests += testGetFullTerritoryNameLocal("Türkmenistan", TERRITORY_TKM, 1);
    nrTests += testGetFullTerritoryNameLocal("تۆركمنيستآن", TERRITORY_TKM, 2);
    nrTests += testGetFullTerritoryNameLocal("", TERRITORY_TKM, 3);
    nrTests += testGetFullTerritoryNameLocalInAlphabet("Türkmenistan", TERRITORY_TKM, 0, ALPHABET_ROMAN);
    nrTests += testGetFullTerritoryNameLocalInAlphabet("", TERRITORY_TKM, 1, ALPHABET_ROMAN);
    nrTests += testGetFullTerritoryNameLocalInAlphabet("Түркменистан", TERRITORY_TKM, 0, ALPHABET_CYRILLIC);
    nrTests += testGetFullTerritoryNameLocalInAlphabet("", TERRITORY_TKM, 1, ALPHABET_CYRILLIC);
    nrTests += testGetFullTerritoryNameLocalInAlphabet("تۆركمنيستآن", TERRITORY_TKM, 0, ALPHABET_ARABIC);
    nrTests += testGetFullTerritoryNameLocalInAlphabet("", TERRITORY_TKM, 1, ALPHABET_ARABIC);
    nrTests += testGetFullTerritoryNameLocalInAlphabet("", TERRITORY_KAZ, 0, ALPHABET_GREEK);
    nrTests += testGetFullTerritoryNameLocalInAlphabet("", TERRITORY_KAZ, 0, _ALPHABET_MIN);
    nrTests += testGetFullTerritoryNameLocalInAlphabet("", TERRITORY_KAZ, 0, _ALPHABET_MAX);

    return nrTests;
}


/**
 * ALPHABET TESTS.
 */

static int testAlphabetAssertion(char *msg, int condition, char *format, int a) {
    if (condition == 0) {
        foundError();
        printf("*** ERROR *** %s, ", msg);
        printf(format, a);
        printf("\n");
    }
    return 1;
}


static int testAlphabets(void) {
    int nrTests = 0;
    int j;
    const char *str, *expect;
    static const char *TEST_PAIRS[] = {
            "irl xx.xxxx", "IRL XX.XXXX",     // Grid24
            "cck XX.XX", "CCK XX.XX",       // nameless22
            "cze XX.XXX", "CZE XX.XXX",      // nameless23
            "NLD XXX.XX", "NLD XXX.XX",      // nameless32
            "VAT 5d.dd", "VAT 5D.DD",       // Grid22
            "NLD XX.XXX", "NLD XX.XXX",      // Grid23
            "bhr xxx.xx", "BHR XXX.XX",      // Grid32
            "FRA XXX.XXX", "FRA XXX.XXX",     // Grid33
            "irl xx.xxxx", "IRL XX.XXXX",     // Grid24
            "cub xxxx.xx", "CUB XXXX.XX",     // Grid42
            "ben xxxx.xxx", "BEN XXXX.XXX",    // Grid34
            "USA xxxx.xxxx", "USA XXXX.XXXX",   // Grid44
            "US-AZ hhh.hh", "US-AZ HHH.HH",    // HGrid32
            "Bel hhh.hhh", "BEL HHH.HHH",     // HGrid33
            "PAN hh.hhhh", "PAN HH.HHHH",     // HGrid24
            "GRC hhhh.hh", "GRC HHHH.HH",     // HGrid42
            "NZL hhhh.hhh", "NZL HHHH.HHH",    // HGrid43
            "KAZ hhh.hhhh", "KAZ HHH.HHHH",    // HGrid34
            "RUS xxxx.xxxx", "RUS XXXX.XXXX",   // HGrid44
            "CN-SH hhhh.hhhh", "CN-SH HHHH.HHHH", // HGrid44
            "VAT hhhhh.hhhh", "VAT HHHHH.HHHH",  // HGrid54
            "hhhhh.hhhh", "HHHHH.HHHH",      // HGrid54
            "TUV hh.hhh", "TUV HH.HHH",      // AutoHeader23
            "LVA L88.ZVR", "LVA L88.ZVR",     // AutoHeader33
            "WLF XLG.3GP", "WLF XLG.3GP",     // HGrid33 R
            "VAT j0q3.27r", "VAT J0Q3.27R",    // HGrid43 R
            "PAK hhhh.hhhh", "PAK HHHH.HHHH",   // HGrid44 R
            "xxx.xxxx", "XXX.XXXX",
            "nld XX.XX", "NLD XX.XX",
            ".123", "",
            "49.4V", "49.4V",
            "X123.P456", "X123.P456",
            "z789.b012", "Z789.B012",
            "", "",
            "-", "",
            ".", "",
            "-123", "",
            "-xyz", "",
            ".xyz", "",
            "12.34", "",
            "  OMN  112.3xx ", "OMN 112.3XX",
            " 49.4V", "49.4V",
            "NLD 49.4V-xx123", "NLD 49.4V-XX123",
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
            "12.34xx-PQRSTZVW", "12.34XX-PQRSTZVW",
            "VVX.xxx-PQRSTZVW", "VVX.XXX-PQRSTZVW",
            "x123.xx-PQRSTZVW", "X123.XX-PQRSTZVW",
            "xxx.xxxx-PQRSTZVW", "XXX.XXXX-PQRSTZVW",
            "12xx.xxx-PQRSTZVW", "12XX.XXX-PQRSTZVW",
            "xxxx.xx12-PQRSTZVW", "XXXX.XX12-PQRSTZVW",
            "99zxx.xxxx-PQRSTZVW", "99ZXX.XXXX-PQRSTZVW"
    };

    printf("%d alphabets\n", _ALPHABET_MAX);

    for (j = 0; j < (int) (sizeof(TEST_PAIRS) / sizeof(TEST_PAIRS[0])); j += 2) {
        enum Alphabet i;
        for (i = _ALPHABET_MIN + 1; i < _ALPHABET_MAX; i++) {
            UWORD enc[MAX_MAPCODE_RESULT_UTF16_LEN + 1];
            char dec[MAX_MAPCODE_RESULT_ASCII_LEN + 1];
            // see if alphabets (re)convert as expected
            str = TEST_PAIRS[j];
            expect = TEST_PAIRS[j + 1];
            convertMapcodeToAlphabetUtf16(enc, str, i);
            {
                char utf8[MAX_MAPCODE_RESULT_UTF8_LEN + 1];
                MapcodeElements mapcodeElements;
                double lat1, lon1, lat2, lon2, lat3, lon3;
                convertMapcodeToAlphabetUtf8(utf8, str, i);
                decodeMapcodeToLatLonUtf16(&lat1, &lon1, enc, TERRITORY_FRA, &mapcodeElements);
                decodeMapcodeToLatLonUtf8(&lat2, &lon2, utf8, TERRITORY_FRA, &mapcodeElements);
                decodeMapcodeToLatLonUtf8(&lat3, &lon3, str, TERRITORY_FRA, &mapcodeElements);
                ++nrTests;
                if (lat1 != lat2 || lon1 != lon2 || lat1 != lat3 || lon1 != lon3) {
                    foundError();
                    printf("*** ERROR *** Difference in ascii/utf8/utf16 decoding %s\n", str);
                }
            }
            myConvertToRoman(dec, enc);
            ++nrTests;
            if (strcmp(dec, expect)) {
                foundError();
                printf("*** ERROR *** myConvertToRoman(convertMapcodeToAlphabetUtf16(\"%s\",%d))=\"%s\"\n", str,
                       (int) i, dec);
            }
        }
    }
    return nrTests;
}


static int testAlphabetRobustness(void) {
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
    for (i = 0; i < (int) sizeof(largeString1) - 1; ++i) {
        largeString1[i] = (char) ((i % 223) + 32);
    }
    largeString1[sizeof(largeString1) - 1] = 0;
    for (i = 0; i < (int) sizeof(largeString2) - 1; ++i) {
        largeString2[i] = (char) ((i % 223) + 32);
    }
    largeString2[sizeof(largeString2) - 1] = 0;

    for (a = _ALPHABET_MIN + 1; a < _ALPHABET_MAX; a++) {

        pu = convertMapcodeToAlphabetUtf16(u1, "", a);
        nrTests += testAlphabetAssertion("convertMapcodeToAlphabetUtf16 cannot return 0", pu != 0, "alphabet=%d", a);
        nrTests += testAlphabetAssertion("convertMapcodeToAlphabetUtf16 must return empty string", pu[0] == 0,
                                         "alphabet=%d", a);

        ps = myConvertToRoman(s1, u1);
        nrTests += testAlphabetAssertion("myConvertToRoman cannot return 0", ps != 0, "alphabet=%d", a);
        nrTests += testAlphabetAssertion("myConvertToRoman must return empty string", ps[0] == 0, "alphabet=%d", a);

        pu = convertMapcodeToAlphabetUtf16(largeUnicodeString1, largeString1, ALPHABET_ROMAN);
        nrTests += testAlphabetAssertion("convertMapcodeToAlphabetUtf16 cannot return 0", pu != 0, "alphabet=%d", a);

        ps = myConvertToRoman(largeString1, pu);
        nrTests += testAlphabetAssertion("myConvertToRoman cannot return 0", ps != 0, "alphabet=%d", a);
        nrTests += testAlphabetAssertion("myConvertToRoman must return size",
                                         strlen(ps) < (sizeof(largeString1) / sizeof(largeString1[0])),
                                         "alphabet=%d", a);

        pu = convertMapcodeToAlphabetUtf16(largeUnicodeString2, largeString2, ALPHABET_ROMAN);
        nrTests += testAlphabetAssertion("convertMapcodeToAlphabetUtf16 cannot return 0", pu != 0, "alphabet=%d", a);

        ps = myConvertToRoman(largeString2, pu);
        nrTests += testAlphabetAssertion("myConvertToRoman cannot return 0", ps != 0, "alphabet=%d", a);
        nrTests += testAlphabetAssertion("myConvertToRoman must return size",
                                         strlen(ps) < (sizeof(largeString2) / sizeof(largeString2[0])),
                                         "alphabet=%d", a);
    }
    return nrTests;
}


static int testAlphabetPerTerritory(void) {
    int nrTests = 0;
    int i, j;
    for (i = _TERRITORY_MIN + 1; i < _TERRITORY_MAX; i++) {
        const TerritoryAlphabets *alphabetsForTerritory = getAlphabetsForTerritory((enum Territory) i);
        ++nrTests;
        if (alphabetsForTerritory->count < 1 || alphabetsForTerritory->count > MAX_ALPHABETS_PER_TERRITORY) {
            foundError();
            printf("*** ERROR *** Bad getAlphabetsForTerritory(%d) count: %d\n", i, alphabetsForTerritory->count);
        }
        for (j = 0; j < alphabetsForTerritory->count; j++) {
            ++nrTests;
            if (alphabetsForTerritory->alphabet[j] < 0 ||
                alphabetsForTerritory->alphabet[j] >= _ALPHABET_MAX) {
                foundError();
                printf("*** ERROR *** Bad ALPHABETS_FOR_TERRITORY[%d].alphabet[%d]: %d\n", i, j,
                       alphabetsForTerritory->alphabet[j]);
            }
        }
    }
    return nrTests;
}


int main(const int argc, const char **argv) {
    int nrTests = 0;

    // Ref unused var.
    REF(ISO3166_ALPHA[0]);

    printf("Mapcode C Library Unit Tests\n");
    printf("Library version %s (data version %s)\n", MAPCODE_C_VERSION, MAPCODE_DATA_VERSION);
#ifdef NO_POSIX_THREADS
    printf("Compiler options: NO_POSIX_THREADS\n");
#else
    printf("Using up to %d threads to test in parallel...\n", MAX_THREADS);
#endif

    if (argc != 1 || argv == NULL) {
        printf("*** ERROR *** unittest takes no arguments!\n");
        exit(1);
    }

    printf("-----------------------------------------------------------\nEnvironment tests\n");
    nrTests += testEnvironment();

    printf("-----------------------------------------------------------\nRobustness tests\n");
    nrTests += testRobustness();

    printf("-----------------------------------------------------------\nAlphabet tests\n");
    nrTests += testAlphabetRobustness();
    nrTests += testAlphabets();

    printf("-----------------------------------------------------------\nAlphabet per territory tests\n");
    nrTests += testAlphabetPerTerritory();

    printf("-----------------------------------------------------------\nParser tests\n");
    nrTests += testAlphabetParser();

    printf("-----------------------------------------------------------\nTerritory name tests\n");
    nrTests += testGetFullTerritoryName();

    printf("-----------------------------------------------------------\nTerritory tests\n");
    nrTests += testTerritoriesCsv();
    nrTests += testTerritories();
    nrTests += testTerritoryCode();
    nrTests += testTerritoryInsides();

    printf("-----------------------------------------------------------\nDistance tests\n");
    nrTests += testDistances();

    printf("-----------------------------------------------------------\nMapcode format tests\n");
    nrTests += testMapcodeFormats();
    nrTests += testFailingDecodes();

    printf("-----------------------------------------------------------\nEncode/decode tests\n");
    nrTests += testSingleEncodes();
    nrTests += testSelectedEncodes();
    nrTests += testEncodeDecode();

    printf("-----------------------------------------------------------\nRe-encode tests\n");
    nrTests += testReEncode();

    printf("-----------------------------------------------------------\n");
    printf("Done.\nExecuted %d tests, found %d errors\n", nrTests, nrErrors);
    if (nrErrors > 0) {
        printf("UNIT TESTS FAILED!\n");
    } else {
        printf("Unit tests passed\n");
    }
    return nrErrors;
}
