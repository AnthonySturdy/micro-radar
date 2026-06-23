#pragma once

#include <WiFiManager.h>

namespace WiFiManagerHelpers {
constexpr const char *WiFiManagerName = "MicroRadar-Setup";

static void ConfigureWiFiManager(WiFiManager &wm, LGFX &tft,
                                 bool *portalRan = nullptr) {
  wm.setTitle("Micro Radar - Setup WiFi");
  wm.setCustomHeadElement(
      "<style>body{background:#111;color:#00ff00;font-family:monospace;} "
      "div:has(> a){background:#00ff00;} a:hover{color:#111;}</style>");

  wm.setAPCallback([&tft, portalRan](WiFiManager *wifiManager) {
    if (portalRan) {
      *portalRan = true;
    }
    tft.fillScreen(lgfx::color888(0, 0, 0));
    tft.setTextColor(lgfx::color888(0, 255, 0));

    const int lineHeight = tft.fontHeight() + 8;
    const int centerX = 160;
    const int centerY = 120;
    tft.drawCentreString("- SETUP -", centerX, centerY - 1.5 * lineHeight);
    tft.drawCentreString("Connect to this WiFi hotspot:", centerX, centerY - 0.5 * lineHeight);
    tft.drawCentreString(WiFiManagerName, centerX, centerY + 0.5 * lineHeight);
    tft.drawCentreString("Navigate to http://192.168.4.1", centerX, centerY + 1.5 * lineHeight);
  });
}
} // namespace WiFiManagerHelpers