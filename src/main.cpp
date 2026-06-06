#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFiManager.h>
#include <cmath>

#include "LGFX.h"
#include "WiFiManagerHelpers.h"
#include "ConfigurationWebServer.h"
#include "HttpRequestManager.h"
#include "OpenSkyAuthTokenHandler.h"
#include "models/Aircraft.h"

#define SCREEN_SIZE 240

LGFX tft;
LGFX_Sprite backbuffer(&tft);

WiFiManager wm;
ConfigurationWebServer configServer;
HttpRequestManager http;
OpenSkyAuthTokenHandler authHandler(&http);

void setup()
{
  Serial.begin(115200);
  // delay(1000); // avoids immediate serial output being cut off - uncomment if needed

  // initialise LGFX + screen
  tft.init();
  tft.invertDisplay(true);
  pinMode(3, OUTPUT);
  digitalWrite(3, HIGH);

  backbuffer.setColorDepth(8);
  backbuffer.createSprite(SCREEN_SIZE, SCREEN_SIZE);

  // establish WiFi connection
  tft.fillScreen(lgfx::color888(0, 0, 0));
  tft.setTextColor(lgfx::color888(0, 255, 0));
  tft.drawCentreString("Connecting to WiFi...", SCREEN_SIZE / 2, SCREEN_SIZE / 2);

  WiFiManagerHelpers::ConfigureWiFiManager(wm);
  wm.autoConnect(WiFiManagerHelpers::WiFiManagerName);

  // begin background server for configuration
  configServer.Initialise();
}

void loop()
{
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
  double lat = 51.454863,
    lon = -0.320663,
    rad = 0.2;
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

  // present
  JsonDocument doc;
  deserializeJson(doc, airplaneStateResponse);
  auto aircraft = JsonParser::ParseArray<Aircraft>(doc["states"]);

  backbuffer.fillScreen(lgfx::color888(0, 0, 0)); // clear buffer

  for (auto& ac : aircraft) {
    float normLat = (ac.latitude - (lat - rad)) / (2.0 * rad);
    float normLon = (ac.longitude - (lon - rad)) / (2.0 * rad);
    backbuffer.drawPixel(normLon * SCREEN_SIZE, normLat * SCREEN_SIZE, (ac.onGround ? lgfx::color888(255, 0, 0) : lgfx::color888(0, 255, 0)));
  }

  backbuffer.pushSprite(0, 0);

  delay(21000);
}