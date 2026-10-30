#pragma once

#include <algorithm>
#include <cmath>

namespace iron {

// The fight camera on stages wider than the screen: it follows the midpoint
// of the two fighters, eases towards it, never leaves the stage, and always
// keeps both fighters in view. Horizontal only, and no zoom: pixel art at
// a non-integer scale shimmers (ADR-0017). Logical pixels throughout.
struct Camera {
    static constexpr float kEase = 0.18f;  // share of the remaining distance covered per tick
    static constexpr float kMargin = 24.0f; // a fighter's origin stays this far inside the screen edge

    float x = 0; // left edge of the view in world pixels

    // Where the view wants to be for fighters at a and b.
    [[nodiscard]] static float target(float a, float b, float stageWidth, float screenWidth) {
        return std::clamp((a + b) / 2 - screenWidth / 2, 0.0f, std::max(0.0f, stageWidth - screenWidth));
    }

    void snap(float a, float b, float stageWidth, float screenWidth) { x = target(a, b, stageWidth, screenWidth); }

    // One 60 Hz tick of following.
    void follow(float a, float b, float stageWidth, float screenWidth) {
        const float goal = target(a, b, stageWidth, screenWidth);
        x += (goal - x) * kEase;
        if (std::fabs(goal - x) < 0.5f) x = goal;
        // Both fighters stay in view even while easing.
        const float low = std::max(a, b) + kMargin - screenWidth, high = std::min(a, b) - kMargin;
        if (low <= high) x = std::clamp(x, low, high);
        x = std::clamp(x, 0.0f, std::max(0.0f, stageWidth - screenWidth));
    }

    // Pixel-aligned offset for drawing (pixel art must not land between pixels).
    [[nodiscard]] float offset() const { return std::round(x); }
};

} // namespace iron
