// src/Cloud.cpp
#include "../include/Cloud.h"

Cloud::Cloud(const Texture2D& tex, const Vector2 initialPosition, const float spd)
    : texture(tex), position(initialPosition), speed(spd)
{
}

Cloud::~Cloud() = default;

// 更新云彩状态，每帧调用
void Cloud::Update(const float deltaTime)
{
    // 根据速度和时间差更新云彩的X轴位置 (向左移动)
    position.x -= speed * deltaTime;
}

// 绘制云彩
void Cloud::Draw() const
{
    DrawTextureV(texture, position, WHITE);
}

// 检查云彩是否完全移出屏幕左侧
bool Cloud::IsOffScreen() const
{
    // 如果云彩的右边缘小于0 (屏幕最左侧)，则认为其移出屏幕
    return (position.x + texture.width < 0);
}
