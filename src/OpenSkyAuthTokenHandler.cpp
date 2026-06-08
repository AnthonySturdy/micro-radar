#include "OpenSkyAuthTokenHandler.h"
#include <ArduinoJson.h>

String OpenSkyAuthTokenHandler::FetchBearerToken(const String& url, const String& clientId, const String& clientSecret)
{
    String body = "grant_type=client_credentials";
    body += "&client_id=" + clientId;
    body += "&client_secret=" + clientSecret;

    const String resp = http.Post(
        url,
        body,
        {
            {"Content-Type", "application/x-www-form-urlencoded"}
        }
    );

    StaticJsonDocument<512> doc;
    deserializeJson(doc, resp);
    const String token = doc["access_token"] | String("");

    return token;
}

String OpenSkyAuthTokenHandler::GetValidToken(const String& clientId, const String& clientSecret)
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
