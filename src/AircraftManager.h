#pragma once

#include <map>

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
    std::map<String, TrackedAircraft> trackedAircraft;


    unsigned long fetchInterval = 0;
    unsigned long lastFetch = 999999;

    ConfigurationWebServer& configServer;
    OpenSkyAuthTokenHandler& authHandler;
    HttpRequestManager& http;
    LGFX& tft;

public:
    AircraftManager(ConfigurationWebServer& config, OpenSkyAuthTokenHandler& auth, HttpRequestManager& httpManager, LGFX& tftGfx)
        : configServer(config), authHandler(auth), http(httpManager), tft(tftGfx)
    {
        // Calculate how often we can call OpenSky API before being rate limited
        const unsigned int msPerDay = 24 * 60 * 60 * 1000;
        int dailyRequestBudget = 400 - 5; // non-authed tokens minus buffer

        if (!config.GetStoredString("opensky-id").isEmpty() && !config.GetStoredString("opensky-secret").isEmpty())
            dailyRequestBudget = 4000 - 5; // authed tokens minus buffer

        fetchInterval = msPerDay / dailyRequestBudget;
    }
    ~AircraftManager() = default;

    void Initialise();
    void Update();
    void Draw(LGFX_Sprite& backbuffer);
};