#pragma once

#include <ArduinoJson.h>
#include <vector>

namespace JsonParser {
    template<typename T>
    T Parse(const JsonVariant& doc);

    template<typename T>
    std::vector<T> ParseArray(const JsonArray& array) {
        std::vector<T> results;
        results.reserve(array.size());

        for (JsonVariant item : array) {
            results.push_back(Parse<T>(item));
        }

        return results;
    }

}