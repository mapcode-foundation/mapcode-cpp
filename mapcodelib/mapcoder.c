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
#include "mapcoder.h"

#undef SUPPORT_HIGH_PRECISION

/*
  no threadsafe:
    disambiguate_iso3
*/


#ifndef RELEASENEAT
#else

#include "basics.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Structures
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct {
  // input
  long    lat32,lon32;
  double  fraclat,fraclon;
  // output
  Mapcodes *mapcodes;
} encodeRec;

typedef struct {
  // input
  const char *orginput;   // original full input string
  char minput[MAX_MAPCODE_RESULT_LEN]; // room to manipulate clean copy of input
  const char *mapcode;    // input mapcode (first character of proper mapcode excluding territory code)
  const char *extension;  // input extension (or empty)
  int context;            // input territory context (or negative)
  const char *iso;        // input territory alphacode (context)
  // output
  double lat,lon;         // result
  long lat32,lon32;       // result in integer arithmetic (millionts of degrees)
} decodeRec;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Data access
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int firstrec(int ccode) { return data_start[ccode]; }
static int lastrec(int ccode)  { return data_start[ccode+1]-1; }

static int ParentTerritoryOf(int ccode) // returns parent, or -1
{
  if (ccode>=usa_from && ccode<=usa_upto) return ccode_usa;
  if (ccode>=ind_from && ccode<=ind_upto) return ccode_ind;
  if (ccode>=can_from && ccode<=can_upto) return ccode_can;
  if (ccode>=aus_from && ccode<=aus_upto) return ccode_aus;
  if (ccode>=mex_from && ccode<=mex_upto) return ccode_mex;
  if (ccode>=bra_from && ccode<=bra_upto) return ccode_bra;
  if (ccode>=rus_from && ccode<=rus_upto) return ccode_rus;
  if (ccode>=chn_from && ccode<=chn_upto) return ccode_chn;
  return -1;
}

static int isSubdivision( int ccode )  { return (ParentTerritoryOf(ccode)>=0); }

static int coDex(int m)             { int c=mminfo[m].flags & 31; return 10*(c/5) + ((c%5)+1); }
static int prefixLength(int m)      { int c=mminfo[m].flags & 31; return (c/5); }
static int postfixLength(int m)     { int c=mminfo[m].flags & 31; return ((c%5)+1); }
static int isNameless(int m)        { return mminfo[m].flags & 64; }
static int recType(int m)           { return (mminfo[m].flags>>7) & 3; }
static int isRestricted(int m)      { return mminfo[m].flags & 512; }
static int isSpecialShape22(int m)  { return mminfo[m].flags & 1024; }
static char headerLetter(int m)     { return encode_chars[ (mminfo[m].flags>>11)&31 ]; }
static long smartDiv(int m)         { return (mminfo[m].flags>>16); }

#define getboundaries(m,minx,miny,maxx,maxy) get_boundaries(m,&(minx),&(miny),&(maxx),&(maxy))
static void get_boundaries(int m,long *minx, long *miny, long *maxx, long *maxy )
{
  const mminforec *mm = &mminfo[m];
  *minx = mm->minx;
  *miny = mm->miny;
  *maxx = mm->maxx;
  *maxy = mm->maxy;
}

static int isInRange(long x,long minx,long maxx) // returns nonzero if x in the range minx...maxx
{
  if ( minx<=x && x<maxx ) return 1;
  if (x<minx) x+=360000000; else x-=360000000; // 1.32 fix FIJI edge case
  if ( minx<=x && x<maxx ) return 1;
  return 0;
}

static int fitsInside( long x, long y, int m )
{
  const mminforec *mm = &mminfo[m];
  return ( mm->miny <= y && y < mm->maxy && isInRange(x,mm->minx,mm->maxx) );
}

static long xDivider4( long miny, long maxy )
{
  if (miny>=0) // both above equator? then miny is closest
    return xdivider19[ (miny)>>19 ];
  if (maxy>=0) // opposite sides? then equator is worst
    return xdivider19[0];
  return xdivider19[ (-maxy)>>19 ]; // both negative, so maxy is closest to equator
}

static int fitsInsideWithRoom( long x, long y, int m )
{
  const mminforec *mm = &mminfo[m];
  long xdiv8 = xDivider4(mm->miny,mm->maxy)/4; // should be /8 but there's some extra margin
  return ( mm->miny-60 <= y && y < mm->maxy+60 && isInRange(x,mm->minx - xdiv8, mm->maxx + xdiv8) );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Lowlevel ccode, iso, and disambiguation
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static const char *get_entity_iso3(char *entity_iso3_result,int ccode)
{
  if (ccode<0 || ccode>=MAX_MAPCODE_TERRITORY_CODE) ccode=ccode_earth; // solve bad args
  memcpy(entity_iso3_result,entity_iso+ccode*4,3);
  entity_iso3_result[3]=0;
  return entity_iso3_result;
}

// static char disambiguate_iso3[4] = { '1','?','?',0 } ; // cache for disambiguation
static int disambiguate_str( const char *s, int len ) // returns disambiguation >=1, or negative if error
{
  int res;
  const char *p=(len==2 ? parents2 : parents3);
  const char *f;
  char country[4];
  if (s[0]==0 || s[1]==0) return -27; // solve bad args
  if (len!=2 && len!=3) return -923; // solve bad args
  memcpy(country,s,len); country[len]=0;
  {
    char *t;
    for(t=country;*t!=0;t++)
        *t = toupper(*t);
  }
  f=strstr(p,country);
  if (f==NULL)
    return -23; // unknown country
  res = 1 + ((f-p)/(len+1));
  return res;
}


// returns coode, or negative if invalid
static int ccode_of_iso3(const char *in_iso, int parentcode )
{
  static char disambiguate_iso3[4] = { '1','?','?',0 } ; // cache for disambiguation
  const char *aliases=ALIASES;
  char iso[4];
  const char *s;

  if (in_iso && in_iso[0] && in_iso[1] )
  {
    if (in_iso[2])
    {
      if (in_iso[2]=='-')
      {
        parentcode=disambiguate_str(in_iso,2);
        in_iso+=3;
      }
      else if (in_iso[3]=='-')
      {
        parentcode=disambiguate_str(in_iso,3);
        in_iso+=4;
      }
    }
  } else return -23; // solve bad args

  if (parentcode>0)
    disambiguate_iso3[0] = (char)('0'+parentcode);

  // make (uppercased) copy of at most three characters
  iso[0]=toupper(in_iso[0]);
  if (iso[0]) iso[1]=toupper(in_iso[1]);
  if (iso[1]) iso[2]=toupper(in_iso[2]);
  iso[3]=0;

  if ( iso[2]==0 || iso[2]==' ' ) // 2-letter iso code?
  {
    disambiguate_iso3[1] = iso[0];
    disambiguate_iso3[2] = iso[1];

    s = strstr(entity_iso,disambiguate_iso3); // search disambiguated 2-letter iso
    if (s==NULL)
    {
      s = strstr(aliases,disambiguate_iso3); // search in aliases
      if ( s && s[3]=='=' )
      {
        memcpy(iso,s+4,3);
        s = strstr(entity_iso,iso); // search disambiguated 2-letter iso
      } else s=NULL;
    }
    if (s==NULL)
    {
      // find the FIRST disambiguation option, if any
      for(s=entity_iso-1;;)
      {
        s = strstr(s+1,disambiguate_iso3+1);
        if (s==NULL)
          break;
        if (s && s[-1]>='1' && s[-1]<='9') {
          s--;
          break;
        }
      }
      if (s==NULL)
      {
        // find first disambiguation option in aliases, if any
        for(s=aliases-1;;)
        {
          s = strstr(s+1,disambiguate_iso3+1);
          if (s==NULL)
            break;
          if (s && s[-1]>='1' && s[-1]<='9') {
            memcpy(iso,s+3,3);
            s = strstr(entity_iso,iso); // search disambiguated 2-letter iso
            break;
          }
        }
      }

      if (s==NULL)
        return -26;
    }
  }
  else
  {
    s = strstr(entity_iso,iso); // search 3-letter iso
    if (s==NULL)
    {
        s = strstr(aliases,iso); // search in aliases
        if ( s && s[3]=='=' )
        {
          memcpy(iso,s+4,3);
          s = strstr(entity_iso,iso);
        } else s=NULL;
    }
    if (s==NULL)
      return -23;
  }
  // return result
  return (s-entity_iso)/4;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  HIGH PRECISION
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void encodeExtension(char *result,long extrax4,long extray,long dividerx4,long dividery,int extraDigits,int ydirection,const encodeRec *enc) // append extra characters to result for more precision
{
#ifndef SUPPORT_HIGH_PRECISION // old integer-arithmetic version
  if (extraDigits<0) extraDigits=0; else if (extraDigits>2) extraDigits=2;
  while (extraDigits-->0)
  {
    long gx=((30*extrax4)/dividerx4);
    long gy=((30*extray )/dividery );
    long column1=(gx/6);
    long column2=(gx%6);
    long row1=(gy/5);
    long row2=(gy%5);
    // add postfix:
    char *s = result+strlen(result);
    *s++ = '-';
    *s++ = encode_chars[ row1*5+column1 ];
    if (extraDigits-->0)
      *s++ = encode_chars[ row2*6+column2 ];
    *s++ = 0;
  }
#else // new floating-point version
  char *s = result+strlen(result);
  double encx = (extrax4+4* enc->fraclon) / (dividerx4);
  double ency = (extray +   enc->fraclat*ydirection) / (dividery );

  if (extraDigits<0) extraDigits=0; else if (extraDigits>MAX_PRECISION_DIGITS) extraDigits=MAX_PRECISION_DIGITS;

  if (extraDigits>0)
    *s++ = '-';

  while (extraDigits-->0)
  {
    long gx,gy,column1,column2,row1,row2;

    encx*=30; gx=(long)encx; if (gx<0)
      gx=0;
    else if (gx>29)
      gx=29;
    ency*=30; gy=(long)ency; if (gy<0)
      gy=0;
    else if (gy>29)
      gy=29;
    column1=(gx/6);
    column2=(gx%6);
    row1=(gy/5);
    row2=(gy%5);
    // add postfix:
    *s++ = encode_chars[ row1*5+column1 ];
    if (extraDigits-->0)
      *s++ = encode_chars[ row2*6+column2 ];
    *s = 0;
    encx-=gx;
    ency-=gy;
  }
#endif
}

#define decodeChar(c) decode_chars[(unsigned char)c] // force c to be in range of the index, between 0 and 255

// this routine takes the integer-arithmeteic decoding results (in millionths of degrees), adds any floating-point precision digits, and returns the result (still in millionths)
static void decodeExtension(decodeRec *dec, long dividerx4,long dividery,int ydirection)
{
  const char *extrapostfix = dec->extension;
#ifndef SUPPORT_HIGH_PRECISION // old integer-arithmetic version
  long extrax,extray;
  if (*extrapostfix) {
    int column1,row1,column2,row2,c1,c2;
    c1 = extrapostfix[0];
    c1 = decodeChar(c1);
    if (c1<0) c1=0; else if (c1>29) c1=29;
    row1 =(c1/5); column1 = (c1%5);
    c2 = (extrapostfix[1]) ? extrapostfix[1] : 72; // 72='H'=code 15=(3+2*6)
    c2 = decodeChar(c2);
    if (c2<0) c2=0; else if (c2>29) c2=29;
    row2 =(c2/6); column2 = (c2%6);

    extrax = ((column1*12 + 2*column2 + 1)*dividerx4+120)/240;
    extray = ((row1*10 + 2*row2 + 1)*dividery  +30)/ 60;
  }
  else {
    extrax = (dividerx4/8);
    extray = (dividery/2);
  }
  extray *= ydirection;
  dec->lon32 = extrax + dec->lon32;
  dec->lat32 = extray + dec->lat32;
#else // new floating-point version
  double dividerx=dividerx4/4.0,processor=1.0;
  dec->lon=0;
  dec->lat=0;
  while (*extrapostfix) {
    int column1,row1,column2,row2;
    double halfcolumn=0;
    int c1 = *extrapostfix++;
    c1 = decodeChar(c1);
    if (c1<0) c1=0; else if (c1>29) c1=29;
    row1 =(c1/5); column1 = (c1%5);
    if (*extrapostfix) {
      int c2 = decodeChar(*extrapostfix++);
      if (c2<0) c2=0; else if (c2>29) c2=29;
      row2 =(c2/6); column2 = (c2%6);
    }
    else {
      row2 = 2;
      halfcolumn = 0.5;
      column2 = 3;
    }

    processor *= 30;
    dec->lon += ((column1*6 + column2 ))/processor;
    dec->lat += ((row1*5 + row2 - halfcolumn))/processor;
  }

  dec->lon += (0.5/processor);
  dec->lat += (0.5/processor);

  dec->lon *= dividerx;
  dec->lat *= (dividery * ydirection);

  dec->lon += dec->lon32;
  dec->lat += dec->lat32;

  // also convert back to long
  dec->lon32 = (long)dec->lon;
  dec->lat32 = (long)dec->lat;
#endif
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  LOWEST-LEVEL BASE31 ENCODING/DECODING
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// encode 'value' into result[nrchars]
static void encodeBase31( char *result, long value, int nrchars )
{
  result[nrchars]=0; // zero-terminate!
  while ( nrchars-- > 0 )
  {
    result[nrchars] = encode_chars[ value % 31 ];
    value /= 31;
  }
}

// decode 'code' until either a dot or an end-of-string is encountered
static long decodeBase31( const char *code )
{
  long value=0;
  while (*code!='.' && *code!=0 )
  {
    value = value*31 + decodeChar(*code++);
  }
  return value;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  SECOND-LEVEL ECCODING/DECODING : RELATIVE
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void encode_triple( char *result, long difx,long dify )
{
    if ( dify < 4*34 ) // first 4(x34) rows of 6(x28) wide
    {
      encodeBase31( result,   ((difx/28) + 6*(dify/34)), 1 );
      encodeBase31( result+1, ((difx%28)*34 +(dify%34)), 2 );
    }
    else // bottom row
    {
      encodeBase31( result,   (difx/24)  + 24           , 1 );
      encodeBase31( result+1, (difx%24)*40 + (dify-136) , 2 );
    }
} // encode_triple


static void decode_triple( const char *result, long *difx, long *dify )
{
      // decode the first character
      int c1 = decodeChar(*result++);
      if ( c1<24 )
      {
        long m = decodeBase31(result);
        *difx = (c1%6) * 28 + (m/34);
        *dify = (c1/6) * 34 + (m%34);
      }
      else // bottom row
      {
        long x = decodeBase31(result);
        *dify = (x%40) + 136;
        *difx = (x/40) + 24*(c1-24);
      }
} // decode_triple







////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  SECOND-LEVEL ECCODING/DECODING : GRID
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static long encodeSixWide( long x, long y, long width, long height )
{
  long v;
  long D=6;
  long col = x/6;
  long maxcol = (width-4)/6;
  if ( col>=maxcol )
  {
    col=maxcol;
    D = width-maxcol*6;
  }
  v = (height * 6 * col) + (height-1-y)*D + (x-col*6);
  return v;
}

static void decodeSixWide( long v, long width, long height, long *x, long *y )
{
  long w;
  long D=6;
  long col = v / (height*6);
  long maxcol = (width-4)/6;
  if ( col>=maxcol )
  {
    col=maxcol;
    D = width-maxcol*6;
  }
  w = v - (col * height * 6 );

  *x = col*6 + (w%D);
  *y = height-1 - (w/D);
}


// decodes dec->mapcode in context of territory rectangle m
static int decodeGrid(decodeRec *dec, int m, int hasHeaderLetter )
{
  const char* input = (hasHeaderLetter ? dec->mapcode+1 : dec->mapcode);
  int codexlen = strlen(input)-1;
  long prelen = (long)(strchr(input,'.')-input);
  char result[MAX_PROPER_MAPCODE_LEN+1];

  if (codexlen>MAX_PROPER_MAPCODE_LEN) return -109;
  strcpy(result,input);
  if (prelen==1 && codexlen==5)
  {
    result[1]=result[2]; result[2]='.'; prelen++;
  }

  {
    long postlen = codexlen-prelen;
    long codex = 10*prelen + postlen;
    long divx,divy;

    divy = smartDiv(m);
    if (divy==1)
    {
      divx = xside[prelen];
      divy = yside[prelen];
    }
    else
    {
      long pw = nc[prelen];
      divx = ( pw / divy );
      divx = (long)( pw / divy );
    }

    if ( prelen==4 && divx==xside[4] && divy==yside[4] )
    {
      char t = result[1]; result[1]=result[2]; result[2]=t;
    }

    {
      long relx=-1,rely=-1,v=0;
      long minx,maxx,miny,maxy;
      getboundaries(m,minx,miny,maxx,maxy);

      if ( prelen <= MAXFITLONG )
      {
        v = decodeBase31(result);

        if ( divx!=divy && codex>24 ) // D==6 special grid, useful when prefix is 3 or more, and not a nice 961x961
        { // DECODE
          decodeSixWide( v,divx,divy, &relx,&rely );
        }
        else
        {
          relx = (v/divy);
          rely = (v%divy);
          rely = divy-1-rely;
        }

      }

      {
        long ygridsize = (maxy-miny+divy-1)/divy; // lonlat per cell
        long xgridsize = (maxx-minx+divx-1)/divx; // lonlat per cell

        if (relx<0 || rely<0 || relx>=divx || rely>=divy)
          return -111;

        // and then encodde relative to THE CORNER of this cell
        rely = miny + (rely*ygridsize);
        relx = minx + (relx*xgridsize);

        {
          long dividery = ( (((ygridsize))+yside[postlen]-1)/yside[postlen] );
          long dividerx = ( (((xgridsize))+xside[postlen]-1)/xside[postlen] );
          // decoderelative

          {
            char *r = result+prelen+1;
            long difx,dify;

            if ( postlen==3 ) // decode special
            {
              decode_triple( r, &difx,&dify );
            }
            else
            {
              long v;
              if ( postlen==4 ) { char t = r[1]; r[1]=r[2]; r[2]=t; } // swap
              v = decodeBase31(r);
              difx = ( v/yside[postlen] );
              dify = ( v%yside[postlen] );
              if ( postlen==4 ) { char t = r[1]; r[1]=r[2]; r[2]=t; } // swap back
            }

            // reverse y-direction
            dify = yside[postlen]-1-dify;

            dec->lon32 = relx+(difx * dividerx);
            dec->lat32 = rely+(dify * dividery);
            decodeExtension(dec,  dividerx<<2,dividery,1); // grid
          } // decoderelative

        }
      }
    }
  }
  return 0;
}





static void encodeGrid( char* result, const encodeRec *enc, int const m, int extraDigits, char headerLetter )
{
  long minx,miny,maxx,maxy;
  long y= enc->lat32,x= enc->lon32;
  int orgcodex=coDex(m);
  int codex=orgcodex;
  if (codex==21) codex=22;
  if (codex==14) codex=23;
  getboundaries(m,minx,miny,maxx,maxy);

  *result=0;
  if (headerLetter) result++;

  { // encode
    long divx,divy;
    long prelen = codex/10;
    long postlen = codex%10;

    divy = smartDiv(m);
    if (divy==1)
    {
      divx = xside[prelen];
      divy = yside[prelen];
    }
    else
    {
      long pw = nc[prelen];
      divx = ( pw / divy );
      divx = (long)( pw / divy );
    }

    { // grid
      long ygridsize = (maxy-miny+divy-1)/divy;
      long xgridsize = (maxx-minx+divx-1)/divx;
      long rely = y-miny;
      long relx = x-minx;

      if ( relx<0 )
      {
        relx+=360000000; x+=360000000;
      }
      else if ( relx>=360000000 ) // 1.32 fix FIJI edge case
      {
        relx-=360000000; x-=360000000;
      }

      rely /= ygridsize;
      relx /= xgridsize;

      if (relx>=divx || rely>=divy) return;

      { // prefix
        long v;
        if ( divx!=divy && codex>24 )
          v = encodeSixWide( relx,rely, divx,divy );
        else
          v = relx*divy + (divy-1-rely);
        encodeBase31( result, v, prelen);
      } // prefix

      if ( prelen==4 && divx==xside[4] && divy==yside[4] )
      {
        char t = result[1]; result[1]=result[2]; result[2]=t;
      }

      rely = miny + (rely*ygridsize);
      relx = minx + (relx*xgridsize);

      { // postfix
        long dividery = ( (((ygridsize))+yside[postlen]-1)/yside[postlen] );
        long dividerx = ( (((xgridsize))+xside[postlen]-1)/xside[postlen] );
        long extrax,extray;

        {
          char *resultptr = result+prelen;


          long difx = x-relx;
          long dify = y-rely;

          *resultptr++ = '.';

          extrax = difx % dividerx;
          extray = dify % dividery;
          difx /= dividerx;
          dify /= dividery;


          // reverse y-direction
          dify = yside[postlen]-1-dify;

          if ( postlen==3 ) // encode special
          {
            encode_triple( resultptr, difx,dify );
          }
          else
          {
            encodeBase31(resultptr,       (difx)*yside[postlen]+dify, postlen   );
            // swap 4-long codes for readability
            if ( postlen==4 )
            {
              char t = resultptr[1]; resultptr[1]=resultptr[2]; resultptr[2]=t;
            }
          }
        }

        if (orgcodex==14)
        {
          result[2]=result[1]; result[1]='.';
        }

        encodeExtension(result,extrax<<2,extray,dividerx<<2,dividery,extraDigits,1,enc); // grid
        if (headerLetter) { result--; *result=headerLetter; }
      } // postfix
    } // grid
  }  // encode
}





// find first territory rectangle of the same type as m
static int firstNamelessRecord(int codex, int m, int firstcode) {
    int i = m;
    while (i >= firstcode && coDex(i) == codex && isNameless(i)) i--;
    return (i+1);
}

// count all territory rectangles of the same type as m
static int countNamelessRecords(int codex, int m, int firstcode) {
    int i = firstNamelessRecord(codex, m, firstcode);
    int e = m;
    while (coDex(e) == codex) e++;
    return (e-i);
}




// decodes dec->mapcode in context of territory rectangle m, territory dec->context
// Returns x<0 in case of error, or record#
static int decodeNameless(decodeRec *dec, int m )
{
  int A,F;
  char input[8];
  int codexm = coDex(m);
  int dc = (codexm != 22) ? 2 : 3;

  int codexlen = strlen(dec->mapcode)-1;
  if ( codexlen!=4 && codexlen!=5 )
    return -2; // solve bad args

  // copy without dot
  strcpy(input,dec->mapcode);
  strcpy(input+dc,dec->mapcode+dc+1);


  A = countNamelessRecords(codexm,m,firstrec(dec->context));
  F = firstNamelessRecord( codexm,m,firstrec(dec->context));

  if ( (A<2 && codexm!=21) || A<1 )
    return -3;

  { // check just in case
    int p = 31/A;
    int r = 31%A;
    long v;
    long SIDE;
    int swapletters=0;
    long xSIDE;
    long X=-1;
    long miny,maxy,minx,maxx;

    // make copy of input, so we can swap around letters during the decoding
    char result[32];
    strcpy(result,input);

    // now determine X = index of first area, and SIDE
    if ( codexm!=21 && A<=31 )
    {
      int offset = decodeChar(*result);

      if ( offset < r*(p+1) )
      {
        X = offset / (p+1);
      }
      else
      {
        swapletters=(p==1 && codexm==22);
        X = r + (offset-(r*(p+1))) / p;
      }
    }
    else if ( codexm!=21 && A<62 )
    {

      X = decodeChar(*result);
      if ( X < (62-A) )
      {
        swapletters=(codexm==22);
      }
      else
      {
        X = X+(X-(62-A));
      }
    }
    else // code==21 || A>=62
    {
      long BASEPOWER = (codexm==21) ? 961*961 : 961*961*31;
      long BASEPOWERA = (BASEPOWER/A);

      if (A==62) BASEPOWERA++; else BASEPOWERA = 961*(BASEPOWERA/961);

      v = decodeBase31(result);
      X  = (long)(v/BASEPOWERA);
      v %= BASEPOWERA;
    }



    if (swapletters)
    {
      m = F + X;
      if ( ! isSpecialShape22(m) )
      {
        char t = result[codexlen-3]; result[codexlen-3]=result[codexlen-2]; result[codexlen-2]=t;
      }
    }

    // adjust v and X
    if ( codexm!=21 && A<=31 )
    {

      v = decodeBase31(result);
      if (X>0)
      {
        v -= (X*p + (X<r ? X : r)) * (961*961);
      }

    }
    else if ( codexm!=21 && A<62 )
    {

      v = decodeBase31(result+1);
      if ( X >= (62-A) )
        if ( v >= (16*961*31) )
        {
          v -= (16*961*31);
          X++;
        }
    }

    m = F + X;
    SIDE = smartDiv(m);

    getboundaries(m,minx,miny,maxx,maxy);
    if ( isSpecialShape22(m) )
    {
      xSIDE = SIDE*SIDE;
      SIDE = 1+((maxy-miny)/90); // side purely on y range
      xSIDE = xSIDE / SIDE;
    }
    else
    {
      xSIDE=SIDE;
    }

    // decode
    {
      long dx,dy;

      if ( isSpecialShape22(m) )
      {
        decodeSixWide(v,xSIDE,SIDE,&dx,&dy);
        dy=SIDE-1-dy;
      }
      else
      {
        dy = v%SIDE;
        dx = v/SIDE;
      }



      if ( dx>=xSIDE )
      {
        return -123;
      }

      {
        long dividerx4 = xDivider4(miny,maxy); // *** note: dividerx4 is 4 times too large!
        long dividery = 90;

        dec->lon32 = minx + ((dx * dividerx4)/4); // *** note: FIRST multiply, then divide... more precise, larger rects
        dec->lat32 = maxy - (dy*dividery);
        decodeExtension(dec, dividerx4,dividery,-1); // nameless

#ifdef SUPPORT_HIGH_PRECISION // BUGFIX!
        dec->lon += ((dx * dividerx4)%4)/4.0;
#endif

        return m;
      }
    }
  }
}





static void repack_if_alldigits(char *input,int aonly)
{
  char *s=input;
  int alldigits=1; // assume all digits
  char *e;
  char *dotpos=NULL;

  for (e=s;*e!=0 && *e!='-';e++) if (*e<'0' || *e>'9') { if (*e=='.' && !dotpos) dotpos=e; else { alldigits=0; break; } }
  e--; s=e-1;
  if (alldigits && dotpos && s>dotpos) // e is last char, s is one before, both are beyond dot, all characters are digits
  {
    if (aonly) // v1.50 - encode only using the letter A
    {
      int v = ((*input)-'0')*100 + ((*s)-'0')*10 + ((*e)-'0');
      *input='A';
      *s = encode_chars[v/32];
      *e = encode_chars[v%32];
    }
    else // encode using A,E,U
    {
      int v = ((*s)-'0') *10 + ((*e)-'0');
      *s = encode_chars[(v/34)+31];
      *e = encode_chars[v%34];
    }
  }
}

static int unpack_if_alldigits(char *input) // returns 1 if unpacked, 0 if left unchanged, negative if unchanged and an error was detected
{ // rewrite all-digit codes
  char *s=input;
  char *dotpos=NULL;
  int aonly=(*s=='A' || *s=='a'); if (aonly) s++; //*** v1.50
  for (;*s!=0 && s[2]!=0 && s[2]!='-';s++)
  {
    if (*s=='-')
      break;
    else if (*s=='.' && !dotpos)
      dotpos=s;
    else if ( decodeChar(*s)<0 || decodeChar(*s)>9 )
      return 0;  // nondigit, so stop
  }

  if (dotpos)
  {
      if (aonly) // v1.50 encoded only with A's
      {
        int v = (s[0]=='A' || s[0]=='a' ? 31 : decodeChar(s[0])) * 32 + (s[1]=='A' || s[1]=='a' ? 31 : decodeChar(s[1]));
        *input  = '0'+(v/100);
        s[0]= '0'+((v/10)%10);
        s[1]= '0'+(v%10);
        return 1;
      } // v1.50

    if ( *s=='a' || *s=='e' || *s=='u' || *s=='A' || *s=='E' || *s=='U' ) // thus, all digits, s[2]=0, after dot
    {
      char *e=s+1;  // s is vowel, e is lastchar

      int v=0;
      if (*s=='e' || *s=='E') v=34;
      else if (*s=='u' || *s=='U') v=68;

      if (*e=='a' || *e=='A') v+=31;
      else if (*e=='e' || *e=='E') v+=32;
      else if (*e=='u' || *e=='U') v+=33;
      else if (decodeChar(*e)<0) return -9; // invalid last character!
      else v+=decodeChar(*e);

      if (v<100)
      {
        *s = encode_chars[(unsigned int)v/10];
        *e = encode_chars[(unsigned int)v%10];
      }
      return 1;
    }
  }
  return 0; // no vowel just before end
}


static int stateletter(int ccode) // parent
{
  if (ccode>=usa_from && ccode<=usa_upto) return 1; //ccode_usa
  if (ccode>=ind_from && ccode<=ind_upto) return 2; //ccode_ind
  if (ccode>=can_from && ccode<=can_upto) return 3; //ccode_can
  if (ccode>=aus_from && ccode<=aus_upto) return 4; //ccode_aus
  if (ccode>=mex_from && ccode<=mex_upto) return 5; //ccode_mex
  if (ccode>=bra_from && ccode<=bra_upto) return 6; //ccode_bra
  if (ccode>=rus_from && ccode<=rus_upto) return 7; //ccode_rus
  if (ccode>=chn_from && ccode<=chn_upto) return 8; //ccode_chn
  return 0;
}





// returns -1 (error), or m (also returns *result!=0 in case of success)
static int encodeNameless( char *result, const encodeRec* enc, int input_ctry, int codexm, int extraDigits, int m )
{
  // determine how many nameless records there are (A), and which one is this (X)...
  long y= enc->lat32,x= enc->lon32;
  int  A = countNamelessRecords(codexm, m, firstrec(input_ctry));
  long X = m - firstNamelessRecord( codexm, m, firstrec(input_ctry));

  *result=0;

  {
    int p = 31/A;
    int r = 31%A; // the first r items are p+1
    long codexlen = (codexm/10)+(codexm%10);
    // determine side of square around centre
    long SIDE;

    long storage_offset;
    long miny,maxy;
    long minx,maxx;

    long xSIDE,orgSIDE;


    if ( codexm!=21 && A<=31 )
    {
      int size=p; if (X<r) size++; // example: A=7, p=4 r=3:  size(X)={5,5,5,4,4,4,4}
      storage_offset= (X*p + (X<r ? X : r)) * (961*961); // p=4,r=3: offset(X)={0,5,10,15,19,23,27}-31
    }
    else if ( codexm!=21 && A<62 )
    {
      if ( X < (62-A) )
      {
        storage_offset = X*(961*961);
      }
      else
      {
        storage_offset = (62-A +           ((X-62+A)/2) )*(961*961);
        if ( (X+A) & 1 )
        {
          storage_offset += (16*961*31);
        }
      }
    }
    else
    {
      long BASEPOWER = (codexm==21) ? 961*961 : 961*961*31;
      long BASEPOWERA = (BASEPOWER/A);
      if (A==62)
        BASEPOWERA++;
      else
        BASEPOWERA = (961) *           (BASEPOWERA/961);

      storage_offset = X * BASEPOWERA;
    }
    SIDE = smartDiv(m);

    getboundaries(m,minx,miny,maxx,maxy);
    orgSIDE=xSIDE=SIDE;
    if ( isSpecialShape22(m) ) //  - keep the existing rectangle!
    {
        SIDE = 1+((maxy-miny)/90); // new side, based purely on y-distance
        xSIDE = (orgSIDE*orgSIDE) / SIDE;
    }

    if (fitsInside(x,y,m))
    {
      long v = storage_offset;

      long dividerx4 = xDivider4(miny,maxy); // *** note: dividerx4 is 4 times too large!
#ifdef SUPPORT_HIGH_PRECISION // precise encoding: take fraction into account!
      long xFracture = (long)(4* enc->fraclon);
#else
      long xFracture = 0;
#endif
      long dx = (4*(x-minx)+xFracture)/dividerx4; // like div, but with floating point value
      long extrax4 = (x-minx)*4 - dx*dividerx4; // like modulus, but with floating point value

      long dividery = 90;
      long dy     = (maxy-y)/dividery;  // between 0 and SIDE-1
      long extray = (maxy-y)%dividery;

#ifdef SUPPORT_HIGH_PRECISION // precise encoding: check if fraction takes this out of range
      if (extray==0 &&  enc->fraclat>0) {
          if (dy==0)
            return -1; // fraction takes this coordinate out of range
          dy--;
          extray += dividery;

      }
#endif

      if ( isSpecialShape22(m) )
      {
        v += encodeSixWide(dx,SIDE-1-dy,xSIDE,SIDE);
      }
      else
      {
        v +=  (dx*SIDE + dy);
      }

      encodeBase31( result, v, codexlen+1 ); // nameless
      {
        int dotp=codexlen;
        if (codexm==13)
          dotp--;
        memmove(result+dotp,result+dotp-1,4); result[dotp-1]='.';
      }

      if ( ! isSpecialShape22(m) )
      if ( codexm==22 && A<62 && orgSIDE==961 )
      {
        char t = result[codexlen-2]; result[codexlen-2]=result[codexlen]; result[codexlen]=t;
      }

      encodeExtension(result,extrax4,extray,dividerx4,dividery,extraDigits,-1,enc); // nameless

      return m;

    } // in range
  }
  return -1;
}




// decodes dec->mapcode in context of territory rectangle m
static int decodeAutoHeader(decodeRec *dec,  int m )
{
  const char *input = dec->mapcode;
  int err = -1;
  int firstcodex = coDex(m);
  char *dot = strchr(input,'.');

  long STORAGE_START=0;
  long value;

  if (dot==NULL)
    return -201;

  value = decodeBase31(input); // decode top
  value *= (961*31);

  for ( ; coDex(m)==firstcodex && recType(m)>1; m++ )
  {
    long minx,miny,maxx,maxy;
    getboundaries(m,minx,miny,maxx,maxy);

    {
      // determine how many cells
      long H = (maxy-miny+89)/90; // multiple of 10m
      long xdiv = xDivider4(miny,maxy);
      long W = ( (maxx-minx)*4 + (xdiv-1) ) / xdiv;
      long product;

      // decode
      // round up to multiples of YSIDE3*XSIDE3...
      H = YSIDE3*( (H+YSIDE3-1)/YSIDE3 );
      W = XSIDE3*( (W+XSIDE3-1)/XSIDE3 );
      product = (W/XSIDE3)*(H/YSIDE3)*961*31;
      {
        long GOODROUNDER = coDex(m)>=23 ? (961*961*31) : (961*961);
        if ( recType(m)==2 ) // *+ pipe!
          product = ((STORAGE_START+product+GOODROUNDER-1)/GOODROUNDER)*GOODROUNDER - STORAGE_START;
      }


      if ( value >= STORAGE_START && value < STORAGE_START + product )
      {
        // decode
        long dividerx = (maxx-minx+W-1)/W;
        long dividery = (maxy-miny+H-1)/H;

        value -= STORAGE_START;
        value /= (961*31);

        err=0;

        // PIPELETTER DECODE
        {
          long difx,dify;
          decode_triple(dot+1,&difx,&dify); // decode bottom 3 chars
          {
            long vx = (value / (H/YSIDE3))*XSIDE3 + difx; // is vx/168
            long vy = (value % (H/YSIDE3))*YSIDE3 + dify; // is vy/176

            dec->lat32 = maxy - vy*dividery;
            dec->lon32 = minx + vx*dividerx;
            if ( dec->lon32<minx || dec->lon32>=maxx || dec->lat32<miny || dec->lat32>maxy ) // *** CAREFUL! do this test BEFORE adding remainder...
            {
              return -122; // invalid code
            }
            decodeExtension(dec,  dividerx<<2,dividery,-1); // autoheader decode

          }
        }

        break;
      }
      STORAGE_START += product;
    }
  } // for j
  return 0;
}

// encode in m (know to fit)
static int encodeAutoHeader( char *result, const encodeRec *enc, int m, int extraDigits )
{
  int i;
  long STORAGE_START=0;
  long y= enc->lat32,x= enc->lon32;

  // search back to first of the group
  int firstindex = m;
  int codex = coDex(m);
  while ( recType(firstindex - 1) > 1 && coDex(firstindex - 1) == codex)
      firstindex--;

  for (i = firstindex; coDex(i) == codex; i++)
  {
    long W,H,xdiv,product;
    long minx,miny,maxx,maxy;

    getboundaries(i,minx,miny,maxx,maxy);
    // determine how many cells
    H = (maxy-miny+89)/90; // multiple of 10m
    xdiv = xDivider4(miny,maxy);
    W = ( (maxx-minx)*4 + (xdiv-1) ) / xdiv;

    // encodee
    // round up to multiples of YSIDE3*XSIDE3...
    H = YSIDE3*( (H+YSIDE3-1)/YSIDE3 );
    W = XSIDE3*( (W+XSIDE3-1)/XSIDE3 );
    product = (W/XSIDE3)*(H/YSIDE3)*961*31;
    if ( recType(i)==2 ) { // plus pipe
      long GOODROUNDER = codex>=23 ? (961*961*31) : (961*961);
      product = ((STORAGE_START+product+GOODROUNDER-1)/GOODROUNDER)*GOODROUNDER - STORAGE_START;
    }

    if ( i==m )
    {
      // encode
      long dividerx = (maxx-minx+W-1)/W;
      long vx =     (x-minx)/dividerx;
      long extrax = (x-minx)%dividerx;

      long dividery = (maxy-miny+H-1)/H;
      long vy =     (maxy-y)/dividery;
      long extray = (maxy-y)%dividery;

      long codexlen = (codex/10)+(codex%10);
      long value = (vx/XSIDE3) * (H/YSIDE3);

#ifdef SUPPORT_HIGH_PRECISION // precise encoding: check if fraction takes this out of range
      if ( extray==0 &&  enc->fraclat>0 ) {
        if (vy==0) {
          STORAGE_START += product;
          continue;
        }
        vy--;
        extray += dividery;
      }
#endif

      value += (vy/YSIDE3);

      // PIPELETTER ENCODE
      encodeBase31( result, (STORAGE_START/(961*31)) + value, codexlen-2 );
      result[codexlen-2]='.';
      encode_triple( result+codexlen-1, vx%XSIDE3, vy%YSIDE3 );

      encodeExtension( result,extrax<<2,extray,dividerx<<2,dividery,extraDigits,-1,enc); // autoheader
      return m;
    }

    STORAGE_START += product;
  }
  return i-1; // return last autoheader record as the (failing) record
}





static int debugStopAt=-1;
static void encoderEngine( int ccode, const encodeRec *enc, int stop_with_one_result, int extraDigits, int result_override )
{
  int from,upto;
  long y= enc->lat32,x= enc->lon32;

  if (enc==NULL || ccode<0 || ccode>ccode_earth)
    return; // bad arguments

  from = firstrec(ccode);
  upto = lastrec(ccode);

  if (ccode != ccode_earth)
    if ( ! fitsInside(x,y,upto) )
      return;

  ///////////////////////////////////////////////////////////
  // look for encoding options
  ///////////////////////////////////////////////////////////
  {
    int i;
    char result[128];
    int result_counter=0;

    *result=0;
    for( i=from; i<=upto; i++ )
    {
      int codex = coDex(i);
      if (codex<54)
      {
        if (fitsInside(x,y,i))
        {
          if ( isNameless(i) )
          {
            int ret = encodeNameless( result, enc, ccode, codex, extraDigits, i );
            if (ret>=0)
            {
              i=ret;
            }
          }
          else if ( recType(i)>1 )
          {
            encodeAutoHeader(result,enc,i,extraDigits);
          }
          else if ( i==upto && isRestricted(i) && isSubdivision(ccode) ) // if the last item is a reference to a state's country
          {
            // *** do a recursive call for the parent ***
            encoderEngine( ParentTerritoryOf(ccode), enc, stop_with_one_result,extraDigits,ccode );
            return; /**/
          }
          else // must be grid
          {
              if ( result_counter>0 || !isRestricted(i) ) // skip isRestricted records unless there already is a result
              {
                char headerletter = (recType(i)==1) ? headerLetter(i) : 0;
                encodeGrid( result, enc, i, extraDigits,headerletter );
              }
          }

          // =========== handle result (if any)
          if (*result)
          {
            result_counter++;

            repack_if_alldigits(result,0);

            if (debugStopAt<0 || debugStopAt==i) {
              int cc = (result_override>=0 ? result_override : ccode);
              if (*result && enc->mapcodes && enc->mapcodes->count < MAX_NR_OF_MAPCODE_RESULTS) {
                char *s = enc->mapcodes->mapcode[enc->mapcodes->count++];
                if (cc==ccode_earth)
                  strcpy( s , result );
                else {
                  getTerritoryIsoName(s,cc+1,0);
                  strcat(s," ");
                  strcat(s,result);
                }
              }
              if (debugStopAt==i) return;
            }
            if (stop_with_one_result) return;
            *result=0; // clear for next iteration
          }
        }
      }
    } // for i
  }
}


// returns nonzero if error
static int decoderEngine( decodeRec *dec )
{
  int parentcode=-1;
  int err=-255; // unknown error
  int ccode,len;
  char *minus;
  const char *iso3;
  char *s = dec->minput;

  // copy input, cleaned of leading and trailing whitespace, into private, non-const buffer
  {
    const char *r = dec->orginput;
    while (*r>0 && *r<=32) r++; // skip lead
    len=strlen(r);
    if (len>MAX_MAPCODE_RESULT_LEN-1) len=MAX_MAPCODE_RESULT_LEN-1;
    while (len>0 && r[len-1]>=0 && r[len-1]<=32) len--; // remove trail
    memcpy(s,r,len); s[len]=0;
  }

  // make input (excluding territory) uppercase, and replace digits 0 and 1 with O and I
  {
    char *t = strchr(s,' '); if (t==NULL) t=s;
    for(;*t!=0;t++)
    {
      if (*t>='a' && *t<='z') *t += ('A'-'a');
      if (*t=='O') *t='0';
      if (*t=='I') *t='1';
    }
  }

  // check if extension, determine length without extension
  minus = strchr(s+4,'-');
  if (minus)
    len=minus-s;

  // make sure there is valid context
  iso3 = convertTerritoryCodeToIsoName(dec->context,0); // get context string (or empty string)
  if (*iso3==0) iso3="AAA";
  parentcode=disambiguate_str(iso3,strlen(iso3)); // pass for future context disambiguation

  // insert context if none in input
  if (len>0 && len<=9 && strchr(s,' ')==0) // just a non-international mapcode without a territory code?
  {
      int ilen=strlen(iso3);
      memmove(s+ilen+1,s,strlen(s)+1);
      s[ilen]=' ';
      memcpy(s,iso3,ilen);
      len+=(1+ilen);
      iso3="";
  }
  // parse off extension
  s[len]=0;
  if (minus)
    dec->extension = &s[len+1];
  else
    dec->extension="";

  // now split off territory, make point to start of proper mapcode
  if (len>8 && (s[3]==' ' || s[3]=='-') && (s[6]==' ' || s[7]==' ')) // assume ISO3 space|minus ISO23 space MAPCODE
  {
    parentcode=disambiguate_str(s,3);
    if (parentcode<0) return parentcode;
    s+=4; len-=4;
  }
  else if (len>7 && (s[2]==' ' || s[2]=='-') && (s[5]==' ' || s[6]==' ')) // assume ISO2 space|minus ISO23 space MAPCODE
  {
    parentcode=disambiguate_str(s,2);
    if (parentcode<0) return parentcode;
    s+=3; len-=3;
  }

  if (len>4 && s[3]==' ') // assume ISO whitespace MAPCODE, overriding iso3
  {
    iso3=s; // overrides iso code!
    s+=4;len-=4;
  }
  else if (len>3 && s[2]==' ') // assume ISO2 whitespace MAPCODE, overriding iso3
  {
    iso3=s; // overrides iso code!
    s+=3;len-=3;
  }

  while (*s>0 && *s<=32) {s++;len--;} // skip further whitespace

  // returns nonzero if error
  ccode = ccode_of_iso3(iso3,parentcode);
  if (ccode==ccode_mex && len<8) ccode=ccode_of_iso3("5MX",-1); // special case for mexico country vs state

  // master_decode(s,ccode)
  {
  const char *dot = NULL; // dot position in input
  int prelen;   // input prefix length
  int postlen;  // input postfix length
  int codex;    // input codex
  int from,upto; // record range for territory
  int i;


  // unpack digits (a-lead or aeu-encoded
  int voweled = unpack_if_alldigits(s);
  if (voweled<0)
    return -7;

  // debug support: U-lead pre-processing
  if (*s=='u' || *s=='U')  {
    s++;
    len--;
    voweled=1;
  }

  if (len>10) return -8;

  // find dot and check that all characters are valid
  {
    int nrd=0; // nr of true digits
    const char *r=s;
    for ( ; *r!=0; r++ )
    {
      if (*r=='.') {
        if (dot)
          return -5; // more than one dot
        dot=r;
      }
      else if ( decodeChar(*r) <  0 ) // invalid char?
        return -4;
      else if ( decodeChar(*r) < 10 ) // digit?
        nrd++;
    }
    if (dot==NULL)
      return -2;
    else if (!voweled && nrd+1==len) // everything but the dot is digit, so MUST be voweled!
      return -998;
  }

//////////// AT THIS POINT, dot=FIRST DOT, input=CLEAN INPUT (no vowels) ilen=INPUT LENGTH

  // analyse input
  prelen = (dot-s);
  postlen = len-1-prelen;
  codex = prelen*10 + postlen;
  if ( prelen<2 || prelen>5 || postlen<2 || postlen>4 )
    return -3;

  if (len==10) {
    // international mapcodes must be in international context
    ccode = ccode_earth;
  }
  else if (isSubdivision(ccode))
  {
    // long mapcodes must be interpreted in the parent of a subdivision

	  int parent = ParentTerritoryOf(ccode);
	  if (len==9 || (len==8 && (parent==ccode_ind || parent==ccode_mex )))
		  ccode = parent;
  }

  // remember final territory context
  dec->context = ccode;
  dec->mapcode = s;

  err  = -817;
  from = firstrec(ccode);
  upto = lastrec(ccode);

   // try all ccode rectangles to decode s (pointing to first character of proper mapcode)
   for ( i = from; i <= upto; i++ )
   {
    int codexi = coDex(i);
	  if ( recType(i)==0 && !isNameless(i) && (codexi==codex || (codex==22 && codexi==21) ) )
	  {
      err = decodeGrid( dec, i, 0 );

      if (isRestricted(i)) {
        int fitssomewhere=0;
        int j;
        for (j=i-1; j>=from; j--) { // look in previous rects
          if (!isRestricted((j))) {
            if ( fitsInsideWithRoom(dec->lon32,dec->lat32,j) ) {
              fitssomewhere=1;
              break;
            }
          }
        }
        if (!fitssomewhere) {
          err=-1234;
        }
      }

		  break;
	  }
	  else if ( recType(i)==1 && prefixLength(i)+1==prelen && postfixLength(i)==postlen && headerLetter(i)==*s )
	  {
      err = decodeGrid( dec, i, 1 );
		  break;
	  }
	  else if (isNameless(i) &&
	   (	(codexi==21 && codex==22)
	   ||	(codexi==22 && codex==32)
	   ||	(codexi==13 && codex==23) ) )
	  {
      i = decodeNameless( dec, i);
      if (i<0) err=i; else err=0;
		  break;
	  }
	  else if ( recType(i)>=2 && postlen==3 && prefixLength(i)+postfixLength(i)==prelen+2 )
    {
		  err = decodeAutoHeader( dec, i );
		  break;
	  }
   } // for
  }


#ifdef SUPPORT_HIGH_PRECISION
  // convert from millionths
  if (err) {
    dec->lat = dec->lon = 0;
  }
  else {
    dec->lat /= (double)1000000.0;
    dec->lon /= (double)1000000.0;
  }
#else
  // convert from millionths
  if (err) dec->lat32 = dec->lon32 = 0;
  dec->lat = dec->lat32/(double)1000000.0;
  dec->lon = dec->lon32/(double)1000000.0;
#endif

  // normalise between =180 and 180
  if (dec->lat <  -90.0) dec->lat  = -90.0;
  if (dec->lat >   90.0) dec->lat  =  90.0;
  if (dec->lon < -180.0) dec->lon += 360.0;
  if (dec->lon >= 180.0) dec->lon -= 360.0;

  // store as integers for legacy's sake
  dec->lat32 = (long)(dec->lat*1000000);
  dec->lon32 = (long)(dec->lon*1000000);

  // make sure decode result fits the country
  if (err==0)
    if ( ccode != ccode_earth )
      if ( ! fitsInsideWithRoom(dec->lon32,dec->lat32, lastrec(ccode) ) ) {
        err=-2222;
  }

  return err;
}



#ifdef SUPPORT_FOREIGN_ALPHABETS

// WARNING - these alphabets have NOT yet been released as standard! use at your own risk! check www.mapcode.com for details.
static UWORD asc2lan[MAX_LANGUAGES][36] = // A-Z equivalents for ascii characters A to Z, 0-9
{
  {0x0041,0x0042,0x0043,0x0044,0x0045,0x0046,0x0047,0x0048,0x0049,0x004a,0x004b,0x004c,0x004d,0x004e,0x004f,0x0050,0x0051,0x0052,0x0053,0x0054,0x0055,0x0056,0x0057,0x0058,0x0059,0x005a, 0x0030,0x0031,0x0032,0x0033,0x0034,0x0035,0x0036,0x0037,0x0038,0x0039}, // roman
  {0x0391,0x0392,0x039e,0x0394,0x003f,0x0395,0x0393,0x0397,0x0399,0x03a0,0x039a,0x039b,0x039c,0x039d,0x039f,0x03a1,0x0398,0x03a8,0x03a3,0x03a4,0x003f,0x03a6,0x03a9,0x03a7,0x03a5,0x0396, 0x0030,0x0031,0x0032,0x0033,0x0034,0x0035,0x0036,0x0037,0x0038,0x0039}, // greek
  {0x0410,0x0412,0x0421,0x0414,0x0415,0x0416,0x0413,0x041d,0x0418,0x041f,0x041a,0x041b,0x041c,0x0417,0x041e,0x0420,0x0424,0x042f,0x0426,0x0422,0x042d,0x0427,0x0428,0x0425,0x0423,0x0411, 0x0030,0x0031,0x0032,0x0033,0x0034,0x0035,0x0036,0x0037,0x0038,0x0039}, // cyrillic
  {0x05d0,0x05d1,0x05d2,0x05d3,0x05e3,0x05d4,0x05d6,0x05d7,0x05d5,0x05d8,0x05d9,0x05da,0x05db,0x05dc,0x05e1,0x05dd,0x05de,0x05e0,0x05e2,0x05e4,0x05e5,0x05e6,0x05e7,0x05e8,0x05e9,0x05ea, 0x0030,0x0031,0x0032,0x0033,0x0034,0x0035,0x0036,0x0037,0x0038,0x0039}, // hebrew
  {0x0905,0x0915,0x0917,0x0918,0x090f,0x091a,0x091c,0x091f,0x003f,0x0920,0x0923,0x0924,0x0926,0x0927,0x003f,0x0928,0x092a,0x092d,0x092e,0x0930,0x092b,0x0932,0x0935,0x0938,0x0939,0x0921, 0x0966,0x0967,0x0968,0x0969,0x096a,0x096b,0x096c,0x096d,0x096e,0x096f}, // hindi
  {0x0d12,0x0d15,0x0d16,0x0d17,0x0d0b,0x0d1a,0x0d1c,0x0d1f,0x0d07,0x0d21,0x0d24,0x0d25,0x0d26,0x0d27,0x0d20,0x0d28,0x0d2e,0x0d30,0x0d31,0x0d32,0x0d09,0x0d34,0x0d35,0x0d36,0x0d38,0x0d39, 0x0d66,0x0d67,0x0d68,0x0d69,0x0d6a,0x0d6b,0x0d6c,0x0d6d,0x0d6e,0x0d6f}, // malay
  {0x10a0,0x10a1,0x10a3,0x10a6,0x10a4,0x10a9,0x10ab,0x10ac,0x10b3,0x10ae,0x10b0,0x10b1,0x10b2,0x10b4,0x10ad,0x10b5,0x10b6,0x10b7,0x10b8,0x10b9,0x10a8,0x10ba,0x10bb,0x10bd,0x10be,0x10bf, 0x0030,0x0031,0x0032,0x0033,0x0034,0x0035,0x0036,0x0037,0x0038,0x0039}, // Georgian
  {0x30a2,0x30ab,0x30ad,0x30af,0x30aa,0x30b1,0x30b3,0x30b5,0x30a4,0x30b9,0x30c1,0x30c8,0x30ca,0x30cc,0x30a6,0x30d2,0x30d5,0x30d8,0x30db,0x30e1,0x30a8,0x30e2,0x30e8,0x30e9,0x30ed,0x30f2, 0x0030,0x0031,0x0032,0x0033,0x0034,0x0035,0x0036,0x0037,0x0038,0x0039}, // Katakana
  {0x0e30,0x0e01,0x0e02,0x0e04,0x0e32,0x0e07,0x0e08,0x0e09,0x0e31,0x0e0a,0x0e11,0x0e14,0x0e16,0x0e17,0x0e0d,0x0e18,0x0e1a,0x0e1c,0x0e21,0x0e23,0x0e2c,0x0e25,0x0e27,0x0e2d,0x0e2e,0x0e2f, 0x0e50,0x0e51,0x0e52,0x0e53,0x0e54,0x0e55,0x0e56,0x0e57,0x0e58,0x0e59}, // Thai
  {0x0eb0,0x0e81,0x0e82,0x0e84,0x0ec3,0x0e87,0x0e88,0x0e8a,0x0ec4,0x0e8d,0x0e94,0x0e97,0x0e99,0x0e9a,0x0ec6,0x0e9c,0x0e9e,0x0ea1,0x0ea2,0x0ea3,0x0ebd,0x0ea7,0x0eaa,0x0eab,0x0ead,0x0eaf, 0x0030,0x0031,0x0032,0x0033,0x0034,0x0035,0x0036,0x0037,0x0038,0x0039}, // Laos
  {0x0556,0x0532,0x0533,0x0534,0x0535,0x0538,0x0539,0x053a,0x053b,0x053d,0x053f,0x0540,0x0541,0x0543,0x0555,0x0547,0x0548,0x054a,0x054d,0x054e,0x0545,0x054f,0x0550,0x0551,0x0552,0x0553, 0x0030,0x0031,0x0032,0x0033,0x0034,0x0035,0x0036,0x0037,0x0038,0x0039}, // armenian
  {0x0985,0x098c,0x0995,0x0996,0x098f,0x0997,0x0999,0x099a,0x003f,0x099d,0x09a0,0x09a1,0x09a2,0x09a3,0x003f,0x09a4,0x09a5,0x09a6,0x09a8,0x09aa,0x0993,0x09ac,0x09ad,0x09af,0x09b2,0x09b9, 0x09e6,0x09e7,0x09e8,0x09e9,0x09ea,0x09eb,0x09ec,0x09ed,0x09ee,0x09ef}, // Bengali
  {0x0a05,0x0a15,0x0a17,0x0a18,0x0a0f,0x0a1a,0x0a1c,0x0a1f,0x003f,0x0a20,0x0a23,0x0a24,0x0a26,0x0a27,0x003f,0x0a28,0x0a2a,0x0a2d,0x0a2e,0x0a30,0x0a2b,0x0a32,0x0a35,0x0a38,0x0a39,0x0a21, 0x0a66,0x0a67,0x0a68,0x0a69,0x0a6a,0x0a6b,0x0a6c,0x0a6d,0x0a6e,0x0a6f}, // Gurmukhi
  {0x0f58,0x0f40,0x0f41,0x0f42,0x0f64,0x0f44,0x0f45,0x0f46,0x003f,0x0f47,0x0f4a,0x0f4c,0x0f4e,0x0f4f,0x003f,0x0f51,0x0f53,0x0f54,0x0f56,0x0f5e,0x0f65,0x0f5f,0x0f61,0x0f62,0x0f63,0x0f66, 0x0f20,0x0f21,0x0f22,0x0f23,0x0f24,0x0f25,0x0f26,0x0f27,0x0f28,0x0f29}, // Tibetan
};

static struct { UWORD min; UWORD max; const char *convert; } unicode2asc[] =
{
  {0x0041,0x005a,"ABCDEFGHIJKLMNOPQRSTUVWXYZ"}, // Roman
  {0x0391,0x03a9,"ABGDFZHQIKLMNCOJP?STYVXRW"}, // Greek
  {0x0410,0x042f,"AZBGDEFNI?KLMHOJPCTYQXSVW????U?R"}, // Cyrillic
  {0x05d0,0x05ea,"ABCDFIGHJKLMNPQ?ROSETUVWXYZ"}, // Hebrew
  {0x0905,0x0939,"A?????????E?????B?CD?F?G??HJZ?KL?MNP?QU?RS?T?V??W??XY"}, // Hindi
  {0x0d07,0x0d39,"I?U?E??????A??BCD??F?G??HOJ??KLMNP?????Q?RST?VWX?YZ"}, // Malay
  {0x10a0,0x10bf,"AB?CE?D?UF?GHOJ?KLMINPQRSTVW?XYZ"}, // Georgisch
  {0x30a2,0x30f2,"A?I?O?U?EB?C?D?F?G?H???J???????K??????L?M?N?????P??Q??R??S?????TV?????WX???Y????Z"}, // Katakana
  {0x0e01,0x0e32,"BC?D??FGHJ??O???K??L?MNP?Q?R????S?T?V?W????UXYZAIE"}, // Thai
  {0x0e81,0x0ec6,"BC?D??FG?H??J??????K??L?MN?P?Q??RST???V??WX?Y?ZA????????????U?????EI?O"}, // Lao
  {0x0532,0x0556,"BCDE??FGHI?J?KLM?N?U?PQ?R??STVWXYZ?OA"}, // Armenian
  {0x0985,0x09b9,"A??????B??E???U?CDF?GH??J??KLMNPQR?S?T?VW?X??Y??????Z"}, // Bengali
  {0x0a05,0x0a39,"A?????????E?????B?CD?F?G??HJZ?KL?MNP?QU?RS?T?V??W??XY"}, // Gurmukhi
  {0x0f40,0x0f66,"BCD?FGHJ??K?L?MN?P?QR?S?A?????TV?WXYEUZ"}, // Tibetan

  {0x0966,0x096f,""}, // Hindi
  {0x0d66,0x0d6f,""}, // Malai
  {0x0e50,0x0e59,""}, // Thai
  {0x09e6,0x09ef,""}, // Bengali
  {0x0a66,0x0a6f,""}, // Gurmukhi
  {0x0f20,0x0f29,""}, // Tibetan

  // lowercase variants: greek, georgisch
  {0x03B1,0x03c9,"ABGDFZHQIKLMNCOJP?STYVXRW"}, // Greek lowercase
  {0x10d0,0x10ef,"AB?CE?D?UF?GHOJ?KLMINPQRSTVW?XYZ"}, // Georgisch lowercase
  {0x0562,0x0586,"BCDE??FGHI?J?KLM?N?U?PQ?R??STVWXYZ?OA"}, // Armenian lowercase
  {0,0,NULL}
};





char *convertToRoman(char *asciibuf, int maxlen, const UWORD* s)
{
  char *w = asciibuf;
  const char *e = w+maxlen-1;
  for ( ; *s!=0 && w<e ; s++  )
  {
    if ( *s>=1 && *s<='z' ) // normal ascii
      *w++ = (char)(*s);
    else
    {
      int i,found=0;
      for (i=0; unicode2asc[i].min!=0 ;i++)
      {
        if ( *s>=unicode2asc[i].min && *s<=unicode2asc[i].max )
        {
          const char *cv = unicode2asc[i].convert;
          if (*cv==0) cv="0123456789";
          *w++ = cv[ *s - unicode2asc[i].min ];
          found=1; break;
        }
      }
      if (!found) { *w++='?'; break; }
    }
  }
  *w=0;
  if (*asciibuf=='A') // v1.50
  {
    unpack_if_alldigits(asciibuf);
    repack_if_alldigits(asciibuf,0);
  }
  return asciibuf;
}


static UWORD* encode_utf16(UWORD* unibuf,int maxlen,const char *mapcode,int language) // convert mapcode to language (0=roman 1=greek 2=cyrillic 3=hebrew)
{
  UWORD *w = unibuf;
  const UWORD *e = w+maxlen-1;
  const char *r = mapcode;
  while( *r!=0 && w<e )
  {
    char c = *r++;
    if ( c>='a' && c<='z' ) c+=('A'-'a');
    if ( c<0 || c>'Z' ) // not in any valid range?
      *w++ = '?';
    else if ( c<'A' ) // valid but not a letter (e.g. a dot, a space...)
      *w++ = c; // leave untranslated
    else
      *w++ = asc2lan[language][c-'A'];
  }
  *w=0;
  return unibuf;
}


#endif









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
                      // WHI DOT DET VOW ZER HYP
  /* 0 start        */ {  0 ,ERR, 1 , 1 ,ERR,ERR }, // looking for very first detter
  /* 1 gotL         */ { ERR,ERR, 2 , 2 ,ERR,ERR }, // got one detter, MUST get another one
  /* 2 gotLL        */ { 18 , 6 , 3 , 3 ,ERR,14  }, // GOT2: white: got territory + start prefix | dot: 2.X mapcode | det:3letter | hyphen: 2-state
  /* 3 gotLLL       */ { 18 , 6 , 4 ,ERR,ERR,14  }, // white: got territory + start prefix | dot: 3.X mapcode | det:4letterprefix | hyphen: 3-state
  /* 4 gotprefix4   */ { ERR, 6 , 5 ,ERR,ERR,ERR }, // dot: 4.X mapcode | det: got 5th prefix letter
  /* 5 gotprefix5   */ { ERR, 6 ,ERR,ERR,ERR,ERR }, // got 5char so MUST get dot!
  /* 6 prefix.      */ { ERR,ERR, 7 , 7 ,Prt,ERR }, // MUST get first letter after dot
  /* 7 prefix.L     */ { ERR,ERR, 8 , 8 ,Prt,ERR }, // MUST get second letter after dot
  /* 8 prefix.LL    */ { 22 ,ERR, 9 , 9 , GO,11  }, // get 3d letter after dot | X.2- | X.2 done!
  /* 9 prefix.LLL   */ { 22 ,ERR,10 ,10 , GO,11  }, // get 4th letter after dot | X.3- | X.3 done!
  /*10 prefix.LLLL  */ { 22 ,ERR,ERR,ERR, GO,11  }, // X.4- | x.4 done!

  /*11 mc-          */ { ERR,ERR,12 ,ERR,Prt,ERR }, // MUST get first precision letter
  /*12 mc-L         */ { 22 ,ERR,13 ,ERR, GO,ERR }, // Get 2nd precision letter | done X.Y-1
  /*13 mc-LL*       */ { 22 ,ERR,13 ,ERR, GO,ERR }, // *** keep reading precision detters *** until whitespace or done

  /*14 ctry-        */ { ERR,ERR,15 ,15 ,ERR,ERR }, // MUST get first state letter
  /*15 ctry-L       */ { ERR,ERR,16 ,16 ,ERR,ERR }, // MUST get 2nd state letter
  /*16 ctry-LL      */ { 18 ,ERR,17 ,17 ,ERR,ERR }, // white: got CCC-SS and get prefix | got 3d letter
  /*17 ctry-LLL     */ { 18 ,ERR,ERR,ERR,ERR,ERR }, // got CCC-SSS so MUST get whitespace and then get prefix

  /*18 startprefix  */ { 18 ,ERR,19 ,19 ,ERR,ERR }, // skip more whitespace, MUST get 1st prefix letter
  /*19 gotprefix1   */ { ERR,ERR,20 ,ERR,ERR,ERR }, // MUST get second prefix letter
  /*20 gotprefix2   */ { ERR, 6 ,21 ,ERR,ERR,ERR }, // dot: 2.X mapcode | det: 3d perfix letter
  /*21 gotprefix3   */ { ERR, 6 , 4 ,ERR,ERR,ERR }, // dot: 3.x mapcode | det: got 4th prefix letter

  /*22 whitespace   */ { 22 ,ERR,ERR,ERR, GO,ERR }  // whitespace until end of string
  };



  // pass fullcode=1 to recognise territory and mapcode, pass fullcode=0 to only recognise proper mapcode (without optional territory)
  // returns 0 if ok, negative in case of error (where -999 represents "may BECOME a valid mapcode if more characters are added)
  int compareWithMapcodeFormat(const char *s,int fullcode)
  {
    int nondigits=0,vowels=0;
    int state=(fullcode ? 0 : 18); // initial state
    for(;;s++) {
      int newstate,token;
      // recognise token: decode returns -2=a -3=e -4=0, 0..9 for digit or "o" or "i", 10..31 for char, -1 for illegal char
      if (*s=='.' ) token=TOKENDOT;
      else if (*s=='-' ) token=TOKENHYPH;
      else if (*s== 0  ) token=TOKENZERO;
      else if (*s==' ' || *s=='\t') token=TOKENSEP;
      else {
        signed char c = decode_chars[(unsigned char)*s];
        if (c<0) { // vowel or illegal?
          token=TOKENVOWEL; vowels++;
          if (c==-1) // illegal?
            return -4;
        } else if (c<10) { // digit
          token=TOKENCHR; // digit
        } else { // charcter B-Z
          token=TOKENCHR;
          if (state!=11 && state!=12 && state!=13) nondigits++;
        }
      }
      newstate = fullmc_statemachine[state][token];
      if (newstate==ERR)
        return -(1000+10*state+token);
      else if (newstate==GO )
        return (nondigits ? (vowels>0 ? -6 : 0) : (vowels>0 && vowels<=2 ? 0 : -5));
      else if (newstate==Prt)
        return -999;
      else if (newstate==18)
        nondigits=vowels=0;
      state=newstate;
    }
  }



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Engine
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// pass point to an array of pointers (at least 42), will be made to point to result strings...
// returns nr of results;
static int encodeLatLonToMapcodes_internal( char **v, Mapcodes *mapcodes, double lat, double lon, int tc, int stop_with_one_result, int extraDigits ) // 1.31 allow to stop after one result
{
  encodeRec enc;
  enc.mapcodes = mapcodes;
  enc.mapcodes->count=0;

  if (lat<-90) lat=-90;
  if (lat> 90) lat= 90;
  if (lon<-180) lon+=360;
  if (lon>=180) lon-=360;
#ifndef SUPPORT_HIGH_PRECISION
  lat*=1000000; if (lat<0) lat-=0.5; else lat+=0.5;
  lon*=1000000; if (lon<0) lon-=0.5; else lon+=0.5;
  enc.lat32=(long)lat;
  enc.lon32=(long)lon;
#else // precise encoding: do NOT round, instead remember the fraction...
  lat+= 90;
  lon+=180;
  lat*=1000000;
  lon*=1000000;
  enc.lat32=(long)lat;
  enc.lon32=(long)lon;
  enc.fraclat=lat-enc.lat32;
  enc.fraclon=lon-enc.lon32;
  // for 8-digit precision, cells are divided into 810,000 by 810,000 minicells.
  enc.fraclat *= 810000; if (enc.fraclat<1) enc.fraclat=0; else { if (enc.fraclat>809999) { enc.fraclat=0; enc.lat32++; } else enc.fraclat /= 810000; }
  enc.fraclon *= 810000; if (enc.fraclon<1) enc.fraclon=0; else { if (enc.fraclon>809999) { enc.fraclon=0; enc.lon32++; } else enc.fraclon /= 810000; }
  enc.lat32-= 90000000;
  enc.lon32%=360000000;
  enc.lon32-=180000000;
#endif

  if (tc==0) // ALL results?
  {
    int i;
    for(i=0;i<MAX_MAPCODE_TERRITORY_CODE;i++) {
      encoderEngine(i,&enc,stop_with_one_result,extraDigits,-1);
      if ((stop_with_one_result||debugStopAt>=0) && enc.mapcodes->count>0) break;
    }
  }
  else
  {
    encoderEngine((tc-1),&enc,stop_with_one_result,extraDigits,-1);
  }

  if (v)
  {
    int i;
    for(i=0;i<enc.mapcodes->count;i++) {
      char *s = &enc.mapcodes->mapcode[i][0];
      char *p = strchr( s, ' ' );
      if (p==NULL) {
        v[i*2+1] = (char*) "AAA";
        v[i*2  ] = s;
      }
      else {
        *p++=0;
        v[i*2+1] = s;
        v[i*2  ] = p;
      }
    }
  }

  return enc.mapcodes->count;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Wrapper for LBS team
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// threadsafe
char* getTerritoryIsoName(char *result, int tc, int format) // formats: 0=full 1=short (returns empty string in case of error)
{
  if (tc<1 || tc>MAX_MAPCODE_TERRITORY_CODE)
    *result=0;
  else {
    int p=stateletter(tc-1);
    char iso3[4];
    const char *ei = get_entity_iso3(iso3,tc-1);
    if (*ei>='0' && *ei<='9') ei++;
    if (format==0 && p)
    {
      memcpy(result,&parents2[p*3-3],2);
      result[2]='-';
      strcpy(result+3,ei);
    }
    else
    {
      strcpy(result,ei);
    }
  }
  return result;
}



int getParentCountryOf(int tc) // returns negative if tc is not a code that has a parent country
{
    int parentccode=ParentTerritoryOf(tc-1); // returns parent ccode or -1
    if (parentccode>=0) return parentccode+1;
    return -1;
}

int getCountryOrParentCountry(int tc) // returns tc if tc is a country, parent country if tc is a state, -1 if tc is invalid
{
  if (tc>0 && tc<MAX_MAPCODE_TERRITORY_CODE)
  {
    int tp=getParentCountryOf(tc);
    if (tp>0) return tp;
    return tc;
  }
  return -1;
}

int convertTerritoryIsoNameToCode(const char *string,int optional_tc) // optional_tc: pass 0 or negative if unknown
{
  int ccode = optional_tc-1;
  if (string==NULL) return -1;
  while (*string>0 && *string<=32) string++; // skip leading whitespace
  if (ccode<0 || strchr(string,'-')>=0 || strlen(string)>3 )
  {
    ccode=ccode_of_iso3(string,-1); // ignore optional_tc
  }
  else // there is a ccode, there is no hyphen in the string, and the string is as most 3 chars
  {
    char tmp[12];
    int tc = getCountryOrParentCountry(optional_tc);

    strcpy(tmp,convertTerritoryCodeToIsoName(tc,1)); // short parent country code
    strcat(tmp,"-");
    strcat(tmp,string);
    ccode=ccode_of_iso3(tmp,-1);
  }
  if (ccode<0) return -1; else return ccode+1;
}



// decode string into lat,lon; returns negative in case of error
int decodeMapcodeToLatLon( double *lat, double *lon, const char *input, int context_tc ) // context_tc is used to disambiguate ambiguous short mapcode inputs; pass 0 or negative if not available
{
  if (lat==NULL || lon==NULL || input==NULL)
  {
    return -100;
  }
  else
  {
    int ret;
    decodeRec dec;
    dec.orginput = input;
    dec.context = context_tc;

    ret = decoderEngine(&dec);
    *lat = dec.lat;
    *lon = dec.lon;
    return ret;
  }
}

#ifdef SUPPORT_FOREIGN_ALPHABETS

UWORD* convertToAlphabet(UWORD* unibuf, int maxlength, const char *mapcode,int alphabet) // 0=roman, 2=cyrillic
{
  if ( asc2lan[alphabet][4]==0x003f ) // alphabet has no letter E
    if ( strchr(mapcode,'E') || strchr(mapcode,'U') || strchr(mapcode,'e') || strchr(mapcode,'u') ) // v1.50 get rid of E and U
    {
      // safely copy mapcode into temporary buffer u
      char u[MAX_MAPCODE_RESULT_LEN];
      int len = strlen(mapcode);
      if (len>=MAX_MAPCODE_RESULT_LEN)
        len=MAX_MAPCODE_RESULT_LEN-1;
      memcpy(u,mapcode,len);
      u[len]=0;
      unpack_if_alldigits(u);
      repack_if_alldigits(u,1);
      mapcode=u;
    }
  return encode_utf16(unibuf,maxlength,mapcode,alphabet);
}


static char asciibuf[MAX_MAPCODE_RESULT_LEN];
const char *decodeToRoman(const UWORD* s)
{
  return convertToRoman(asciibuf,MAX_MAPCODE_RESULT_LEN,s);
}

static UWORD unibuf[MAX_MAPCODE_RESULT_LEN];
const UWORD* encodeToAlphabet(const char *mapcode,int alphabet) // 0=roman, 2=cyrillic
{
  return convertToAlphabet(unibuf,MAX_MAPCODE_RESULT_LEN,mapcode,alphabet);
}




#endif

int encodeLatLonToSingleMapcode( char *result, double lat, double lon, int tc, int extraDigits )
{
  char *v[2];
  Mapcodes rlocal;
  int ret=encodeLatLonToMapcodes_internal(v,&rlocal,lat,lon,tc,1,extraDigits);
  *result=0;
  if (ret<=0) // no solutions?
    return -1;
  // prefix territory unless international
  if (strcmp(v[1],"AAA")!=0) {
    strcpy(result,v[1]);
    strcat(result," ");
  }
  strcat(result,v[0]);
  return 1;
}

// encodeLatLonToMapcodes is not threadsafe!
int encodeLatLon( Mapcodes *results, double lat, double lon, int tc, int extraDigits )
{
  return encodeLatLonToMapcodes_internal(NULL,results,lat,lon,tc,0,extraDigits);
}

// Legacy: NOT threadsafe
Mapcodes rglobal;
int encodeLatLonToMapcodes( char **v, double lat, double lon, int tc, int extraDigits )
{
  return encodeLatLonToMapcodes_internal(v,&rglobal,lat,lon,tc,0,extraDigits);
}

// Legacy: NOT threadsafe
static char makeiso_bufbytes[16];
static char *makeiso_buf;
const char *convertTerritoryCodeToIsoName(int tc,int format)
{
  if (makeiso_buf==makeiso_bufbytes) makeiso_buf=makeiso_bufbytes+8; else makeiso_buf=makeiso_bufbytes;
  return (const char*)getTerritoryIsoName(makeiso_buf,tc,format);
}


#endif // RELEASENEAT
