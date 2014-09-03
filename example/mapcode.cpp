/**
 * Copyright (C) 2014 Stichting Mapcode Foundation
 * For terms of use refer to http://www.mapcode.com/downloads.html
 */

/**
 * This application uses the Mapcode C library to encode and decode Mapcodes.
 * It also serves as an example of how to use this library in a C environment.
 *
 * It also offers additional options to generate "test sets" of coordinates
 * and Mapcodes to check other Mapcode implementations against reference data.
 *
 * These test sets consist of:
 *
 * - a number of "grid distributed" coordinates, which forms a set of coordinates
 *   and their Mapcodes, wrapped as a grid around the Earth;
 *
 * - a number of "random uniformly distributed" coordinates, which forms a set of
 *   random coordiantes on the surface of Earth; or
 *
 * - a set which consists of typical Mapcode "boundaries" and "edge cases", based
 *   on the internal implementation of the boundaries database of the Mapcode
 *   implementation.
 */

#include <stdio.h>
#include <math.h>
#include <time.h>
#include "../mapcodelib/mapcoder.c"

#define my_isnan(x) (false)
#define my_round(x) ((long) (floor((x) + 0.5)))


static const char*  VERSION             = "1";
static const int    SELF_CHECK          = 1;
static const int    SELF_CHECK_EXIT     = 0;

static const int    NORMAL_ERROR    = 1;
static const int    INTERNAL_ERROR  = 2;

/**
 * Some global constants to be used.
 */
static const double PI              = 3.14159265358979323846;
static const int    SHOW_PROGRESS   = 125;
static const double DELTA           = 0.001;



/**
 * These statistics are stored globally so they can be updated easily by the
 * generateAndOutputMapcodes() method.
 */
static int      totalNrOfPoints         = 0;
static int      totalNrOfResults        = 0;
static int      largestNrOfResults      = 0;
static double   latLargestNrOfResults   = 0.0;
static double   lonLargestNrOfResults   = 0.0;



/**
 * The usage() method explains how this application can be used. It is called
 * whenever a incorrect amount or combination of parameters is entered.
 */
static void usage(const char* appName) {
    printf("MAPCODE (version %s.%s%s)\n", mapcode_cversion, VERSION, SELF_CHECK ? ", self-checking" : "");
    printf("Copyright (C) 2014 Stichting Mapcode Foundation\n");
    printf("\n");
    printf("Usage:\n");
    printf("    %s [-d | --decode] <default-territory> <mapcode> [<mapcode> ...]\n", appName);
    printf("\n");
    printf("       Decode a Mapcode to a lat/lon. The default territory code is used if\n");
    printf("       the Mapcode is a shorthand local code\n");
    printf("\n");
    printf("    %s [-e | --encode] <lat:-90..90> <lon:-180..180> [territory]>\n", appName);
    printf("\n");
    printf("       Encode a lat/lon to a Mapcode. If the territory code is specified, the\n");
    printf("       encoding will only succeeed if the lat/lon is located in the territory.\n");
    printf("\n");
    printf("    %s [-b | --boundaries]\n", appName);
    printf("    %s [-g | --grid] <nrOfPoints>\n", appName);
    printf("    %s [-r | --random] <nrOfPoints> [<seed>]\n", appName);
    printf("\n");
    printf("       Create a test set of lat/lon pairs based on the Mapcode boundaries database\n");
    printf("       as a fixed 3D grid or random uniformly distributed set of lat/lons with their\n");
    printf("       (x, y, z) coordinates and all Mapcode aliases.\n");
    printf("\n");
    printf("       The output format is:\n");
    printf("           <number-of-aliases> <lat-deg> <lon-deg> <x> <y> <z>\n");
    printf("           <territory> <mapcode>      (repeated 'number-of-aliases' times)\n");
    printf("                                      (empty lines and next record)\n");
    printf("       Ranges:\n");
    printf("           number-of-aliases : >= 1\n");
    printf("           lat-deg, lon-deg  : [-90..90], [-180..180]\n");
    printf("           x, y, z           : [-1..1]\n");
    printf("\n");
    printf("       The lat/lon pairs will be distributed over the 3D surface of the Earth\n");
    printf("       and the (x, y, z) coordinates are placed on a sphere with radius 1.\n");
    printf("       The (x, y, z) coordinates are primarily meant for visualization of the data set.\n");
    printf("\n");
    printf("\n     Notes on the use of stdout and stderr:\n");
    printf("\n     stdout: used for outputting 3D point data; stderr: used for statistics.");
    printf("\n     You can redirect stdout to a destination file, while stderr will show progress.\n");
    printf("\n");
    printf("       The result code is 0 when no error occurred, 1 if an input error occurred and 2\n");
    printf("       if an internal error occurred.\n");
}



/**
 * The method radToDeg() converts radians to degrees.
 */
static double radToDeg(double rad){
    return (rad / PI) * 180.0;
}



/**
 * The method degToRad() converts degrees to radians.
 */
static double degToRad(double deg){
    return (deg / 180.0) * PI;
}



/**
 * Given a single number between 0..1, generate a latitude, longitude (in degrees) and a 3D
 * (x, y, z) point on a sphere with a radius of 1.
 */
static void unitToLatLonDeg(
    const double unit1, const double unit2, double* latDeg, double* lonDeg) {

    // Calculate uniformly distributed 3D point on sphere (radius = 1.0):
    // http://mathproofs.blogspot.co.il/2005/04/uniform-random-distribution-on-sphere.html
    const double theta0 = (2.0 * PI) * unit1;
    const double theta1 = acos(1.0 - (2.0 * unit2));
    double x = sin(theta0) * sin(theta1);
    double y = cos(theta0) * sin(theta1);
    double z = cos(theta1);

    // Convert Carthesian 3D point into lat/lon (radius = 1.0):
    // http://stackoverflow.com/questions/1185408/converting-from-longitude-latitude-to-cartesian-coordinates
    const double latRad = asin(z);
    const double lonRad = atan2(y, x);

    // Convert radians to degrees.
    *latDeg = my_isnan(latRad) ? 90.0 : radToDeg(latRad);
    *lonDeg = my_isnan(lonRad) ? 180.0 : radToDeg(lonRad);
}



/**
 * The method convertLatLonToXYZ() convertes a lat/lon pair to a (x, y, z) coordinate
 * on a sphere with radius 1.
 */
static void convertLatLonToXYZ(double latDeg, double lonDeg, double* x, double* y, double* z) {
    double latRad = degToRad(latDeg);
    double lonRad = degToRad(lonDeg);
    *x = cos(latRad) * cos(lonRad);
    *z = sin(latRad);
}



/**
 * This methods provides a self check for encoding lat/lon to Mapcode.
 */
static void selfCheckLatLonToMapcode(const double lat, double lon, const char* territory, const char* mapcode) {
    int context = convertTerritoryIsoNameToCode(territory, 0);
    char* results[MAX_NR_OF_MAPCODE_RESULTS];
    const double limitLat = (lat < -90.0) ? -90.0 : ((lat > 90.0) ? 90.0 : lat);
    const double limitLon = (lon < -180.0) ? -180.0 : ((lon > 180.0) ? 180.0 : lon);
    const int nrResults = encodeLatLonToMapcodes(results, limitLat, limitLon, context);
    if (nrResults <= 0) {
        fprintf(stderr, "internal error: encoding lat/lon to Mapcode failure; "
            "cannot encode lat=%f, lon=%f (default territory=%s)\n",
            lat, lon, territory);
        if (SELF_CHECK_EXIT) {
            exit(INTERNAL_ERROR);
        }
        return;
    }
    int found = 0;
    for (int i = 0; !found && (i < nrResults); ++i) {
        const char* foundMapcode = results[(i * 2)];
        const char* foundTerritory = results[(i * 2) + 1];
        found = ((strcmp(territory, foundTerritory) == 0) && (strcmp(mapcode, foundMapcode) == 0));
    }
    if (!found) {
        fprintf(stderr, "internal error: encoding lat/lon to Mapcode failure; "
            "Mapcode '%s %s' decodes to lat=%f(%f), lon=%f(%f), "
            "which does not encode back to '%s %s'\n",
            territory, mapcode, lat, limitLat, lon, limitLon, territory, mapcode);
        if (SELF_CHECK_EXIT) {
            exit(INTERNAL_ERROR);
        }
        return;
    }
}



/**
 * This method provides a self-check for decoding a Mapcode to lat/lon.
 */
static void selfCheckMapcodeToLatLon(const char* territory, const char* mapcode,
    const double lat, const double lon) {
    double foundLat;
    double foundLon;
    int foundContext = convertTerritoryIsoNameToCode(territory, 0);
    int err = decodeMapcodeToLatLon(&foundLat, &foundLon, mapcode, foundContext);
    if (err != 0) {
        fprintf(stderr, "internal error: decoding Mapcode to lat/lon failure; "
            "cannot decode '%s %s')\n", territory, mapcode);
        if (SELF_CHECK_EXIT) {
            exit(INTERNAL_ERROR);
        }
        return;
    }
    double deltaLat = ((foundLat - lat) >= 0.0 ? (foundLat - lat) : -(foundLat - lat));
    double deltaLon = ((foundLon - lon) >- 0.0 ? (foundLon - lon) : -(foundLon - lon));
    if (deltaLon > 180.0) {
        deltaLon = 360.0 - deltaLon;
    }
    if ((deltaLat > DELTA) || (deltaLon > DELTA)) {
        fprintf(stderr, "internal error: decoding Mapcode to lat/lon failure; "
            "lat=%f, lon=%f produces Mapcode %s %s, "
            "which decodes to lat=%f (delta=%f), lon=%f (delta=%f)\n",
            lat, lon, territory, mapcode, foundLat, deltaLat, foundLon, deltaLon);
        if (SELF_CHECK_EXIT) {
            exit(INTERNAL_ERROR);
        }
        return;
    }
}



/**
 * The method asCoordinate() generates and returns a printable coordinate
 * precisely as it would be interpreted internally by Mapcode encoding
 * (i.e. correctly rounded to the nearest one-millionth of a degree).
 * As target, pass a buffer for at least 12 characters (including zero termination).
 * If target = 0, an internal scratch buffer is used (the THIRD call will
 * overwrite the first call).
 */
static char asCoordinateBuffer[24];
static int ascoptr;
static const char* asCoordinate(double coord, char* target)
{
    long c = (long) ((coord * 1000000) + ((coord < 0) ? -0.5 : 0.5));
    int negative = (c < 0);
    if (negative) {
        c = -c;
    }
    if (target == 0) {
        target = &asCoordinateBuffer[ascoptr];
        ascoptr= ((ascoptr != 0) ? 0 : 12);
    }
    sprintf(target,"%s%d.%06d", (negative ? "-" : ""), c / 1000000, c % 1000000);
    return target;
}


/**
 * The method printMapcode() generates and outputs Mapcodes for a lat/lon pair.
 * If iShowError != 0, then encoding errors are output to stderr, otherwise they
 * are ignored.
 */
static void generateAndOutputMapcodes(double lat, double lon, int iShowError) {

    char* results[MAX_NR_OF_MAPCODE_RESULTS];
    int context = 0;
    const int nrResults = encodeLatLonToMapcodes(results, lat, lon, context);
    if (nrResults <= 0) {
        if (iShowError) {
            fprintf(stderr, "error: cannot encode lat=%s, lon=%s)\n", asCoordinate(lat, 0), asCoordinate(lon, 0));
            exit(NORMAL_ERROR);
        }
    }

    double x;
    double y;
    double z;
    convertLatLonToXYZ(lat, lon, &x, &y, &z);
    printf("%d %s %s %lf %lf %lf\n", nrResults, asCoordinate(lat, 0), asCoordinate(lon, 0), x, y, z);
    for (int j = 0; j < nrResults; ++j) {
        const char* foundMapcode = results[(j * 2)];
        const char* foundTerritory = results[(j * 2) + 1];

        // Output result line.
        printf("%s %s\n", foundTerritory, foundMapcode);

        // Self-checking code to see if encoder produces this Mapcode for the lat/lon.
        if (SELF_CHECK) {
            selfCheckLatLonToMapcode(lat, lon, foundTerritory, foundMapcode);
            selfCheckMapcodeToLatLon(foundTerritory, foundMapcode, lat, lon);
        }
    }

    // Add empty line.
    printf("\n");

    if (nrResults > largestNrOfResults) {
        largestNrOfResults = nrResults;
        latLargestNrOfResults = lat;
        lonLargestNrOfResults = lon;
    }
    totalNrOfResults += nrResults;
}



/**
 * This method resets the statistics counters.
 */
static void resetStatistics(int nrOfPoints) {
    totalNrOfPoints = nrOfPoints;
    largestNrOfResults = 0;
    latLargestNrOfResults = 0.0;
    lonLargestNrOfResults = 0.0;
}



/**
 * This method outputs the statistics.
 */
static void outputStatistics() {
    fprintf(stderr, "\nStatistics:\n");
    fprintf(stderr, "Total number of 3D points generated     = %d\n", totalNrOfPoints);
    fprintf(stderr, "Total number of Mapcodes generated      = %d\n", totalNrOfResults);
    fprintf(stderr, "Average number of Mapcodes per 3D point = %f\n",
        ((float) totalNrOfResults) / ((float) totalNrOfPoints));
    fprintf(stderr, "Largest number of results for 1 Mapcode = %d at (%f, %f)\n",
        largestNrOfResults, latLargestNrOfResults, lonLargestNrOfResults);
}



/**
 * This method shows a progress indication.
 */
static void showProgress(int i) {
    fprintf(stderr, "[%d%%] Processed %d of %d regions (generated %d Mapcodes)...\r",
        (int) ((((float) i / ((float) totalNrOfPoints)) * 100.0) + 0.5),
        i, totalNrOfPoints, totalNrOfResults);
}



/**
 * This is the main() method which is called from the command-line.
 * Return code 0 means success. Any other values means some sort of error occurred.
 */
int main(const int argc, const char** argv)
{
    // Provide usage message if no arguments specified.
    const char* appName = argv[0];
    if (argc < 2) {
        usage(appName);
        return NORMAL_ERROR;
    }

    // First argument: command.
    const char* cmd = argv[1];
    if ((strcmp(cmd, "-d") == 0) || (strcmp(cmd, "--decode") == 0)) {

        // ------------------------------------------------------------------
        // Decode: [-d | --decode] <default-territory> <mapcode> [<mapcode> ...]
        // ------------------------------------------------------------------
        if (argc < 4) {
            fprintf(stderr, "error: incorrect number of arguments\n\n");
            usage(appName);
            return NORMAL_ERROR;
        }

        const char* defaultTerritory = argv[2];
        double lat;
        double lon;

        // Get the territory context.
        int context = convertTerritoryIsoNameToCode(defaultTerritory, 0);

        // Decode every Mapcode.
        for (int i = 3; i < argc; ++i) {

            // Decode the Mapcode to a lat/lon.
            const char* mapcode = argv[i];
            int err = decodeMapcodeToLatLon(&lat, &lon, mapcode, context);
            if (err != 0) {
                fprintf(stderr, "error: cannot decode '%s %s'\n", defaultTerritory, mapcode);
                return NORMAL_ERROR;
            }

            // Output the decoded lat/lon.
            printf("%f %f\n", lat, lon);

            // Self-checking code to see if encoder produces this Mapcode for the lat/lon.
            if (SELF_CHECK) {
                selfCheckLatLonToMapcode(lat, lon, defaultTerritory, mapcode);
            }
        }
    }
    else if ((strcmp(cmd, "-e") == 0) || (strcmp(cmd, "--encode") == 0)) {

        // ------------------------------------------------------------------
        // Encode: [-e | --encode] <lat:-90..90> <lon:-180..180> [territory]>
        // ------------------------------------------------------------------
        if ((argc != 4) && (argc != 5)) {
            fprintf(stderr, "error: incorrect number of arguments\n\n");
            usage(appName);
            return NORMAL_ERROR;
        }
        if ((!isdigit(*argv[2]) && (*argv[2] != '-')) || (!isdigit(*argv[3]) && (*argv[3] != '-'))) {
            fprintf(stderr, "error: latitude and longitude must be numeric\n");
            usage(appName);
            return NORMAL_ERROR;
        }
        const double lat = atof(argv[2]);
        const double lon = atof(argv[3]);

        // Get territory context.
        int context = 0;
        const char* defaultTerritory = "AAA";
        if (argc == 5) {
            context = convertTerritoryIsoNameToCode(argv[4], 0);
            defaultTerritory = argv[4];
        }

        // Encode the lat/lon to a set of Mapcodes.
        char* results[MAX_NR_OF_MAPCODE_RESULTS];
        const int nrResults = encodeLatLonToMapcodes(results, lat, lon, context);
        if (nrResults <= 0) {
            fprintf(stderr, "error: cannot encode lat=%f, lon=%f (default territory=%s)\n",
                lat, lon, defaultTerritory);
            return NORMAL_ERROR;
        }

        // Output the Mapcode.
        for (int i = 0; i < nrResults; ++i) {
            const char* foundMapcode = results[(i * 2)];
            const char* foundTerritory = results[(i * 2) + 1];
            printf("%s %s\n", foundTerritory, foundMapcode);

            // Self-checking code to see if decoder produces the lat/lon for all of these Mapcodes.
            if (SELF_CHECK) {
                selfCheckMapcodeToLatLon(foundTerritory, foundMapcode, lat, lon);
            }
        }
    }
    else if ((strcmp(cmd, "-b") == 0) || (strcmp(cmd, "--boundaries") == 0)) {

        // ------------------------------------------------------------------
        // Generate a test set based on the Mapcode boundaries.
        // ------------------------------------------------------------------
        if (argc != 2) {
            fprintf(stderr, "error: incorrect number of arguments\n\n");
            usage(appName);
            return NORMAL_ERROR;
        }

        resetStatistics(NR_BOUNDARY_RECS);
        for (int i = 0; i < totalNrOfPoints; ++i) {
            long minLonE6;
            long maxLonE6;
            long minLatE6;
            long maxLatE6;
            double minLon;
            double maxLon;
            double minLat;
            double maxLat;
            double lat;
            double lon;

            get_boundaries(i, &minLonE6, &minLatE6, &maxLonE6, &maxLatE6);
            minLon = ((double) minLonE6) / 1.0E6;
            maxLon = ((double) maxLonE6) / 1.0E6;
            minLat = ((double) minLatE6) / 1.0E6;
            maxLat = ((double) maxLatE6) / 1.0E6;

            // Try center.
            lat = (maxLat - minLat ) / 2.0;
            lon = (maxLon - minLon ) / 2.0;

            // Try center.
            generateAndOutputMapcodes(lat, lon, 0);

            // Try corners.
            generateAndOutputMapcodes(minLat, minLon, 0);
            generateAndOutputMapcodes(minLat, maxLon, 0);
            generateAndOutputMapcodes(maxLat, minLon, 0);
            generateAndOutputMapcodes(maxLat, maxLon, 0);

            // Try JUST inside.
            double factor = 1.0;
            for (int j = 1; j < 6; ++j) {

                double d = 1.0 / factor;
                generateAndOutputMapcodes(minLat + d, minLon + d, 0);
                generateAndOutputMapcodes(minLat + d, maxLon - d, 0);
                generateAndOutputMapcodes(maxLat - d, minLon + d, 0);
                generateAndOutputMapcodes(maxLat - d, maxLon - d, 0);

                // Try JUST outside.
                generateAndOutputMapcodes(minLat - d, minLon - d, 0);
                generateAndOutputMapcodes(minLat - d, maxLon + d, 0);
                generateAndOutputMapcodes(maxLat + d, minLon - d, 0);
                generateAndOutputMapcodes(maxLat + d, maxLon + d, 0);
                factor = factor * 10.0;
            }

            // Try 22m outside.
            generateAndOutputMapcodes(minLat - 22, (maxLon - minLon) / 2, 0);
            generateAndOutputMapcodes(minLat - 22, (maxLon - minLon) / 2, 0);
            generateAndOutputMapcodes(maxLat + 22, (maxLon - minLon) / 2, 0);
            generateAndOutputMapcodes(maxLat + 22, (maxLon - minLon) / 2, 0);

            if ((i % SHOW_PROGRESS) == 0) {
                showProgress(i);
            }
        }
        outputStatistics();
    }
    else if ((strcmp(cmd, "-g") == 0) || (strcmp(cmd, "--grid") == 0) ||
        (strcmp(cmd, "-r") == 0) || (strcmp(cmd, "--random") == 0)) {

        // ------------------------------------------------------------------
        // Generate grid test set:    [-g | --grid]   <nrOfPoints>
        // Generate uniform test set: [-r | --random] <nrOfPoints> [<seed>]
        // ------------------------------------------------------------------
        if ((argc < 3) || (argc > 4)) {
            fprintf(stderr, "error: incorrect number of arguments\n\n");
            usage(appName);
            return NORMAL_ERROR;
        }
        int nrOfPoints = atoi(argv[2]);
        if (nrOfPoints < 1) {
            fprintf(stderr, "error: total number of points to generate must be >= 1\n\n");
            usage(appName);
            return NORMAL_ERROR;
        }
        int random = (strcmp(cmd, "-r") == 0) || (strcmp(cmd, "--random") == 0);
        if (random) {
            if (argc == 4) {
                const int seed = atoi(argv[3]);
                srand(seed);
            }
            else {
                srand(time(0));
            }
        }
        else {
            if (argc > 3) {
                fprintf(stderr, "error: cannot specify seed for -g/--grid\n\n");
                usage(appName);
                return NORMAL_ERROR;
            }
        }

        // Statistics.
        resetStatistics(nrOfPoints);
        int totalNrOfResults = 0;

        int gridX = 0;
        int gridY = 0;
        int line = my_round(sqrt(totalNrOfPoints));
        for (int i = 0; i < totalNrOfPoints; ++i) {
            double lat;
            double lon;
            double unit1;
            double unit2;

            if (random) {
                unit1 = ((double) rand()) / RAND_MAX;
                unit2 = ((double) rand()) / RAND_MAX;
            }
            else {
                unit1 = ((double) gridX) / line;
                unit2 = ((double) gridY) / line;

                if (gridX < line) {
                    ++gridX;
                }
                else {
                    gridX = 0;
                    ++gridY;
                }
            }

            unitToLatLonDeg(unit1, unit2, &lat, &lon);
            generateAndOutputMapcodes(lat, lon, 1);

            if ((i % SHOW_PROGRESS) == 0) {
                showProgress(i);
            }
        }
        outputStatistics();
    }
    else {

        // ------------------------------------------------------------------
        // Usage.
        // ------------------------------------------------------------------
        usage(appName);
        return NORMAL_ERROR;
    }
    fprintf(stderr, "done\n");
    return 0;
}
