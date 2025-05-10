// src/Cloud.cpp
#include "../include/Cloud.h" // 假设 Cloud.h 在 include 目录下

Cloud::Cloud(Texture2D tex, Vector2 initialPosition, float spd)
    : texture(tex), position(initialPosition), speed(spd)
{
}

Cloud::~Cloud()
{
    // Texture2D 是一个包含ID和尺寸的结构体，
    // 实际的纹理资源由 Game 类加载和卸载，Cloud 对象本身不需要卸载纹理
}

void Cloud::Update(float deltaTime)
{
    position.x -= speed * deltaTime;
}

void Cloud::Draw() const
{
    if (texture.id > 0) // 确保纹理有效
    {
        DrawTextureV(texture, position, WHITE);
    }
}

bool Cloud::IsOffScreen() const
{
    // 假设云彩从右向左移动，当云彩的右边缘完全移出屏幕左边缘时，视为出屏
    return (position.x + texture.width < 0);
}
