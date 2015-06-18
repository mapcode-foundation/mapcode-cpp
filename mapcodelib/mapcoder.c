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
#define VERSION160

#ifndef RELEASENEAT
#else

#include "basics.h"

#define ISEARTH(c) ((c)==ccode_earth)
#define ISGOODAREA(m) ((m)>=0)
#define  CCODE_ERR   -1

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Global variables
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static const char *cached_iso3="@";   // cache for ccode_of_iso3
static char disambiguate_iso3[4] = { '1','?','?',0 } ; // cache for disambiguation
static char entity_iso3_result[4];  // cache for entity_iso3
static int current_ccode=-1; // cache for setup_country


#define MAXGLOBALRESULTS 32 // The worst actually seems to be 14, which is at 52.050500, 113.468600
#define WORSTCASE_MAPCODE_BYTES 16 // worst case is high-precision earth xxxxx.yyyy-zz, rounded upwords to multiple of 4 bytes (assuming latin-only results)
static char global_storage[2048]; // cyclic cache for results
static int storage_ptr;
static char *addstorage(const char *str)
{
  int len=strlen(str)+1; // bytes needed;
  storage_ptr &= (2048-1);
  if (storage_ptr<0 || storage_ptr+len>2048-2) storage_ptr=0;
  strcpy(global_storage+storage_ptr,str);
  storage_ptr+=len;
  return global_storage+storage_ptr-len;
}
static char **global_results;
static int nr_global_results;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Data access
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int firstrec(int ccode) { return data_start[ccode]; }
static int lastrec(int ccode)  { return data_start[ccode+1]-1; }

static int mIsUsaState( int ccode )       { return (ccode>=usa_from && ccode<=usa_upto); } // ccode_usa
static int mIsIndiaState( int ccode )     { return (ccode>=ind_from && ccode<=ind_upto); } // ccode_ind
static int mIsCanadaState( int ccode )    { return (ccode>=can_from && ccode<=can_upto); } // ccode_can
static int mIsAustraliaState( int ccode ) { return (ccode>=aus_from && ccode<=aus_upto); } // ccode_aus
static int mIsMexicoState( int ccode )    { return (ccode>=mex_from && ccode<=mex_upto); } // ccode_mex
static int mIsBrazilState( int ccode )    { return (ccode>=bra_from && ccode<=bra_upto); } // ccode_bra
static int mIsChinaState( int ccode )     { return (ccode>=chn_from && ccode<=chn_upto); } // ccode_chn
static int mIsRussiaState( int ccode )    { return (ccode>=rus_from && ccode<=rus_upto); } // ccode_rus
static int isSubdivision( int ccode )  { return mIsUsaState(ccode) || mIsIndiaState(ccode) || mIsCanadaState(ccode) || mIsAustraliaState(ccode) || mIsMexicoState(ccode) || mIsBrazilState(ccode) || mIsChinaState(ccode) || mIsRussiaState(ccode); }

// V160: remove iscountry, renames rcodex pipetype isnameless pipeletter smartdiv fast_encode/BASE31 encode6/SixWide addpostfix=encodeExtension add2res=, codexlow=>postlen, dx=>prelen
static int coDex(int m)             { int c=mminfo[m].flags & 31; return 10*(c/5) + ((c%5)+1); }
// int iscountry(int m)      { return mminfo[m].flags & 32; }
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Helper routines
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void makeup(char *s) // strupr for latin
{
  char *t;
  for(t=s;*t!=0;t++)
      *t = toupper(*t);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Lowlevel ccode, iso, and disambiguation
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static const char *entity_iso3(int ccode)
{
  if (ccode<0 || ccode>=MAX_MAPCODE_TERRITORY_CODE) ccode=ccode_earth; // solve bad args
  memcpy(entity_iso3_result,entity_iso+ccode*4,3);
  entity_iso3_result[3]=0;
  return entity_iso3_result;
}

static void disambiguate( int country ) // 1=USA 2=IND 3=CAN 4=AUS 5=MEX 6=BRA 7=RUS 8=CHI
{
  disambiguate_iso3[0] = country+'0';
}

static int disambiguate_str( const char *s, int len ) // returns disambiguation >=1, or negative if error
{
  int res;
  const char *p=(len==2 ? parents2 : parents3);
  const char *f;
  char country[4];
  if (s[0]==0 || s[1]==0) return -27; // solve bad args
  if (len!=2 && len!=3) return -923; // solve bad args
  memcpy(country,s,len); country[len]=0; makeup(country);
  f=strstr(p,country);
  if (f==NULL)
    return -23; // unknown country
  res = 1 + ((f-p)/(len+1));
  disambiguate(res);
  return res;
}

// returns coode, or negative if invalid
static int ccode_of_iso3(const char *in_iso)
{
  char iso[4];
  const char *s = cached_iso3; // assume cached index

  if (in_iso && in_iso[0] && in_iso[1] )
  {
    if (in_iso[2])
    {
      if (in_iso[2]=='-')
      {
        disambiguate_str(in_iso,2);
        in_iso+=3;
        cached_iso3="@";
      }
      else if (in_iso[3]=='-')
      {
        disambiguate_str(in_iso,3);
        in_iso+=4;
        cached_iso3="@";
      }
    }
  } else return -23; // solve bad args

  // make (uppercased) copy of at most three characters
  iso[0]=toupper(in_iso[0]);
  if (iso[0]) iso[1]=toupper(in_iso[1]);
  if (iso[1]) iso[2]=toupper(in_iso[2]);
  iso[3]=0;

  if ( memcmp(s,iso,3)!=0 ) // not equal to cache?
  {
    const char *aliases=ALIASES;
    if ( iso[2]==0 || iso[2]==' ' ) // 2-letter iso code?
    {
      disambiguate_iso3[1] = iso[0];
      disambiguate_iso3[2] = iso[1];
      if ( memcmp(s,disambiguate_iso3,3)!=0 ) // still not equal to cache?
      {
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
          cached_iso3 = s; // cache new result!
        }
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
      cached_iso3 = s; // cache new result!
    }
  }
  // return result
  return (s-entity_iso)/4;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  HIGH PRECISION
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// V160: add ExtraDigits as parameter
static double fraclon,fraclat;
static void encodeExtension(char *result,long extrax4,long extray,long dividerx4,long dividery,int extraDigits,int ydirection) // append extra characters to result for more precision
{
#ifndef VERSION160 // old integer-arithmetic version
  #define MAXPRECISIONDIGITS 2
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
  #define MAXPRECISIONDIGITS 8
  char *s = result+strlen(result);
  double encx = (extrax4+4*fraclon) / (dividerx4);
  double ency = (extray +  fraclat*ydirection) / (dividery );

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



static const char *extrapostfix="";
static double extrax,extray;
static void decodeExtension(long *nx,long *ny, long dividerx4,long dividery,int ydirection) // add extra precision to a coordinate based on extra chars
{
#ifndef VERSION160 // old integer-arithmetic version
  // long extrax,extray;
  if (*extrapostfix) {
    int column1,row1,column2,row2,c1,c2;
    c1 = extrapostfix[0];
    c1 = decode_chars[c1];
    if (c1<0) c1=0; else if (c1>29) c1=29;
    row1 =(c1/5); column1 = (c1%5);
    c2 = (extrapostfix[1]) ? extrapostfix[1] : 72; // 72='H'=code 15=(3+2*6)
    c2 = decode_chars[c2];
    if (c2<0) c2=0; else if (c2>29) c2=29;
    row2 =(c2/6); column2 = (c2%6);

    extrax = ((column1*12 + 2*column2 + 1)*dividerx4+120)/240;
    extray = ((row1*10 + 2*row2 + 1)*dividery  +30)/ 60;
  }
  else {
    extrax = (dividerx4/8);
    extray = (dividery/2);
  }

  extrax = *nx + extrax;
  extray = *ny + extray*ydirection;

  *nx = extrax;
  *ny = extray;
#else // new floating-point version
  double dividerx=dividerx4/4.0,processor=1.0;
  extrax=0;
  extray=0;
  while (*extrapostfix) {
    int column1,row1,column2,row2;
    double halfcolumn=0;
    int c1 = *extrapostfix++;
    c1 = decode_chars[c1];
    if (c1<0) c1=0; else if (c1>29) c1=29;
    row1 =(c1/5); column1 = (c1%5);
    if (*extrapostfix) {
      int c2 = decode_chars[(int)*extrapostfix++];
      if (c2<0) c2=0; else if (c2>29) c2=29;
      row2 =(c2/6); column2 = (c2%6);
    }
    else {
      row2 = 2;
      halfcolumn = 0.5;
      column2 = 3;
    }

    processor *= 30;
    extrax += ((column1*6 + column2 ))/processor;
    extray += ((row1*5 + row2 - halfcolumn))/processor;
  }

  extrax += (0.5/processor);
  extray += (0.5/processor);

  extrax *= dividerx;
  extray *= dividery;

  extrax = *nx + extrax;
  extray = *ny + extray*ydirection;

  *nx = (long)extrax;
  *ny = (long)extray;
#endif
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  SECOND-LEVEL ECCODING/DECODING : GRID
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static long x_divider( long miny, long maxy )
{
  if (miny>=0) // both above equator? then miny is closest
    return xdivider19[ (miny)>>19 ];
  if (maxy>=0) // opposite sides? then equator is worst
    return xdivider19[0];
  return xdivider19[ (-maxy)>>19 ]; // both negative, so maxy is closest to equator
}

static int isInRange(long x,long minx,long maxx) // returns nonzero if x in the range minx...maxx
{
  if ( minx<=x && x<maxx ) return 1;
  if (x<minx) x+=360000000; else x-=360000000; // 1.32 fix FIJI edge case
  if ( minx<=x && x<maxx ) return 1;
  return 0;
}

static int isInArea(long x,long y,int ccode) // returns nonzero if x,y in area (i.e. certain to generate at least one code); (0 can also mean iso3 is an invalid iso code)
{
  if (ccode<0) return 0; // solve bad args
  {
    long minx,miny,maxx,maxy;
    getboundaries(lastrec(ccode),minx,miny,maxx,maxy);
    if ( y <  miny ) return 0; // < miny!
    if ( y >= maxy ) return 0; // >= miny!
    return isInRange(x,minx,maxx);
  }
}

// TODO: Unused code - remove or place in mapcoder.h and expose with doubles.
static int findAreaFor(long x,long y,int ccode) // returns next ccode returns -1 when no more  contain x,y; pass -1 the first time, pass previous ccode to find the next return value
{
  while( (++ccode) < MAX_MAPCODE_TERRITORY_CODE )
    if ( isInArea(x,y,ccode) )
      return ccode;
  return -1;
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
    value = value*31 + decode_chars[(int)*code++];
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
      int c1 = decode_chars[(int)*result++];
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


static void decodeGrid( const char* input,long *nx,long *ny, int m, long minx, long miny, long maxx, long maxy )
{
  int codexlen = strlen(input)-1;
  long dc = (long)(strchr(input,'.')-input);
  char result[MAX_PROPER_MAPCODE_LEN+1];

  if (codexlen>MAX_PROPER_MAPCODE_LEN) return; // solve bad args
  strcpy(result,input);
  if (dc==1 && codexlen==5)
  {
    result[1]=result[2]; result[2]='.'; dc++;
  }

  {
    long postlen = codexlen-dc;
    long codex = 10*dc + postlen;
    long divx,divy;

    divy = smartDiv(m);
    if (divy==1)
    {
      divx = xside[dc];
      divy = yside[dc];
    }
    else
    {
      long pw = nc[dc];
      divx = ( pw / divy );
      divx = (long)( pw / divy );
    }

    if ( dc==4 && divx==xside[4] && divy==yside[4] )
    {
      char t = result[1]; result[1]=result[2]; result[2]=t;
    }

    {
      long relx=-1,rely=-1,v=0;

      if ( dc <= MAXFITLONG )
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
          return;

        // and then encodde relative to THE CORNER of this cell
        rely = miny + (rely*ygridsize);
        relx = minx + (relx*xgridsize);

        {
          long dividery = ( (((ygridsize))+yside[postlen]-1)/yside[postlen] );
          long dividerx = ( (((xgridsize))+xside[postlen]-1)/xside[postlen] );
          // decoderelative( result+dc+1,nx,ny, relx,rely, dividery,dividerx );

          {
            char *r = result+dc+1;
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

            *nx = relx+(difx * dividerx);
            *ny = rely+(dify * dividery);
            decodeExtension(nx,ny,  dividerx<<2,dividery,1); // grid
          } // decoderelative

        }
      }
    }
  }
} // decodeGrid




static int use_high_precision=0; // GLOBAL nr of letters of high-precision postfix (if any)
static void encodeGrid( char* result, long x,long y, int m, int codex, long minx, long miny, long maxx, long maxy )
{
  int orgcodex=codex;
  *result=0;
  if ( maxx<=minx || maxy<=miny )
    return;
  if (codex==14) codex=23;

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

        encodeExtension(result,extrax<<2,extray,dividerx<<2,dividery,use_high_precision,1); // grid
      } // postfix
    } // grid
  }  // encode
} // encodeGrid





// *********************************************************************************************************************

// indexes
static int iso_start;
static int iso_end;

// counts:
static int iso_count13;
static int iso_count21;
static int iso_count22;
static int iso_count23;
static int iso_count32;
static int iso_count24;
static int iso_count33;
static int iso_count34;
static int iso_count42;
static int iso_count43;
static int iso_count44;

// first occurances
static int iso_first13;
static int iso_first21;
static int iso_first22;
static int iso_first23;
static int iso_first32;
static int iso_first24;
static int iso_first33;
static int iso_first34;
static int iso_first42;
static int iso_first43;
static int iso_first44;

static int iso_firststar14;
static int iso_firststar22;
static int iso_firststar23;
static int iso_firststar32;
static int iso_firststar24;
static int iso_firststar33;
static int iso_firststar34;
static int iso_firststar44;

static int iso_firstpipe14;
static int iso_firstpipe22;
static int iso_firstpipe23;
static int iso_firstpipe32;
static int iso_firstpipe24;
static int iso_firstpipe33;
static int iso_firstpipe34;
static int iso_firstpipe44;

// pipe letters
static char iso_headerletter14[32];
static char iso_headerletter22[32];
static char iso_headerletter23[32];
static char iso_headerletter32[32];
static char iso_headerletter24[32];
static char iso_headerletter33[32];
static char iso_headerletter34[32];
static char iso_headerletter44[32];

static void setup_country( int newccode )
{
  int iso_count14;
  int iso_first14;

  int iso_pipecount14;
  int iso_pipecount22;
  int iso_pipecount23;
  int iso_pipecount32;
  int iso_pipecount24;
  int iso_pipecount33;
  int iso_pipecount34;
  int iso_pipecount44;


  if ( current_ccode!=newccode ) // cache check
  {
    if (newccode<0 || newccode>=MAX_MAPCODE_TERRITORY_CODE) newccode=ccode_earth; // solve bad args
    current_ccode=newccode;

    // init
    iso_start=-1;
    iso_end=-1;

    iso_count13=0;
    iso_count14=0;
    iso_count21=0;
    iso_count22=0;
    iso_count23=0;
    iso_count32=0;
    iso_count24=0;
    iso_count33=0;
    iso_count34=0;
    iso_count42=0;
    iso_count43=0;
    iso_count44=0;
    iso_pipecount14=0;
    iso_pipecount22=0;
    iso_pipecount23=0;
    iso_pipecount32=0;
    iso_pipecount24=0;
    iso_pipecount33=0;
    iso_pipecount34=0;
    iso_pipecount44=0;
    iso_firstpipe14=-1;
    iso_firstpipe22=-1;
    iso_firstpipe23=-1;
    iso_firstpipe32=-1;
    iso_firstpipe24=-1;
    iso_firstpipe33=-1;
    iso_firstpipe34=-1;
    iso_firstpipe44=-1;
    iso_first13=-1;
    iso_first14=-1;
    iso_first21=-1;
    iso_first22=-1;
    iso_first23=-1;
    iso_first32=-1;
    iso_first24=-1;
    iso_first33=-1;
    iso_first34=-1;
    iso_first42=-1;
    iso_first43=-1;
    iso_first44=-1;
    iso_firststar14=-1;
    iso_firststar22=-1;
    iso_firststar23=-1;
    iso_firststar32=-1;
    iso_firststar24=-1;
    iso_firststar33=-1;
    iso_firststar34=-1;
    iso_firststar44=-1;

    {
      int i;

      iso_start = firstrec(current_ccode);
      iso_end   = lastrec(current_ccode);

      for ( i=iso_start; i<=iso_end; i++ )
      {
        int codex = coDex(i);
        int pipe = recType(i);

        if ( codex==13 ) { iso_count13++; if ( iso_first13<0 ) iso_first13=i; }
        if ( codex==21 ) { iso_count21++; if ( iso_first21<0 ) iso_first21=i; }
        if ( codex==22 ) { if (pipe) { if (iso_firstpipe22<0) iso_firstpipe22=i; if (pipe<2)       iso_headerletter22[iso_pipecount22++]=headerLetter(i); else if (iso_firststar22<0) iso_firststar22=i;} else { iso_count22++; if ( iso_first22<0 ) iso_first22=i; } }
        if ( codex==14 ) { if (pipe) { if (iso_firstpipe14<0) iso_firstpipe14=i; if (pipe<2)       iso_headerletter14[iso_pipecount14++]=headerLetter(i); else if (iso_firststar14<0) iso_firststar14=i;} else { iso_count14++; if ( iso_first14<0 ) iso_first14=i; } }
        if ( codex==23 ) { if (pipe) { if (iso_firstpipe23<0) iso_firstpipe23=i; if (pipe<2)       iso_headerletter23[iso_pipecount23++]=headerLetter(i); else if (iso_firststar23<0) iso_firststar23=i;} else { iso_count23++; if ( iso_first23<0 ) iso_first23=i; } }
        if ( codex==32 ) { if (pipe) { if (iso_firstpipe32<0) iso_firstpipe32=i; if (pipe<2)       iso_headerletter32[iso_pipecount32++]=headerLetter(i); else if (iso_firststar32<0) iso_firststar32=i;} else { iso_count32++; if ( iso_first32<0 ) iso_first32=i; } }
        if ( codex==24 ) { if (pipe) { if (iso_firstpipe24<0) iso_firstpipe24=i; if (pipe<2)       iso_headerletter24[iso_pipecount24++]=headerLetter(i); else if (iso_firststar24<0) iso_firststar24=i;} else { iso_count24++; if ( iso_first24<0 ) iso_first24=i; } }
        if ( codex==33 ) { if (pipe) { if (iso_firstpipe33<0) iso_firstpipe33=i; if (pipe<2)       iso_headerletter33[iso_pipecount33++]=headerLetter(i); else if (iso_firststar33<0) iso_firststar33=i;} else { iso_count33++; if ( iso_first33<0 ) iso_first33=i; } }
        if ( codex==34 ) { if (pipe) { if (iso_firstpipe34<0) iso_firstpipe34=i; if (pipe<2)       iso_headerletter34[iso_pipecount34++]=headerLetter(i); else if (iso_firststar34<0) iso_firststar34=i;} else { iso_count34++; if ( iso_first34<0 ) iso_first34=i; } }
        if ( codex==42 ) { iso_count42++; if ( iso_first42<0 ) iso_first42=i; }
        if ( codex==43 ) { iso_count43++; if ( iso_first43<0 ) iso_first43=i; }
        if ( codex==44 ) { if (pipe) { if (iso_firstpipe44<0) iso_firstpipe44=i; if (pipe<2)       iso_headerletter44[iso_pipecount44++]=headerLetter(i); else if (iso_firststar44<0) iso_firststar44=i;} else { iso_count44++; if ( iso_first44<0 ) iso_first44=i; } }
      }
    }

    iso_headerletter14[iso_pipecount14]=0;
    iso_headerletter22[iso_pipecount22]=0;
    iso_headerletter23[iso_pipecount23]=0;
    iso_headerletter32[iso_pipecount32]=0;
    iso_headerletter24[iso_pipecount24]=0;
    iso_headerletter33[iso_pipecount33]=0;
    iso_headerletter34[iso_pipecount34]=0;
    iso_headerletter44[iso_pipecount44]=0;
  }
} // setup_country


static int count_city_coordinates_for_country( int codex ) // returns count
{
  if (codex==21) return iso_count21;
  if (codex==22) return iso_count22;
  if (codex==13) return iso_count13;
  return 0;
}


static int city_for_country( int X, int codex ) // returns m
{
  if ( codex==21 ) return iso_first21+X;
  if ( codex==13 ) return iso_first13+X;
  return iso_first22+X;
}


// Returns x<0 in case of error, or record#
static int decodeNameless( const char *orginput, long *x, long *y,         int input_ctry, int codex, long debug_oldx, long debug_oldy )
{
  int A;
  char input[8];
  int codexlen = strlen(orginput)-1;
  int dc = (codex != 22) ? 2 : 3;

  if ( codexlen!=4 && codexlen!=5 )
    return -2; // solve bad args

  // copy without dot (its only 4 or 5 chars)
  strcpy(input,orginput);
  strcpy(input+dc,orginput+dc+1);

  setup_country(input_ctry);

  A = count_city_coordinates_for_country( codex );
  if ( (A<2 && codex!=21) || (codex==21 && A<1) )
    return -3;

  {
    int p = 31/A;
    int r = 31%A;
    long v;
    int m;
    long SIDE;
    int swapletters=0;
    long xSIDE;
    long X=-1;
    long miny,maxy,minx,maxx;

    // make copy of input, so we can swap around letters during the decoding
    char result[32];
    strcpy(result,input);

    // now determine X = index of first area, and SIDE
    if ( codex!=21 && A<=31 )
    {
      int offset = decode_chars[(int)*result];

      if ( offset < r*(p+1) )
      {
        X = offset / (p+1);
      }
      else
      {
        swapletters=(p==1 && codex==22);
        X = r + (offset-(r*(p+1))) / p;
      }
    }
    else if ( codex!=21 && A<62 )
    {

      X = decode_chars[(int)*result];
      if ( X < (62-A) )
      {
        swapletters=(codex==22);
      }
      else
      {
        X = X+(X-(62-A));
      }
    }
    else // code==21 || A>=62
    {
      long BASEPOWER = (codex==21) ? 961*961 : 961*961*31;
      long BASEPOWERA = (BASEPOWER/A);

      if (A==62) BASEPOWERA++; else BASEPOWERA = 961*(BASEPOWERA/961);

      v = decodeBase31(result);
      X  = (long)(v/BASEPOWERA);
      v %= BASEPOWERA;
    }

    if (swapletters)
    {
      m = city_for_country(           X,codex);
      if ( ! isSpecialShape22(m) )
      {
        char t = result[codexlen-3]; result[codexlen-3]=result[codexlen-2]; result[codexlen-2]=t;
      }
    }

    // adjust v and X
    if ( codex!=21 && A<=31 )
    {

      v = decodeBase31(result);
      if (X>0)
      {
        v -= (X*p + (X<r ? X : r)) * (961*961);
      }

    }
    else if ( codex!=21 && A<62 )
    {

      v = decodeBase31(result+1);
      if ( X >= (62-A) )
        if ( v >= (16*961*31) )
        {
          v -= (16*961*31);
          X++;
        }
    }

    m = city_for_country(           X,codex);
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
        long dividerx4 = x_divider(miny,maxy); // *** note: dividerx4 is 4 times too large!
        long dividery = 90;

        *x = minx + ((dx * dividerx4)/4); // *** note: FIRST multiply, then divide... more precise, larger rects
        *y = maxy - (dy*dividery);

        decodeExtension(x,y, dividerx4,dividery,-1); // nameless

#ifdef VERSION160 // BUGFIX! @@@
        extrax += ((dx * dividerx4)%4)/4.0;
#endif

        return m;
      }
    }
  }
  return -19;
} // decodeNameless



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
    else if ( decode_chars[(int)*s]<0 || decode_chars[(int)*s]>9 )
      return 0;  // nondigit, so stop
  }

  if (dotpos)
  {
      if (aonly) // v1.50 encoded only with A's
      {
        int v = (s[0]=='A' || s[0]=='a' ? 31 : decode_chars[(int)s[0]]) * 32 + (s[1]=='A' || s[1]=='a' ? 31 : decode_chars[(int)s[1]]);
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
      else if (decode_chars[(int)*e]<0) return -9; // invalid last character!
      else v+=decode_chars[(int)*e];

      if (v<100)
      {
        *s = encode_chars[(int)v/10];
        *e = encode_chars[(int)v%10];
      }
      return 1;
    }
  }
  return 0; // no vowel just before end
}


#define VERSION_1_32// 1.32 true recursive processing
#ifdef VERSION_1_32 // 1.32 true recursive processing
static int result_override=-1;
#endif
static const char* makeiso(int cc,int longcode); // 0=short / 1=XX-YYY for states, XXX for country / 2=shortest unique

static void addresult(char *resultbuffer, char *result, long x,long y, int ccode)
{
  // add to global array (if any)
#ifdef VERSION_1_32 // 1.32 true recursive processing
  if (result_override>=0) ccode=result_override; // 1.32 true recursive processing
#endif
  if (*result && global_results && nr_global_results>=0 && nr_global_results+1<(2*MAXGLOBALRESULTS)) {
    global_results[nr_global_results++] = addstorage(result);
    global_results[nr_global_results++] = addstorage(makeiso(ccode,1));
  }
  // add to buffer (if any)
  if (resultbuffer)
  {
    if (*resultbuffer) strcat(resultbuffer," ");
    strcat(resultbuffer,result);
  }
} // addresult


// returns -1 (error), or m (also returns *result!=0 in case of success)
static int encodeNameless( char *resultbuffer, char *result, long x, long y, int input_ctry, int CODEX, int forcecoder )
{
  int A;

  *result=0;

  setup_country(input_ctry);
  A = count_city_coordinates_for_country( CODEX );

  // determine A = nr of 2.2
  if ( A>1 )
  {
    int p = 31/A;
    int r = 31%A; // the first r items are p+1
    long codexlen = (CODEX/10)+(CODEX%10);
    // determine side of square around centre
    long SIDE;
    // now determine X = index of first area
    long X=0;
    // loop through country records
    int j   = (CODEX==21 ? iso_first21 : (CODEX==22 ? iso_first22 : iso_first13));
    int e = j+(CODEX==21 ? iso_count21 : (CODEX==22 ? iso_count22 : iso_count13));
    for ( X=0; j<e; j++,X++ )
    {
      int m = (j);

      if (forcecoder<0     || forcecoder==m)
      {
        long storage_offset;
        long miny,maxy;
        long minx,maxx;

        long xSIDE,orgSIDE;


        if ( CODEX!=21 && A<=31 )
        {
          int size=p; if (X<r) size++; // example: A=7, p=4 r=3:  size(X)={5,5,5,4,4,4,4}
          storage_offset= (X*p + (X<r ? X : r)) * (961*961); // p=4,r=3: offset(X)={0,5,10,15,19,23,27}-31
        }
        else if ( CODEX!=21 && A<62 )
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
          long BASEPOWER = (CODEX==21) ? 961*961 : 961*961*31;
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



        if ( miny<=y && y<maxy && isInRange(x,minx,maxx) )
        {
          long v = storage_offset;

          long dividerx4 = x_divider(miny,maxy); // *** note: dividerx4 is 4 times too large!
#ifdef VERSION160 // precise encoding: take fraction into account!
          long xFracture = (long)(4*fraclon);
#else
          long xFracture = 0;
#endif
          long dx = (4*(x-minx)+xFracture)/dividerx4; // like div, but with floating point value
          long extrax4 = (x-minx)*4 - dx*dividerx4; // like modulus, but with floating point value

          long dividery = 90;
          long dy     = (maxy-y)/dividery;  // between 0 and SIDE-1
          long extray = (maxy-y)%dividery;

  #ifdef VERSION160 // precise encoding: check if fraction takes this out of range
          if (extray==0 && fraclat>0) {
              if (dy==0)
                continue; // fraction takes this coordinate out of range
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
            if (CODEX==13)
              dotp--;
            memmove(result+dotp,result+dotp-1,4); result[dotp-1]='.';
          }

          if ( ! isSpecialShape22(m) )
          if ( CODEX==22 && A<62 && orgSIDE==961 )
          {
            char t = result[codexlen-2]; result[codexlen-2]=result[codexlen]; result[codexlen]=t;
          }

          encodeExtension(result,extrax4,extray,dividerx4,dividery,use_high_precision,-1); // nameless

          return m;

        } // in range

      } // forcecoder?
    }
  } // there are multiple 2.2 !

  return -1;
} // encodeNameless




// returns nonzero if error
//    -182 no countryname
static int master_decode(  long *nx,long *ny, // <- store result in nx,ny
          const char *org_input,
          const char *default_cityname, int input_ctry, // <- context, in case input has no name, or no countryname
          long debug_oldx,long debug_oldy // <- optional "original coordinates"
          )
{
 const char *dot=NULL;
 int err=0;
 int m=-1; // key record for decoding

 int ilen=strlen(org_input);
 char input[MAX_CLEAN_MAPCODE_LEN+1];
 if (ilen<5 || ilen>MAX_CLEAN_MAPCODE_LEN)
   return -8;
 strcpy(input,org_input);

 {
   char *min = strchr(input,'-');
   if (min) {
     *min++=0;
     extrapostfix=&org_input[min-input];
     if (strlen(extrapostfix)>MAX_PRECISION_DIGITS)
       return -19; // extension too long
     ilen=strlen(input);
   } else extrapostfix="";
 }


 *nx=*ny=0; // reset to zero in case of error

 {
  int voweled = unpack_if_alldigits(input);
  if (voweled<0)
    return -7;

  // debug support: U-lead pre-processing
  if (*input=='u' || *input=='U')  {
    strcpy(input,input+1);
    ilen--;
    voweled=1;
  }

  if (ilen>10) return -8;

  // find dot and check that all characters are valid
  {
    int nrd=0; // nr of true digits
    const char *s=input;
    for ( ; *s!=0; s++ )
    {
      if (*s=='.')
        { if (dot) return -5; else dot=s; }
      else if ( decode_chars[(int)*s]<0 )
        return -4; // invalid char
      else if ( decode_chars[(int)*s]<10 ) // digit?
        nrd++;
    }
    if (dot==NULL)
      return -2;
    else if (!voweled && nrd+1==ilen) // there is a dot, and everything else is a digit!
      return -998;
  }

//////////// AT THIS POINT, dot=FIRST DOT, input=CLEAN INPUT (no vowels) ilen=INPUT LENGTH

  // analyse input
  {
    int prelen = (dot-input);
    int postlen = ilen-1-prelen;
    if ( prelen<2 || prelen>5 || postlen<2 || postlen>4 )
      return -3;
  }

//////////////////////////////////

    if ( ilen>=10 )
      input_ctry = ccode_earth;

    if (input_ctry<0) return input_ctry;
    setup_country(input_ctry);

    // special case: 43 or worse, for a state
    if ( (ilen==8 || ilen==9) && iso_count43==1 && isRestricted((iso_first43)) && ( mIsIndiaState((input_ctry )) || mIsMexicoState((input_ctry )) ) )
    {
      input_ctry =
          mIsMexicoState((input_ctry )) ? ccode_mex :
          mIsIndiaState((input_ctry )) ? ccode_ind :
          CCODE_ERR;
      setup_country(input_ctry);
    }
    else if ( ilen==9 && iso_count44==1 && isRestricted((iso_first44)) && isSubdivision((input_ctry ))   )
    {
      input_ctry =
        mIsUsaState((input_ctry )) ? ccode_usa :
        mIsCanadaState((input_ctry )) ? ccode_can :
        mIsAustraliaState((input_ctry )) ? ccode_aus :
        mIsBrazilState((input_ctry )) ? ccode_bra :
        mIsChinaState((input_ctry )) ? ccode_chn :
        mIsRussiaState((input_ctry )) ? ccode_rus :
        CCODE_ERR;
      setup_country(input_ctry);
    }

    if (
             ( ilen==5 && input[2]=='.' && iso_count21==0 && iso_count22==1 )
          || ( ilen==5 && input[2]=='.' && iso_count21==1 )
          || ( ilen==6 && input[2]=='.' && iso_count23==1 )
          || ( ilen==6 && input[3]=='.' && iso_count32==1 )
          || ( ilen==7 && input[2]=='.' && iso_count24==1 )
          || ( ilen==7 && input[3]=='.' && iso_count33==1 )
          || ( ilen==7 && input[4]=='.' && iso_count42==1 )
          || ( ilen==8 && input[3]=='.' && iso_count34==1 )
          || ( ilen==8 && input[4]=='.' && iso_count43==1 )
          || ( ilen==9 && input[4]=='.' && iso_count44==1 )
       )
      {
          long minx,miny,maxx,maxy;
          if ( ilen==5 && iso_count21==1) m = (iso_first21);
          if ( ilen==5 && iso_count21==0) m = (iso_first22);
          if ( ilen==6 && input[2]=='.' ) m = (iso_first23);
          if ( ilen==6 && input[3]=='.' ) m = (iso_first32);
          if ( ilen==7 && input[2]=='.' ) m = (iso_first24);
          if ( ilen==7 && input[3]=='.' ) m = (iso_first33);
          if ( ilen==7 && input[4]=='.' ) m = (iso_first42);
          if ( ilen==8 && input[3]=='.' ) m = (iso_first34);
          if ( ilen==8 && input[4]=='.' ) m = (iso_first43);
          if ( ilen==9 ) m = (iso_first44);

          getboundaries(m,minx,miny,maxx,maxy);
          decodeGrid( input,nx,ny, m, minx,miny,maxx,maxy);

          //
          if (isRestricted(m)) {
            int j,fitssomewhere=0;
            for (j=iso_start; (j)<m; j++) { // look in previous rects
              long minx,miny,maxx,maxy,xdiv8;
              if (isRestricted((j))) continue;
              getboundaries((j),minx,miny,maxx,maxy);
              // 1.33 fix to not remove valid results just across the edge of a territory
              xdiv8 = x_divider(miny,maxy)/4; // should be /8 but there's some extra margin
              if ( miny-60<=*ny && *ny<maxy+60 && isInRange(*nx,minx-xdiv8,maxx+xdiv8) ) { fitssomewhere=1; break; }
            }
            if (!fitssomewhere) {
              err=-1234;
            }
          }


      }
      else if ( ilen==6 && input[3]=='.' && iso_count22>1 ) // multiple 22 into 3.2
      {
        m   = decodeNameless( input, nx,ny, input_ctry, 22, debug_oldx,debug_oldy);
        if (m<0) err=m; else err=0;
      }
      else if ( ilen==6 && input[2]=='.' && iso_count13>1 ) // multiple 13 into 2.3
      {
        m   = decodeNameless( input, nx,ny, input_ctry, 13, debug_oldx,debug_oldy);
        if (m<0) err=m; else err=0;
      }
      else if ( ilen==5 ) // multiple 21 into 2.2
      {
        if (iso_count21<=0)
          err=-191;
        else if ( input[2]!='.' )
          err=-9;
        else {
          m   = decodeNameless( input, nx,ny, input_ctry, 21, debug_oldx,debug_oldy);
          if (m<0) err=m; else err=0;
        }
      }
      else if (
                 ( ilen==7 && iso_firststar23>=0 && input[3]=='.')
              || ( ilen==6 && iso_firststar22>=0 && input[2]=='.')
        )
      {
        // decodeAutoHeader
        int start = (ilen==6 ? iso_firststar22 : iso_firststar23);

        long STORAGE_START=0;
        long value;
        int j;

        value = decodeBase31(input); // decode top
        value *= (961*31);

        err=-1;
        for ( j=start; coDex(j)==coDex(start) && recType(j)>1; j++ )
        {
          long minx,miny,maxx,maxy;


          m=(j);
          getboundaries(m,minx,miny,maxx,maxy);

          {
            // determine how many cells
            long H = (maxy-miny+89)/90; // multiple of 10m
            long xdiv = x_divider(miny,maxy);
            long W = ( (maxx-minx)*4 + (xdiv-1) ) / xdiv;
            long product = W*H;

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

              err=0; // decoding headerLetter!

              // PIPELETTER DECODE
              {
                long difx,dify;
                decode_triple(input+ilen-3,&difx,&dify); // decode bottom 3 chars
                {
                  long vx = (value / (H/YSIDE3))*XSIDE3 + difx; // is vx/168
                  long vy = (value % (H/YSIDE3))*YSIDE3 + dify; // is vy/176
                  *ny = maxy - vy*dividery;
                  *nx = minx + vx*dividerx;

                  if ( *nx<minx || *nx>=maxx || *ny<miny || *ny>maxy ) // *** CAREFUL! do this test BEFORE adding remainder...
                  {
                    err = -122; // invalid code
                  }

                  decodeExtension(nx,ny, dividerx<<2,dividery,-1); // autoheader decode

                }
              }

              break;
            }
            STORAGE_START += product;
          }
        } // for j
      }
// ************** PIPELETTER 32 33 42 34 43 54=world
      else if (
                   ( ilen== 6 && iso_firstpipe22>=0 && iso_firststar22<0 && input[3]=='.' ) // lettered 22-pipes -> 32
                || ( ilen== 7 && iso_firstpipe23>=0 && iso_firststar23<0 && input[3]=='.' ) // lettered 23-pipes -> 33
                || ( ilen== 7 && iso_firstpipe32>=0 && iso_firststar32<0 && input[4]=='.' ) // lettered 32-pipes -> 42
                || ( ilen== 7 && iso_firstpipe14>=0 && iso_firststar14<0 && input[2]=='.' ) // lettered 14-pipes -> 24
                || ( ilen== 8 && iso_firstpipe24>=0 && iso_firststar24<0 && input[3]=='.' ) // lettered 24-pipes -> 34
                || ( ilen== 8 && iso_firstpipe33>=0 && iso_firststar33<0 && input[4]=='.' ) // lettered 33-pipes -> 43
                || ( ilen== 9 && iso_firstpipe34>=0 && iso_firststar34<0 && input[4]=='.' ) // lettered 34-pipes -> 44
                || ( ilen==10 && iso_firstpipe44>=0 && iso_firststar44<0 && input[5]=='.' ) // lettered 44-pipes -> 54
              )
      {
        const char *pipeptr;
        char letter=toupper(*input);
        if (letter=='I') letter='1';
        if (letter=='O') letter='0';
        switch (ilen)
        {
          case  6: pipeptr = strchr(iso_headerletter22,letter); if (pipeptr) m=(iso_firstpipe22+(pipeptr-iso_headerletter22)); break;
          case  7: if (input[3]=='.') {
                   pipeptr = strchr(iso_headerletter23,letter); if (pipeptr) m=(iso_firstpipe23+(pipeptr-iso_headerletter23)); break;
          }else if (input[2]=='.') {
                   pipeptr = strchr(iso_headerletter14,letter); if (pipeptr) m=(iso_firstpipe14+(pipeptr-iso_headerletter14)); break;
          }else{
                   pipeptr = strchr(iso_headerletter32,letter); if (pipeptr) m=(iso_firstpipe32+(pipeptr-iso_headerletter32)); break;
          }
          case  8: if (input[3]=='.') {
                   pipeptr = strchr(iso_headerletter24,letter); if (pipeptr) m=(iso_firstpipe24+(pipeptr-iso_headerletter24)); break;
          }else{
                   pipeptr = strchr(iso_headerletter33,letter); if (pipeptr) m=(iso_firstpipe33+(pipeptr-iso_headerletter33)); break;
          }
          case  9: pipeptr = strchr(iso_headerletter34,letter); if (pipeptr) m=(iso_firstpipe34+(pipeptr-iso_headerletter34)); break;
          case 10: pipeptr = strchr(iso_headerletter44,letter); if (pipeptr) m=(iso_firstpipe44+(pipeptr-iso_headerletter44)); break;
        }

        if (!ISGOODAREA(m))
        {
          err = -98;
        }
        else
        {
          long minx,miny,maxx,maxy;
          ilen--;
          getboundaries(m,minx,miny,maxx,maxy);
          decodeGrid( input+1,nx,ny, m, minx,miny,maxx,maxy);
          err=0;
        }
      }
// ************** fail!
      else // THIS MEANS EVERYTHING HAS FAILED!!!
      {
        err=-99;
      }
 }

// *********************** done, return results...


  if (err==0)
  {
    // * make sure it fits the country *
    if (!ISEARTH(input_ctry))
    {
      long minx,miny,maxx,maxy,xdiv8;
      getboundaries((iso_end),minx,miny,maxx,maxy);
      xdiv8 = x_divider(miny,maxy)/4; // should be /8 but there's some extra margin
      if ( ! ( miny-60<=*ny && *ny<maxy+60 && isInRange(*nx,minx-xdiv8,maxx+xdiv8) ) ) // no fit?
      {
        err=-2222;
      }
    }
  }


  return err;
} // master_decode




// make sure resultbuffer is large enough!
// x,y = lon/lat times 1.000.000
static int debugStopAt=-1;
static void master_encode( char *resultbuffer, int the_ctry, long x, long y, int forcecoder, int stop_with_one_result, int allow_world, int skip_parentstate )
{
  int result_ctry=the_ctry;
  int i;
  if (resultbuffer) *resultbuffer=0;

  if (the_ctry<0) return;

  // V160 - cut y to [-90,90], and normalise all x to [-180,180>
  if (y > 90000000) y = 90000000; else if (y< -90000000) y =-90000000;
  x %= 360000000;
  if (x>=180000000) x-=360000000; else if (x<-180000000) x+=360000000;

  ///////////////////////////////////////////////////////////
  // turn into 3-letter ISO code!
  ///////////////////////////////////////////////////////////
  setup_country( the_ctry );
  i=iso_start;


  if (!ISEARTH(the_ctry))
  {
    long minx,miny,maxx,maxy;
    getboundaries((iso_end),minx,miny,maxx,maxy);
    if ( ! (miny<=y && y<maxy && isInRange(x,minx,maxx) ) )
    {
      i=iso_end+1; // skip to "Earth" (if supported)
      if (!allow_world /*&& skip_parentstate*/) return;
    }
  }


  ///////////////////////////////////////////////////////////
  // look for encoding options
  ///////////////////////////////////////////////////////////
  {
    char result[128];
    long minx,miny,maxx,maxy;
    long STORAGE_START=0;
    int forcecoder_starpipe = (forcecoder>=0    && recType(forcecoder)>1 );
    int result_counter=0;
    *result=0;
    for( ; i<NR_BOUNDARY_RECS; i++ )
    {
      int m=(i);
      int codex;

      if ( i==iso_end && isRestricted(m) && isSubdivision(the_ctry) ) // last item is a reference to a state's country
      {
        if (skip_parentstate)
          return;
        if (forcecoder>=0   ) // we would have found it the normal way!
          return;

#ifdef VERSION_1_32 // 1.32 true recursive processing
        result_override=the_ctry;  // 1.32 true recursive processing
#endif

        the_ctry =
          mIsUsaState(the_ctry) ? ccode_usa :
          mIsCanadaState(the_ctry) ? ccode_can :
          mIsIndiaState(the_ctry) ? ccode_ind :
          mIsMexicoState(the_ctry) ? ccode_mex :
          mIsBrazilState(the_ctry) ? ccode_bra :
          mIsChinaState(the_ctry) ? ccode_chn :
          mIsRussiaState(the_ctry) ? ccode_rus :
          ccode_aus;


#ifdef VERSION_1_32 // 1.32 true recursive processing
        master_encode( resultbuffer, the_ctry, x,y, forcecoder, stop_with_one_result,/*allow-world*/0,1 );
        result_override=-1;
        return; /**/
#else
        setup_country( the_ctry );
        i=iso_start-1; continue;
#endif
      }

      if ( i>iso_end )
      {
        if (forcecoder>=0    || !allow_world )
          break;
        // find first WORLD record...
        result_ctry=the_ctry=ccode_earth;
        setup_country(the_ctry);
        // now process from here...
        i=iso_start-1; continue;
      }


      if ( isRestricted(m) && result_counter==0 && forcecoder<0 ) // skip isRestricted records unless there already is a result
        continue;


      if ( forcecoder>=0    && m!=forcecoder && (forcecoder_starpipe==0 || recType(m)<=1)                ) continue; // not the right forcecoder (unless starpipe)

      codex = coDex(m);
      if ( codex==54 ) continue; // exlude antarctica "inner coding" options


      getboundaries(m,minx,miny,maxx,maxy);

      if ( isNameless(m) )
      {
        if ( y<miny || y>=maxy || !isInRange(x,minx,maxx) ) continue;
        {
          int ret=encodeNameless( resultbuffer, result, x,y, the_ctry, codex,  m );
          if (ret>=0)
          {
            i=ret;
          }
        }
      }
      else if ( recType(m)>1 )
      {
        // encodeAutoHeader
        if ( i<=iso_start || coDex(i-1)!=codex || recType(i-1)<=1 ) // is this the FIRST starpipe?
        {
          STORAGE_START=0;
        }


          {
            // determine how many cells
            long H = (maxy-miny+89)/90; // multiple of 10m
            long xdiv = x_divider(miny,maxy);
            long W = ( (maxx-minx)*4 + (xdiv-1) ) / xdiv;
            long product = W*H;

            // encodee
            // round up to multiples of YSIDE3*XSIDE3...
            H = YSIDE3*( (H+YSIDE3-1)/YSIDE3 );
            W = XSIDE3*( (W+XSIDE3-1)/XSIDE3 );
            product = (W/XSIDE3)*(H/YSIDE3)*961*31;
            {
            long GOODROUNDER = codex>=23 ? (961*961*31) : (961*961);
            if ( recType(m)==2 ) // plus pipe
              product = ((STORAGE_START+product+GOODROUNDER-1)/GOODROUNDER)*GOODROUNDER - STORAGE_START;
            }


          if ( ( !ISGOODAREA(forcecoder) || forcecoder==m ) && ( miny<=y && y<maxy && isInRange(x,minx,maxx) ) )
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

    #ifdef VERSION160 // precise encoding: check if fraction takes this out of range
            if ( extray==0 && fraclat>0 ) {
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

            encodeExtension( result,extrax<<2,extray,dividerx<<2,dividery,use_high_precision,-1); // autoheader

            {
              #ifdef SUPPRESS_MULTIPLE_PIPES
              while ( i<iso_end && coDex(i+1)==codex )
                i++;
              #endif

            }
          }

          STORAGE_START += product;
        }
      }
      else // must be grid, possibly a multi-area
      {
          int multi_area = (recType(m)==1 ? 1 : 0); // has pipe letter?
          if (codex==21) { if (iso_count21==1) codex=22; else continue; }

          if ( miny<=y && y<maxy && isInRange(x,minx,maxx) )
          if ( codex!=55 ) // exclude Earth 55
          {
            encodeGrid( result+multi_area, x,y, m, codex, minx,miny,maxx,maxy );
            if (multi_area && result[multi_area])
            {
              *result = headerLetter(m);
            }
          }
      } // codex>21 grid, may be multi-area

      // =========== handle result if any
      if (*result) // GOT A RESULT?
      {
        result_counter++;

        repack_if_alldigits(result,0);

        if (debugStopAt<0 || debugStopAt==i)
          addresult(resultbuffer,result,x,y,result_ctry); // 1.29 - also add parents recursively
        if (debugStopAt==i) return;
        if (stop_with_one_result) return;
        *result=0;
      }
    } // for i
  }
} // master_encode






#ifndef REMOVE_VALIDCODE
static int valid_code( const char *input ) // returns < 0 if invalid, 0 if potentially valid, 1 if potentially a complete code (i.e. from 2.2 to 5.4, using valid chars)
{
  const unsigned char *i = (const unsigned char *)input;
  int dots=0,prefix=0,postfix=0,alldigits=1,voweled=0;
  for ( ; *i!=0; i++ )
  {
    if ( *i=='.' )
    {
      if ( dots++ )
        return -1; // more than one dot!
      if (prefix<2)
        return -2; // prefix less than 2 characters!
    }
    else
    {
      if ( decode_chars[*i]<0 )
      {

        if ( decode_chars[*i]<-1 ) // i.e. A, E or U
        {
          if ( (*i=='A' || *i=='a') && i==(const unsigned char*)input ) // A is the first letter
          {
            voweled=1;
            prefix--;  // do not count as prefix!
          }
          else if (alldigits && !voweled && i[1]=='.') // vowel after digits just before dot
          {
            voweled=1;
          }
          else if (alldigits && voweled && i[1]==0 && i[-1]!='.') // last char of voweled entry, not right after dot?
            ;
          else
            return -6; // invalid char: vowel
        }
        else

        {
          return -3; // other invalid character
        }
      }
      else // consonant or digit
      {
        if (*i<'0' || *i>'9') // nondigit
        {
          alldigits=0;
          if (voweled && i[1]!=0) return -7; // nondigit in voweled entry, not in the last position: invalid
        }
      }
      if (dots)
      {
        postfix++;
        if (postfix>4) return -4; // more than 4 letters postfix
      }
      else
      {
        prefix++;
        if (prefix>5) return -5; // more than 5 letters prefix
      }
    }
  }

  // return if it looks like a "complete" code (i.e. one that can be decoded
  if ( postfix<2 ) return -8;
  if (alldigits && !voweled) return -11;
  // note: at this point, prefix is between 2 and 5, and postfix is between 2 and 4 (12 options)
  if ( postfix==4 )
  {
    return 1; // the 4 options 2.4 3.4 4.4 5.4 are all possible!
  }
  // note: at this point, prefix is between 2 and 5, and postfix is between 2 and 3
  if ( prefix==5 ) return -9; // the 2 options 5.2 5.3 are impossible!
  // note: at this point, prefix is between 2 and 4, and postfix is between 2 and 3
  return 1; // the other 6 options (2.2 3.2 4.2 2.3 3.3 4.3) are all possible
}
#endif // REMOVE_VALIDCODE


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

static int stateparent(int ccode) // returns parent, or -1
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

static char makeiso_bufbytes[16];
static char *makeiso_buf;
static const char* makeiso(int cc,int longcode) // 0=short / 1=XX-YYY for states, XXX for country / 2=shortest unique
{
  if (cc<0 || cc>=MAX_MAPCODE_TERRITORY_CODE) return ""; else
  {
    int p=stateletter(cc);
    const char* ei=entity_iso3(cc);
    if (*ei>='0' && *ei<='9') ei++;
    if (makeiso_buf==makeiso_bufbytes) makeiso_buf=makeiso_bufbytes+8; else makeiso_buf=makeiso_bufbytes;
    if (longcode && p)
    {
      memcpy(makeiso_buf,&parents2[p*3-3],2);
      makeiso_buf[2]='-';
      strcpy(makeiso_buf+3,ei);
    }
    else
    {
      strcpy(makeiso_buf,ei);
    }
    return makeiso_buf;
  }
}



static int resultlen(const char *result)
{
  const char* hpos = strchr(result,'-');
  if (hpos)
    return hpos-result;
  return strlen(result);
}

// TODO: Unused code - remove or place in mapcoder.h.
// returns empty string if error (e.g. bad iso);
// returns nonzero if a code was generated
static int mapcode_encode(char *result,long y, long x, const char *iso3, char *isofound)
{
  int cc=ccode_of_iso3(iso3);
  master_encode(result,cc,x,y,-1,1,1,0);
  if (isofound)
  {
    *isofound=0;
    if (*result) {
      if (resultlen(result)==10)
        strcpy(isofound,makeiso(ccode_earth,1));
      else
        strcpy(isofound,makeiso(cc,1));
    }
  }
  return *result;
}


// returns nonzero if error
// if ccode_found!=NULL
static int full_mapcode_decode(long *y, long *x, const char *iso3, const char *orginput,int *ccode_found,char *clean_input)
{
  char input[MAX_MAPCODE_RESULT_LEN];
  int err,ccode,len;
  char *s=(char*)orginput;
  int ilen=strlen(iso3);

  if (iso3) disambiguate_str(iso3,strlen(iso3));

  if (clean_input) *clean_input=0;

  while (*s>0 && *s<=32) s++; // skip lead
  len=strlen(s);

  if (len>MAX_MAPCODE_RESULT_LEN-1) len=MAX_MAPCODE_RESULT_LEN-1;
  memcpy(input,s,len); s=input; s[len]=0;


 {
   char *min = strchr(s+4,'-');
   if (min) {
     *min++=0;
     extrapostfix=&orginput[min-s];
     len=strlen(s);
   } else extrapostfix="";
 }

  // remove trail
  while (len>0 && s[len-1]>=0 && s[len-1]<=32) { s[--len]=0; }

  if (len>0 && len<10 && ilen<7 && strchr(s,' ')==0) // no space in input (so just a mapcode?); and room for ISO-space-MAPCODE?
  {
      memmove(s+ilen+1,s,len+1);
      s[ilen]=' ';
      memcpy(s,iso3,ilen);
      len+=(1+ilen);
      iso3="";
  }


  if (len>8 && (s[3]==' ' || s[3]=='-') && (s[6]==' ' || s[7]==' ')) // assume ISO3 space|minus ISO23 space MAPCODE
  {
    int p=disambiguate_str(s,3);
    if (p<0) return p;
    s+=4; len-=4;
  }
  else if (len>7 && (s[2]==' ' || s[2]=='-') && (s[5]==' ' || s[6]==' ')) // assume ISO2 space|minus ISO23 space MAPCODE
  {
    int p=disambiguate_str(s,2);
    if (p<0) return p;
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
  ccode = ccode_of_iso3(iso3);
  if (ccode==ccode_mex && len<8) ccode=ccode_of_iso3("5MX"); // special case for mexico country vs state

  if (ccode_found) *ccode_found=ccode;

  {
    char *t;
    for(t=s;*t!=0;t++)
    {
      if (*t>='a' && *t<='z') *t += ('A'-'a');
      if (*t=='O') *t='0';
      if (*t=='I') *t='1';
    }
  }

  if (*extrapostfix) {
    strcat(s,"-");
    strcat(s,extrapostfix);
    err = master_decode(  x,y,s,"",ccode,0,0);
  }
  else {
    err = master_decode(  x,y,s,"",ccode,0,0);
  }

  if (clean_input && len<=10)
  {
    strcpy(clean_input,s);
    if (*extrapostfix)
    {
      strcat(clean_input,"-");
      strcat(clean_input,extrapostfix);
    }
    makeup(clean_input);
  }

  if (err)
  {
    #ifdef REMOVE_VALIDCODE
      *x=*y=0;
    #else
      if (ccode<0 || valid_code(s)<0 ) // bad iso or bad input
      {
        *x=*y=0;
      }
      else if (err && ccode>=0)
      {
        long miny,maxy,minx,maxx;
        setup_country(ccode);
        getboundaries((data_start[ccode]),minx,miny,maxx,maxy);
        *x = (minx+maxx)/2;
        *y = (miny+maxy)/2;
      }
    #endif // REMOVE_VALIDCODE
  }

  // normalise between =180 and 180
  if ( *x>180000000 ) *x-=360000000; else if ( *x<-180000000 ) *x+=360000000;

  return err;
}

// TODO: Unused code - remove or place in mapcoder.h.
static int mapcode_decode(long *y, long *x, const char *iso3, const char *orginput,int *ccode_found)
{
  return full_mapcode_decode(y,x,iso3,orginput,ccode_found,NULL);
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





static char asciibuf[16];
static const char *decode_utf16(const UWORD* s)
{
  char *w = asciibuf;
  const char *e = w+16-1;
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


static UWORD unibuf[16];
static const UWORD* encode_utf16(const char *mapcode,int language) // convert mapcode to language (0=roman 1=greek 2=cyrillic 3=hebrew)
{
  UWORD *w = unibuf;
  const UWORD *e = w+16-1;
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









#define TOKENSEP  0
#define TOKENDOT  1
#define TOKENCHR  2
#define TOKENZERO 3
#define TOKENHYPH 4
#define ERR -1
#define Prt -9 // partial
#define GO  99

  static signed char fullmc_statemachine[23][5] = {
                      // WHI DOT DET ZER HYP
  /* 0 start        */ {  0 ,ERR, 1 ,ERR,ERR }, // looking for very first detter
  /* 1 gotL         */ { ERR,ERR, 2 ,ERR,ERR }, // got one detter, MUST get another one
  /* 2 gotLL        */ { 18 , 6 , 3 ,ERR,14  }, // GOT2: white: got territory + start prefix | dot: 2.X mapcode | det:3letter | hyphen: 2-state
  /* 3 gotLLL       */ { 18 , 6 , 4 ,ERR,14  }, // white: got territory + start prefix | dot: 3.X mapcode | det:4letterprefix | hyphen: 3-state
  /* 4 gotprefix4   */ { ERR, 6 , 5 ,ERR,ERR }, // dot: 4.X mapcode | det: got 5th prefix letter
  /* 5 gotprefix5   */ { ERR, 6 ,ERR,ERR,ERR }, // got 5char so MUST get dot!
  /* 6 prefix.      */ { ERR,ERR, 7 ,Prt,ERR }, // MUST get first letter after dot
  /* 7 prefix.L     */ { ERR,ERR, 8 ,Prt,ERR }, // MUST get second letter after dot
  /* 8 prefix.LL    */ { 22 ,ERR, 9 , GO,11  }, // get 3d letter after dot | X.2- | X.2 done!
  /* 9 prefix.LLL   */ { 22 ,ERR,10 , GO,11  }, // get 4th letter after dot | X.3- | X.3 done!
  /*10 prefix.LLLL  */ { 22 ,ERR,ERR, GO,11  }, // X.4- | x.4 done!

  /*11 mc-          */ { ERR,ERR,12 ,Prt,ERR }, // MUST get first precision letter
  /*12 mc-L         */ { 22 ,ERR,13 , GO,ERR }, // Get 2nd precision letter | done X.Y-1
  /*13 mc-LL        */ { 22 ,ERR,ERR, GO,ERR }, // done X.Y-2 (*or skip whitespace*)

  /*14 ctry-        */ { ERR,ERR,15 ,ERR,ERR }, // MUST get first state letter
  /*15 ctry-L       */ { ERR,ERR,16 ,ERR,ERR }, // MUST get 2nd state letter
  /*16 ctry-LL      */ { 18 ,ERR,17 ,ERR,ERR }, // white: got CCC-SS and get prefix | got 3d letter
  /*17 ctry-LLL     */ { 18 ,ERR,ERR,ERR,ERR }, // got CCC-SSS so MUST get whitespace and then get prefix

  /*18 startprefix  */ { 18 ,ERR,19 ,ERR,ERR }, // skip more whitespace, MUST get 1st prefix letter
  /*19 gotprefix1   */ { ERR,ERR,20 ,ERR,ERR }, // MUST get second prefix letter
  /*20 gotprefix2   */ { ERR, 6 ,21 ,ERR,ERR }, // dot: 2.X mapcode | det: 3d perfix letter
  /*21 gotprefix3   */ { ERR, 6 , 4 ,ERR,ERR }, // dot: 3.x mapcode | det: got 4th prefix letter

  /*22 whitespace   */ { 22 ,ERR,ERR, GO,ERR }  // whitespace until end of string

  };

  // pass fullcode=1 to recognise territory and mapcode, pass fullcode=0 to only recognise proper mapcode (without optional territory)
  // returns 0 if ok, negative in case of error (where -999 represents "may BECOME a valid mapcode if more characters are added)
  int compareWithMapcodeFormat(const char *s,int fullcode)
  {
    int nondigits=0;
    int state=(fullcode ? 0 : 18); // initial state
    for(;;s++) {
      int newstate,token;
      // recognise token
      if (*s>='0' && *s<='9') token=TOKENCHR;
      else if ((*s>='a' && *s<='z') || (*s>='A' && *s<='Z'))
        { token=TOKENCHR; if (state!=11 && state!=12) nondigits++; }
      else if (*s=='.' ) token=TOKENDOT;
      else if (*s=='-' ) token=TOKENHYPH;
      else if (*s== 0  ) token=TOKENZERO;
      else if (*s==' ' || *s=='\t') token=TOKENSEP;
      else return -4; // invalid character
      newstate = fullmc_statemachine[state][token];
      if (newstate==ERR)
        return -(1000+10*state+token);
      if (newstate==GO )
        return (nondigits>0 ? 0 : -5);
      if (newstate==Prt)
        return -999;
      state=newstate;
      if (state==18)
        nondigits=0;
    }
  }

// TODO: Unused code - remove or place in mapcoder.h.
static int lookslike_mapcode(const char *s)  // return -999 if partial, 0 if ok, negative if not
{
  // old-style recognizer, does not suport territory context
  return compareWithMapcodeFormat(s,0);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Freebee: interpret coordinates, separated by either space or comma (if comma, whitespace allowed everywhere else)
//   [signletters*] v1 [degreesymbol [v2 [minutesym [v3 [secondsym]]]]] [letter]
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static unsigned char *addnum(unsigned char *r,long v,int fill2,unsigned char unit)
{
  if (v>=100) { *r++=(unsigned char)('0'+(v/100)); v%=100; }
  if (fill2 || v>=10) { *r++=(unsigned char)('0'+(v/10)); v%=10; }
  *r++=(unsigned char)('0'+v);
  if (unit) *r++=unit;
  *r=0;
  return r;
}

static void asdms1(unsigned char *r,double v,long max,unsigned char degsym) // worst case is -###d##'##", 11 char ex zts (or 10 if max=90)
{
  long d,m,s;
  if (v<0) { *r++='-'; v=-v; }
  s = (long)(3600*v+0.5); // round to seconds
  if (s>max) s=max;
  d=s/3600;
  r=addnum(r,d,0,degsym);
  s%=3600;
  if (s)
  {
    m=s/60;
    r=addnum(r,m,1,'\'');
    s%=60;
    if (s)
      r=addnum(r,s,1,'\"');
  }
}

// TODO: Unused code - remove or place in mapcoder.h.
static unsigned char *asdms(unsigned char *r,double lat,double lon,unsigned char degsym) // worst case EXcluding zts is 22 bytes: -##d##'##",-###d##'##"
{
  asdms1(r,lat,90*3600,degsym);
  strcat((char*)r,",");
  asdms1(r+strlen((char*)r),lon,180*3600,degsym);
  return r;
}

static int interpret_coord( const unsigned char *i, int islat, double *result )
{
  int expnow=0; // expect now: 0=deg 1=min 2=sec 3=none
  double d=0,m=0,s=0;

  #define isdig(c) ((c)>='0' && (c)<='9')
  #define iswhite(c) ((c)>0 && (c)<=32)
  #define skipwhite(i) {while (iswhite(*i)) i++;}
  #define skipnum(i) {while (isdig(*i)) i++;}
  #define skipfp(i) {skipnum(i); if (*i=='.') {i++;skipnum(i);}}

  const char *winds = islat ? "nsNS+- " : "ewEW+- ";

  // skip white spaces, signs and appropriate wind direction letters
  char *p;
  int sign=1;

  while (*i && (p=(char*)strchr(winds,*i))!=NULL) { { if ( (p-(char*)winds) & 1 ) sign*=-1; } i++; }

  // we are now at a lead digit, or there is an error
  if (!isdig(*i))
    return -12; // digit expected

  while ( expnow<4 && isdig(*i) )
  {
    // get value
    double v=atof((char*)i); skipfp(i); skipwhite(i);
    if ( *i && strchr("oOdD\x0F8\x0BA\x0B0\x0A7",*i) ) // degree symbol? $F8=248degree $BA=186 $B0=176 $A7=167
    {
      i++; skipwhite(i);
      d=v; expnow=1; // after degree, expect minutes
    }
    else if ( *i && strchr("\'`mM\x0B4\x093",*i) && i[1]!=i[0] ) // minute symbol (NOT repeated, else assume seconds symbol)  $B4=180 $93=147
    {
      i++; skipwhite(i);
      m=v; expnow=2; // after minutes, expect seconds
    }
    else if ( *i && strchr("\"\'`\x094\x084",*i) ) // seconds symbol? $94=148 $84=132
    {
      if (i[0]==i[1]) i++; i++; skipwhite(i);
      s=v; expnow=3; // after seconds, expect something explicitly specified
    }
    else // anything else? assume that we got what we expected
    {
      if (expnow==0) d=v; else if (expnow==1) m=v; else if (expnow==2) s=v; else return -18; // -18 = trailing unitless number
      expnow=4;
    }
  }

  // allow all posisble final endsigns
  { while (*i && (p=(char*)strchr(winds,*i))!=NULL) { if ( (p-(char*)winds) & 1 ) sign*=-1; i++; } }

  // we now MUST be at the end of the string!
  if (*i)
    return -17; // trash trailing

  *result = (d+m/60.0+s/3600.0) * sign;
  if ( islat && (*result< -90 || *result> 90)) return -13;
  if (!islat && (*result<-180 || *result>180)) return -14;

  return 0;
}

// TODO: Unused code - remove or place in mapcoder.h.
static int interpret_coords( unsigned char *i, double *x, double *y  ) // returns negative if impossible
{
  unsigned char *sep=NULL;
  unsigned char *altsep=NULL;
  unsigned char *s;
  unsigned char *firstcompass=NULL;
  skipwhite(i); // skip leading nonsense
  for (s=i;*s!=0;s++)
  {
    switch (*s)
    {
    case 'N':case 'S':case 'W':case 'E':
    case 'n':case 's':case 'w':case 'e':
      if (firstcompass==NULL) firstcompass=s; // determine first compass letter before comma (if any)
      break;
    case '+':case '-':
    case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':
    case '\"': case 148: case 132: // second CP1252
    case 'm': case 'M': case '\'': case '`': case 180: case 147: // minute CP1252
    case 'o': case 'O': case 'd': case 'D': case 248: case 186: case 176: case 167: // 167:cp850
    case '.':
      break;
    case ' ':
      if (altsep) altsep=i; else altsep=s;
      while (*s==' ') s++;
      s--;
      break;
    case ',': case 130:
      if (sep) return -1; // more than one comma!
      sep=s;
      if (firstcompass==NULL) firstcompass=s;
      break;
    default:
      return -2; // invalid char
    }
  }
  if (sep==NULL && altsep!=NULL) sep=altsep; // use whitespace as sep if no comma
  if (sep==NULL) return -3; // no separator
  if (sep==i) return -4; // separator as lead

  {
    int e;

    if (firstcompass && *firstcompass && strchr("EWew",*firstcompass))
    {
      unsigned char t=*sep; *sep=0;
      e=interpret_coord(i,0,x);
      *sep=t;
      if (!e) e=interpret_coord(sep+1,1,y);
    }
    else
    {
      unsigned char t=*sep; *sep=0;
      e=interpret_coord(i,1,y);
      *sep=t;
      if (!e) e=interpret_coord(sep+1,0,x);
    }

    if (e) return e;
  }

  return 0; // POSSIBLE
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Wrapper for LBS team
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


const char *convertTerritoryCodeToIsoName(int tc,int format) // formats: 0=full 1=short (returns empty string in case of error)
{
  return makeiso(tc-1,(format==0));
}

int getParentCountryOfState(int tc) // returns negative if tc is not a code that has a parent country
{
    int parentccode=stateparent(tc-1); // returns parent ccode or -1
    if (parentccode>=0) return parentccode+1;
    return -1;
}

int getCountryOrParentCountry(int tc) // returns tc if tc is a country, parent country if tc is a state, -1 if tc is invalid
{
  if (tc>0 && tc<MAX_MAPCODE_TERRITORY_CODE)
  {
    int tp=getParentCountryOfState(tc);
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
    ccode=ccode_of_iso3(string); // ignore optional_tc
  }
  else // there is a ccode, there is no hyphen in the string, and the string is as most 3 chars
  {
    char tmp[12];
    int tc = getCountryOrParentCountry(optional_tc);

    strcpy(tmp,convertTerritoryCodeToIsoName(tc,1)); // short parent country code
    strcat(tmp,"-");
    strcat(tmp,string);
    ccode=ccode_of_iso3(tmp);
  }
  if (ccode<0) return -1; else return ccode+1;
}

// pass point to an array of pointers (at least 64), will be made to point to result strings...
// returns nr of results;
int encodeLatLonToMapcodes_full( char **v, double lat, double lon, int tc, int stop_with_one_result, int extraDigits ) // 1.31 allow to stop after one result
{
  long lat32,lon32;
  int ccode=tc-1;
  if (tc==0) ccode=0;
  if (ccode<0 || ccode>=MAX_MAPCODE_TERRITORY_CODE) return 0;
  if (lat<-90 || lat>90 || lon<-180 || lon>180) return 0;
  if (lon<-180) lon+=360;
  if (lon>=180) lon-=360;
  if (extraDigits<0) extraDigits=0; else if (extraDigits>MAXPRECISIONDIGITS) extraDigits=MAXPRECISIONDIGITS; use_high_precision=extraDigits;
#ifndef VERSION160
  lat*=1000000; if (lat<0) lat-=0.5; else lat+=0.5;
  lon*=1000000; if (lon<0) lon-=0.5; else lon+=0.5;
  lat32=(long)lat;
  lon32=(long)lon;
#else // precise encoding: do NOT round, instead remember the fraction...
  lat+=180;
  lon+=360;
  lat*=1000000;
  lon*=1000000;
  lat32=(long)lat;
  lon32=(long)lon;
  fraclat=lat-lat32;
  fraclon=lon-lon32;
  // for 8-digit precision, cells are divided into 810,000 by 810,000 minicells.
  fraclat *= 810000; if (fraclat<1) fraclat=0; else { if (fraclat>809999) { fraclat=0; lat32++; } else fraclat /= 810000; }
  fraclon *= 810000; if (fraclon<1) fraclon=0; else { if (fraclon>809999) { fraclon=0; lon32++; } else fraclon /= 810000; }

  /*
  {
    long fraclonTOP,fraclatTOP,BOTTOM;
         if ( extraDigits==0 ) { BOTTOM=     1;}
    else if ( extraDigits<=2 ) { BOTTOM=    30;}
    else if ( extraDigits<=4 ) { BOTTOM=   900;}
    else if ( extraDigits<=6 ) { BOTTOM= 27000;}
    else if ( extraDigits<=8 ) { BOTTOM=810000;}

    fraclatTOP = (long)( fraclat * BOTTOM + 0.4 );
    fraclonTOP = (long)( fraclon * BOTTOM + 0.4 );
    if (fraclatTOP==BOTTOM) { fraclatTOP=0; lat32++; }
    if (fraclatTOP<=0) fraclat=0;
    if (fraclonTOP==BOTTOM) { fraclonTOP=0; lon32++; }
    if (fraclonTOP<=0) fraclon=0;
  }
  */

  lat32-=180000000;
  lon32-=360000000;
#endif
  nr_global_results=0;
  global_results=v;
  if (ccode==0)
  {
    int i;
    for(i=0;i<MAX_MAPCODE_TERRITORY_CODE;i++) {
      master_encode(NULL,i,lon32,lat32,-1,stop_with_one_result,0,0); // 1.29 - also add parents recursively
      if ((stop_with_one_result||debugStopAt) && nr_global_results>0) break;
    }
  }
  else
  {
    master_encode(NULL,ccode,lon32,lat32,-1,stop_with_one_result,0,0); // 1.29 - also add parents recursively
  }
  global_results=NULL;
  return nr_global_results/2;
}

// decode string into nx,ny; returns negative in case of error
int decodeMapcodeToLatLon( double *lat, double *lon, const char *string, int context_tc ) // context_tc is used to disambiguate ambiguous short mapcode inputs; pass 0 or negative if not available
{
  if (lat==NULL || lon==NULL || string==NULL)
  {
    return -100;
  }
  else
  {
    long llat,llon;
    const char *iso3 = convertTerritoryCodeToIsoName(context_tc,0);
    int err=full_mapcode_decode(&llat,&llon,iso3,string,NULL,NULL);
#ifdef VERSION160 // precise encoding: result built in extrax/y
    *lat = extray/(double)1000000.0;
    *lon = extrax/(double)1000000.0;
#else
    *lat = llat/1000000.0;
    *lon = llon/1000000.0;
#endif
    while (*lat < -90.0) *lat = -90.0;
    while  (*lat > 90.0) *lat = 90.0;
    while (*lon < -180.0) *lon += 360.0; // @@@ V160 bugfix
    while (*lon >= 180.0) *lon -= 360.0; // @@@ V160 bugfix
    return err;
  }
}

#ifdef SUPPORT_FOREIGN_ALPHABETS

const char *decodeToRoman(const UWORD* s)
{
  return decode_utf16(s);
}

const UWORD* encodeToAlphabet(const char *mapcode,int alphabet) // 0=roman, 2=cyrillic
{
  if ( asc2lan[alphabet][4]==0x003f ) // alphabet has no letter E
    if ( strchr(mapcode,'E') || strchr(mapcode,'U') || strchr(mapcode,'e') || strchr(mapcode,'u') ) // v1.50 get rid of E and U
    {
      char u[16];
      strcpy(u,mapcode);
      unpack_if_alldigits(u);
      repack_if_alldigits(u,1);
      return encode_utf16(u,alphabet);
    }
  return encode_utf16(mapcode,alphabet);
}

#endif

int encodeLatLonToMapcodes( char **v, double lat, double lon, int tc, int extraDigits )
{
  return encodeLatLonToMapcodes_full(v,lat,lon,tc,0,extraDigits);
}

int encodeLatLonToSingleMapcode( char *result, double lat, double lon, int tc, int extraDigits )
{
  char *v[2];
  int ret=encodeLatLonToMapcodes_full(v,lat,lon,tc,1,extraDigits);
  *result=0;
  if (ret>0) {
    if (strcmp(v[1],"AAA")!=0) {
      strcpy(result,v[1]);
      strcat(result," ");
    }
    strcat(result,v[0]);
  }
  return ret;
}


#endif // RELEASENEAT

