// Utils.h
#ifndef UTILS_H
#define UTILS_H

#include <random>

// inline float randF(const float min, const float max)
// {
//     if (min >= max) return min;
//     return min + (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * (max - min);
// }

inline float randF(const float min, const float max)
{
    if (min >= max) return min;
    thread_local std::mt19937 generator(std::random_device{}());
    std::uniform_real_distribution distribution(min, max);
    return distribution(generator);
}

inline int randI(const int min, const int max)
{
    if (min >= max) return min;
    thread_local std::mt19937 generator(std::random_device{}());
    std::uniform_int_distribution distribution(min, max - 1); // [min, max)
    return distribution(generator);
}

// std::mt19937 是梅森旋转算法，随机性和周期都远好于 rand()。
// std::uniform_real_distribution 能保证在指定的区间内均匀分布。

#endif // UTILS_H
