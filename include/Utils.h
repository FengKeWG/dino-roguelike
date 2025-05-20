// Utils.h
#ifndef UTILS_H
#define UTILS_H

#include <cstdlib>
#include "raylib.h"

inline float GetRandomFloat(const float min, const float max)
{
    if (min >= max) return min;
    return min + (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * (max - min);
}

#endif // UTILS_H
