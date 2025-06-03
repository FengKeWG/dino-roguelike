// src/Obstacle.cpp
#include "../include/Obstacle.h"

Obstacle::Obstacle(const float startX, const float groundY, const float obstacleSpeed, const Texture2D& tex)
    : position({0.0f, 0.0f}),
      speed(obstacleSpeed),
      collisionRect({0.0f, 0.0f, 20.0f, 50.0f}),
      texture(tex)
{
    const auto obstacleHeight = static_cast<float>(texture.height);
    const auto obstacleWidth = static_cast<float>(texture.width);
    position = {startX, (groundY + 8.0f) - obstacleHeight};
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
    DrawTexture(texture, static_cast<int>(position.x), static_cast<int>(position.y), WHITE);
}

void Obstacle::UpdateCollisionRect()
{
    collisionRect.x = position.x;
    collisionRect.y = position.y;
    collisionRect.width = static_cast<float>(texture.width);
    collisionRect.height = static_cast<float>(texture.height);
}

// 获取障碍物的碰撞矩形
Rectangle Obstacle::GetCollisionRect() const
{
    return collisionRect;
}

bool Obstacle::IsOffScreen() const
{
    return (position.x + GetWidth()) < 0;
}

float Obstacle::GetWidth() const
{
    return static_cast<float>(texture.width);
}

float Obstacle::getSpeed() const
{
    return speed;
}

void Obstacle::setSpeed(const float newSpeed)
{
    speed = newSpeed;
}
