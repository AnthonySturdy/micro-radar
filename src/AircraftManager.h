#pragma once

#include <map>
#include <vector>

#include "models/TrackedAircraft.h"
#include "ConfigurationWebServer.h"
#include "OpenSkyAuthTokenHandler.h"
#include "LGFX.h"

class AircraftManager
{
private:
    double lat = 0.0;
    double lon = 0.0;
    double rad = 0.2;
    double displayRad = 0.2;
    String selectedIcao = "";
    std::map<String, TrackedAircraft> trackedAircraft;
    std::vector<std::vector<std::pair<double, double>>> mapPaths;
    
    // Units and Routes
    String altUnit = "m";
    std::map<String, std::pair<String, String>> routeCache;
    String lastRouteCallsign = "";
    unsigned long lastRouteFetchTime = 0;

    bool displayInfoText = true;
    bool displayTriangles = true;

    unsigned long fetchInterval = 0;
    unsigned long lastFetch = 999999;

    ConfigurationWebServer& configServer;
    OpenSkyAuthTokenHandler& authHandler;
    HttpRequestManager& http;
    LGFX& tft;

    void FetchRouteForCallsign(const String& callsign);

    void DrawRadarCircles(LGFX_Sprite& backbuffer) const;
    void DrawMapOutlines(LGFX_Sprite& backbuffer) const;
    std::pair<int, int> ProjectCoordinateToScreen(float predLat, float predLon) const;
    void DrawAircraftInfo(LGFX_Sprite& backbuffer, int x, int y, const TrackedAircraft& tracked) const;
    void DrawAircraftTriangle(LGFX_Sprite& backbuffer, int x, int y, const TrackedAircraft& tracked) const;
    void DrawDashboard(LGFX_Sprite& backbuffer);

public:
    AircraftManager(ConfigurationWebServer& config, OpenSkyAuthTokenHandler& auth, HttpRequestManager& httpManager, LGFX& tftGfx)
        : configServer(config), authHandler(auth), http(httpManager), tft(tftGfx)
    {
    }
    ~AircraftManager() = default;

    void Initialise();
    void Update();
    void Draw(LGFX_Sprite& backbuffer);
    void HandleTouch(int tx, int ty);
};