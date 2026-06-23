#include "ConfigurationWebServer.h"
#include <ESPmDNS.h>

// HTML stored in flash
// %PLACEHOLDER% tokens are substituted at serve time by the template processor
static const char CONFIG_HTML[] PROGMEM = R"(
<html>
    <head>
        <meta name="viewport" content="width=device-width, initial-scale=1">
        <title>Configure Micro Radar</title>
        <script src="https://cdn.jsdelivr.net/npm/@tailwindcss/browser@4.3.0"></script>
    </head>
    <body class="font-mono bg-gray-900 text-green-500 min-h-screen p-4 sm:p-0 text-md sm:text-sm">
        <fieldset class="border border-green-500 p-5 w-full max-w-2xl mx-auto sm:m-10">
            <legend class="px-2">Configure Micro Radar</legend>

            <form id="cfg" action="/save" method="POST" class="flex flex-col gap-4 sm:gap-2">

                <div class="flex flex-col sm:flex-row gap-4 sm:gap-5">
                    <label class="flex flex-col sm:flex-row gap-2 flex-1">
                        <span>Latitude:</span>
                        <input
                            name="latitude"
                            type="number"
                            min="-90"
                            step="0.000001"
                            max="90"
                            value='%LATITUDE%'
                            class="border border-green-500 bg-gray-900 w-full px-3 py-2 text-lg sm:text-base sm:px-1 sm:py-0">
                    </label>

                    <label class="flex flex-col sm:flex-row gap-2 flex-1">
                        <span>Longitude:</span>
                        <input
                            name="longitude"
                            type="number"
                            min="-180"
                            step="0.000001"
                            max="180"
                            value='%LONGITUDE%'
                            class="border border-green-500 bg-gray-900 w-full px-3 py-2 text-lg sm:text-base sm:px-1 sm:py-0">
                    </label>
                </div>

                <label class="flex flex-col sm:flex-row items-start sm:items-center gap-2">
                    <span>Radius (in &deg;):</span>
                    <input
                        name="radius"
                        type="number"
                        min="0.000001"
                        step="0.000001"
                        max="2.499999"
                        value='%RADIUS%'
                        class="flex-1 border border-green-500 bg-gray-900 w-full px-3 py-2 text-lg sm:text-base sm:px-1 sm:py-0">
                </label>

                <label class="flex flex-col sm:flex-row items-start sm:items-center gap-2">
                    <span>OpenSkyAPI Client ID:</span>
                    <input
                        name="opensky-id"
                        value='%OPENSKY_ID%'
                        class="flex-1 border border-green-500 bg-gray-900 w-full px-3 py-2 text-lg sm:text-base sm:px-1 sm:py-0">
                </label>

                <label class="flex flex-col sm:flex-row items-start sm:items-center gap-2">
                    <span>OpenSkyAPI Client Secret:</span>
                    <input
                        name="opensky-secret"
                        value='%OPENSKY_SECRET%'
                        class="flex-1 border border-green-500 bg-gray-900 w-full px-3 py-2 text-lg sm:text-base sm:px-1 sm:py-0">
                </label>

                <label class="flex flex-col gap-2">
                    <span>Map Paths (JSON coordinate lines):</span>
                    <textarea
                        name="mappaths"
                        rows="3"
                        placeholder="[[[lat1, lon1], [lat2, lon2]], [[lat3, lon3], [lat4, lon4]]]"
                        class="border border-green-500 bg-gray-900 w-full px-3 py-2 text-lg sm:text-base font-mono">%MAP_PATHS%</textarea>
                </label>

                <div class="flex flex-col sm:flex-row gap-4 sm:justify-between">
                    <label class="flex flex-col sm:flex-row items-start sm:items-center gap-2">
                        <span>Radar sweep:</span>
                        <input
                            name="scanline"
                            type="checkbox"
                            %SCANLINE%
                            class="px-3 sm:px-1 accent-green-500">
                    </label>
                    <label class="flex flex-col sm:flex-row items-start sm:items-center gap-2">
                        <span>Aircraft Info:</span>
                        <input
                            name="infotext"
                            type="checkbox"
                            %INFOTEXT%
                            class="px-3 sm:px-1 accent-green-500">
                    </label>
                    <label class="flex flex-col sm:flex-row items-start sm:items-center gap-2">
                        <span>Directional Aircraft:</span>
                        <input
                            name="triangle"
                            type="checkbox"
                            %TRIANGLE%
                            class="px-3 sm:px-1 accent-green-500">
                    </label>
                </div>

                <label class="flex flex-col sm:flex-row items-start sm:items-center gap-2">
                    <span>Altitude Unit:</span>
                    <select
                        name="altunit"
                        class="border border-green-500 bg-gray-900 px-3 py-2 text-lg sm:text-base sm:px-1 sm:py-0">
                        <option value="m" %ALTUNIT_M%>Meters (m)</option>
                        <option value="ft" %ALTUNIT_FT%>Feet (ft)</option>
                    </select>
                </label>

                <div class="flex flex-col sm:flex-row gap-4 sm:gap-5">
                    <input
                        type="submit"
                        value="Save"
                        class="bg-green-500 text-black mt-4 px-4 py-3 text-lg sm:text-base sm:px-2 sm:py-0 self-start cursor-pointer">

                        <div id="result" class="mt-4 px-1 sm:px-10"></div>
                </div>
            </form>
        </fieldset>

        <script>
            document.getElementById('cfg').addEventListener('submit', function(e) {
                e.preventDefault();
                fetch(this.action, { method: 'POST', body: new FormData(this) })
                    .then(r => r.text())
                    .then(html => document.getElementById('result').innerHTML = html);
            });
        </script>
    </body>
</html>
)";

void ConfigurationWebServer::Initialise() {
    // start mDNS and check result
    if (!MDNS.begin("microradar")) {
        Serial.println("[WARN] Failed to start mDNS. Continuing without mDNS...");
    }

    // cache all values on boot to avoid reading from NVS in the main loop
    prefs.begin("config", false); // Open in read/write to automatically create namespace if it doesn't exist
    cache["latitude"] = prefs.getString("latitude", "");
    cache["longitude"] = prefs.getString("longitude", "");
    cache["radius"] = prefs.getString("radius", "1.0");
    cache["opensky-id"] = prefs.getString("opensky-id", "");
    cache["opensky-secret"] = prefs.getString("opensky-secret", "");
    cache["scanline"] = prefs.getString("scanline", "true");
    cache["infotext"] = prefs.getString("infotext", "true");
    cache["triangle"] = prefs.getString("triangle", "true");
    cache["mappaths"] = prefs.getString("mappaths", "");
    cache["altunit"] = prefs.getString("altunit", "m");
    prefs.end();

    // Handle visit to config web server
    server.on("/", HTTP_GET, [&](AsyncWebServerRequest* request) {
        Serial.println("[GET] Handling request to config web server...");

        // read all values up front so the processor lambda can capture by value
        prefs.begin("config", true);
        const String latitude = prefs.getString("latitude", "");
        const String longitude = prefs.getString("longitude", "");
        const String radius = prefs.getString("radius", "1.0");
        const String openskyClientId = prefs.getString("opensky-id", "");
        String openskySecret = prefs.getString("opensky-secret", "");
        const String scanlineEnabled = prefs.getString("scanline", "true");
        const String infoTextEnabled = prefs.getString("infotext", "true");
        const String triangleEnabled = prefs.getString("triangle", "true");
        const String mapPaths = prefs.getString("mappaths", "");
        const String altUnit = prefs.getString("altunit", "m");
        prefs.end();

        // mask secret before sending to client
        std::fill(openskySecret.begin(), openskySecret.end(), '*');

        // template processor called once per %PLACEHOLDER% token found in CONFIG_HTML.
        AsyncWebServerResponse* response = request->beginResponse(
            200, "text/html",
            (const uint8_t*)CONFIG_HTML, sizeof(CONFIG_HTML) - 1,
            [latitude, longitude, radius, openskyClientId, openskySecret, scanlineEnabled, infoTextEnabled, triangleEnabled, mapPaths, altUnit]
            (const String& var) -> String {
                if (var == "LATITUDE")       return latitude;
                if (var == "LONGITUDE")      return longitude;
                if (var == "RADIUS")         return radius;
                if (var == "OPENSKY_ID")     return openskyClientId;
                if (var == "OPENSKY_SECRET") return openskySecret;
                if (var == "SCANLINE")       return scanlineEnabled == "true" ? "checked" : "";
                if (var == "INFOTEXT")       return infoTextEnabled == "true" ? "checked" : "";
                if (var == "TRIANGLE")       return triangleEnabled == "true" ? "checked" : "";
                if (var == "MAP_PATHS")      return mapPaths;
                if (var == "ALTUNIT_M")      return altUnit == "m" ? "selected" : "";
                if (var == "ALTUNIT_FT")     return altUnit == "ft" ? "selected" : "";
                return "";
            }
        );
        request->send(response);
        }
    );

    // Handle save submission to web server
    server.on("/save", HTTP_POST, [&](AsyncWebServerRequest* request) {
        Serial.println("[POST] Handling form submission to config web server...");

        // safe parameter retrieval helper lambda
        auto TrySaveParam = [request, this](const char* paramName) {
            const auto* param = request->getParam(paramName, true);
            if (param == nullptr)
                return false;

            prefs.putString(paramName, param->value());
            return true;
            };

        prefs.begin("config", false);

        TrySaveParam("latitude");
        TrySaveParam("longitude");
        TrySaveParam("radius");
        TrySaveParam("opensky-id");
        TrySaveParam("mappaths");
        TrySaveParam("altunit");

        const auto* param = request->getParam("opensky-secret", true);
        if (param != nullptr) {
            const String& secret = param->value();
            if (secret.indexOf('*') == -1) { // Special handling for secret: don't overwrite with masked value
                prefs.putString("opensky-secret", secret);
            }
        }

        prefs.putString("scanline", request->hasParam("scanline", true) ? "true" : "false");
        prefs.putString("triangle", request->hasParam("triangle", true) ? "true" : "false");
        prefs.putString("infotext", request->hasParam("infotext", true) ? "true" : "false");
        prefs.end();

        request->send(200, "text/html", "Saved - restarting device...");
        ESP.restart();
        }
    );

    server.begin();
}

const String ConfigurationWebServer::GetStoredString(const char* key)
{
    auto it = cache.find(key);
    if (it != cache.end()) {
        return it->second;
    }
    return "";
}