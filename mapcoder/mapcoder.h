// Releases: always set RELEASE+RELEASENEAT, 
// (0) India borders: define TESTINDIA
// (1) NavCore3: define RELEASE_FOR_NAVCORE, undefine CSVAPP
// (2) NavKit: undefine RELEASE_FOR_NAVCORE 
// (3) mapcoder: define CSVAPP, RELEASE_FOR_NAVCORE, RELEASENEAT
// (4) isotable: undefine NEAT/NAV/CSV (make sure you #define DUMP_ISOTABLE)
// (5) web: undefine RELEASE_FOR_NAVCORE, undefine CSVAPP

#define RELEASENEAT // use clean code (mapcoder.c)
//#define RELEASE_FOR_NAVCORE
//#define CSVAPP // disable for isotable

#define UWORD unsigned short int // 2-byte unsigned integer

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  #defines
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// *** should be defined!
 #define RELEASE
#define SUPPORT_AUTOSPLIT // if rect is "almost" perfect xside/yside, make it perfect
#define PREPROCESS_NUMERIC_CODES
#define INCLUDEWORLD 
#define LIMIT_TO_OUTRECT
#define RESTRICTUSELESS
#define INTEGER_ARITHMETIC_ONLY // from now on, do only integer arithmentic, no floating point
#define REMOVE_VALIDCODE // deprecated routine

#ifndef RELEASE_FOR_NAVCORE
  #define SUPPORT_FOREIGN_ALPHABETS
  #define SUPPORT_DMS
  #define SUPPORT_LBS_WRAPPER
#endif


// *** should be undefined
#ifndef RELEASE
  #define EXIT_WHEN_ERROR
  #define VERBOSE
  #define PRINTDISTANCE
  #define PRINTDISTANCE_IN_FILE
  // #define PRINTWARNINGS
#endif

// *** should be undefined!
// #define SUPPRESS_MULTIPLE_PIPES // when encoding - undefined since NavCore3

#ifdef CSVAPP
  #undef SUPPORT_DMS
  #undef SUPPORT_LBS_WRAPPER
#endif




////////////////////////////////////////////////////////////////
//
//  disambiguation: call disambiguate(x) depending on the map that is loaded
//
////////////////////////////////////////////////////////////////

// pass a number to disambiguate a 2-letter state codes
//    1 - assume USA
//    2 - assume India
//    3 - assume Canada
//    4 - assume Australia
//    5 - assume Mexico
//    6 - assume Brazil
//    7 - assume Russia
//    8 - assume China
void disambiguate( int country );

// Pass a string (not necessarily zero-term,inated) and its length (e.g. "NLD",3 or "US",2)
// No need to check the return value, but:
//       returns negative if this string did not lean to disambiguation
//       returns a positive number (1-8) if it did
int disambiguate_str( const char *s, int len ); // returns disambiguation >=1, or negative if error


////////////////////////////////////////////////////////////////
//
//  Low-level routines, 
//
////////////////////////////////////////////////////////////////

// retuns the "ccode" for a given ISO code (3 letter country code, or 2-letter state code)
int ccode_of_iso3(const char *in_iso);

// returns iso3 for ccode
const char *entity_iso3(int ccode);

// returns full ISO3166 iso code (between 3 and 7 characters, e.g. AU-QLD is Queensland state, Australia)
const char *makeiso(int cc,int longcode);

// setup country - call before accessing any of the other low-level routines
void setup_country( int ccode );

// returns nonzero if x,y is in area of ccode
int isInArea(long x,long y,int ccode);

// returns next ccode that contains x,y;
// pass -1 the first time, pass previous ccode to find the next return value;
// returns -1 when no more areas contain x,y;) 
int findAreaFor(long x,long y,int ccode);

#ifdef SUPPORT_DMS
int interpret_coords( unsigned char *i, double *x, double *y  ); // returns negative if i does not look like a valid latitude/longitude
// display as Degrees Minutes Seconds (precise to ca 15 meters)
// pass degree symbol appropriate to your operating system in degsym, e.g. 176 (in ISO-8859-1 and its extension CP1252), although the ordinal indicator 167=º and 248 (Latin1) are also often used
unsigned char *asdms(unsigned char *r,double lat,double lon,unsigned char degsym); // make sure r is at least 23 bytes (for worstcase including zts: -##d##'##",-###d##'##")
#endif

#ifdef SUPPORT_FOREIGN_ALPHABETS
////////////////////////////////////////////////////////////////
//
//  Foreign languages
//
////////////////////////////////////////////////////////////////
#define MAXLANS 14 // 0=Roman 1=Greek 2=Cyrillic 3=Hebrew 4=Hindi 5=Malai 6=Georgisch 7=Katakana 8=Thai 9=Lao 10=Armenian 11=Bengali 12=Gurmukhi 13=Tibetan
const UWORD* encode_utf16(const char *mapcode,int language); // convert mapcode to UTF16 in given language (0=roman 1=greek ...)
const char *decode_utf16(const UWORD* s); // convert unicode (UTF16) to mapcode
#endif // ifdef SUPPORT_FOREIGN_ALPHABETS


////////////////////////////////////////////////////////////////
//
//  High-level routines
//
////////////////////////////////////////////////////////////////

#ifndef REMOVE_VALIDCODE
// returns < 0 if invalid, 0 if potentially valid, 1 if potentially a complete code (i.e. from 2.2 to 5.4, using valid chars)
int valid_code( const char *input ); 
#endif

// returns 0 if ok, negative if too different (no dot, two dots, two hyphens, three spaces, >5 letters before dot (unless the last one is an A), 5 letters after dot, invalid character, <2 or >3 letters before a hyphen)
// return -999 if the mapcode looks OK but is PARTIAL (or INCOMPLETE); example incomplete mapcodes are XX.X or XXXXX.XXX
int lookslike_mapcode(const char *s);

// encode x,y in a given country or state (states are disambiguated based on the last call to disambiguate();)
// result must be 11 bytes for worst-case (Earth) zts encode; returns empty string if error (e.g. bad iso);
// returns nonzero if a code was generated
// * if iso_found not NULL, it is filled with the "full" zts iso code (must be at least 7 bytes! worst-case result: AU-QLD)
int mapcode_encode(char *result,long y, long x, const char *iso3,char *iso_found);


// decode an input into coordinates x,y (within the context of a country or state; states are disambiguated based on the last call to disambiguate();)
// returns nonzero if error
// * if ccode_found is not NULL, it is filled with the RECOGNISED, DISAMBIGUATED code (and display entity_iso3(ccode_found) rather than iso3); 
//   note that ccode_found can be negative if the iso3 is not recognised or the input contains an unrecognised iso code as prefix!
// * if clean_input is not NULL, it is filled with the "clean" zts input (must be 11 chars to store the worst "earth" code)
int full_mapcode_decode(long *y, long *x, const char *iso3, const char *input,int *ccode_found,char *clean_input);
int mapcode_decode(long *y, long *x, const char *iso3, const char *input,int *ccode_found);


#ifdef SUPPORT_LBS_WRAPPER

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Wrapper for LBS team
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// formats: 0=full 1=short (state codes may be ambiguous)
const char *tc2text(int tc,int format); // returns temporary pointer to a string (will be destroyed by the next call)
int text2tc(const char *string,int optional_tc); // returns negative, or tc
int tcparent(int tc); // returns tc of parent country (if tc is a state), or negative
int tccontext(int tc); // returns tc if tc is a country, parent country if tc is a state, -1 if tc is invalid

int mc2coord( double *lat, double *lon, const char *string, int context_tc ); // context_tc is used to disambiguate ambiguous short mapcode inputs; pass 0 or negative if not available

// pass array of at least 64 strings, each at least 16 bytes
// returns nr of results
// pass tc=0 to get mapcodes in ALL territories
int coord2mc( char **v, double lat, double lon, int tc ); // returns n = nr of results (stored in v[0]...v[2*n-1]

const char *decode_to_roman(const UWORD* s);
const UWORD* encode_to_alphabet(const char *mapcode,int alphabet); // 0=roman, 2=cyrillic, 4=hindi, 12=gurmukhi

#endif
