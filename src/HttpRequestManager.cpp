#include "HttpRequestManager.h"

#include <sstream>

String HttpRequestManager::BuildQueryString(std::vector<std::pair<String, String>>& params)
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

String HttpRequestManager::Get(String url, std::vector<std::pair<String, String>> params, std::vector<std::pair<String, String>> headers) {
    String queryParams = BuildQueryString(params); // create query params string

    http.begin(url + queryParams);

    // add headers to request
    for (auto& header : headers) {
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

String HttpRequestManager::Post(String url, String body, std::vector<std::pair<String, String>> headers)
{
    http.begin(url);

    // add headers to request
    for (auto& header : headers) {
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
