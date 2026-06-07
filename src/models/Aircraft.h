#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <vector>

#include "JsonParser.h"

// Maps to OpenSky /states/all response
// Field indices match the documented state vector array order
struct Aircraft {
    String icao24;          // [0]  unique ICAO 24-bit transponder address
    String callsign;        // [1]  flight callsign (8 chars), may be null
    String originCountry;   // [2]  country inferred from ICAO address
    long   timePosition;    // [3]  unix timestamp of last position update, null if >15s ago
    long   lastContact;     // [4]  unix timestamp of last message received from transponder
    float  longitude;       // [5]  WGS-84 longitude in decimal degrees
    float  latitude;        // [6]  WGS-84 latitude in decimal degrees
    float  baroAltitude;    // [7]  barometric altitude in metres
    bool   onGround;        // [8]  true if surface position report
    float  velocity;        // [9]  ground speed in m/s
    float  trueTrack;       // [10] heading in degrees clockwise from north
    float  verticalRate;    // [11] climb/descent rate in m/s (positive = climbing)
    // [12] sensors — null unless filtered by sensor, skipped
    float  geoAltitude;     // [13] geometric altitude in metres
    String squawk;          // [14] transponder squawk code
    bool   spi;             // [15] special purpose indicator flag
    int    positionSource;  // [16] 0=ADS-B, 1=ASTERIX, 2=MLAT, 3=FLARM
    int    category;        // [17] aircraft category (0=unknown, see docs for full list)
};

namespace JsonParser {

    template<>
    Aircraft Parse<Aircraft>(const JsonVariant& state);

}