#include "AircraftManager.h"

constexpr int SCREEN_SIZE = 240;
constexpr int SCREEN_SIZE_DIV_2 = (SCREEN_SIZE / 2);

#include <ArduinoJson.h>

void AircraftManager::Initialise()
{
    // get centre point + radius
    lat = configServer.GetStoredString("latitude").toDouble();
    lon = configServer.GetStoredString("longitude").toDouble();
    rad = configServer.GetStoredString("radius").toDouble();
    displayRad = rad;
    if (displayRad <= 0.0) displayRad = 0.2;

    // configuration
    const String renderText = configServer.GetStoredString("infotext");
    const String renderTris = configServer.GetStoredString("triangle");
    if (!renderText.isEmpty()) displayInfoText = renderText == "true" ? true : false;
    if (!renderTris.isEmpty()) displayTriangles = renderTris == "true" ? true : false;
    altUnit = configServer.GetStoredString("altunit");
    if (altUnit.isEmpty()) altUnit = "m";

    // Parse map coordinate paths
    mapPaths.clear();
    String mapPathsJson = configServer.GetStoredString("mappaths");
    if (!mapPathsJson.isEmpty()) {
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, mapPathsJson);
        if (!error) {
            JsonArray outerArray = doc.as<JsonArray>();
            for (JsonVariant pathVar : outerArray) {
                if (pathVar.is<JsonArray>()) {
                    std::vector<std::pair<double, double>> path;
                    JsonArray pathArray = pathVar.as<JsonArray>();
                    for (JsonVariant pointVar : pathArray) {
                        if (pointVar.is<JsonArray>()) {
                            JsonArray pointArray = pointVar.as<JsonArray>();
                            if (pointArray.size() >= 2) {
                                double pLat = pointArray[0].as<double>();
                                double pLon = pointArray[1].as<double>();
                                path.push_back({pLat, pLon});
                            }
                        }
                    }
                    if (path.size() >= 2) {
                        mapPaths.push_back(path);
                    }
                }
            }
            Serial.printf("Parsed %d map paths.\n", mapPaths.size());
        } else {
            Serial.print("[WARN] Failed to parse map paths JSON: ");
            Serial.println(error.c_str());
        }
    }

    // calculate how often we can call OpenSky API before being rate limited
    constexpr int MS_PER_DAY = 24 * 60 * 60 * 1000;
    constexpr int ANONYMOUS_TOKENS_PER_DAY = 400;
    constexpr int AUTHED_TOKENS_PER_DAY = 4000;
    constexpr int TOKEN_BUFFER = 3;
    int dailyRequestBudget = ANONYMOUS_TOKENS_PER_DAY - TOKEN_BUFFER; // non-authed tokens minus buffer

    const String token = authHandler.GetValidToken(configServer.GetStoredString("opensky-id"), configServer.GetStoredString("opensky-secret"));
    if (!token.isEmpty())
        dailyRequestBudget = AUTHED_TOKENS_PER_DAY - TOKEN_BUFFER; // authed tokens minus buffer

    fetchInterval = MS_PER_DAY / dailyRequestBudget;
}

void AircraftManager::Update()
{
    unsigned long now = millis();

    // fetch cycle
    if (now - lastFetch >= fetchInterval) {
        lastFetch = now;

        // auth
        const String token = authHandler.GetValidToken(
            configServer.GetStoredString("opensky-id"),
            configServer.GetStoredString("opensky-secret")
        );

        std::vector<std::pair<String, String>> headers = {};
        if (!token.isEmpty()) headers.push_back({ "Authorization", "Bearer " + token });

        // request
        HttpResult result = http.Get(
            "https://opensky-network.org/api/states/all",
            {
              {"lamin", String(lat - rad)},
              {"lamax", String(lat + rad)},
              {"lomin", String(lon - rad)},
              {"lomax", String(lon + rad)}
            },
            headers
        );

        // If request failed, skip this update
        if (!result.success) {
            Serial.print("[WARN] OpenSky API request failed: ");
            Serial.println(result.errorMessage);
            return;
        }

        // track
        JsonDocument doc;
        deserializeJson(doc, result.response);
        auto aircraft = JsonParser::ParseArray<Aircraft>(doc["states"]);
        Serial.printf("OpenSky API returned %d states in bounding box.\n", aircraft.size());
        now = millis(); // override with post-parse timestamp

        for (auto& ac : aircraft) {
            auto it = trackedAircraft.find(ac.icao24);
            if (it == trackedAircraft.end())
                trackedAircraft.emplace(ac.icao24, TrackedAircraft{ ac, now });
            else
                it->second.Update(ac, now);
        }

        // remove any planes that disappeared from the feed
        for (auto it = trackedAircraft.begin(); it != trackedAircraft.end(); ) {
            bool aircraftPresent = std::any_of(aircraft.begin(), aircraft.end(), [&](const Aircraft& ac) { return ac.icao24 == it->first; });
            if (!aircraftPresent)
                it = trackedAircraft.erase(it);
            else
                ++it;
        }
    }
}

void AircraftManager::Draw(LGFX_Sprite& backbuffer)
{
    DrawRadarCircles(backbuffer);
    DrawMapOutlines(backbuffer);

    for (auto& [icao, tracked] : trackedAircraft) {
        if (tracked.state.onGround) continue;

        tracked.Tick();
        auto [predLat, predLon] = tracked.GetDisplayPosition();
        auto [x, y] = ProjectCoordinateToScreen(predLat, predLon);

        // Filter out aircraft symbols that are outside the circular radar boundary
        int dx = x - 119;
        int dy = y - 119;
        if (dx * dx + dy * dy > 119 * 119) continue;

        if (displayInfoText)
            DrawAircraftInfo(backbuffer, x, y, tracked);

        if (displayTriangles)
            DrawAircraftTriangle(backbuffer, x, y, tracked);
        else
            backbuffer.fillCircle(x, y, 3, lgfx::color888(0, 255, 0));
    }

    DrawDashboard(backbuffer);
}

void AircraftManager::DrawRadarCircles(LGFX_Sprite& backbuffer) const
{
    constexpr int CENTRE = 119;
    constexpr int OUTER = 119;

    backbuffer.drawCircle(CENTRE, CENTRE, OUTER, lgfx::color888(0, 200, 0));
    backbuffer.drawCircle(CENTRE, CENTRE, (OUTER / 3) * 2, lgfx::color888(0, 64, 0));
    backbuffer.drawCircle(CENTRE, CENTRE, OUTER / 3, lgfx::color888(0, 32, 0));
}

std::pair<int, int> AircraftManager::ProjectCoordinateToScreen(float predLat, float predLon) const
{
    const float dLon = predLon - lon;
    const float dLat = predLat - lat;

    const float normLon = (dLon + displayRad) / (2.0f * displayRad);
    const float normLat = (dLat + displayRad) / (2.0f * displayRad);

    const int x = static_cast<int>(normLon * 240);
    const int y = static_cast<int>(240 - (normLat * 240));

    return { x, y };
}

void AircraftManager::DrawAircraftInfo(LGFX_Sprite& backbuffer, int x, int y, const TrackedAircraft& tracked) const
{
    const int lineHeight = tft.fontHeight() + 1;

    backbuffer.setTextSize(1);
    backbuffer.setTextColor(lgfx::color888(0, 128, 0));
    backbuffer.drawString(tracked.state.callsign, x + 5, y + 5);
    backbuffer.drawString(String(tracked.state.velocity) + "m/s", x + 5, y + 5 + lineHeight);
    backbuffer.drawString(String(tracked.state.baroAltitude) + "m", x + 5, y + 5 + lineHeight * 2);
}

void AircraftManager::DrawAircraftTriangle(LGFX_Sprite& backbuffer, int x, int y, const TrackedAircraft& tracked) const
{
    const float dx = std::sin(radians(tracked.state.trueTrack));
    const float dy = -std::cos(radians(tracked.state.trueTrack));
    const float px = -dy;
    const float py = dx;

    constexpr float TRIANGLE_LENGTH = 6.0f;
    constexpr float TRIANGLE_WIDTH = 3.0f;

    const float tipX = x + dx * TRIANGLE_LENGTH;
    const float tipY = y + dy * TRIANGLE_LENGTH;
    const float leftX = x - dx * TRIANGLE_LENGTH * 0.5f + px * TRIANGLE_WIDTH * 0.5f;
    const float leftY = y - dy * TRIANGLE_LENGTH * 0.5f + py * TRIANGLE_WIDTH * 0.5f;
    const float rightX = x - dx * TRIANGLE_LENGTH * 0.5f - px * TRIANGLE_WIDTH * 0.5f;
    const float rightY = y - dy * TRIANGLE_LENGTH * 0.5f - py * TRIANGLE_WIDTH * 0.5f;

    backbuffer.fillTriangle(tipX, tipY, leftX, leftY, rightX, rightY, lgfx::color888(0, 255, 0));
}

void AircraftManager::DrawDashboard(LGFX_Sprite& backbuffer)
{
    // Draw divider line
    backbuffer.drawFastVLine(240, 0, 240, lgfx::color888(0, 100, 0));

    // Find the aircraft to display
    const TrackedAircraft* target = nullptr;
    if (!selectedIcao.isEmpty()) {
        auto it = trackedAircraft.find(selectedIcao);
        if (it != trackedAircraft.end()) {
            target = &it->second;
        } else {
            selectedIcao = ""; // Selected aircraft is no longer tracked
        }
    }

    // If no aircraft selected, find the closest one
    if (target == nullptr) {
        double minDistance = 99999999.0;
        for (auto& [icao, tracked] : trackedAircraft) {
            if (tracked.state.onGround) continue;
            auto [pLat, pLon] = tracked.GetDisplayPosition();
            double dLat = pLat - lat;
            double dLon = pLon - lon;
            double dist = dLat * dLat + dLon * dLon;
            if (dist < minDistance) {
                minDistance = dist;
                target = &tracked;
            }
        }
    }

    // Draw header
    backbuffer.setTextColor(lgfx::color888(0, 255, 0));
    backbuffer.setTextSize(1);
    backbuffer.drawString("AIRCRAFT", 246, 5);
    backbuffer.drawFastHLine(242, 17, 76, lgfx::color888(0, 100, 0));

    if (target != nullptr) {
        int y = 25;
        int spacing = 14;

        backbuffer.setTextColor(lgfx::color888(0, 255, 0));
        String callsign = target->state.callsign;
        callsign.trim();
        if (callsign.isEmpty()) {
            callsign = target->state.icao24;
            callsign.toUpperCase();
        }
        backbuffer.drawString(callsign, 246, y);
        y += spacing;

        // Fetch route for selected/closest aircraft callsign
        FetchRouteForCallsign(callsign);

        backbuffer.setTextColor(lgfx::color888(0, 180, 0));
        if (altUnit == "ft") {
            int altFeet = (int)(target->state.baroAltitude * 3.28084f);
            backbuffer.drawString("ALT: " + String(altFeet) + "ft", 246, y);
        } else {
            backbuffer.drawString("ALT: " + String((int)target->state.baroAltitude) + "m", 246, y);
        }
        y += spacing;
        backbuffer.drawString("SPD: " + String((int)target->state.velocity) + "m/s", 246, y);
        y += spacing;
        backbuffer.drawString("HDG: " + String((int)target->state.trueTrack) + "d", 246, y);
        y += spacing;
        
        // Calculate approximate distance in km (1 degree lat ~= 111 km)
        auto [pLat, pLon] = target->GetDisplayPosition();
        double dy = (pLat - lat) * 111.0;
        double dx = (pLon - lon) * 111.0 * std::cos(radians(lat));
        double distKm = std::sqrt(dx * dx + dy * dy);
        backbuffer.drawString("DST: " + String(distKm, 1) + "km", 246, y);
        y += spacing;

        // Draw Route (origin -> destination) under DST
        backbuffer.setTextColor(lgfx::color888(0, 180, 255));
        auto it = routeCache.find(callsign);
        if (it != routeCache.end() && !it->second.first.isEmpty()) {
            backbuffer.drawString(it->second.first + "->" + it->second.second, 246, y);
        } else {
            backbuffer.drawString("RTE: UNKN", 246, y);
        }
    } else {
        backbuffer.setTextColor(lgfx::color888(0, 128, 0));
        backbuffer.drawString("No planes", 246, 30);
    }

    // Draw interactive Zoom/Range controls at the bottom
    backbuffer.drawFastHLine(242, 160, 76, lgfx::color888(0, 100, 0));
    backbuffer.setTextColor(lgfx::color888(0, 255, 0));
    backbuffer.drawString("RANGE", 246, 166);

    // Zoom In button (+)
    backbuffer.drawRect(246, 182, 30, 26, lgfx::color888(0, 200, 0));
    backbuffer.drawCentreString("+", 261, 188);

    // Zoom Out button (-)
    backbuffer.drawRect(282, 182, 30, 26, lgfx::color888(0, 200, 0));
    backbuffer.drawCentreString("-", 297, 188);

    // Current radius value
    backbuffer.setTextColor(lgfx::color888(0, 180, 0));
    backbuffer.drawString("r: " + String(displayRad, 2), 246, 218);
}

void AircraftManager::HandleTouch(int tx, int ty)
{
    // Zoom Buttons
    if (tx >= 240) {
        if (tx >= 246 && tx <= 276 && ty >= 182 && ty <= 208) {
            displayRad -= 0.05;
            if (displayRad < 0.05) displayRad = 0.05;
            Serial.print("Zoomed In. displayRad = ");
            Serial.println(displayRad);
        }
        else if (tx >= 282 && tx <= 312 && ty >= 182 && ty <= 208) {
            displayRad += 0.05;
            if (displayRad > 2.0) displayRad = 2.0;
            Serial.print("Zoomed Out. displayRad = ");
            Serial.println(displayRad);
        }
    }
    // Radar Area plane selection
    else {
        double minDistance = 144.0; // 12 pixel radius squared (144.0)
        String foundIcao = "";
        
        for (auto& [icao, tracked] : trackedAircraft) {
            if (tracked.state.onGround) continue;
            auto [predLat, predLon] = tracked.GetDisplayPosition();
            auto [x, y] = ProjectCoordinateToScreen(predLat, predLon);
            
            double dx = tx - x;
            double dy = ty - y;
            double dist = dx * dx + dy * dy;
            if (dist < minDistance) {
                minDistance = dist;
                foundIcao = icao;
            }
        }
        
        if (!foundIcao.isEmpty()) {
            selectedIcao = foundIcao;
            Serial.print("Selected aircraft: ");
            Serial.println(selectedIcao);
        } else {
            selectedIcao = "";
            Serial.println("Cleared selection");
        }
    }
}

void AircraftManager::DrawMapOutlines(LGFX_Sprite& backbuffer) const
{
    for (const auto& path : mapPaths) {
        if (path.size() < 2) continue;
        
        for (size_t i = 0; i < path.size() - 1; ++i) {
            auto [x1, y1] = ProjectCoordinateToScreen(path[i].first, path[i].second);
            auto [x2, y2] = ProjectCoordinateToScreen(path[i+1].first, path[i+1].second);
            
            // Check if either endpoint is inside the circular boundary
            int dx1 = x1 - 119;
            int dy1 = y1 - 119;
            int dx2 = x2 - 119;
            int dy2 = y2 - 119;
            bool p1_in = (dx1 * dx1 + dy1 * dy1) <= 119 * 119;
            bool p2_in = (dx2 * dx2 + dy2 * dy2) <= 119 * 119;
            
            if (p1_in || p2_in) {
                // Draw line in green (brighter for visibility)
                backbuffer.drawLine(x1, y1, x2, y2, lgfx::color888(0, 100, 0));
            }
        }
    }
}

void AircraftManager::FetchRouteForCallsign(const String& callsign)
{
    String cleanCallsign = callsign;
    cleanCallsign.trim();

    // Ignore invalid/empty callsigns or already requested callsigns in this session
    if (cleanCallsign.isEmpty() || cleanCallsign == lastRouteCallsign) {
        return;
    }

    // Check cache first
    if (routeCache.find(cleanCallsign) != routeCache.end()) {
        return;
    }

    unsigned long now = millis();
    // Rate limit route fetches to once every 5 seconds max
    if (now - lastRouteFetchTime < 5000) {
        return;
    }

    lastRouteCallsign = cleanCallsign;
    lastRouteFetchTime = now;

    Serial.print("Fetching route for callsign: ");
    Serial.println(cleanCallsign);

    HttpResult result = http.Get("https://opensky-network.org/api/routes", {{"callsign", cleanCallsign}});
    if (result.success) {
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, result.response);
        if (!error) {
            String origin = doc["route"][0].as<String>();
            String dest = doc["route"][1].as<String>();
            origin.trim();
            dest.trim();
            if (!origin.isEmpty() && !dest.isEmpty()) {
                routeCache[cleanCallsign] = {origin, dest};
                Serial.printf("Route found: %s -> %s\n", origin.c_str(), dest.c_str());
            } else {
                routeCache[cleanCallsign] = {"UNK", "UNK"};
            }
        } else {
            routeCache[cleanCallsign] = {"UNK", "UNK"};
        }
    } else {
        Serial.print("Route fetch failed: ");
        Serial.println(result.errorMessage);
    }
}