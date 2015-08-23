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


static const char parentletter[MAX_CCODE + 1] =
    {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2,
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 4, 4, 4,
        4, 4, 4, 4, 4, 4, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
        6, 6, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 3, 3, 3, 3, 3, 3, 3, 3,
        3, 3, 3, 3, 0, 0, 0, 0, 0, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 0,
        0, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
        8, 0, 0, 0, 0, 0,0};


static const char parentnumber[MAX_CCODE + 1] =
    {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 2, 4, 6, 1, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3,
        7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 8, 0, 0, 0, 0,0};

#define NRTERREC 916
typedef struct {
    const char *alphaCode;
    int ccode;
} alphaRec;
static const alphaRec alphaSearch[NRTERREC] = {
        {"1AK",  393},
        {"1AL",  364},
        {"1AR",  365},
        {"1AS",  27},
        {"1AZ",  388},
        {"1CA",  391},
        {"1CO",  386},
        {"1CT",  346},
        {"1DC",  343},
        {"1DE",  345},
        {"1FL",  372},
        {"1GA",  370},
        {"1GU",  50},
        {"1HI",  351},
        {"1IA",  368},
        {"1ID",  380},
        {"1IL",  369},
        {"1IN",  356},
        {"1KS",  379},
        {"1KY",  357},
        {"1LA",  363},
        {"1MA",  350},
        {"1MD",  352},
        {"1ME",  355},
        {"1MI",  383},
        {"1MID", 351},
        {"1MN",  382},
        {"1MO",  373},
        {"1MP",  48},
        {"1MS",  362},
        {"1MT",  390},
        {"1NC",  366},
        {"1ND",  375},
        {"1NE",  378},
        {"1NH",  348},
        {"1NJ",  347},
        {"1NM",  389},
        {"1NV",  387},
        {"1NY",  367},
        {"1OH",  360},
        {"1OK",  374},
        {"1OR",  385},
        {"1PA",  361},
        {"1PR",  76},
        {"1RI",  344},
        {"1SC",  354},
        {"1SD",  377},
        {"1TN",  358},
        {"1TX",  392},
        {"1UM",  529},
        {"1UT",  381},
        {"1VA",  359},
        {"1VI",  38},
        {"1VT",  349},
        {"1WA",  376},
        {"1WI",  371},
        {"1WV",  353},
        {"1WY",  384},
        {"2AN",  274},
        {"2AP",  303},
        {"2AR",  285},
        {"2AS",  286},
        {"2BR",  287},
        {"2CG",  294},
        {"2CH",  273},
        {"2CT",  294},
        {"2DD",  271},
        {"2DL",  276},
        {"2DN",  272},
        {"2GA",  289},
        {"2GJ",  299},
        {"2HP",  292},
        {"2HR",  284},
        {"2JH",  295},
        {"2JK",  293},
        {"2KA",  296},
        {"2KL",  290},
        {"2LD",  275},
        {"2MH",  304},
        {"2ML",  277},
        {"2MN",  279},
        {"2MP",  301},
        {"2MZ",  281},
        {"2NL",  278},
        {"2OD",  298},
        {"2OR",  298},
        {"2PB",  283},
        {"2PY",  306},
        {"2RJ",  297},
        {"2SK",  282},
        {"2TG",  302},
        {"2TN",  291},
        {"2TR",  280},
        {"2UK",  288},
        {"2UP",  305},
        {"2UT",  288},
        {"2WB",  300},
        {"3AB",  395},
        {"3BC",  394},
        {"3MB",  399},
        {"3NB",  401},
        {"3NL",  400},
        {"3NS",  402},
        {"3NT",  405},
        {"3NU",  406},
        {"3ON",  396},
        {"3PE",  403},
        {"3QC",  397},
        {"3SK",  398},
        {"3YT",  404},
        {"4ACT", 308},
        {"4CC",  4},
        {"4CX",  21},
        {"4HM",  42},
        {"4JBT", 309},
        {"4NF",  11},
        {"4NSW", 307},
        {"4NT",  310},
        {"4QLD", 315},
        {"4SA",  311},
        {"4TAS", 312},
        {"4VIC", 313},
        {"4WA",  314},
        {"5AG",  236},
        {"5AGU", 236},
        {"5BC",  253},
        {"5BCN", 253},
        {"5BCS", 256},
        {"5BS",  256},
        {"5CAM", 248},
        {"5CH",  264},
        {"5CHH", 264},
        {"5CHP", 255},
        {"5CL",  237},
        {"5CM",  248},
        {"5CO",  262},
        {"5COA", 262},
        {"5COL", 237},
        {"5CS",  255},
        {"5DF",  233},
        {"5DG",  261},
        {"5DIF", 233},
        {"5DUR", 261},
        {"5GR",  251},
        {"5GRO", 251},
        {"5GT",  243},
        {"5GUA", 243},
        {"5HG",  239},
        {"5HID", 239},
        {"5JA",  258},
        {"5JAL", 258},
        {"5ME",  240},
        {"5MEX", 240},
        {"5MI",  249},
        {"5MIC", 249},
        {"5MO",  235},
        {"5MOR", 235},
        {"5MX",  240},
        {"5NA",  242},
        {"5NAY", 242},
        {"5NL",  252},
        {"5NLE", 252},
        {"5OA",  260},
        {"5OAX", 260},
        {"5PB",  244},
        {"5PUE", 244},
        {"5QE",  238},
        {"5QR",  246},
        {"5QUE", 238},
        {"5ROO", 246},
        {"5SI",  247},
        {"5SIN", 247},
        {"5SL",  250},
        {"5SLP", 250},
        {"5SO",  263},
        {"5SON", 263},
        {"5TAB", 241},
        {"5TAM", 259},
        {"5TB",  241},
        {"5TL",  234},
        {"5TLA", 234},
        {"5TM",  259},
        {"5VE",  254},
        {"5VER", 254},
        {"5YU",  245},
        {"5YUC", 245},
        {"5ZA",  257},
        {"5ZAC", 257},
        {"6AC",  327},
        {"6AL",  318},
        {"6AM",  342},
        {"6AP",  325},
        {"6BA",  338},
        {"6CE",  326},
        {"6DF",  316},
        {"6ES",  320},
        {"6GO",  336},
        {"6MA",  335},
        {"6MG",  339},
        {"6MS",  337},
        {"6MT",  340},
        {"6PA",  341},
        {"6PB",  322},
        {"6PE",  324},
        {"6PI",  332},
        {"6PR",  328},
        {"6RJ",  319},
        {"6RN",  321},
        {"6RO",  330},
        {"6RR",  329},
        {"6RS",  334},
        {"6SC",  323},
        {"6SE",  317},
        {"6SP",  331},
        {"6TO",  333},
        {"7AD",  416},
        {"7AL",  482},
        {"7ALT", 483},
        {"7AMU", 493},
        {"7ARK", 454},
        {"7AST", 434},
        {"7BA",  463},
        {"7BE",  426},
        {"7BEL", 426},
        {"7BRY", 431},
        {"7BU",  474},
        {"7CE",  420},
        {"7CH",  473},
        {"7CHE", 473},
        {"7CHU", 487},
        {"7CU",  421},
        {"7DA",  437},
        {"7IN",  415},
        {"7IRK", 476},
        {"7IVA", 422},
        {"7KAM", 488},
        {"7KB",  418},
        {"7KC",  419},
        {"7KDA", 443},
        {"7KEM", 481},
        {"7KGD", 414},
        {"7KGN", 472},
        {"7KHA", 492},
        {"7KHM", 469},
        {"7KI",  458},
        {"7KIR", 458},
        {"7KK",  480},
        {"7KL",  444},
        {"7KLU", 429},
        {"7KM",  469},
        {"7KO",  453},
        {"7KOS", 441},
        {"7KR",  451},
        {"7KRS", 428},
        {"7KYA", 485},
        {"7LEN", 446},
        {"7LIP", 423},
        {"7MAG", 486},
        {"7ME",  459},
        {"7MO",  455},
        {"7MOS", 435},
        {"7MOW", 412},
        {"7MUR", 450},
        {"7NEN", 452},
        {"7NGR", 439},
        {"7NIZ", 456},
        {"7NVS", 477},
        {"7OMS", 479},
        {"7ORE", 460},
        {"7ORL", 424},
        {"7PER", 462},
        {"7PM",  462},
        {"7PNZ", 457},
        {"7PO",  490},
        {"7PRI", 490},
        {"7PSK", 440},
        {"7ROS", 447},
        {"7RYA", 433},
        {"7SA",  494},
        {"7SAK", 489},
        {"7SAM", 466},
        {"7SAR", 467},
        {"7SE",  417},
        {"7SMO", 436},
        {"7SPE", 413},
        {"7STA", 442},
        {"7SVE", 470},
        {"7TA",  465},
        {"7TAM", 430},
        {"7TOM", 478},
        {"7TT",  430},
        {"7TUL", 425},
        {"7TVE", 445},
        {"7TY",  484},
        {"7TYU", 471},
        {"7UD",  464},
        {"7ULY", 461},
        {"7VGG", 448},
        {"7VLA", 427},
        {"7VLG", 449},
        {"7VOR", 438},
        {"7YAN", 468},
        {"7YAR", 432},
        {"7YEV", 491},
        {"7ZAB", 475},
        {"811",  499},
        {"812",  498},
        {"813",  516},
        {"814",  509},
        {"815",  518},
        {"821",  507},
        {"822",  515},
        {"823",  519},
        {"831",  497},
        {"832",  504},
        {"833",  503},
        {"834",  506},
        {"835",  505},
        {"836",  510},
        {"837",  508},
        {"841",  511},
        {"842",  514},
        {"843",  520},
        {"844",  513},
        {"845",  521},
        {"846",  500},
        {"850",  502},
        {"851",  522},
        {"852",  512},
        {"853",  523},
        {"854",  524},
        {"861",  517},
        {"862",  525},
        {"863",  526},
        {"864",  501},
        {"865",  527},
        {"871",  109},
        {"891",  61},
        {"892",  8},
        {"8AH",  506},
        {"8BJ",  499},
        {"8CQ",  502},
        {"8FJ",  505},
        {"8GD",  513},
        {"8GS",  525},
        {"8GX",  521},
        {"8GZ",  512},
        {"8HA",  511},
        {"8HB",  514},
        {"8HE",  516},
        {"8HI",  500},
        {"8HK",  61},
        {"8HL",  519},
        {"8HN",  520},
        {"8JL",  515},
        {"8JS",  504},
        {"8JX",  510},
        {"8LN",  507},
        {"8MC",  8},
        {"8NM",  518},
        {"8NX",  501},
        {"8QH",  526},
        {"8SC",  522},
        {"8SD",  508},
        {"8SH",  497},
        {"8SN",  517},
        {"8SX",  509},
        {"8TJ",  498},
        {"8TW",  109},
        {"8XJ",  527},
        {"8XZ",  524},
        {"8YN",  523},
        {"8ZJ",  503},
        {"AAA",  532},
        {"AB",   395},
        {"ABW",  25},
        {"AC",   327},
        {"ACT",  308},
        {"AD",   416},
        {"AFG",  206},
        {"AG",   236},
        {"AGO",  224},
        {"AGU",  236},
        {"AH",   506},
        {"AIA",  18},
        {"AK",   393},
        {"AL",   318},
        {"ALA",  198},
        {"ALB",  102},
        {"ALT",  483},
        {"AM",   342},
        {"AMU",  493},
        {"AN",   274},
        {"AND",  49},
        {"AP",   303},
        {"AR",   285},
        {"ARE",  130},
        {"ARG",  270},
        {"ARK",  454},
        {"ARM",  104},
        {"AS",   286},
        {"ASC",  35},
        {"ASM",  27},
        {"AST",  434},
        {"ATA",  531},
        {"ATF",  75},
        {"ATG",  44},
        {"AU",   408},
        {"AUS",  408},
        {"AUT",  131},
        {"AZ",   388},
        {"AZE",  132},
        {"BA",   338},
        {"BC",   394},
        {"BCN",  253},
        {"BCS",  256},
        {"BDI",  100},
        {"BE",   426},
        {"BEL",  106},
        {"BEN",  144},
        {"BES",  33},
        {"BFA",  172},
        {"BGD",  151},
        {"BGR",  141},
        {"BHR",  57},
        {"BHS",  85},
        {"BIH",  118},
        {"BJ",   499},
        {"BLM",  5},
        {"BLR",  160},
        {"BLZ",  95},
        {"BMU",  14},
        {"BOL",  219},
        {"BR",   287},
        {"BRA",  409},
        {"BRB",  43},
        {"BRN",  74},
        {"BRY",  431},
        {"BS",   256},
        {"BTN",  110},
        {"BU",   474},
        {"BVT",  13},
        {"BWA",  200},
        {"CA",   391},
        {"CAF",  204},
        {"CAM",  248},
        {"CAN",  495},
        {"CC",   4},
        {"CCK",  4},
        {"CE",   326},
        {"CG",   294},
        {"CH",   273},
        {"CHE",  111},
        {"CHH",  264},
        {"CHL",  209},
        {"CHN",  528},
        {"CHP",  255},
        {"CHU",  487},
        {"CIV",  177},
        {"CL",   237},
        {"CM",   248},
        {"CMR",  192},
        {"CN",   528},
        {"CO",   386},
        {"COA",  262},
        {"COD",  267},
        {"COG",  182},
        {"COK",  28},
        {"COL",  221},
        {"COM",  65},
        {"CPT",  530},
        {"CPV",  72},
        {"CQ",   502},
        {"CRI",  117},
        {"CS",   255},
        {"CT",   294},
        {"CU",   421},
        {"CUB",  140},
        {"CUW",  45},
        {"CX",   21},
        {"CXR",  21},
        {"CYM",  32},
        {"CYP",  77},
        {"CZE",  128},
        {"DA",   437},
        {"DC",   343},
        {"DD",   271},
        {"DE",   345},
        {"DEU",  183},
        {"DF",   316},
        {"DG",   261},
        {"DGA",  15},
        {"DIF",  233},
        {"DJI",  96},
        {"DL",   276},
        {"DMA",  56},
        {"DN",   272},
        {"DNK",  113},
        {"DOM",  115},
        {"DUR",  261},
        {"DZA",  268},
        {"ECU",  168},
        {"EGY",  217},
        {"ERI",  145},
        {"ES",   320},
        {"ESH",  169},
        {"ESP",  194},
        {"EST",  114},
        {"ETH",  220},
        {"FIN",  181},
        {"FJ",   505},
        {"FJI",  89},
        {"FL",   372},
        {"FLK",  82},
        {"FRA",  197},
        {"FRO",  63},
        {"FSM",  53},
        {"GA",   289},
        {"GAB",  170},
        {"GBR",  166},
        {"GD",   513},
        {"GEO",  124},
        {"GGY",  17},
        {"GHA",  164},
        {"GIB",  2},
        {"GIN",  167},
        {"GJ",   299},
        {"GLP",  64},
        {"GMB",  80},
        {"GNB",  108},
        {"GNQ",  101},
        {"GO",   336},
        {"GR",   251},
        {"GRC",  149},
        {"GRD",  37},
        {"GRL",  265},
        {"GRO",  251},
        {"GS",   525},
        {"GT",   243},
        {"GTM",  139},
        {"GU",   50},
        {"GUA",  243},
        {"GUF",  129},
        {"GUM",  50},
        {"GUY",  161},
        {"GX",   521},
        {"GZ",   512},
        {"HA",   511},
        {"HB",   514},
        {"HE",   516},
        {"HG",   239},
        {"HI",   351},
        {"HID",  239},
        {"HK",   61},
        {"HKG",  61},
        {"HL",   519},
        {"HM",   42},
        {"HMD",  42},
        {"HN",   520},
        {"HND",  143},
        {"HP",   292},
        {"HR",   284},
        {"HRV",  119},
        {"HTI",  99},
        {"HUN",  136},
        {"IA",   368},
        {"ID",   380},
        {"IDN",  232},
        {"IL",   369},
        {"IMN",  51},
        {"IN",   356},
        {"IND",  407},
        {"IOT",  15},
        {"IRK",  476},
        {"IRL",  125},
        {"IRN",  229},
        {"IRQ",  187},
        {"ISL",  138},
        {"ISR",  92},
        {"ITA",  174},
        {"IVA",  422},
        {"JA",   258},
        {"JAL",  258},
        {"JAM",  79},
        {"JBT",  309},
        {"JEY",  20},
        {"JH",   295},
        {"JK",   293},
        {"JL",   515},
        {"JOR",  134},
        {"JPN",  184},
        {"JS",   504},
        {"JTN",  529},
        {"JX",   510},
        {"KA",   296},
        {"KAM",  488},
        {"KAZ",  269},
        {"KB",   418},
        {"KC",   419},
        {"KDA",  443},
        {"KEM",  481},
        {"KEN",  199},
        {"KGD",  414},
        {"KGN",  472},
        {"KGZ",  159},
        {"KHA",  492},
        {"KHM",  156},
        {"KI",   458},
        {"KIR",  58},
        {"KK",   480},
        {"KL",   290},
        {"KLU",  429},
        {"KM",   469},
        {"KNA",  31},
        {"KO",   453},
        {"KOR",  137},
        {"KOS",  441},
        {"KR",   451},
        {"KRS",  428},
        {"KS",   379},
        {"KWT",  88},
        {"KY",   357},
        {"KYA",  485},
        {"LA",   363},
        {"LAO",  162},
        {"LBN",  78},
        {"LBR",  142},
        {"LBY",  230},
        {"LCA",  52},
        {"LD",   275},
        {"LEN",  446},
        {"LIE",  24},
        {"LIP",  423},
        {"LKA",  123},
        {"LN",   507},
        {"LSO",  105},
        {"LTU",  122},
        {"LUX",  68},
        {"LVA",  121},
        {"MA",   335},
        {"MAC",  8},
        {"MAF",  10},
        {"MAG",  486},
        {"MAR",  188},
        {"MB",   399},
        {"MC",   8},
        {"MCO",  1},
        {"MD",   352},
        {"MDA",  107},
        {"MDG",  201},
        {"MDV",  34},
        {"ME",   355},
        {"MEX",  411},
        {"MG",   339},
        {"MH",   304},
        {"MHL",  26},
        {"MI",   383},
        {"MIC",  249},
        {"MID",  351},
        {"MKD",  97},
        {"ML",   277},
        {"MLI",  223},
        {"MLT",  36},
        {"MMR",  207},
        {"MN",   279},
        {"MNE",  84},
        {"MNG",  228},
        {"MNP",  48},
        {"MO",   373},
        {"MOR",  235},
        {"MOS",  435},
        {"MOW",  412},
        {"MOZ",  212},
        {"MP",   301},
        {"MRT",  218},
        {"MS",   337},
        {"MSR",  19},
        {"MT",   340},
        {"MTQ",  62},
        {"MUR",  450},
        {"MUS",  66},
        {"MWI",  146},
        {"MX",   240},
        {"MYS",  179},
        {"MYT",  39},
        {"MZ",   281},
        {"NA",   242},
        {"NAM",  213},
        {"NAY",  242},
        {"NB",   401},
        {"NC",   366},
        {"NCL",  90},
        {"ND",   375},
        {"NE",   378},
        {"NEN",  452},
        {"NER",  225},
        {"NF",   11},
        {"NFK",  11},
        {"NGA",  215},
        {"NGR",  439},
        {"NH",   348},
        {"NIC",  148},
        {"NIU",  30},
        {"NIZ",  456},
        {"NJ",   347},
        {"NL",   278},
        {"NLD",  112},
        {"NLE",  252},
        {"NM",   389},
        {"NOR",  178},
        {"NPL",  152},
        {"NRU",  6},
        {"NS",   402},
        {"NSW",  307},
        {"NT",   310},
        {"NU",   406},
        {"NV",   387},
        {"NVS",  477},
        {"NX",   501},
        {"NY",   367},
        {"NZL",  171},
        {"OA",   260},
        {"OAX",  260},
        {"OD",   298},
        {"OH",   360},
        {"OK",   374},
        {"OMN",  175},
        {"OMS",  479},
        {"ON",   396},
        {"OR",   298},
        {"ORE",  460},
        {"ORL",  424},
        {"PA",   341},
        {"PAK",  211},
        {"PAN",  127},
        {"PB",   283},
        {"PCN",  12},
        {"PE",   324},
        {"PER",  227},
        {"PHL",  173},
        {"PI",   332},
        {"PLW",  47},
        {"PM",   462},
        {"PNG",  191},
        {"PNZ",  457},
        {"PO",   490},
        {"POL",  176},
        {"PR",   328},
        {"PRI",  76},
        {"PRK",  147},
        {"PRT",  135},
        {"PRY",  186},
        {"PSE",  93},
        {"PSK",  440},
        {"PUE",  244},
        {"PY",   306},
        {"PYF",  71},
        {"QAT",  81},
        {"QC",   397},
        {"QE",   238},
        {"QH",   526},
        {"QLD",  315},
        {"QR",   246},
        {"QUE",  238},
        {"REU",  67},
        {"RI",   344},
        {"RJ",   297},
        {"RN",   321},
        {"RO",   330},
        {"ROO",  246},
        {"ROS",  447},
        {"ROU",  163},
        {"RR",   329},
        {"RS",   334},
        {"RU",   496},
        {"RUS",  496},
        {"RWA",  98},
        {"RYA",  433},
        {"SA",   311},
        {"SAK",  489},
        {"SAM",  466},
        {"SAR",  467},
        {"SAU",  266},
        {"SC",   323},
        {"SD",   377},
        {"SDN",  231},
        {"SE",   317},
        {"SEN",  158},
        {"SGP",  54},
        {"SGS",  70},
        {"SH",   497},
        {"SHN",  35},
        {"SI",   247},
        {"SIN",  247},
        {"SJM",  40},
        {"SK",   282},
        {"SL",   250},
        {"SLB",  103},
        {"SLE",  126},
        {"SLP",  250},
        {"SLV",  94},
        {"SMO",  436},
        {"SMR",  16},
        {"SN",   517},
        {"SO",   263},
        {"SOM",  205},
        {"SON",  263},
        {"SP",   331},
        {"SPE",  413},
        {"SPM",  29},
        {"SRB",  133},
        {"SSD",  203},
        {"STA",  442},
        {"STP",  60},
        {"SUR",  154},
        {"SVE",  470},
        {"SVK",  116},
        {"SVN",  91},
        {"SWE",  190},
        {"SWZ",  87},
        {"SX",   509},
        {"SXM",  9},
        {"SYC",  46},
        {"SYR",  157},
        {"TA",   465},
        {"TAA",  35},
        {"TAB",  241},
        {"TAM",  259},
        {"TAS",  312},
        {"TB",   241},
        {"TCA",  59},
        {"TCD",  226},
        {"TG",   302},
        {"TGO",  120},
        {"THA",  195},
        {"TJ",   498},
        {"TJK",  150},
        {"TKL",  3},
        {"TKM",  193},
        {"TL",   234},
        {"TLA",  234},
        {"TLS",  86},
        {"TM",   259},
        {"TN",   291},
        {"TO",   333},
        {"TOM",  478},
        {"TON",  55},
        {"TR",   280},
        {"TT",   430},
        {"TTO",  73},
        {"TUL",  425},
        {"TUN",  153},
        {"TUR",  210},
        {"TUV",  7},
        {"TVE",  445},
        {"TW",   109},
        {"TWN",  109},
        {"TX",   392},
        {"TY",   484},
        {"TYU",  471},
        {"TZA",  216},
        {"UD",   464},
        {"UGA",  165},
        {"UK",   288},
        {"UKR",  202},
        {"ULY",  461},
        {"UM",   529},
        {"UMI",  529},
        {"UP",   305},
        {"URY",  155},
        {"US",   410},
        {"USA",  410},
        {"UT",   288},
        {"UZB",  189},
        {"VA",   359},
        {"VAT",  0},
        {"VCT",  41},
        {"VE",   254},
        {"VEN",  214},
        {"VER",  254},
        {"VGB",  23},
        {"VGG",  448},
        {"VI",   38},
        {"VIC",  313},
        {"VIR",  38},
        {"VLA",  427},
        {"VLG",  449},
        {"VNM",  180},
        {"VOR",  438},
        {"VT",   349},
        {"VUT",  83},
        {"WA",   314},
        {"WAK",  26},
        {"WB",   300},
        {"WI",   371},
        {"WLF",  22},
        {"WSM",  69},
        {"WV",   353},
        {"WY",   384},
        {"XJ",   527},
        {"XZ",   524},
        {"YAN",  468},
        {"YAR",  432},
        {"YEM",  196},
        {"YEV",  491},
        {"YN",   523},
        {"YT",   404},
        {"YU",   245},
        {"YUC",  245},
        {"ZA",   257},
        {"ZAB",  475},
        {"ZAC",  257},
        {"ZAF",  222},
        {"ZJ",   503},
        {"ZMB",  208},
        {"ZWE",  185}
};

// produced by fast_territories.cpp