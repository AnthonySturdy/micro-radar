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

#ifdef ESP32C3_ELECROW
#include "hal.h"
#endif

// Optional hard-coded Wi-Fi credentials. Leave both blank to skip pre-baking them and use the setup hotspot instead.
const char* preconfiguredWifiSsid = "";
const char* preconfiguredWifiPassword = "";

constexpr int SCREEN_SIZE = 240;
constexpr int SCREEN_SIZE_DIV_2 = (SCREEN_SIZE / 2);

LGFX tft;
#ifdef ESP32C3_ELECROW
CST816D touch(I2C_SDA, I2C_SCL, TP_RST, TP_INT);
#endif
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
  #ifdef ESP32C3_ELECROW
  Wire.begin(I2C_SDA, I2C_SCL);
  init_IO_extender();
  // delay(100);
  set_pin_io(3, true);
  set_pin_io(4, true);
  set_pin_io(2, true);
  tft.init();
  tft.initDMA();
  tft.startWrite();
  delay(200);
  touch.begin();
  #else
  // initialise LGFX + screen
  tft.init();
  pinMode(3, OUTPUT);    // TODO: This pin add 120mA on Crowpanel. Is only for the buzzer.
  digitalWrite(3, HIGH); //       Maybe too in the default board? 
  #endif
  
  backbuffer.setColorDepth(8);
  backbuffer.createSprite(SCREEN_SIZE, SCREEN_SIZE);

  tft.fillScreen(lgfx::color888(0, 0, 0));
  tft.setTextColor(lgfx::color888(0, 255, 0));
  tft.drawCentreString("Connecting to WiFi...", SCREEN_SIZE / 2, SCREEN_SIZE / 2);
  
  // establish WiFi connection
  WiFiManagerHelpers::ConfigureWiFiManager(wm, tft);

  if (strlen(preconfiguredWifiSsid) > 0) {
    WiFi.begin(preconfiguredWifiSsid, preconfiguredWifiPassword);
    WiFi.waitForConnectResult();
  }

  wm.autoConnect(WiFiManagerHelpers::WiFiManagerName);

  // begin background server for configuration
  configServer.Initialise();

  // initialise aircraft manager
  aircraftManager.Initialise();
}

void loop()
{
  aircraftManager.Update();

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

  delay(80);
}

