// Utils.h
#ifndef UTILS_H
#define UTILS_H

#include <cstdlib> // For rand, RAND_MAX
#include "raylib.h" // 如果需要用到raylib的类型

// 在min和max之间获取随机浮点数
inline float GetRandomFloat(float min, float max)
{
    if (min >= max) return min;
    // static_cast<float>(rand()) / RAND_MAX 会得到一个 [0, 1] 区间的浮点数
    return min + (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * (max - min);
}

// 如果你还想包装一下GetRandomValue使其返回float，也可以
inline float GetRandomFloatValue(float min, float max)
{
    if (min >= max) return min;
    // 注意：GetRandomValue返回int，所以这个方法可能不如上面的精确
    // 如果min和max的小数位数不多，可以先放大再缩小
    int iMin = static_cast<int>(min * 100);
    int iMax = static_cast<int>(max * 100);
    return static_cast<float>(GetRandomValue(iMin, iMax)) / 100.0f;
}


#endif // UTILS_H
