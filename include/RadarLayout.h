#pragma once

#include <cmath>
#include <cstdlib>

struct RadarLayout {
    static constexpr int Centre = 119;
    static constexpr int PhysicalRadius = 119;
    static constexpr int MaxOffset = PhysicalRadius - 1;

    int x = Centre;
    int y = Centre;
    int radius = PhysicalRadius;

    static bool ValidOffsets(int offsetX, int offsetY) {
        return std::hypot(static_cast<double>(offsetX), static_cast<double>(offsetY)) <= MaxOffset;
    }

    static bool ParseOffset(const char* value, int& offset) {
        if (*value == '\0') return false;
        char* end = nullptr;
        const long parsed = std::strtol(value, &end, 10);
        if (*end != '\0' || parsed < -MaxOffset || parsed > MaxOffset) return false;
        offset = static_cast<int>(parsed);
        return true;
    }

    static RadarLayout FromOffsets(int offsetX, int offsetY) {
        return {
            Centre + offsetX,
            Centre + offsetY,
            static_cast<int>(std::floor(PhysicalRadius -
                std::hypot(static_cast<double>(offsetX), static_cast<double>(offsetY))))
        };
    }
};
