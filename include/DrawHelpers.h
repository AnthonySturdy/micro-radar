#pragma once

#include "LGFX.h"

void DrawScanLines(LGFX_Sprite& buf, const int x0, const int y0, const int x1, const int y1, const int thickness, const int trailBrightness, const int spacing)
{
    float dx = x1 - x0;
    float dy = y1 - y0;
    float len = sqrt(dx * dx + dy * dy);
    if (len == 0) return;

    // perpendicular unit vector
    float px = -dy / len;
    float py = dx / len;

    for (int i = 0; i <= thickness; i++) {
        // 1.0 at centre, 0.0 at edges
        float t = i / (float)(thickness);
        uint8_t brightness = (uint8_t)(t * trailBrightness);

        const float endX = dx + px * (i * spacing);
        const float endY = dy + py * (i * spacing);
        const float scale = len / sqrt(endX * endX + endY * endY);
        buf.drawLine(
            x0, y0,
            x0 + endX * scale, y0 + endY * scale,
            lgfx::color888(0, brightness, 0)
        );
    }

    const float endX = dx + px * (thickness * spacing);
    const float endY = dy + py * (thickness * spacing);
    const float scale = len / sqrt(endX * endX + endY * endY);
    buf.drawLine(
        x0, y0,
        x0 + endX * scale, y0 + endY * scale,
        lgfx::color888(0, 200, 0)
    );
}