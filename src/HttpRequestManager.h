#pragma once

#include <HTTPClient.h>
#include <vector>

class HttpRequestManager
{
private:
    HTTPClient http;

    String BuildQueryString(const std::vector<std::pair<String, String>>& params) const;

public:
    HttpRequestManager() = default;
    ~HttpRequestManager() = default;

    String Get(const String& url, const std::vector<std::pair<String, String>>& params = {}, const std::vector<std::pair<String, String>>& headers = {});
    String Post(const String& url, const String& body = "", const std::vector<std::pair<String, String>>& headers = {});
};