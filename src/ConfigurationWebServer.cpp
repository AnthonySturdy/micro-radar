#include "ConfigurationWebServer.h"
#include <ESPmDNS.h>
#include <Update.h>
#include <new>
#include "RadarLayout.h"

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

            <p class="mb-4"><a href="/update" class="underline">Update firmware</a></p>

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

                <div class="flex flex-col sm:flex-row gap-4 sm:gap-5">
                    <label class="flex flex-col gap-2 flex-1">
                        <span>Display X offset (pixels):</span>
                        <input name="display-x" type="number" min="-118" max="118" step="1"
                            value="%DISPLAY_X%" required
                            class="border border-green-500 bg-gray-900 w-full px-3 py-2">
                    </label>
                    <label class="flex flex-col gap-2 flex-1">
                        <span>Display Y offset (pixels):</span>
                        <input name="display-y" type="number" min="-118" max="118" step="1"
                            value="%DISPLAY_Y%" required
                            class="border border-green-500 bg-gray-900 w-full px-3 py-2">
                    </label>
                </div>
                <p>Positive offsets move right/down. The radar shrinks to fit the round display.</p>

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

static const char OTA_HTML[] PROGMEM = R"(
<html>
    <head>
        <meta name="viewport" content="width=device-width, initial-scale=1">
        <title>Update Micro Radar</title>
        <script src="https://cdn.jsdelivr.net/npm/@tailwindcss/browser@4.3.0"></script>
    </head>
    <body class="font-mono bg-gray-900 text-green-500 min-h-screen p-4 sm:p-0 text-md sm:text-sm">
        <fieldset class="border border-green-500 p-5 w-full max-w-2xl mx-auto sm:m-10">
            <legend class="px-2">Update Micro Radar</legend>
            <p class="mb-4">Choose a firmware .bin file built for this device.</p>
            <form id="update" class="flex flex-col gap-4">
                <input name="firmware" type="file" accept=".bin,application/octet-stream" required
                    class="border border-green-500 p-2">
                <button type="submit" class="bg-green-500 text-black px-4 py-2 self-start cursor-pointer">
                    Upload firmware
                </button>
                <progress id="progress" value="0" max="100" class="w-full"></progress>
                <p id="result"></p>
            </form>
            <p class="mt-4"><a href="/" class="underline">Back to configuration</a></p>
        </fieldset>
        <script>
            document.getElementById('update').addEventListener('submit', function(event) {
                event.preventDefault();
                const result = document.getElementById('result');
                const progress = document.getElementById('progress');
                const request = new XMLHttpRequest();
                request.open('POST', '/update');
                request.upload.addEventListener('progress', function(event) {
                    if (event.lengthComputable) {
                        progress.value = Math.round(event.loaded / event.total * 100);
                    }
                });
                request.addEventListener('load', function() {
                    result.textContent = request.responseText;
                });
                request.addEventListener('error', function() {
                    result.textContent = 'Upload failed. Check the connection and try again.';
                });
                result.textContent = 'Uploading firmware...';
                request.send(new FormData(this));
            });
        </script>
    </body>
</html>
)";

struct OtaUploadState {
    bool valid = false;
};

void ConfigurationWebServer::Initialise() {
    // start mDNS and check result
    if (!MDNS.begin("microradar")) {
        Serial.println("[WARN] Failed to start mDNS. Continuing without mDNS...");
    }

    // Handle visit to config web server
    server.on("/", HTTP_GET, [&](AsyncWebServerRequest* request) {
        Serial.println("[GET] Handling request to config web server...");

        // read all values up front so the processor lambda can capture by value
        prefs.begin("config", true);
        const String latitude = prefs.getString("latitude", "");
        const String longitude = prefs.getString("longitude", "");
        const String radius = prefs.getString("radius", "1.0");
        const String displayX = prefs.getString("display-x", "0");
        const String displayY = prefs.getString("display-y", "0");
        const String openskyClientId = prefs.getString("opensky-id", "");
        String openskySecret = prefs.getString("opensky-secret", "");
        const String scanlineEnabled = prefs.getString("scanline", "true");
        const String infoTextEnabled = prefs.getString("infotext", "true");
        const String triangleEnabled = prefs.getString("triangle", "true");
        prefs.end();

        // mask secret before sending to client
        std::fill(openskySecret.begin(), openskySecret.end(), '*');

        // template processor called once per %PLACEHOLDER% token found in CONFIG_HTML.
        AsyncWebServerResponse* response = request->beginResponse(
            200, "text/html",
            (const uint8_t*)CONFIG_HTML, sizeof(CONFIG_HTML) - 1,
            [latitude, longitude, radius, displayX, displayY, openskyClientId, openskySecret, scanlineEnabled, infoTextEnabled, triangleEnabled]
            (const String& var) -> String {
                if (var == "LATITUDE")       return latitude;
                if (var == "LONGITUDE")      return longitude;
                if (var == "RADIUS")         return radius;
                if (var == "DISPLAY_X")      return displayX;
                if (var == "DISPLAY_Y")      return displayY;
                if (var == "OPENSKY_ID")     return openskyClientId;
                if (var == "OPENSKY_SECRET") return openskySecret;
                if (var == "SCANLINE")       return scanlineEnabled == "true" ? "checked" : "";
                if (var == "INFOTEXT")       return infoTextEnabled == "true" ? "checked" : "";
                if (var == "TRIANGLE")       return triangleEnabled == "true" ? "checked" : "";
                return "";
            }
        );
        request->send(response);
        }
    );

    // Handle save submission to web server
    server.on("/save", HTTP_POST, [&](AsyncWebServerRequest* request) {
        Serial.println("[POST] Handling form submission to config web server...");

        const auto* displayX = request->getParam("display-x", true);
        const auto* displayY = request->getParam("display-y", true);
        int offsetX = 0;
        int offsetY = 0;
        if ((displayX == nullptr) != (displayY == nullptr) ||
            (displayX && !RadarLayout::ParseOffset(displayX->value().c_str(), offsetX)) ||
            (displayY && !RadarLayout::ParseOffset(displayY->value().c_str(), offsetY)) ||
            !RadarLayout::ValidOffsets(offsetX, offsetY)) {
            Serial.println("[WARN] Rejected invalid display offsets.");
            request->send(400, "text/plain",
                "Display offsets must be whole pixels with a combined distance of at most 118 pixels.");
            return;
        }

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
        if (displayX) prefs.putString("display-x", String(offsetX));
        if (displayY) prefs.putString("display-y", String(offsetY));

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

    server.on("/update", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send_P(200, "text/html", OTA_HTML);
    });

    server.on(
        "/update",
        HTTP_POST,
        [](AsyncWebServerRequest* request) {
            auto* state = static_cast<OtaUploadState*>(request->_tempObject);
            const bool success = state != nullptr && state->valid && !Update.hasError();
            delete state;
            request->_tempObject = nullptr;

            AsyncWebServerResponse* response = request->beginResponse(
                success ? 200 : 500,
                "text/plain",
                success ? "Firmware uploaded. Restarting..." : "Firmware update failed."
            );
            response->addHeader("Connection", "close");
            if (success) {
                request->onDisconnect([]() {
                    ESP.restart();
                });
            }
            request->send(response);
        },
        [](AsyncWebServerRequest* request, const String& filename, size_t index,
           uint8_t* data, size_t len, bool final) {
            if (index == 0) {
                request->_tempObject = new (std::nothrow) OtaUploadState();
                if (request->_tempObject == nullptr) {
                    Serial.println("[OTA] Failed to allocate upload state.");
                    return;
                }

                auto* state = static_cast<OtaUploadState*>(request->_tempObject);
                if (!filename.endsWith(".bin")) {
                    Serial.println("[OTA] Rejected upload: firmware must be a .bin file.");
                    return;
                }

                if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
                    Update.printError(Serial);
                    return;
                }
                state->valid = true;
                Serial.printf("[OTA] Receiving firmware: %s\n", filename.c_str());
            }

            auto* state = static_cast<OtaUploadState*>(request->_tempObject);
            if (state == nullptr || !state->valid) {
                return;
            }

            if (Update.write(data, len) != len) {
                state->valid = false;
                Update.printError(Serial);
                return;
            }

            if (final && !Update.end(true)) {
                state->valid = false;
                Update.printError(Serial);
            }
        }
    );

    server.begin();
}

const String ConfigurationWebServer::GetStoredString(const char* key)
{
    prefs.begin("config", true);
    const String value = prefs.getString(key, "");
    prefs.end();
    return value;
}