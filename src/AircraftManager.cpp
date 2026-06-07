#include "AircraftManager.h"

#define SCREEN_SIZE 240
#define SCREEN_SIZE_DIV_2 (SCREEN_SIZE / 2)

#include <ArduinoJson.h>

void AircraftManager::Initialise()
{
    lat = std::stod(configServer.GetStoredString("latitude").c_str());
    lon = std::stod(configServer.GetStoredString("longitude").c_str());
    rad = std::stod(configServer.GetStoredString("radius").c_str());
}

void AircraftManager::Update()
{
    unsigned long now = millis();

    // fetch cycle
    if (now - lastFetch >= FETCH_INTERVAL) {
        lastFetch = now;

        // auth
        String token = authHandler.GetValidToken(
            configServer.GetStoredString("opensky-id"),
            configServer.GetStoredString("opensky-secret")
        );

        std::vector<std::pair<String, String>> headers = {};
        if (!token.isEmpty()) {
            Serial.println("Token available - adding auth header");
            headers.push_back({ "Authorization", "Bearer " + token });
        }

        // request
        String airplaneStateResponse = http.Get(
            "https://opensky-network.org/api/states/all",
            {
              {"lamin", String(lat - rad)},
              {"lamax", String(lat + rad)},
              {"lomin", String(lon - rad)},
              {"lomax", String(lon + rad)}
            },
            headers
        );

        // track
        JsonDocument doc;
        deserializeJson(doc, airplaneStateResponse);
        auto aircraft = JsonParser::ParseArray<Aircraft>(doc["states"]);
        now = millis(); // override with post-parse timestamp

        for (auto& ac : aircraft) {
            auto it = trackedAircraft.find(ac.icao24);
            if (it == trackedAircraft.end())
                trackedAircraft.emplace(ac.icao24, TrackedAircraft{ ac, now });
            else
                it->second.Update(ac, now);
        }
    }
}

void AircraftManager::Draw(LGFX_Sprite& backbuffer)
{
    const int CENTRE = SCREEN_SIZE_DIV_2 - 1;
    const int OUTER = SCREEN_SIZE_DIV_2 - 1;
    backbuffer.drawCircle(CENTRE, CENTRE, OUTER, lgfx::color888(0, 200, 0));  // outer ring, 1px inside edge
    backbuffer.drawCircle(CENTRE, CENTRE, OUTER / 2, lgfx::color888(0, 64, 0));   // mid ring
    backbuffer.fillCircle(CENTRE, CENTRE, 2, lgfx::color888(0, 64, 0));   // centre dot

    for (auto& [icao, tracked] : trackedAircraft) {
        if (tracked.state.onGround) continue;

        // predict for smooth motion between requests
        tracked.Tick();
        auto [predLat, predLon] = tracked.GetDisplayPosition();

        // project longitude and latitude to screen coords
        float lonScale = cos(lat * DEG_TO_RAD);

        float dLon = (predLon - lon) * lonScale;
        float dLat = predLat - lat;

        float normLon = (dLon + rad) / (2.0f * rad);
        float normLat = (dLat + rad) / (2.0f * rad);

        int x = normLon * SCREEN_SIZE;
        int y = SCREEN_SIZE - (normLat * SCREEN_SIZE);

        // calculate triangle vertices based on travel direction
        float dx = std::sin(radians(tracked.state.trueTrack));
        float dy = -std::cos(radians(tracked.state.trueTrack));

        float px = -dy;
        float py = dx;

        float length = 6.0f;
        float width = 3.0f;

        float tipX = x + dx * length;
        float tipY = y + dy * length;

        float leftX = x - dx * length * 0.5f + px * width * 0.5f;
        float leftY = y - dy * length * 0.5f + py * width * 0.5f;

        float rightX = x - dx * length * 0.5f - px * width * 0.5f;
        float rightY = y - dy * length * 0.5f - py * width * 0.5f;

        // draw text
        backbuffer.setTextSize(1);
        backbuffer.setTextColor(lgfx::color888(0, 128, 0));
        backbuffer.drawString(tracked.state.callsign, x + 5, y + 5);
        backbuffer.drawString(String(tracked.state.velocity) + "m/s", x + 5, y + 5 + (tft.fontHeight() + 1));
        backbuffer.drawString(String(tracked.state.baroAltitude) + "m", x + 5, y + 5 + (tft.fontHeight() * 2 + 1));

        // draw representation of plane
        backbuffer.fillTriangle(tipX, tipY, leftX, leftY, rightX, rightY, lgfx::color888(0, 255, 0));
    }
}
