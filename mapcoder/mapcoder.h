
#define RELEASENEAT // use clean code (mapcoder.c)
#define CSVAPP // disable for isotable and for C library
#define UWORD unsigned short int // 2-byte unsigned integer
#define SUPPORT_FOREIGN_ALPHABETS


// pass array of at least 64 strings, each at least 16 bytes
// returns nr of results
// pass tc=0 to get mapcodes in ALL territories
int coord2mc( char **v, double lat, double lon, int tc ); // returns n = nr of results (stored in v[0]...v[2*n-1]

// returns at most one mapcode (the shortest possible in the rettitory (or in ALL territories if tc=0 is passed); r must be large enough to store 24 characters including a zero terminator;
int coord2mc1( char *result, double lat, double lon, int tc ); // returns positive in case of success (mapcode stored in result)

// Converting a mapcode into a coordinate
int mc2coord( double *lat, double *lon, const char *string, int context_tc ); // context_tc is used to disambiguate ambiguous short mapcode inputs; pass 0 or negative if not available

// pass fullcode=1 to recognise territory and mapcode, pass fullcode=0 to only recognise proper mapcode (without optional territory)
// returns 0 if ok, negative in case of error (where -999 represents "may BECOME a valid mapcode if more characters are added)
int lookslikemapcode(const char *s,int fullcode);


// Routines related to territories
int text2tc(const char *string,int optional_tc); // returns negative, or tc
// formats: 0=full 1=short (state codes may be ambiguous)
const char *tc2text(int tc,int format); // returns temporary pointer to a string (will be destroyed by the next call)

int tccontext(int tc); // returns tc if tc is a country, parent country if tc is a state, -1 if tc is invalid
int tcparent(int tc);  // returns tc of parent country (if tc is a state), or negative

// Routines related to Unicode and/or foreign alphabets
#define MAXLANS 14 // 0=Roman 1=Greek 2=Cyrillic 3=Hebrew 4=Hindi 5=Malai 6=Georgisch 7=Katakana 8=Thai 9=Lao 10=Armenian 11=Bengali 12=Gurmukhi 13=Tibetan
const char *decode_to_roman(const UWORD* s);
const UWORD* encode_to_alphabet(const char *mapcode,int alphabet); // 0=roman, 2=cyrillic, 4=hindi, 12=gurmukhi

