#pragma once

#define LGFX_USE_V1
#include <LovyanGFX.hpp>

class LGFX : public lgfx::LGFX_Device
{
    lgfx::Panel_ST7789 _panel;
    lgfx::Bus_SPI _bus;
    lgfx::Light_PWM _light;
    lgfx::Touch_XPT2046 _touch;

public:
    LGFX(void)
    {
        {
            auto cfg = _bus.config();
            cfg.spi_host = SPI2_HOST; // SPI2_HOST is HSPI on ESP32
            cfg.freq_write = 40000000;
            cfg.freq_read = 16000000;
            cfg.pin_miso = 12;
            cfg.pin_mosi = 13;
            cfg.pin_sclk = 14;
            cfg.pin_dc = 2;
            _bus.config(cfg);
            _panel.setBus(&_bus);
        }
        {
            auto cfg = _panel.config();
            cfg.pin_cs = 15;
            cfg.pin_rst = -1;
            cfg.pin_busy = -1;
            cfg.memory_width = 240;
            cfg.memory_height = 320;
            cfg.panel_width = 240;
            cfg.panel_height = 320;
            cfg.offset_x = 0;
            cfg.offset_y = 0;
            cfg.readable = true;
            cfg.invert = false;
            cfg.rgb_order = false;
            cfg.bus_shared = true;
            _panel.config(cfg);
        }
        {
            auto cfg = _light.config();
            cfg.pin_bl = 21;
            cfg.freq = 44100;
            cfg.pwm_channel = 1;
            cfg.invert = false;
            _light.config(cfg);
            _panel.setLight(&_light);
        }
        {
            auto cfg = _touch.config();
            cfg.x_min = 300;
            cfg.x_max = 3900;
            cfg.y_min = 200;
            cfg.y_max = 3700;
            cfg.pin_int = 36;
            cfg.pin_cs = 33;
            cfg.pin_sclk = 25;
            cfg.pin_mosi = 32;
            cfg.pin_miso = 39;
            cfg.spi_host = SPI3_HOST;
            cfg.bus_shared = false;
            _touch.config(cfg);
            _panel.setTouch(&_touch);
        }
        setPanel(&_panel);
    }
};