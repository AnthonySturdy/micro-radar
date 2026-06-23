#pragma once

#include <ESPAsyncWebServer.h>
#include <Preferences.h>
#include <map>

class ConfigurationWebServer {
private:
    AsyncWebServer server;
    Preferences prefs;
    std::map<String, String> cache;

public:
    ConfigurationWebServer() : server(80), prefs() {}
    ConfigurationWebServer(int port) : server(port), prefs() {}

    void Initialise();
    [[nodiscard]] const String GetStoredString(const char* key);
};