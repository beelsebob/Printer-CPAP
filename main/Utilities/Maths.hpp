#pragma once

#include <algorithm>
#include <cstdint>

template <typename T>
uint8_t invLerpPercentage(const T& v, const T& min, const T& max) {
    const T& clampedV = std::clamp(v, min, max);
    const T& inV = clampedV - min;
    const T& range = max - min;
    return static_cast<uint8_t>((inV * 100) / range);
}

template <typename T>
T lerpPercentage(const T& min, const T& max, uint8_t p) {
    return min + (p * (max - min)) / 100;
}
