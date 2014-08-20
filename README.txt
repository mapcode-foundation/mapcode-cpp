+----
| Copyright (C) 2014 Stichting Mapcode Foundation
| For terms of use refer to http://www.mapcode.com/downloads.html
+----

This directory contains the original C++ Mapcode sources and an application to
- encode lat/lon to Mapcodes,
- decode Mapcodes to lat/lon,
- generate 3D uniformly distributed lat/lon pairs and their Mapcodes.

To build the original Mapcode tool:
  gcc mapcode.c -o mapcode

For help, simply execute 'mapcode' without no arguments. This will show you the
help text:

+----
Usage:
    mapcode [-d | --decode] <default-territory-ISO3> <mapcode> [<mapcode> ...]

       Decode a Mapcode to a lat/lon. The default territory code is used if
       the Mapcode is a shorthand local code

    mapcode [-e | --encode] <lat:-90..90> <lon:-180..180> [territory-ISO3]>

       Encode a lat/lon to a Mapcode. If the territory code is specified, the
       encoding will only succeeed if the lat/lon is located in the territory.

    mapcode [-g | --generate] <nrPoints> [<seed>]

       Create a test set of a number of uniformly distributed lat/lon pairs,
       3D x/y/z points and their Mapcodes). The output format is:
           <nr> <lat> <lon> <x> <y> <z>
           <territory> <mapcode>            (repeated 'nr' rtimes)
           <1 empty line>

       The points will be uniformly distributed over the 3D surface of the Earth
       rather than using uniformly distributed lat/lon values.
       Ranges:
           nr > 1    lat = -90..90    lon = -180..180    x,y,z = -1..1
+----
