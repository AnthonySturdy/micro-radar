#include "RadarLayout.h"
#include <cassert>

int main() {
    const auto original = RadarLayout::FromOffsets(0, 0);
    assert(original.x == 119 && original.y == 119 && original.radius == 119);
    const auto shifted = RadarLayout::FromOffsets(3, -4);
    assert(shifted.x == 122 && shifted.y == 115 && shifted.radius == 114);
    assert(!RadarLayout::ValidOffsets(118, 118));
    assert(!RadarLayout::ValidOffsets(119, 0));
    int offset = 0;
    assert(RadarLayout::ParseOffset("-118", offset) && offset == -118);
    assert(RadarLayout::ParseOffset("0", offset) && offset == 0);
    assert(!RadarLayout::ParseOffset("119", offset));
    assert(!RadarLayout::ParseOffset("1.5", offset));
    assert(!RadarLayout::ParseOffset("bad", offset));
    assert(!RadarLayout::ParseOffset("", offset));
    assert(!RadarLayout::ParseOffset("9999999999999999999999999", offset));
    for (int x = -118; x <= 118; ++x) {
        for (int y = -118; y <= 118; ++y) {
            if (!RadarLayout::ValidOffsets(x, y)) continue;
            const auto layout = RadarLayout::FromOffsets(x, y);
            assert(layout.radius >= 1);
            assert(std::hypot(static_cast<double>(x), static_cast<double>(y)) +
                layout.radius <= RadarLayout::PhysicalRadius);
            assert(layout.x - layout.radius >= 0 && layout.x + layout.radius < 240);
            assert(layout.y - layout.radius >= 0 && layout.y + layout.radius < 240);
        }
    }
}
