#include "HttpRequestManager.h"

#include <sstream>

String HttpRequestManager::BuildQueryString(const std::vector<std::pair<String, String>>& params) const
{
    if (params.empty())
        return "";

    std::stringstream queryStream;
    queryStream << "?";

    bool first = true;
    for (const auto& [key, value] : params)
    {
        if (!first)
            queryStream << "&";

        queryStream << key.c_str() << "=" << value.c_str();
        first = false;
    }

    return queryStream.str().c_str();
}

String HttpRequestManager::Get(const String& url, const std::vector<std::pair<String, String>>& params, const std::vector<std::pair<String, String>>& headers) {
    const String queryParams = BuildQueryString(params); // create query params string
    const String fullUrl = url + queryParams;

    http.begin(fullUrl);

    // add headers to request
    for (const auto& header : headers) {
        http.addHeader(header.first, header.second);
    }

    // send request and handle response 
    int responseCode = http.GET();
    String response = "";

    if (responseCode > 0) {
        response = http.getString();
    }
    else {
        Serial.print("Error: ");
        Serial.println(http.errorToString(responseCode));
    }

    http.end();
    return response;
}

String HttpRequestManager::Post(const String& url, const String& body, const std::vector<std::pair<String, String>>& headers)
{
    http.begin(url);

    // add headers to request
    for (const auto& header : headers) {
        http.addHeader(header.first, header.second);
    }

    // send request and handle response 
    int responseCode = http.POST(body);
    String response = "";

    if (responseCode > 0) {
        response = http.getString();
    }
    else {
        Serial.print("Error: ");
        Serial.println(http.errorToString(responseCode));
    }

    http.end();
    return response;
}
