#include "OpenSkyAuthTokenHandler.h"
#include <ArduinoJson.h>

String OpenSkyAuthTokenHandler::FetchBearerToken(const String& url, const String& clientId, const String& clientSecret)
{
    String body = "grant_type=client_credentials";
    body += "&client_id=" + clientId;
    body += "&client_secret=" + clientSecret;

    const HttpResult resp = http.Post(
        url,
        body,
        {
            {"Content-Type", "application/x-www-form-urlencoded"}
        }
    );

    if (!resp.success) {
        Serial.print("[ERROR] OpenSky token request failed: ");
        Serial.println(resp.errorMessage);
        return "";
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, resp.response);

    if (error) {
        Serial.print("[ERROR] OpenSky token response JSON parse failed: ");
        Serial.println(error.f_str());
        return "";
    }

    const JsonVariant token = doc["access_token"];
    if (!token.is<String>()) {
        Serial.println("[WARN] Missing or non-string 'access_token' in OpenSky API response");
        return "";
    }

    return token.as<String>();
}

const String OpenSkyAuthTokenHandler::GetValidToken(const String& clientId, const String& clientSecret)
{
    const String url = "https://auth.opensky-network.org/auth/realms/opensky-network/protocol/openid-connect/token";

    if (clientId.isEmpty() || clientSecret.isEmpty())
        return "";

    if (bearerToken.isEmpty() || millis() > tokenExpiry) {
        bearerToken = FetchBearerToken(url, clientId, clientSecret);
        tokenExpiry = millis() + (29 * 60 * 1000);  // 29 mins, 1 min buffer
    }

    return bearerToken;
}
