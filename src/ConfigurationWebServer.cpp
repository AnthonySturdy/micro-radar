#include "ConfigurationWebServer.h"

#include <ESPmDNS.h>

void ConfigurationWebServer::Initialise() {
    MDNS.begin("microradar"); // hostname resolution for ip

    // Serve the config page
    server.on("/", HTTP_GET,
        [&](AsyncWebServerRequest* request) {
            Serial.println("[GET] Handling request to config web server...");

            prefs.begin("config", true);  // true = read only
            const String latitude = prefs.getString("latitude", "");
            const String longitude = prefs.getString("longitude", "");
            const String radius = prefs.getString("radius", "1.0");
            const String openskyClientId = prefs.getString("opensky-id", "");
            String openskyClientSecret = prefs.getString("opensky-secret", "");
            prefs.end();

            std::fill(openskyClientSecret.begin(), openskyClientSecret.end(), '*'); // replace secret string with asterisks

            const String html = R"(
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
                                            value=')" + latitude + R"('
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
                                            value=')" + longitude + R"('
                                            class="border border-green-500 bg-gray-900 w-full px-3 py-2 text-lg sm:text-base sm:px-1 sm:py-0">
                                    </label>
                                </div>
                
                                <label class="flex flex-col sm:flex-row items-start sm:items-center gap-2">
                                    <span>Radius (in &deg;):</span>
                                    <input
                                        name="radius"
                                        type="number"
                                        min="0"
                                        step="0.000001"
                                        max="2"
                                        value=')" + radius + R"('
                                        class="flex-1 border border-green-500 bg-gray-900 w-full px-3 py-2 text-lg sm:text-base sm:px-1 sm:py-0">
                                </label>
                
                                <label class="flex flex-col sm:flex-row items-start sm:items-center gap-2">
                                    <span>OpenSkyAPI Client ID:</span>
                                    <input
                                        name="opensky-id"
                                        value=')" + openskyClientId + R"('
                                        class="flex-1 border border-green-500 bg-gray-900 w-full px-3 py-2 text-lg sm:text-base sm:px-1 sm:py-0">
                                </label>
                
                                <label class="flex flex-col sm:flex-row items-start sm:items-center gap-2">
                                    <span>OpenSkyAPI Client Secret:</span>
                                    <input
                                        name="opensky-secret"
                                        value=')" + openskyClientSecret + R"('
                                        class="flex-1 border border-green-500 bg-gray-900 w-full px-3 py-2 text-lg sm:text-base sm:px-1 sm:py-0">
                                </label>
                
                                <input
                                    type="submit"
                                    value="Save"
                                    class="bg-green-500 text-black mt-4 px-4 py-3 text-lg sm:text-base sm:px-2 sm:py-0 self-start cursor-pointer">
                            </form>
                        </fieldset>
                
                        <div id="result" class="max-w-2xl mx-auto mt-4 px-1 sm:px-10"></div>
                
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

            request->send(200, "text/html", html);
        }
    );

    // Handle form submission
    server.on("/save", HTTP_POST,
        [&](AsyncWebServerRequest* request) {
            Serial.println("[POST] Handling form submission to config web server...");

            prefs.begin("config", false);
            if (request->hasParam("latitude", true))
                prefs.putString("latitude", request->getParam("latitude", true)->value());
            if (request->hasParam("longitude", true))
                prefs.putString("longitude", request->getParam("longitude", true)->value());
            if (request->hasParam("radius", true))
                prefs.putString("radius", request->getParam("radius", true)->value());
            if (request->hasParam("opensky-id", true))
                prefs.putString("opensky-id", request->getParam("opensky-id", true)->value());
            if (request->hasParam("opensky-secret", true)) {
                String secret = request->getParam("opensky-secret", true)->value();
                if (secret.indexOf('*') == -1) prefs.putString("opensky-secret", secret); // avoid overwriting with obscured string
            }
            prefs.end();

            request->send(200, "text/html", "Saved - restarting device...");

            ESP.restart();
        }
    );

    server.begin();
}

String ConfigurationWebServer::GetStoredString(const char* key)
{
    prefs.begin("config", true);
    const String value = prefs.getString(key, "");
    prefs.end();
    return value;
}
