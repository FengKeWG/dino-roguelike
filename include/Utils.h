// Utils.h
#ifndef UTILS_H
#define UTILS_H

#include <random>

// inline float randF(const float min, const float max)
// {
//     if (min >= max) return min;
//     return min + (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * (max - min);
// }

// 生成 [min, max] 范围内的随机浮点数 (如果 min >= max, 返回 min)
inline float randF(const float min, const float max)
{
    if (min >= max) return min;
    // 线程局部静态变量，确保每个线程有自己的生成器实例
    thread_local std::mt19937 generator(std::random_device{}());
    std::uniform_real_distribution<float> distribution(min, max);
    return distribution(generator);
}

// 生成 [min, max-1] 范围内的随机整数 (如果 min >= max, 返回 min)
inline int randI(const int min, const int max)
{
    if (min >= max) return min;
    // 线程局部静态变量
    thread_local std::mt19937 generator(std::random_device{}());
    // 分布范围是 [min, max-1]
    std::uniform_int_distribution<int> distribution(min, max - 1);
    return distribution(generator);
}

// std::mt19937 是梅森旋转算法，随机性和周期都远好于 rand()。
// std::uniform_real_distribution 能保证在指定的区间内均匀分布。

#endif // UTILS_H
