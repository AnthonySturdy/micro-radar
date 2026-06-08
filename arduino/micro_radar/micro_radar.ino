#include <LovyanGFX.hpp>
#include <string>

constexpr int SCREEN_SIZE = 240;

// Custom class describing the hardware configuration to LovyanGFX.
// Configured via three structs: SPI bus, display panel, and backlight.
class LGFX : public lgfx::LGFX_Device {

  lgfx::Panel_GC9A01 _panel;   // GC9A01 display driver
  lgfx::Bus_SPI      _bus;     // SPI communication bus
  lgfx::Light_PWM    _light;   // PWM backlight controller

public:
  LGFX(void) {

    // -------------------------------------------------------------------------
    // SPI BUS
    // -------------------------------------------------------------------------
    {
      auto cfg = _bus.config();

      // ESP32-C3 has one user-available SPI controller (SPI0/1 reserved for flash)
      cfg.spi_host = SPI2_HOST;

      // Write clock speed - 40MHz is GC9A01 maximum. Reduce to 27MHz if glitching.
      cfg.freq_write = 40000000;

      // MISO unused - GC9A01 doesn't support SPI readback
      cfg.pin_miso = -1;

      cfg.pin_mosi = 7;    // Data to display
      cfg.pin_sclk = 6;    // SPI clock
      cfg.pin_dc = 2;    // LOW = command, HIGH = pixel data

      _bus.config(cfg);
      _panel.setBus(&_bus);
    }

    // -------------------------------------------------------------------------
    // DISPLAY PANEL
    // -------------------------------------------------------------------------
    {
      auto cfg = _panel.config();

      cfg.pin_cs = 10;   // Chip select - pulls LOW to activate display on SPI bus
      cfg.pin_rst = -1;   // Hardwired on this board, no software control needed
      cfg.pin_busy = -1;   // GC9A01 doesn't use a busy pin

      _panel.config(cfg);
    }

    // -------------------------------------------------------------------------
    // BACKLIGHT
    // -------------------------------------------------------------------------
    {
      auto cfg = _light.config();

      cfg.pin_bl = 3;       // Backlight PWM pin - use tft.setBrightness(0-255)
      cfg.invert = false;   // HIGH = full brightness on this board

      _light.config(cfg);
      _panel.setLight(&_light);
    }

    setPanel(&_panel);
  }
};

// Global display object
LGFX tft;

void setup() {
  tft.init();

  tft.invertDisplay(true);  // GC9A01 on this board has inverted colour mapping by default

  tft.setRotation(0);        // 0-3, rotates in 90 degree steps
  tft.fillScreen(TFT_DARKGREEN);

  tft.fillCircle(120, 120, 110, TFT_OLIVE);

  tft.setTextColor(TFT_ORANGE);
  tft.setTextSize(2);
  std::string text = "Hello, world!";
  int textWidth = tft.textWidth(text.c_str());
  tft.drawString(
    text.c_str(),
    (SCREEN_SIZE / 2) - (textWidth / 2),
    SCREEN_SIZE / 2
  );
}

void loop() {}
