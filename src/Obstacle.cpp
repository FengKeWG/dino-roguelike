#include "../include/Obstacle.h"
#include <iostream>

Obstacle::Obstacle(const float startX, const float groundYPos, const float obstacleSpeed, const Texture2D& tex)
    : speed(obstacleSpeed), texture(tex)
{
    float obstacleHeight = 0;
    float obstacleWidth = 0;

    if (texture.id == 0)
    {
        std::cerr << "Warning: Obstacle texture not loaded!" << std::endl;
        obstacleHeight = 50.0f;
        obstacleWidth = 20.0f;
    }
    else
    {
        obstacleHeight = static_cast<float>(texture.height);
        obstacleWidth = static_cast<float>(texture.width);
    }

    position = {startX, (groundYPos + 8.0f) - obstacleHeight};

    collisionRect = {
        position.x,
        position.y,
        obstacleWidth,
        obstacleHeight
    };
}

Obstacle::~Obstacle() = default;

void Obstacle::Update(const float deltaTime)
{
    position.x -= speed * deltaTime;
    UpdateCollisionRect();
}

void Obstacle::Draw() const
{
    if (texture.id > 0)
    {
        DrawTexture(texture, static_cast<int>(position.x), static_cast<int>(position.y), WHITE);
    }
    else
    {
        DrawRectangleRec(GetCollisionRect(), RED); // 备用绘制
    }
    // DrawRectangleLinesEx(GetCollisionRect(), 1, BLUE); // 调试碰撞框
}

void Obstacle::UpdateCollisionRect()
{
    collisionRect.x = position.x;
    collisionRect.y = position.y;
    if (texture.id > 0)
    {
        collisionRect.width = static_cast<float>(texture.width);
        collisionRect.height = static_cast<float>(texture.height);
    }
}

Rectangle Obstacle::GetCollisionRect() const
{
    return collisionRect;
}

bool Obstacle::IsOffScreen(float /*screenWidth*/) const
{
    // screenWidth 参数暂时不用，因为只判断左边
    // 检查障碍物是否完全移出屏幕左侧
    return (position.x + GetWidth()) < 0;
}

float Obstacle::GetWidth() const
{
    if (texture.id > 0)
    {
        return static_cast<float>(texture.width);
    }
    return 20.0f; // Default
}
