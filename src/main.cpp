#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFiManager.h>

#include "LGFX.h"
#include "WiFiManagerHelpers.h"
#include "ConfigurationWebServer.h"
#include "HttpRequestManager.h"
#include "OpenSkyAuthTokenHandler.h"
#include "AircraftManager.h"
#include "DrawHelpers.h"
#include "models/Aircraft.h"
#include "models/TrackedAircraft.h"

constexpr int SCREEN_SIZE = 240;
constexpr int SCREEN_SIZE_DIV_2 = (SCREEN_SIZE / 2);

LGFX tft;
LGFX_Sprite backbuffer(&tft);

WiFiManager wm;
ConfigurationWebServer configServer;
HttpRequestManager http;
OpenSkyAuthTokenHandler authHandler(http);

AircraftManager aircraftManager(configServer, authHandler, http, tft);

void setup()
{
  Serial.begin(115200);
  // delay(1000); // avoids immediate serial output being cut off - uncomment if needed

  // initialise LGFX + screen
  tft.init();
  tft.setRotation(1); // Set to landscape mode
  tft.invertDisplay(false);
  pinMode(21, OUTPUT);
  digitalWrite(21, HIGH);

  backbuffer.setColorDepth(8);
  backbuffer.createSprite(320, 240); // Full CYD2USB resolution (Landscape)

  // establish WiFi connection
  tft.fillScreen(lgfx::color888(0, 0, 0));
  tft.setTextColor(lgfx::color888(0, 255, 0));
  tft.drawCentreString("Connecting to WiFi...", 160, 105);
  tft.drawCentreString("Visit http://microradar.local to configure", 160, 135);

  bool portalRan = false;
  WiFiManagerHelpers::ConfigureWiFiManager(wm, tft, &portalRan);
  wm.autoConnect(WiFiManagerHelpers::WiFiManagerName);

  if (portalRan) {
    Serial.println("WiFi configured via captive portal. Restarting ESP32 for clean setup...");
    delay(1000);
    ESP.restart();
  }

  // Set calibrated touch data
  uint16_t calData[8] = { 3807, 202, 3846, 3789, 392, 193, 382, 3736 };
  tft.setTouchCalibrate(calData);

  // begin background server for configuration
  configServer.Initialise();

  // initialise aircraft manager
  aircraftManager.Initialise();
}

void loop()
{
  aircraftManager.Update();

  // Handle Touch Input
  static bool wasTouched = false;
  int tx = 0, ty = 0;
  if (tft.getTouch(&tx, &ty)) {
    if (!wasTouched) {
      Serial.printf("Touch registered at screen coords: X=%d, Y=%d\n", tx, ty);
      aircraftManager.HandleTouch(tx, ty);
      wasTouched = true;
    }
  } else {
    wasTouched = false;
  }

  // draw cycle
  backbuffer.fillScreen(lgfx::color888(0, 0, 0));

  String renderScanlines = configServer.GetStoredString("scanline");
  if (renderScanlines.isEmpty() || renderScanlines == "true") {
    DrawScanLines(backbuffer,
      SCREEN_SIZE_DIV_2 - 1,
      SCREEN_SIZE_DIV_2 - 1,
      SCREEN_SIZE_DIV_2 - 1 + (std::cos(millis() / 3000.0f) * SCREEN_SIZE_DIV_2),
      SCREEN_SIZE_DIV_2 - 1 + (std::sin(millis() / 3000.0f) * SCREEN_SIZE_DIV_2),
      20, 128, 5
    );
  }

  aircraftManager.Draw(backbuffer);
  backbuffer.pushSprite(0, 0);
}

