#pragma once

#include "Aircraft.h"

struct TrackedAircraft {
    Aircraft state;
    unsigned long lastSeen;  // millis() at last API update
    unsigned long localTimeAtUpdate;

    std::pair<float, float> PredictPosition() const {
        float dataAgeOnArrival = 0.0f;
        if (state.timePosition > 0 && state.lastContact > 0) {
            // how old was the position fix relative to the last contact timestamp
            dataAgeOnArrival = (float)(state.lastContact - state.timePosition);
        }

        float localElapsed = (millis() - lastSeen) / 1000.0f;
        float dt = localElapsed + dataAgeOnArrival;

        float headingRad = radians(state.trueTrack);

        // convert m/s displacement to degrees
        const float latMetersPerDeg = 111320.0f;
        float deltaLat = (state.velocity * dt * cos(headingRad)) / latMetersPerDeg;
        float deltaLon = (state.velocity * dt * sin(headingRad)) / (latMetersPerDeg * cos(radians(state.latitude)));

        return { state.latitude + deltaLat, state.longitude + deltaLon };
    }
};