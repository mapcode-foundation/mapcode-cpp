/**
 * Copyright (C) 2014 Stichting Mapcode Foundation
 * For terms of use refer to http://www.mapcode.com/downloads.html
 */

#include <stdio.h>
#include <math.h>
#include "mapcoder/mapcoder.c"

static const double PI = 3.14159265358979323846;

static void usage(const char* appName) {
    printf("Usage: \n");
    printf("    %s [-d | --decode] <default-country-ISO3> <mapcode> [<mapcode> ...]\n", appName);
    printf("\n");
    printf("       Decode a Mapcode to a lat/lon. The default country code is used if\n");
    printf("       the Mapcode is a shorthand local code\n");
    printf("\n");
    printf("    %s [-e | --encode] <lat:-90..90> <lon:-180..180> [country-ISO3]>\n", appName);
    printf("\n");
    printf("       Encode a lat/lon to a Mapcode. If the country code is specified, the\n");
    printf("       encoding will only succeeed if the lat/lon is located in the country.\n");
    printf("\n");
    printf("    %s [-g | --generate] <nrPoints> [<seed>]\n", appName);
    printf("\n");
    printf("       Create a test set of a number of uniformly distributed lat/lon pairs,\n");
    printf("       3D x/y/z points and their Mapcodes). The output format is:\n");
    printf("           <nr> <lat> <lon> <x> <y> <z>\n");
    printf("           <country> <mapcode>            (repeated 'nr' rtimes)\n");
    printf("           <1 empty line>\n");
    printf("\n");
    printf("       The points will be uniformly distributed over the 3D surface of the Earth\n");
    printf("       rather than using uniformly distributed lat/lon values.\n");
    printf("       Ranges:\n");
    printf("           nr > 1    lat = -90..90    lon = -180..180    x,y,z = -1..1\n");
    printf("\n");
}

int main(const int argc, const char** argv)
{
    // Provide usage message if no arguments specified.
    const char* appName = argv[0];
    if (argc < 2) {
        usage(appName);
        return -1;
    }

    // First argument: command.
    const char* cmd = argv[1];
    if ((strcmp(cmd, "-d") == 0) || (strcmp(cmd, "--decode") == 0)) {

        // Decode: [-d | --decode] <default-country-ISO3> <mapcode> [<mapcode> ...]
        if (argc < 3) {
            usage(appName);
            return -1;
        }

        const char* defaultCountry = argv[2];
        double lat;
        double lon;
        int context = text2tc(defaultCountry, 0);
        for (int i = 3; i <= argc; ++i) {
            int err = mc2coord(&lat, &lon, argv[i], context);
            if (err != 0) {
                printf("error: cannot decode '%s' (default country='%s')\n", argv[i], argv[1]);
                return -1;
            }
            printf("%g %g\n", lat, lon);
        }
    }
    else if ((strcmp(cmd, "-e") == 0) || (strcmp(cmd, "--encode") == 0)) {

        // Encode: [-e | --encode] <lat:-90..90> <lon:-180..180> [country-ISO3]>
        if ((argc != 4) && (argc != 5)) {
            usage(appName);
            return -1;
        }
        const double lat = atof(argv[2]);
        const double lon = atof(argv[3]);

        const char* results[32];
        int context = 0;
        if (argc == 4) {
            context = text2tc(argv[4], 0);
        }
        const int nrResults = coord2mc(results, lat, lon, context);
        if (nrResults == 0) {
            printf("error: cannot encode lat=%s, lon=%s (default country='%s')\n", argv[2], argv[3], (argc == 5) ? argv[4] : "none");
            return -1;
        }
        for (int i = 0; i < nrResults; ++i) {
            printf("%s %s\n", results[(i * 2) + 1], results[(i * 2)]);
        }
    }
    else if ((strcmp(cmd, "-g") == 0) || (strcmp(cmd, "--generate") == 0)) {

        // Generate uniform test set: [-g | --generate] <nrPoints> [<seed>]
        if ((argc != 3) && (argc != 4)) {
            usage(appName);
            return -1;
        }
        const int nrPoints = atoi(argv[2]);

        if (argc == 4) {
            const int seed = atoi(argv[3]);
            srand(seed);
        }
        else {
            srand(time(0));
        }
        const char* results[32];
        int context = 0;
        for (int i = 0; i < nrPoints; ++i) {

            // Calculate uniformly distributed 3D point on sphere (radius = 1.0):
            // http://mathproofs.blogspot.co.il/2005/04/uniform-random-distribution-on-sphere.html
            const double unitRand = ((double) rand()) / RAND_MAX;
            const double theta0 = (2.0 * PI) * unitRand;
            const double theta1 = acos(1.0 - (2.0 * unitRand));
            const double x = sin(theta0) * sin(theta1);
            const double y = cos(theta0) * sin(theta1);
            const double z = cos(theta1);

            // Convert Carthesian 3D point into lat/lon (radius = 1.0):
            // http://stackoverflow.com/questions/1185408/converting-from-longitude-latitude-to-cartesian-coordinates
            const double latRad = asin(z);
            const double lonRad = atan2(y, x);

            // Convert radians to degrees.
            const double lat = latRad * (180.0 / PI);
            const double lon = lonRad * (180.0 / PI);

            const int nrResults = coord2mc(results, lat, lon, context);
            if (nrResults <= 0) {
                printf("error: cannot encode lat=%g, lon=%g)\n", lat, lon);
                return -1;
            }
            printf("%d %g %g %g %g %g\n", nrResults, lat, lon, x, y, z);
            for (int i = 0; i < nrResults; ++i) {
                printf("%s %s\n", results[i*2 + 1], results[i*2]);
            }
            printf("\n");
        }
    }
    else {

        // Usage.
        usage(appName);
        return -1;
    }
    return 0;
}
