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
#ifndef __INTERNAL_TERRITORY_ALPHABETS_H__
#define __INTERNAL_TERRITORY_ALPHABETS_H__

#ifdef __cplusplus
extern "C" {
#endif

// *** GENERATED FILE (coords.cpp), DO NOT CHANGE OR PRETTIFY ***

#ifdef NO_SUPPORT_ALPHABETS
#error "NO_SUPPORT_ALPHABETS cannot be specified in this file"
#endif

#include "mapcoder.h"

static const TerritoryAlphabets alphabetsForTerritory[_TERRITORY_MAX - _TERRITORY_MIN - 1] = {
        {1, {ALPHABET_ROMAN}}, // VAT Vatican
        {1, {ALPHABET_ROMAN}}, // MCO Monaco
        {1, {ALPHABET_ROMAN}}, // GIB Gibraltar
        {1, {ALPHABET_ROMAN}}, // TKL Tokelau
        {2, {ALPHABET_ROMAN,      ALPHABET_ARABIC}}, // CCK Cocos Islands
        {1, {ALPHABET_ROMAN}}, // BLM Saint-Barthelemy
        {1, {ALPHABET_ROMAN}}, // NRU Nauru
        {1, {ALPHABET_ROMAN}}, // TUV Tuvalu
        {2, {ALPHABET_CHINESE,    ALPHABET_ROMAN}}, // MAC Macau
        {1, {ALPHABET_ROMAN}}, // SXM Sint Maarten
        {1, {ALPHABET_ROMAN}}, // MAF Saint Martin
        {1, {ALPHABET_ROMAN}}, // NFK Norfolk and Philip Island
        {1, {ALPHABET_ROMAN}}, // PCN Pitcairn Islands
        {1, {ALPHABET_ROMAN}}, // BVT Bouvet Island
        {1, {ALPHABET_ROMAN}}, // BMU Bermuda
        {1, {ALPHABET_ROMAN}}, // IOT British Indian Ocean Territory
        {1, {ALPHABET_ROMAN}}, // SMR San Marino
        {1, {ALPHABET_ROMAN}}, // GGY Guernsey
        {1, {ALPHABET_ROMAN}}, // AIA Anguilla
        {1, {ALPHABET_ROMAN}}, // MSR Montserrat
        {1, {ALPHABET_ROMAN}}, // JEY Jersey
        {3, {ALPHABET_CHINESE,    ALPHABET_ARABIC,     ALPHABET_ROMAN}}, // CXR Christmas Island
        {1, {ALPHABET_ROMAN}}, // WLF Wallis and Futuna the
        {1, {ALPHABET_ROMAN}}, // VGB British Virgin Islands
        {1, {ALPHABET_ROMAN}}, // LIE Liechtenstein
        {1, {ALPHABET_ROMAN}}, // ABW Aruba
        {1, {ALPHABET_ROMAN}}, // MHL Marshall Islands
        {1, {ALPHABET_ROMAN}}, // ASM American Samoa
        {1, {ALPHABET_ROMAN}}, // COK Cook Islands
        {1, {ALPHABET_ROMAN}}, // SPM Saint Pierre and Miquelon
        {1, {ALPHABET_ROMAN}}, // NIU Niue
        {1, {ALPHABET_ROMAN}}, // KNA Saint Kitts and Nevis, Federation of
        {1, {ALPHABET_ROMAN}}, // CYM Cayman islands
        {1, {ALPHABET_ROMAN}}, // BES Bonaire, St Eustasuis and Saba
        {2, {ALPHABET_THAANA,     ALPHABET_ROMAN}}, // MDV Maldives
        {1, {ALPHABET_ROMAN}}, // SHN Saint Helena, Ascension and Tristan da Cunha
        {1, {ALPHABET_ROMAN}}, // MLT Malta
        {1, {ALPHABET_ROMAN}}, // GRD Grenada
        {1, {ALPHABET_ROMAN}}, // VIR Virgin Islands of the United States
        {2, {ALPHABET_ROMAN,      ALPHABET_ARABIC}}, // MYT Mayotte
        {2, {ALPHABET_ROMAN,      ALPHABET_CYRILLIC}}, // SJM Svalbard and Jan Mayen
        {1, {ALPHABET_ROMAN}}, // VCT Saint Vincent and the Grenadines
        {1, {ALPHABET_ROMAN}}, // HMD Heard Island and McDonald Islands
        {1, {ALPHABET_ROMAN}}, // BRB Barbados
        {1, {ALPHABET_ROMAN}}, // ATG Antigua and Barbuda
        {1, {ALPHABET_ROMAN}}, // CUW Curacao
        {1, {ALPHABET_ROMAN}}, // SYC Seychelles
        {1, {ALPHABET_ROMAN}}, // PLW Palau
        {1, {ALPHABET_ROMAN}}, // MNP Northern Mariana Islands
        {1, {ALPHABET_ROMAN}}, // AND Andorra, Principality of
        {1, {ALPHABET_ROMAN}}, // GUM Guam
        {1, {ALPHABET_ROMAN}}, // IMN Isle of Mann
        {1, {ALPHABET_ROMAN}}, // LCA Saint Lucia
        {1, {ALPHABET_ROMAN}}, // FSM Micronesia
        {3, {ALPHABET_CHINESE,    ALPHABET_ARABIC,     ALPHABET_ROMAN}}, // SGP Singapore
        {1, {ALPHABET_ROMAN}}, // TON Tonga
        {1, {ALPHABET_ROMAN}}, // DMA Dominica, Commonwealth of
        {2, {ALPHABET_ARABIC,     ALPHABET_ROMAN}}, // BHR Bahrain
        {1, {ALPHABET_ROMAN}}, // KIR Kiribati
        {1, {ALPHABET_ROMAN}}, // TCA Turks and Caicos Islands
        {1, {ALPHABET_ROMAN}}, // STP Sao Tome and Principe
        {2, {ALPHABET_CHINESE,    ALPHABET_ROMAN}}, // HKG Hong Kong
        {1, {ALPHABET_ROMAN}}, // MTQ Martinique
        {1, {ALPHABET_ROMAN}}, // FRO Faroe Islands
        {1, {ALPHABET_ROMAN}}, // GLP Guadeloupe
        {2, {ALPHABET_ARABIC,     ALPHABET_ROMAN}}, // COM Comoros, Union of the
        {1, {ALPHABET_ROMAN}}, // MUS Mauritius
        {1, {ALPHABET_ROMAN}}, // REU Reunion
        {1, {ALPHABET_ROMAN}}, // LUX Luxembourg
        {1, {ALPHABET_ROMAN}}, // WSM Samoa, Independent State of
        {1, {ALPHABET_ROMAN}}, // SGS South Georgia and the South Sandwich Islands
        {1, {ALPHABET_ROMAN}}, // PYF French Polynesia
        {1, {ALPHABET_ROMAN}}, // CPV Cape Verde
        {1, {ALPHABET_ROMAN}}, // TTO Trinidad and Tobago
        {2, {ALPHABET_ARABIC,     ALPHABET_ROMAN}}, // BRN Brunei
        {1, {ALPHABET_ROMAN}}, // ATF French Southern and Antarctic Lands
        {1, {ALPHABET_ROMAN}}, // PRI Puerto Rico
        {2, {ALPHABET_GREEK,      ALPHABET_ROMAN}}, // CYP Cyprus
        {2, {ALPHABET_ARABIC,     ALPHABET_ROMAN}}, // LBN Lebanon
        {1, {ALPHABET_ROMAN}}, // JAM Jamaica
        {1, {ALPHABET_ROMAN}}, // GMB Gambia
        {2, {ALPHABET_ARABIC,     ALPHABET_ROMAN}}, // QAT Qatar, State of
        {1, {ALPHABET_ROMAN}}, // FLK Falkland Islands
        {1, {ALPHABET_ROMAN}}, // VUT Vanuatu
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // MNE Montenegro
        {1, {ALPHABET_ROMAN}}, // BHS Bahamas, Commonwealth of the
        {2, {ALPHABET_ROMAN,      ALPHABET_ARABIC}}, // TLS Timor-Leste
        {1, {ALPHABET_ROMAN}}, // SWZ Swaziland
        {2, {ALPHABET_ARABIC,     ALPHABET_ROMAN}}, // KWT Kuwait, State of
        {2, {ALPHABET_ROMAN,      ALPHABET_DEVANAGARI}}, // FJI Fiji
        {1, {ALPHABET_ROMAN}}, // NCL New Caledonia
        {1, {ALPHABET_ROMAN}}, // SVN Slovenia
        {3, {ALPHABET_HEBREW,     ALPHABET_ARABIC,     ALPHABET_ROMAN}}, // ISR Israel
        {3, {ALPHABET_HEBREW,     ALPHABET_ARABIC,     ALPHABET_ROMAN}}, // PSE Palestinian territories
        {1, {ALPHABET_ROMAN}}, // SLV El Salvador
        {1, {ALPHABET_ROMAN}}, // BLZ Belize
        {2, {ALPHABET_ARABIC,     ALPHABET_ROMAN}}, // DJI Djibouti
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // MKD Macedonia
        {1, {ALPHABET_ROMAN}}, // RWA Rwanda
        {1, {ALPHABET_ROMAN}}, // HTI Haiti
        {1, {ALPHABET_ROMAN}}, // BDI Burundi
        {1, {ALPHABET_ROMAN}}, // GNQ Equatorial Guinea
        {1, {ALPHABET_ROMAN}}, // ALB Albania
        {1, {ALPHABET_ROMAN}}, // SLB Solomon Islands
        {2, {ALPHABET_ARMENIAN,   ALPHABET_ROMAN}}, // ARM Armenia
        {1, {ALPHABET_ROMAN}}, // LSO Lesotho
        {1, {ALPHABET_ROMAN}}, // BEL Belgium
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // MDA Moldova
        {1, {ALPHABET_ROMAN}}, // GNB Guinea-Bissau
        {2, {ALPHABET_CHINESE,    ALPHABET_ROMAN}}, // TWN Taiwan
        {2, {ALPHABET_TIBETAN,    ALPHABET_ROMAN}}, // BTN Bhutan
        {1, {ALPHABET_ROMAN}}, // CHE Switzerland
        {1, {ALPHABET_ROMAN}}, // NLD Netherlands
        {1, {ALPHABET_ROMAN}}, // DNK Denmark
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // EST Estonia
        {1, {ALPHABET_ROMAN}}, // DOM Dominican Republic
        {1, {ALPHABET_ROMAN}}, // SVK Slovakia
        {1, {ALPHABET_ROMAN}}, // CRI Costa Rica
        {2, {ALPHABET_ROMAN,      ALPHABET_CYRILLIC}}, // BIH Bosnia and Herzegovina
        {2, {ALPHABET_ROMAN,      ALPHABET_CYRILLIC}}, // HRV Croatia
        {1, {ALPHABET_ROMAN}}, // TGO Togo
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // LVA Latvia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // LTU Lithuania
        {3, {ALPHABET_SINHALESE,  ALPHABET_TAMIL,      ALPHABET_ROMAN}}, // LKA Sri Lanka
        {3, {ALPHABET_GEORGIAN,   ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // GEO Georgia
        {1, {ALPHABET_ROMAN}}, // IRL Ireland
        {1, {ALPHABET_ROMAN}}, // SLE Sierra Leone
        {1, {ALPHABET_ROMAN}}, // PAN Panama
        {1, {ALPHABET_ROMAN}}, // CZE Czech Republic
        {1, {ALPHABET_ROMAN}}, // GUF French Guiana
        {2, {ALPHABET_ARABIC,     ALPHABET_ROMAN}}, // ARE United Arab Emirates
        {1, {ALPHABET_ROMAN}}, // AUT Austria
        {3, {ALPHABET_ROMAN,      ALPHABET_CYRILLIC,   ALPHABET_ARABIC}}, // AZE Azerbaijan
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // SRB Serbia
        {2, {ALPHABET_ARABIC,     ALPHABET_ROMAN}}, // JOR Jordan
        {1, {ALPHABET_ROMAN}}, // PRT Portugal
        {1, {ALPHABET_ROMAN}}, // HUN Hungary
        {2, {ALPHABET_KOREAN,     ALPHABET_ROMAN}}, // KOR South Korea
        {1, {ALPHABET_ROMAN}}, // ISL Iceland
        {1, {ALPHABET_ROMAN}}, // GTM Guatemala
        {1, {ALPHABET_ROMAN}}, // CUB Cuba
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // BGR Bulgaria
        {1, {ALPHABET_ROMAN}}, // LBR Liberia
        {1, {ALPHABET_ROMAN}}, // HND Honduras
        {1, {ALPHABET_ROMAN}}, // BEN Benin
        {3, {ALPHABET_AMHARIC,    ALPHABET_ARABIC,     ALPHABET_ROMAN}}, // ERI Eritrea
        {1, {ALPHABET_ROMAN}}, // MWI Malawi
        {2, {ALPHABET_KOREAN,     ALPHABET_ROMAN}}, // PRK North Korea
        {1, {ALPHABET_ROMAN}}, // NIC Nicaragua
        {2, {ALPHABET_GREEK,      ALPHABET_ROMAN}}, // GRC Greece
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // TJK Tajikistan
        {2, {ALPHABET_BENGALI,    ALPHABET_ROMAN}}, // BGD Bangladesh, People's Republic of
        {2, {ALPHABET_DEVANAGARI, ALPHABET_ROMAN}}, // NPL Nepal
        {3, {ALPHABET_ARABIC,     ALPHABET_ROMAN,      ALPHABET_TIFINAGH}}, // TUN Tunisia
        {1, {ALPHABET_ROMAN}}, // SUR Suriname
        {1, {ALPHABET_ROMAN}}, // URY Uruguay
        {2, {ALPHABET_KHMER,      ALPHABET_ROMAN}}, // KHM Cambodia
        {2, {ALPHABET_ARABIC,     ALPHABET_ROMAN}}, // SYR Syria
        {1, {ALPHABET_ROMAN}}, // SEN Senegal
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // KGZ Kyrgyzstan
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // BLR Belarus
        {1, {ALPHABET_ROMAN}}, // GUY Guyana
        {2, {ALPHABET_LAO,        ALPHABET_ROMAN}}, // LAO Laos
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // ROU Romania
        {1, {ALPHABET_ROMAN}}, // GHA Ghana
        {1, {ALPHABET_ROMAN}}, // UGA Uganda
        {1, {ALPHABET_ROMAN}}, // GBR United Kingdom
        {1, {ALPHABET_ROMAN}}, // GIN Guinea
        {1, {ALPHABET_ROMAN}}, // ECU Ecuador
        {2, {ALPHABET_ARABIC,     ALPHABET_ROMAN}}, // ESH Western Sahara
        {1, {ALPHABET_ROMAN}}, // GAB Gabon
        {1, {ALPHABET_ROMAN}}, // NZL New Zealand
        {1, {ALPHABET_ROMAN}}, // BFA Burkina Faso
        {1, {ALPHABET_ROMAN}}, // PHL Philippines the
        {1, {ALPHABET_ROMAN}}, // ITA Italy
        {2, {ALPHABET_ARABIC,     ALPHABET_ROMAN}}, // OMN Oman
        {1, {ALPHABET_ROMAN}}, // POL Poland
        {1, {ALPHABET_ROMAN}}, // CIV Ivory Coast
        {1, {ALPHABET_ROMAN}}, // NOR Norway
        {3, {ALPHABET_ROMAN,      ALPHABET_ARABIC,     ALPHABET_CHINESE}}, // MYS Malaysia
        {1, {ALPHABET_ROMAN}}, // VNM Vietnam
        {1, {ALPHABET_ROMAN}}, // FIN Finland
        {1, {ALPHABET_ROMAN}}, // COG Congo-Brazzaville
        {1, {ALPHABET_ROMAN}}, // DEU Germany
        {2, {ALPHABET_KATAKANA,   ALPHABET_ROMAN}}, // JPN Japan
        {1, {ALPHABET_ROMAN}}, // ZWE Zimbabwe
        {1, {ALPHABET_ROMAN}}, // PRY Paraguay
        {2, {ALPHABET_ARABIC,     ALPHABET_ROMAN}}, // IRQ Iraq
        {3, {ALPHABET_ARABIC,     ALPHABET_ROMAN,      ALPHABET_TIFINAGH}}, // MAR Morocco
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // UZB Uzbekistan
        {1, {ALPHABET_ROMAN}}, // SWE Sweden
        {1, {ALPHABET_ROMAN}}, // PNG Papua New Guinea
        {1, {ALPHABET_ROMAN}}, // CMR Cameroon
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // TKM Turkmenistan
        {1, {ALPHABET_ROMAN}}, // ESP Spain
        {2, {ALPHABET_THAI,       ALPHABET_ROMAN}}, // THA Thailand
        {2, {ALPHABET_ARABIC,     ALPHABET_ROMAN}}, // YEM Yemen
        {1, {ALPHABET_ROMAN}}, // FRA France
        {1, {ALPHABET_ROMAN}}, // ALA Aaland Islands
        {1, {ALPHABET_ROMAN}}, // KEN Kenya
        {1, {ALPHABET_ROMAN}}, // BWA Botswana
        {1, {ALPHABET_ROMAN}}, // MDG Madagascar
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // UKR Ukraine
        {2, {ALPHABET_ROMAN,      ALPHABET_ARABIC}}, // SSD South Sudan
        {1, {ALPHABET_ROMAN}}, // CAF Central African Republic
        {2, {ALPHABET_ARABIC,     ALPHABET_ROMAN}}, // SOM Somalia
        {2, {ALPHABET_ARABIC,     ALPHABET_ROMAN}}, // AFG Afghanistan
        {2, {ALPHABET_BURMESE,    ALPHABET_ROMAN}}, // MMR Myanmar
        {1, {ALPHABET_ROMAN}}, // ZMB Zambia
        {1, {ALPHABET_ROMAN}}, // CHL Chile
        {1, {ALPHABET_ROMAN}}, // TUR Turkey
        {3, {ALPHABET_GURMUKHI,   ALPHABET_ARABIC,     ALPHABET_ROMAN}}, // PAK Pakistan
        {1, {ALPHABET_ROMAN}}, // MOZ Mozambique
        {1, {ALPHABET_ROMAN}}, // NAM Namibia
        {1, {ALPHABET_ROMAN}}, // VEN Venezuela
        {2, {ALPHABET_ROMAN,      ALPHABET_ARABIC}}, // NGA Nigeria
        {1, {ALPHABET_ROMAN}}, // TZA Tanzania
        {2, {ALPHABET_ARABIC,     ALPHABET_ROMAN}}, // EGY Egypt
        {2, {ALPHABET_ARABIC,     ALPHABET_ROMAN}}, // MRT Mauritania
        {1, {ALPHABET_ROMAN}}, // BOL Bolivia
        {3, {ALPHABET_ROMAN,      ALPHABET_AMHARIC,    ALPHABET_ARABIC}}, // ETH Ethiopia
        {1, {ALPHABET_ROMAN}}, // COL Colombia
        {1, {ALPHABET_ROMAN}}, // ZAF South Africa
        {1, {ALPHABET_ROMAN}}, // MLI Mali
        {1, {ALPHABET_ROMAN}}, // AGO Angola
        {1, {ALPHABET_ROMAN}}, // NER Niger
        {2, {ALPHABET_ARABIC,     ALPHABET_ROMAN}}, // TCD Chad
        {1, {ALPHABET_ROMAN}}, // PER Peru
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // MNG Mongolia
        {2, {ALPHABET_ARABIC,     ALPHABET_ROMAN}}, // IRN Iran
        {3, {ALPHABET_ARABIC,     ALPHABET_ROMAN,      ALPHABET_TIFINAGH}}, // LBY Libya
        {2, {ALPHABET_ARABIC,     ALPHABET_ROMAN}}, // SDN Sudan
        {1, {ALPHABET_ROMAN}}, // IDN Indonesia
        {1, {ALPHABET_ROMAN}}, // MX-DIF Federal District, Mexico
        {1, {ALPHABET_ROMAN}}, // MX-TLA Tlaxcala, Mexico
        {1, {ALPHABET_ROMAN}}, // MX-MOR Morelos, Mexico
        {1, {ALPHABET_ROMAN}}, // MX-AGU Aguascalientes, Mexico
        {1, {ALPHABET_ROMAN}}, // MX-CL Colima, Mexico
        {1, {ALPHABET_ROMAN}}, // MX-QUE Queretaro, Mexico
        {1, {ALPHABET_ROMAN}}, // MX-HID Hidalgo, Mexico
        {1, {ALPHABET_ROMAN}}, // MX-ME Mexico State, Mexico
        {1, {ALPHABET_ROMAN}}, // MX-TAB Tabasco, Mexico
        {1, {ALPHABET_ROMAN}}, // MX-NAY Nayarit, Mexico
        {1, {ALPHABET_ROMAN}}, // MX-GUA Guanajuato, Mexico
        {1, {ALPHABET_ROMAN}}, // MX-PUE Puebla, Mexico
        {1, {ALPHABET_ROMAN}}, // MX-YUC Yucatan, Mexico
        {1, {ALPHABET_ROMAN}}, // MX-ROO Quintana Roo, Mexico
        {1, {ALPHABET_ROMAN}}, // MX-SIN Sinaloa, Mexico
        {1, {ALPHABET_ROMAN}}, // MX-CAM Campeche, Mexico
        {1, {ALPHABET_ROMAN}}, // MX-MIC Michoacan, Mexico
        {1, {ALPHABET_ROMAN}}, // MX-SLP San Luis Potosi, Mexico
        {1, {ALPHABET_ROMAN}}, // MX-GRO Guerrero, Mexico
        {1, {ALPHABET_ROMAN}}, // MX-NLE Nuevo Leon, Mexico
        {1, {ALPHABET_ROMAN}}, // MX-BCN Baja California, Mexico
        {1, {ALPHABET_ROMAN}}, // MX-VER Veracruz, Mexico
        {1, {ALPHABET_ROMAN}}, // MX-CHP Chiapas, Mexico
        {1, {ALPHABET_ROMAN}}, // MX-BCS Baja California Sur, Mexico
        {1, {ALPHABET_ROMAN}}, // MX-ZAC Zacatecas, Mexico
        {1, {ALPHABET_ROMAN}}, // MX-JAL Jalisco, Mexico
        {1, {ALPHABET_ROMAN}}, // MX-TAM Tamaulipas, Mexico
        {1, {ALPHABET_ROMAN}}, // MX-OAX Oaxaca, Mexico
        {1, {ALPHABET_ROMAN}}, // MX-DUR Durango, Mexico
        {1, {ALPHABET_ROMAN}}, // MX-COA Coahuila, Mexico
        {1, {ALPHABET_ROMAN}}, // MX-SON Sonora, Mexico
        {1, {ALPHABET_ROMAN}}, // MX-CHH Chihuahua, Mexico
        {1, {ALPHABET_ROMAN}}, // GRL Greenland
        {2, {ALPHABET_ARABIC,     ALPHABET_ROMAN}}, // SAU Saudi Arabia
        {1, {ALPHABET_ROMAN}}, // COD Congo-Kinshasa
        {3, {ALPHABET_ARABIC,     ALPHABET_ROMAN,      ALPHABET_TIFINAGH}}, // DZA Algeria
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // KAZ Kazakhstan
        {1, {ALPHABET_ROMAN}}, // ARG Argentina
        {3, {ALPHABET_DEVANAGARI, ALPHABET_GUJARATI,   ALPHABET_ROMAN}}, // IN-DD Daman and Diu
        {3, {ALPHABET_DEVANAGARI, ALPHABET_GUJARATI,   ALPHABET_ROMAN}}, // IN-DN Dadra and Nagar Haveli
        {3, {ALPHABET_DEVANAGARI, ALPHABET_ROMAN,      ALPHABET_GURMUKHI}}, // IN-CH Chandigarh
        {3, {ALPHABET_DEVANAGARI, ALPHABET_ROMAN,      ALPHABET_BENGALI}}, // IN-AN Andaman and Nicobar
        {3, {ALPHABET_MALAYALAM,  ALPHABET_ROMAN,      ALPHABET_DEVANAGARI}}, // IN-LD Lakshadweep
        {3, {ALPHABET_DEVANAGARI, ALPHABET_GURMUKHI,   ALPHABET_ROMAN}}, // IN-DL Delhi, National Capital Territory of
        {2, {ALPHABET_ROMAN,      ALPHABET_DEVANAGARI}}, // IN-ML Meghalaya
        {2, {ALPHABET_ROMAN,      ALPHABET_DEVANAGARI}}, // IN-NL Nagaland
        {3, {ALPHABET_BENGALI,    ALPHABET_DEVANAGARI, ALPHABET_ROMAN}}, // IN-MN Manipur
        {3, {ALPHABET_BENGALI,    ALPHABET_DEVANAGARI, ALPHABET_ROMAN}}, // IN-TR Tripura
        {2, {ALPHABET_DEVANAGARI, ALPHABET_ROMAN}}, // IN-MZ Mizoram
        {2, {ALPHABET_DEVANAGARI, ALPHABET_ROMAN}}, // IN-SK Sikkim
        {3, {ALPHABET_GURMUKHI,   ALPHABET_DEVANAGARI, ALPHABET_ROMAN}}, // IN-PB Punjab
        {3, {ALPHABET_DEVANAGARI, ALPHABET_GURMUKHI,   ALPHABET_ROMAN}}, // IN-HR Haryana
        {2, {ALPHABET_ROMAN,      ALPHABET_DEVANAGARI}}, // IN-AR Arunachal Pradesh
        {3, {ALPHABET_BENGALI,    ALPHABET_ROMAN,      ALPHABET_DEVANAGARI}}, // IN-AS Assam
        {2, {ALPHABET_DEVANAGARI, ALPHABET_ROMAN}}, // IN-BR Bihar
        {2, {ALPHABET_DEVANAGARI, ALPHABET_ROMAN}}, // IN-UT Uttarakhand
        {2, {ALPHABET_DEVANAGARI, ALPHABET_ROMAN}}, // IN-GA Goa
        {3, {ALPHABET_MALAYALAM,  ALPHABET_ROMAN,      ALPHABET_DEVANAGARI}}, // IN-KL Kerala
        {3, {ALPHABET_TAMIL,      ALPHABET_DEVANAGARI, ALPHABET_ROMAN}}, // IN-TN Tamil Nadu
        {2, {ALPHABET_DEVANAGARI, ALPHABET_ROMAN}}, // IN-HP Himachal Pradesh
        {3, {ALPHABET_DEVANAGARI, ALPHABET_GURMUKHI,   ALPHABET_ROMAN}}, // IN-JK Jammu and Kashmir
        {2, {ALPHABET_DEVANAGARI, ALPHABET_ROMAN}}, // IN-CT Chhattisgarh
        {3, {ALPHABET_DEVANAGARI, ALPHABET_BENGALI,    ALPHABET_ROMAN}}, // IN-JH Jharkhand
        {3, {ALPHABET_KANNADA,    ALPHABET_DEVANAGARI, ALPHABET_ROMAN}}, // IN-KA Karnataka
        {2, {ALPHABET_DEVANAGARI, ALPHABET_ROMAN}}, // IN-RJ Rajasthan
        {3, {ALPHABET_ODIA,       ALPHABET_ROMAN,      ALPHABET_DEVANAGARI}}, // IN-OR Odisha
        {3, {ALPHABET_GUJARATI,   ALPHABET_ROMAN,      ALPHABET_DEVANAGARI}}, // IN-GJ Gujarat
        {3, {ALPHABET_BENGALI,    ALPHABET_ROMAN,      ALPHABET_DEVANAGARI}}, // IN-WB West Bengal
        {2, {ALPHABET_DEVANAGARI, ALPHABET_ROMAN}}, // IN-MP Madhya Pradesh
        {3, {ALPHABET_TELUGU,     ALPHABET_DEVANAGARI, ALPHABET_ROMAN}}, // IN-TG Telangana
        {3, {ALPHABET_TELUGU,     ALPHABET_ROMAN,      ALPHABET_DEVANAGARI}}, // IN-AP Andhra Pradesh
        {2, {ALPHABET_DEVANAGARI, ALPHABET_ROMAN}}, // IN-MH Maharashtra
        {2, {ALPHABET_DEVANAGARI, ALPHABET_ROMAN}}, // IN-UP Uttar Pradesh
        {3, {ALPHABET_MALAYALAM,  ALPHABET_TELUGU,     ALPHABET_DEVANAGARI}}, // IN-PY Puducherry
        {1, {ALPHABET_ROMAN}}, // AU-NSW New South Wales, Australia
        {1, {ALPHABET_ROMAN}}, // AU-ACT Australian Capital Territory, Australia
        {1, {ALPHABET_ROMAN}}, // AU-JBT Jervis Bay Territory, Australia
        {1, {ALPHABET_ROMAN}}, // AU-NT Northern Territory, Australia
        {1, {ALPHABET_ROMAN}}, // AU-SA South Australia, Australia
        {1, {ALPHABET_ROMAN}}, // AU-TAS Tasmania, Australia
        {1, {ALPHABET_ROMAN}}, // AU-VIC Victoria, Australia
        {1, {ALPHABET_ROMAN}}, // AU-WA Western Australia, Australia
        {1, {ALPHABET_ROMAN}}, // AU-QLD Queensland, Australia
        {1, {ALPHABET_ROMAN}}, // BR-DF Distrito Federal, Brazil
        {1, {ALPHABET_ROMAN}}, // BR-SE Sergipe, Brazil
        {1, {ALPHABET_ROMAN}}, // BR-AL Alagoas, Brazil
        {1, {ALPHABET_ROMAN}}, // BR-RJ Rio de Janeiro, Brazil
        {1, {ALPHABET_ROMAN}}, // BR-ES Espirito Santo, Brazil
        {1, {ALPHABET_ROMAN}}, // BR-RN Rio Grande do Norte, Brazil
        {1, {ALPHABET_ROMAN}}, // BR-PB Paraiba, Brazil
        {1, {ALPHABET_ROMAN}}, // BR-SC Santa Catarina, Brazil
        {1, {ALPHABET_ROMAN}}, // BR-PE Pernambuco, Brazil
        {1, {ALPHABET_ROMAN}}, // BR-AP Amapa, Brazil
        {1, {ALPHABET_ROMAN}}, // BR-CE Ceara, Brazil
        {1, {ALPHABET_ROMAN}}, // BR-AC Acre, Brazil
        {1, {ALPHABET_ROMAN}}, // BR-PR Parana, Brazil
        {1, {ALPHABET_ROMAN}}, // BR-RR Roraima, Brazil
        {1, {ALPHABET_ROMAN}}, // BR-RO Rondonia, Brazil
        {1, {ALPHABET_ROMAN}}, // BR-SP Sao Paulo, Brazil
        {1, {ALPHABET_ROMAN}}, // BR-PI Piaui, Brazil
        {1, {ALPHABET_ROMAN}}, // BR-TO Tocantins, Brazil
        {1, {ALPHABET_ROMAN}}, // BR-RS Rio Grande do Sul, Brazil
        {1, {ALPHABET_ROMAN}}, // BR-MA Maranhao, Brazil
        {1, {ALPHABET_ROMAN}}, // BR-GO Goias, Brazil
        {1, {ALPHABET_ROMAN}}, // BR-MS Mato Grosso do Sul, Brazil
        {1, {ALPHABET_ROMAN}}, // BR-BA Bahia, Brazil
        {1, {ALPHABET_ROMAN}}, // BR-MG Minas Gerais, Brazil
        {1, {ALPHABET_ROMAN}}, // BR-MT Mato Grosso, Brazil
        {1, {ALPHABET_ROMAN}}, // BR-PA Para, Brazil
        {1, {ALPHABET_ROMAN}}, // BR-AM Amazonas, Brazil
        {1, {ALPHABET_ROMAN}}, // US-DC District of Columbia, USA
        {1, {ALPHABET_ROMAN}}, // US-RI Rhode Island, USA
        {1, {ALPHABET_ROMAN}}, // US-DE Delaware, USA
        {1, {ALPHABET_ROMAN}}, // US-CT Connecticut, USA
        {1, {ALPHABET_ROMAN}}, // US-NJ New Jersey, USA
        {1, {ALPHABET_ROMAN}}, // US-NH New Hampshire, USA
        {1, {ALPHABET_ROMAN}}, // US-VT Vermont, USA
        {1, {ALPHABET_ROMAN}}, // US-MA Massachusetts, Commonwealth of, USA
        {1, {ALPHABET_ROMAN}}, // US-HI Hawaii, USA
        {1, {ALPHABET_ROMAN}}, // US-MD Maryland, USA
        {1, {ALPHABET_ROMAN}}, // US-WV West Virginia, USA
        {1, {ALPHABET_ROMAN}}, // US-SC South Carolina, USA
        {1, {ALPHABET_ROMAN}}, // US-ME Maine, USA
        {1, {ALPHABET_ROMAN}}, // US-IN Indiana, USA
        {1, {ALPHABET_ROMAN}}, // US-KY Kentucky, Commonwealth of, USA
        {1, {ALPHABET_ROMAN}}, // US-TN Tennessee, USA
        {1, {ALPHABET_ROMAN}}, // US-VA Virginia, Commonwealth of, USA
        {1, {ALPHABET_ROMAN}}, // US-OH Ohio, USA
        {1, {ALPHABET_ROMAN}}, // US-PA Pennsylvania, Commonwealth of, USA
        {1, {ALPHABET_ROMAN}}, // US-MS Mississippi, USA
        {1, {ALPHABET_ROMAN}}, // US-LA Louisiana, USA
        {1, {ALPHABET_ROMAN}}, // US-AL Alabama, USA
        {1, {ALPHABET_ROMAN}}, // US-AR Arkansas, USA
        {1, {ALPHABET_ROMAN}}, // US-NC North Carolina, USA
        {1, {ALPHABET_ROMAN}}, // US-NY New York, USA
        {1, {ALPHABET_ROMAN}}, // US-IA Iowa, USA
        {1, {ALPHABET_ROMAN}}, // US-IL Illinois, USA
        {1, {ALPHABET_ROMAN}}, // US-GA Georgia, USA
        {1, {ALPHABET_ROMAN}}, // US-WI Wisconsin, USA
        {1, {ALPHABET_ROMAN}}, // US-FL Florida, USA
        {1, {ALPHABET_ROMAN}}, // US-MO Missouri, USA
        {1, {ALPHABET_ROMAN}}, // US-OK Oklahoma, USA
        {1, {ALPHABET_ROMAN}}, // US-ND North Dakota, USA
        {1, {ALPHABET_ROMAN}}, // US-WA Washington, USA
        {1, {ALPHABET_ROMAN}}, // US-SD South Dakota, USA
        {1, {ALPHABET_ROMAN}}, // US-NE Nebraska, USA
        {1, {ALPHABET_ROMAN}}, // US-KS Kansas, USA
        {1, {ALPHABET_ROMAN}}, // US-ID Idaho, USA
        {1, {ALPHABET_ROMAN}}, // US-UT Utah, USA
        {1, {ALPHABET_ROMAN}}, // US-MN Minnesota, USA
        {1, {ALPHABET_ROMAN}}, // US-MI Michigan, USA
        {1, {ALPHABET_ROMAN}}, // US-WY Wyoming, USA
        {1, {ALPHABET_ROMAN}}, // US-OR Oregon, USA
        {1, {ALPHABET_ROMAN}}, // US-CO Colorado, USA
        {1, {ALPHABET_ROMAN}}, // US-NV Nevada, USA
        {1, {ALPHABET_ROMAN}}, // US-AZ Arizona, USA
        {1, {ALPHABET_ROMAN}}, // US-NM New Mexico, USA
        {1, {ALPHABET_ROMAN}}, // US-MT Montana, USA
        {1, {ALPHABET_ROMAN}}, // US-CA California, USA
        {1, {ALPHABET_ROMAN}}, // US-TX Texas, USA
        {1, {ALPHABET_ROMAN}}, // US-AK Alaska, USA
        {1, {ALPHABET_ROMAN}}, // CA-BC British Columbia, Canada
        {1, {ALPHABET_ROMAN}}, // CA-AB Alberta, Canada
        {1, {ALPHABET_ROMAN}}, // CA-ON Ontario, Canada
        {1, {ALPHABET_ROMAN}}, // CA-QC Quebec, Canada
        {1, {ALPHABET_ROMAN}}, // CA-SK Saskatchewan, Canada
        {1, {ALPHABET_ROMAN}}, // CA-MB Manitoba, Canada
        {1, {ALPHABET_ROMAN}}, // CA-NL Newfoundland and Labrador, Canada
        {1, {ALPHABET_ROMAN}}, // CA-NB New Brunswick, Canada
        {1, {ALPHABET_ROMAN}}, // CA-NS Nova Scotia, Canada
        {1, {ALPHABET_ROMAN}}, // CA-PE Prince Edward Island, Canada
        {1, {ALPHABET_ROMAN}}, // CA-YT Yukon, Canada
        {1, {ALPHABET_ROMAN}}, // CA-NT Northwest Territories, Canada
        {1, {ALPHABET_ROMAN}}, // CA-NU Nunavut, Canada
        {3, {ALPHABET_DEVANAGARI, ALPHABET_BENGALI,    ALPHABET_ROMAN}}, // IND India
        {1, {ALPHABET_ROMAN}}, // AUS Australia, Commonwealth of
        {1, {ALPHABET_ROMAN}}, // BRA Brazil
        {1, {ALPHABET_ROMAN}}, // USA USA
        {1, {ALPHABET_ROMAN}}, // MEX Mexico
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-MOW Moscow, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-SPE Saint Petersburg, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-KGD Kaliningrad Oblast, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-IN Ingushetia, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-AD Adygea Republic, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-SE North Ossetia-Alania Republic, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-KB Kabardino-Balkar Republic, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-KC Karachay-Cherkess Republic, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-CE Chechen Republic, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-CU Chuvash Republic, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-IVA Ivanovo Oblast, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-LIP Lipetsk Oblast, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-ORL Oryol Oblast, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-TUL Tula Oblast, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-BE Belgorod Oblast, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-VLA Vladimir Oblast, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-KRS Kursk Oblast, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-KLU Kaluga Oblast, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-TAM Tambov Oblast, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-BRY Bryansk Oblast, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-YAR Yaroslavl Oblast, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-RYA Ryazan Oblast, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-AST Astrakhan Oblast, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-MOS Moscow Oblast, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-SMO Smolensk Oblast, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-DA Dagestan Republic, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-VOR Voronezh Oblast, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-NGR Novgorod Oblast, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-PSK Pskov Oblast, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-KOS Kostroma Oblast, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-STA Stavropol Krai, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-KDA Krasnodar Krai, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-KL Kalmykia Republic, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-TVE Tver Oblast, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-LEN Leningrad Oblast, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-ROS Rostov Oblast, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-VGG Volgograd Oblast, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-VLG Vologda Oblast, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-MUR Murmansk Oblast, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-KR Karelia Republic, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-NEN Nenets Autonomous Okrug, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-KO Komi Republic, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-ARK Arkhangelsk Oblast, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-MO Mordovia Republic, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-NIZ Nizhny Novgorod Oblast, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-PNZ Penza Oblast, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-KI Kirov Oblast, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-ME Mari El Republic, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-ORE Orenburg Oblast, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-ULY Ulyanovsk Oblast, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-PM Perm Krai, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-BA Bashkortostan Republic, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-UD Udmurt Republic, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-TA Tatarstan Republic, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-SAM Samara Oblast, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-SAR Saratov Oblast, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-YAN Yamalo-Nenets, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-KM Khanty-Mansi, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-SVE Sverdlovsk Oblast, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-TYU Tyumen Oblast, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-KGN Kurgan Oblast, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-CH Chelyabinsk Oblast, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-BU Buryatia Republic, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-ZAB Zabaykalsky Krai, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-IRK Irkutsk Oblast, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-NVS Novosibirsk Oblast, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-TOM Tomsk Oblast, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-OMS Omsk Oblast, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-KK Khakassia Republic, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-KEM Kemerovo Oblast, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-AL Altai Republic, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-ALT Altai Krai, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-TY Tuva Republic, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-KYA Krasnoyarsk Krai, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-MAG Magadan Oblast, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-CHU Chukotka Okrug, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-KAM Kamchatka Krai, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-SAK Sakhalin Oblast, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-PO Primorsky Krai, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-YEV Jewish Autonomous Oblast, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-KHA Khabarovsk Krai, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-AMU Amur Oblast, Russia
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RU-SA Sakha Republic, Russia
        {1, {ALPHABET_ROMAN}}, // CAN Canada
        {2, {ALPHABET_CYRILLIC,   ALPHABET_ROMAN}}, // RUS Russia
        {2, {ALPHABET_CHINESE,    ALPHABET_ROMAN}}, // CN-SH Shanghai Municipality, China
        {2, {ALPHABET_CHINESE,    ALPHABET_ROMAN}}, // CN-TJ Tianjin Municipality, China
        {2, {ALPHABET_CHINESE,    ALPHABET_ROMAN}}, // CN-BJ Beijing Municipality, China
        {2, {ALPHABET_CHINESE,    ALPHABET_ROMAN}}, // CN-HI Hainan Province, China
        {2, {ALPHABET_CHINESE,    ALPHABET_ROMAN}}, // CN-NX Ningxia Hui Autonomous Region, China
        {2, {ALPHABET_CHINESE,    ALPHABET_ROMAN}}, // CN-CQ Chongqing Municipality, China
        {2, {ALPHABET_CHINESE,    ALPHABET_ROMAN}}, // CN-ZJ Zhejiang Province, China
        {2, {ALPHABET_CHINESE,    ALPHABET_ROMAN}}, // CN-JS Jiangsu Province, China
        {2, {ALPHABET_CHINESE,    ALPHABET_ROMAN}}, // CN-FJ Fujian Province, China
        {2, {ALPHABET_CHINESE,    ALPHABET_ROMAN}}, // CN-AH Anhui Province, China
        {2, {ALPHABET_CHINESE,    ALPHABET_ROMAN}}, // CN-LN Liaoning Province, China
        {2, {ALPHABET_CHINESE,    ALPHABET_ROMAN}}, // CN-SD Shandong Province, China
        {2, {ALPHABET_CHINESE,    ALPHABET_ROMAN}}, // CN-SX Shanxi Province, China
        {2, {ALPHABET_CHINESE,    ALPHABET_ROMAN}}, // CN-JX Jiangxi Province, China
        {2, {ALPHABET_CHINESE,    ALPHABET_ROMAN}}, // CN-HA Henan Province, China
        {2, {ALPHABET_CHINESE,    ALPHABET_ROMAN}}, // CN-GZ Guizhou Province, China
        {2, {ALPHABET_CHINESE,    ALPHABET_ROMAN}}, // CN-GD Guangdong Province, China
        {2, {ALPHABET_CHINESE,    ALPHABET_ROMAN}}, // CN-HB Hubei Province, China
        {2, {ALPHABET_CHINESE,    ALPHABET_ROMAN}}, // CN-JL Jilin Province, China
        {2, {ALPHABET_CHINESE,    ALPHABET_ROMAN}}, // CN-HE Hebei Province, China
        {2, {ALPHABET_CHINESE,    ALPHABET_ROMAN}}, // CN-SN Shaanxi Province, China
        {2, {ALPHABET_CHINESE,    ALPHABET_ROMAN}}, // CN-NM Nei Mongol Autonomous Region, China
        {2, {ALPHABET_CHINESE,    ALPHABET_ROMAN}}, // CN-HL Heilongjiang Province, China
        {2, {ALPHABET_CHINESE,    ALPHABET_ROMAN}}, // CN-HN Hunan Province, China
        {2, {ALPHABET_CHINESE,    ALPHABET_ROMAN}}, // CN-GX Guangxi Zhuang Autonomous Region, China
        {2, {ALPHABET_CHINESE,    ALPHABET_ROMAN}}, // CN-SC Sichuan Province, China
        {2, {ALPHABET_CHINESE,    ALPHABET_ROMAN}}, // CN-YN Yunnan Province, China
        {3, {ALPHABET_TIBETAN,    ALPHABET_CHINESE,    ALPHABET_ROMAN}}, // CN-XZ Xizang Autonomous Region, China
        {2, {ALPHABET_CHINESE,    ALPHABET_ROMAN}}, // CN-GS Gansu Province, China
        {2, {ALPHABET_CHINESE,    ALPHABET_ROMAN}}, // CN-QH Qinghai Province, China
        {2, {ALPHABET_CHINESE,    ALPHABET_ROMAN}}, // CN-XJ Xinjiang Uyghur Autonomous Region, China
        {2, {ALPHABET_CHINESE,    ALPHABET_ROMAN}}, // CHN China, People's Republic of
        {1, {ALPHABET_ROMAN}}, // UMI United States Minor Outlying Islands
        {1, {ALPHABET_ROMAN}}, // CPT Clipperton Island
        {1, {ALPHABET_ROMAN}}, // ATA Antarctica
        {1, {ALPHABET_ROMAN}}, // AAA International
};

#ifdef __cplusplus
}
#endif

#endif // __INTERNAL_TERRITORY_ALPHABETS_H__

